#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

#include <functional>

using namespace LiteFX::Rendering::Backends;

struct LocalDescriptorBindingPoint {
	D3D12_DESCRIPTOR_RANGE_TYPE Type;
	DescriptorBindingPoint BindingPoint;

	inline bool operator==(const LocalDescriptorBindingPoint& other) const noexcept {
		return other.Type == this->Type && other.BindingPoint.Register == this->BindingPoint.Register && other.BindingPoint.Space == this->BindingPoint.Space;
	}
};

template <>
struct std::hash<LocalDescriptorBindingPoint> {
	std::size_t operator()(const LocalDescriptorBindingPoint& p) const noexcept {
		std::size_t res = 17;
		res = res * 31 + std::hash<UInt32>()(p.Type);
		res = res * 31 + std::hash<UInt32>()(p.BindingPoint.Space);
		res = res * 31 + std::hash<UInt32>()(p.BindingPoint.Register);
		return res;
	}
};

constexpr auto HitGroupNameTemplate = L"HitGroup_{0}"sv;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RayTracingPipeline::DirectX12RayTracingPipelineImpl : public Implement<DirectX12RayTracingPipeline> {
public:
	friend class DirectX12RayTracingPipelineBuilder;
	friend class DirectX12RayTracingPipeline;

private:
	SharedPtr<DirectX12PipelineLayout> m_layout;
	SharedPtr<const DirectX12ShaderProgram> m_program;
	const ShaderRecordCollection m_shaderRecordCollection;
	UInt32 m_maxRecursionDepth { 10 }, m_maxPayloadSize{ 0 }, m_maxAttributeSize{ 32 };;
	const DirectX12Device& m_device;
	ComPtr<ID3D12StateObject> m_pipelineState;

public:
	DirectX12RayTracingPipelineImpl(DirectX12RayTracingPipeline* parent, const DirectX12Device& device, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, UInt32 maxRecursionDepth, UInt32 maxPayloadSize, UInt32 maxAttributeSize, ShaderRecordCollection&& shaderRecords) :
		base(parent), m_device(device), m_layout(layout), m_program(shaderProgram), m_shaderRecordCollection(std::move(shaderRecords)), m_maxRecursionDepth(maxRecursionDepth), m_maxPayloadSize(maxPayloadSize), m_maxAttributeSize(maxAttributeSize)
	{
		if (maxRecursionDepth > D3D12_RAYTRACING_MAX_DECLARABLE_TRACE_RECURSION_DEPTH) [[unlikely]]
			throw ArgumentOutOfRangeException("maxRecursionDepth", 0u, static_cast<UInt32>(D3D12_RAYTRACING_MAX_DECLARABLE_TRACE_RECURSION_DEPTH), maxRecursionDepth, "The specified ray tracing recursion depth too large.");

		if (maxAttributeSize > D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES) [[unlikely]]
			throw ArgumentOutOfRangeException("maxAttributeSize", 0u, static_cast<UInt32>(D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES), maxAttributeSize, "The specified ray attribute size was too large.");
	}

	DirectX12RayTracingPipelineImpl(DirectX12RayTracingPipeline* parent, const DirectX12Device& device, ShaderRecordCollection&& shaderRecords) :
		base(parent), m_device(device), m_shaderRecordCollection(std::move(shaderRecords))
	{
		m_program = std::dynamic_pointer_cast<const DirectX12ShaderProgram>(m_shaderRecordCollection.program());
	}

public:
	void initialize()
	{
		if (m_program == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("shaderProgram", "The shader program must be initialized.");
		if (m_layout == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("layout", "The pipeline layout must be initialized.");
		if (m_program != m_shaderRecordCollection.program()) [[unlikely]]
			throw InvalidArgumentException("shaderRecords", "The ray tracing pipeline shader program must be the same as used to build the shader record collection.");

		LITEFX_TRACE(DIRECTX12_LOG, "Creating ray-tracing pipeline (\"{1}\") for layout {0} (records: {2})...", fmt::ptr(reinterpret_cast<void*>(m_layout.get())), m_parent->name(), m_shaderRecordCollection.shaderRecords().size());
		
		// Validate shader stage usage.
		auto modules = m_program->modules();
		bool hasComputeShaders    = std::ranges::find_if(modules, [](const auto& module) { return LITEFX_FLAG_IS_SET(ShaderStage::Compute, module->type()); }) != modules.end();
		bool hasRayTracingShaders = std::ranges::find_if(modules, [](const auto& module) { return LITEFX_FLAG_IS_SET(ShaderStage::RayTracingPipeline, module->type()); }) != modules.end();
		bool hasMeshShaders       = std::ranges::find_if(modules, [](const auto& module) { return LITEFX_FLAG_IS_SET(ShaderStage::MeshPipeline, module->type()); }) != modules.end();
		bool hasDirectShaders     = std::ranges::find_if(modules, [](const auto& module) { return LITEFX_FLAG_IS_SET(ShaderStage::RasterizationPipeline, module->type()); }) != modules.end();
		
		if (hasComputeShaders) [[unlikely]]
			throw InvalidArgumentException("shaderProgram", "The shader program contains a compute shader, which is not supported in a ray-tracing pipeline.");
		else if (hasDirectShaders) [[unlikely]]
			throw InvalidArgumentException("shaderProgram", "The shader program contains a graphics shader, which is not supported in a ray-tracing pipeline");
		else if (hasMeshShaders) [[unlikely]]
			throw InvalidArgumentException("shaderProgram", "The shader program contains a mesh shader, which is not supported in a ray-tracing pipeline");
		
		LITEFX_TRACE(DIRECTX12_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(m_program.get()), modules.size());

		// Start by describing the shader modules individually.
		struct ShaderModuleSubobjectData {
			D3D12_EXPORT_DESC ExportDesc;
			WString Name;
			WString EntryPoint;
			D3D12_DXIL_LIBRARY_DESC LibraryDesc;
			ShaderStage Type;
			const IShaderModule* Module;
		};

		auto shaderModuleSubobjects = m_program->modules() | std::views::transform([](auto module) {
			ShaderModuleSubobjectData data = {
				.Name = Widen(module->fileName()),
				.EntryPoint = Widen(module->entryPoint()),
				.Type = module->type(),
				.Module = module
			};

			data.LibraryDesc = {
				.DXILLibrary = {
					.pShaderBytecode = module->handle()->GetBufferPointer(),
					.BytecodeLength = module->handle()->GetBufferSize()
				}
			};

			return data;
		}) | std::ranges::to<Array<ShaderModuleSubobjectData>>();

		// Initialize the submodule array with the shader modules first.
		auto subobjects = shaderModuleSubobjects | std::views::transform([](auto& subobjectData) {
			// Only setup names and addresses at this point, since before the address may change.
			subobjectData.ExportDesc.Name = subobjectData.Name.c_str();
			subobjectData.ExportDesc.ExportToRename = subobjectData.EntryPoint.c_str();
			subobjectData.LibraryDesc.NumExports = 1;
			subobjectData.LibraryDesc.pExports = &subobjectData.ExportDesc;

			return D3D12_STATE_SUBOBJECT { .Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, .pDesc = &subobjectData.LibraryDesc };
		}) | std::ranges::to<Array<D3D12_STATE_SUBOBJECT>>();

		// Define hit groups from the shader exports.
		struct HitGroupData {
			WString Name;
			WString IntersectionShaderName { };
			WString AnyHitShaderName { };
			WString ClosestHitShaderName { };
			D3D12_HIT_GROUP_DESC HitGroupDesc;
		};

		auto hitGroupSubobjects = m_shaderRecordCollection.shaderRecords() | 
			std::views::filter([](const UniquePtr<const IShaderRecord>& record) { return record->type() == ShaderRecordType::HitGroup || record->type() == ShaderRecordType::Intersection; }) |
			std::views::transform([i = 0](const UniquePtr<const IShaderRecord>& record) mutable {
				HitGroupData hitGroup { .Name = std::format(HitGroupNameTemplate, i++) };

				if (record->type() == ShaderRecordType::Intersection)
				{
					auto intersectionShader = std::get<const IShaderModule*>(record->shaderGroup());
					hitGroup.HitGroupDesc = D3D12_HIT_GROUP_DESC { .Type = D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE };
					hitGroup.IntersectionShaderName = Widen(intersectionShader->fileName());
				}
				else // if (record->type() == ShaderRecordType::HitGroup)
				{
					auto& group = std::get<IShaderRecord::MeshGeometryHitGroup>(record->shaderGroup());
					hitGroup.HitGroupDesc = D3D12_HIT_GROUP_DESC { .Type = D3D12_HIT_GROUP_TYPE_TRIANGLES };

					if (group.AnyHitShader != nullptr)
						hitGroup.AnyHitShaderName = Widen(group.AnyHitShader->fileName());

					if (group.ClosestHitShader != nullptr)
						hitGroup.ClosestHitShaderName = Widen(group.ClosestHitShader->fileName());
				}

				return hitGroup;
			}) | std::ranges::to<Array<HitGroupData>>();

		// Next use the shader module exports to describe the shader groups in the shader binding table.
		subobjects.append_range(hitGroupSubobjects | std::views::transform([](auto& subobjectData) {
			// Only setup names at this point, since before the address may change.
			subobjectData.HitGroupDesc.HitGroupExport = subobjectData.Name.c_str();
			subobjectData.HitGroupDesc.IntersectionShaderImport = subobjectData.IntersectionShaderName.empty() ? nullptr : subobjectData.IntersectionShaderName.c_str();
			subobjectData.HitGroupDesc.ClosestHitShaderImport = subobjectData.ClosestHitShaderName.empty() ? nullptr : subobjectData.ClosestHitShaderName.c_str();
			subobjectData.HitGroupDesc.AnyHitShaderImport = subobjectData.AnyHitShaderName.empty() ? nullptr : subobjectData.AnyHitShaderName.c_str();

			return D3D12_STATE_SUBOBJECT {
				.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP,
				.pDesc = &subobjectData.HitGroupDesc
			};
		}));

		// Define local root signatures and their associations. 
		// NOTE: The current architecture does only allow a single payload to be specified to pass to a single descriptor. This simplifies re-using root signatures,
		//       as we can assume two root signatures are equal, if the space, register and type of their only descriptor are equal.
		struct RootSignatureAssociation {
			ComPtr<ID3D12RootSignature> RootSignature;
			Array<LPCWSTR> ModuleNames;
			D3D12_LOCAL_ROOT_SIGNATURE StateDesc { };
			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION AssocDesc { };
		};

		// Create and associate all the root signatures.
		Dictionary<LocalDescriptorBindingPoint, RootSignatureAssociation> rootSignatures;

		std::ranges::for_each(shaderModuleSubobjects, [&](auto& subobject) {
			// Test if there is a shader-local descriptor.
			if (!subobject.Module->shaderLocalDescriptor().has_value())
				return;

			// Get the descriptor set that contains the descriptor and retrieve it's type.
			LocalDescriptorBindingPoint binding = { .BindingPoint = subobject.Module->shaderLocalDescriptor().value() };
			auto& descriptorSet = m_layout->descriptorSet(binding.BindingPoint.Space);
			auto& descriptor = descriptorSet.descriptor(binding.BindingPoint.Register);

			// Check if the descriptor is actually a local one.
			if (!descriptor.local()) [[unlikely]]
				throw RuntimeException("The descriptor at a shader-local binding point must also be declared local, since it will be part of the global root signature otherwise.");

			switch (descriptor.descriptorType())
			{
				case DescriptorType::ConstantBuffer:    binding.Type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
				case DescriptorType::AccelerationStructure:
				case DescriptorType::Buffer:
				case DescriptorType::StructuredBuffer:
				case DescriptorType::ByteAddressBuffer:
				case DescriptorType::Texture:           binding.Type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
				case DescriptorType::RWBuffer:
				case DescriptorType::RWStructuredBuffer:
				case DescriptorType::RWByteAddressBuffer:
				case DescriptorType::RWTexture:         binding.Type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
				case DescriptorType::Sampler:			throw RuntimeException("Shader-local samplers are not supported.");
				case DescriptorType::InputAttachment:   throw RuntimeException("Shader-local input attachments are not supported.");
			}

			if (!rootSignatures.contains(binding))
			{
				// Create a new root signature.
				CD3DX12_ROOT_PARAMETER1 rootParameter;

				switch (binding.Type)
				{
				case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
					//rootParameter.InitAsConstantBufferView(binding.BindingPoint.Register, binding.BindingPoint.Space);
					rootParameter.InitAsConstants(descriptor.elementSize() / 4, binding.BindingPoint.Register, binding.BindingPoint.Space);
					break;
				case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
					// NOTE: SRVs and UAVs must be passed as GPU-virtual addresses to the shader-local data.
					rootParameter.InitAsShaderResourceView(binding.BindingPoint.Register, binding.BindingPoint.Space);
					break;
				case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
					// NOTE: SRVs and UAVs must be passed as GPU-virtual addresses to the shader-local data.
					rootParameter.InitAsUnorderedAccessView(binding.BindingPoint.Register, binding.BindingPoint.Space);
					break;
				default:
					std::unreachable();
				}

				// Create root signature descriptor.
				ComPtr<ID3DBlob> signature, error;
				String errorString = "";
				CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
				rootSignatureDesc.Init_1_1(1, &rootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
				HRESULT hr = ::D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);

				if (error != nullptr)
					errorString = String(reinterpret_cast<TCHAR*>(error->GetBufferPointer()), error->GetBufferSize());

				raiseIfFailed(hr, "Unable to serialize shader-local root signature: {0}", errorString);

				// Create the root signature.
				ComPtr<ID3D12RootSignature> rootSignature;
				raiseIfFailed(m_device.handle()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), "Unable to create root signature for shader-local payload.");

				// Add the root signature to the binding associations set.
				rootSignatures[binding].RootSignature = rootSignature;
				rootSignatures[binding].StateDesc = {
					.pLocalRootSignature = rootSignature.Get()
				};
			}

			// Add the current module name to the root signature association.
			rootSignatures[binding].ModuleNames.push_back(subobject.Name.c_str());
			rootSignatures[binding].AssocDesc.NumExports = static_cast<UInt32>(rootSignatures[binding].ModuleNames.size());
			rootSignatures[binding].AssocDesc.pExports = rootSignatures[binding].ModuleNames.data();
		});
		
		// Reserve enough space for all sub-objects to prevent dangling pointers due to vector resizing and instance copying.
		subobjects.reserve(subobjects.size() + (rootSignatures.size() * 2) + 4); // 4 subobjects for shader + pipeline config, global root signature and shader associations.

		// Define local root signature associations.
		for (auto& rootSignature : rootSignatures | std::views::values)
		{
			subobjects.push_back(D3D12_STATE_SUBOBJECT {
				.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE,
				.pDesc = &rootSignature.StateDesc
			});

			rootSignature.AssocDesc.pSubobjectToAssociate = &subobjects.back();

			subobjects.push_back(D3D12_STATE_SUBOBJECT {
				.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION,
				.pDesc = &rootSignature.AssocDesc
			});
		}

		// Define the payload and attribute sizes.
		// NOTE: Currently it is not possible to access ray payload and attribute sizes through reflection (see https://github.com/microsoft/DirectXShaderCompiler/issues/2952).
		D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {
			.MaxPayloadSizeInBytes = m_maxPayloadSize,
			.MaxAttributeSizeInBytes = m_maxAttributeSize
		};

		subobjects.push_back(D3D12_STATE_SUBOBJECT {
			.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG,
			.pDesc = &shaderConfig
		});

		// Associate all shader module exports with the shader config.
		// NOTE: We could use different configs for different shaders, depending on the payload size, but this would make managing the shader binding table way 
		//       more involved.
		auto shaderGroupNames = shaderModuleSubobjects |
			std::views::filter([](auto& moduleDesc) { return LITEFX_FLAG_IS_SET(ShaderStage::RayGeneration | ShaderStage::Callable | ShaderStage::Miss, moduleDesc.Type); }) |
			std::views::transform([](auto& moduleDesc) { return moduleDesc.Name.c_str(); }) | std::ranges::to<Array<LPCWSTR>>();
		
		shaderGroupNames.append_range(hitGroupSubobjects | std::views::transform([](auto& hitGroup) { return hitGroup.Name.c_str(); }));

		D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderConfigAssoc = {
			.pSubobjectToAssociate = &subobjects.back(),
			.NumExports = static_cast<UInt32>(shaderGroupNames.size()),
			.pExports = shaderGroupNames.data()
		};

		subobjects.push_back(D3D12_STATE_SUBOBJECT {
			.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION,
			.pDesc = &shaderConfigAssoc
		});

		// Define ray tracing pipeline config.
		D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {
			.MaxTraceRecursionDepth = m_maxRecursionDepth
		};
		
		subobjects.push_back(D3D12_STATE_SUBOBJECT {
			.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG,
			.pDesc = &pipelineConfig
		});

		// Finally, add the global root signature.
		D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature { .pGlobalRootSignature = std::as_const(*m_layout).handle().Get() };

		subobjects.push_back(D3D12_STATE_SUBOBJECT {
			.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE,
			.pDesc = &globalRootSignature
		});

		// Define pipeline description from sub-objects.
		D3D12_STATE_OBJECT_DESC pipelineDesc = {
			.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
			.NumSubobjects = static_cast<UInt32>(subobjects.size()),
			.pSubobjects = subobjects.data()
		};

		// Create the pipeline.
		ComPtr<ID3D12StateObject> pipeline;
		raiseIfFailed(m_device.handle()->CreateStateObject(&pipelineDesc, IID_PPV_ARGS(&pipeline)), "Unable to create ray tracing pipeline state.");

#ifndef NDEBUG
		pipeline->SetName(Widen(m_parent->name()).c_str());
#endif

		m_pipelineState = pipeline;
	}

	UniquePtr<IDirectX12Buffer> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups)
	{
		// Query the interface used to obtain the shader identifiers.
		ComPtr<ID3D12StateObjectProperties> stateProperties;
		raiseIfFailed(m_pipelineState.As(&stateProperties), "Unable to query ray tracing pipeline state properties.");

		// NOTE: It is assumed that the shader record collection did not change between pipeline creation and SBT allocation (hence its const-ness)!
		offsets = { };

		// Find the maximum payload size amongst the included shader records.
		auto filterByGroupType = [groups](auto& record) -> bool {
			switch (record->type())
			{
			case ShaderRecordType::RayGeneration: return LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::RayGeneration);
			case ShaderRecordType::Miss:          return LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::Miss);
			case ShaderRecordType::Callable:      return LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::Callable);
			case ShaderRecordType::Intersection:
			case ShaderRecordType::HitGroup:      return LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::HitGroup);
			default: std::unreachable(); // Must be caught during pipeline creation!
			}
		};

		auto localDataSize = std::ranges::max(m_shaderRecordCollection.shaderRecords() | std::views::filter(filterByGroupType) | std::views::transform([](auto& record) { return record->localDataSize(); }));

		// Compute the record size by aligning the handle and payload sizes.
		auto recordSize = Math::align<UInt64>(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + localDataSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

		// Insert empty records at the end of each table so that the table start offsets align with D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT.
		Dictionary<ShaderBindingGroup, UInt32> alignmentRecords;
		alignmentRecords[ShaderBindingGroup::RayGeneration] = LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::RayGeneration) ?
			(recordSize * std::ranges::count_if(m_shaderRecordCollection.shaderRecords(), [](auto& record) { return LITEFX_FLAG_IS_SET(record->type(), ShaderRecordType::RayGeneration); }) % D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT) / D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT : 0u;
		alignmentRecords[ShaderBindingGroup::Miss] = LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::Miss) ? 
			(recordSize * std::ranges::count_if(m_shaderRecordCollection.shaderRecords(), [](auto& record) { return LITEFX_FLAG_IS_SET(record->type(), ShaderRecordType::Miss); }) % D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT) / D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT : 0u;
		alignmentRecords[ShaderBindingGroup::Callable] = LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::Callable) ? 
			(recordSize * std::ranges::count_if(m_shaderRecordCollection.shaderRecords(), [](auto& record) { return LITEFX_FLAG_IS_SET(record->type(), ShaderRecordType::Callable); }) % D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT) / D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT : 0u;
		alignmentRecords[ShaderBindingGroup::HitGroup] = LITEFX_FLAG_IS_SET(groups, ShaderBindingGroup::HitGroup) ? 
			(recordSize * std::ranges::count_if(m_shaderRecordCollection.shaderRecords(), [](auto& record) { return LITEFX_FLAG_IS_SET(record->type(), ShaderRecordType::HitGroup) || LITEFX_FLAG_IS_SET(record->type(), ShaderRecordType::Intersection); }) % D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT) / D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT : 0u;
		
		// Count the shader records that go into the SBT.
		auto totalRecordCount = std::ranges::distance(m_shaderRecordCollection.shaderRecords() | std::views::filter(filterByGroupType)) +
			alignmentRecords[ShaderBindingGroup::RayGeneration] + alignmentRecords[ShaderBindingGroup::Miss] + alignmentRecords[ShaderBindingGroup::Callable] + alignmentRecords[ShaderBindingGroup::HitGroup];

		// Allocate a buffer for the shader binding table.
		// NOTE: Updating the SBT to change shader-local data is currently unsupported. Instead, bind-less resources should be used.
		auto result = m_device.factory().createBuffer(BufferType::ShaderBindingTable, ResourceHeap::Dynamic, recordSize, totalRecordCount, ResourceUsage::TransferSource);

		// Write each record group by group.
		UInt32 record{ 0 }, hitGroupId{ 0 };
		Array<Byte> recordData(recordSize, 0x00);

		// Write each shader binding group that should be included.
		for (auto group : { ShaderBindingGroup::RayGeneration, ShaderBindingGroup::Miss, ShaderBindingGroup::Callable, ShaderBindingGroup::HitGroup })
		{
			auto groupFilter = [group](auto& record) -> bool { 
				switch (group)
				{
				case ShaderBindingGroup::RayGeneration: return record->type() == ShaderRecordType::RayGeneration;
				case ShaderBindingGroup::Miss:          return record->type() == ShaderRecordType::Miss;
				case ShaderBindingGroup::Callable:      return record->type() == ShaderRecordType::Callable;
				case ShaderBindingGroup::HitGroup:      return record->type() == ShaderRecordType::HitGroup || record->type() == ShaderRecordType::Intersection;
				default: std::unreachable(); // Same as above.
				}
			};

			auto getRecordIdentifier = [&stateProperties, group, hitGroupIndex = 0](auto& record) mutable -> const void* {
				switch (group)
				{
				case ShaderBindingGroup::RayGeneration:
				case ShaderBindingGroup::Miss:
				case ShaderBindingGroup::Callable:
				{
					WString identifier = Widen(std::get<const IShaderModule*>(record->shaderGroup())->fileName());
					return stateProperties->GetShaderIdentifier(identifier.c_str());
				}
				case ShaderBindingGroup::HitGroup:
				{
					WString identifier = std::format(HitGroupNameTemplate, hitGroupIndex++);
					return stateProperties->GetShaderIdentifier(identifier.c_str());
				}
				default:
					std::unreachable(); // Same as above.
				}
			};

			if (LITEFX_FLAG_IS_SET(groups, group))
			{
				// Get the number of shaders in the group.
				auto filteredRecords = m_shaderRecordCollection.shaderRecords() | std::views::filter(groupFilter);
				auto recordCount = std::ranges::distance(filteredRecords);

				// Store the group offset and size and obtain the shader identifier.
				switch (group)
				{
				case ShaderBindingGroup::RayGeneration:
					offsets.RayGenerationGroupOffset = record * recordSize;
					offsets.RayGenerationGroupSize = recordCount * recordSize;
					offsets.RayGenerationGroupStride = recordSize;
					break;
				case ShaderBindingGroup::Miss:
					offsets.MissGroupOffset = record * recordSize;
					offsets.MissGroupSize = recordCount * recordSize;
					offsets.MissGroupStride = recordSize;
					break;
				case ShaderBindingGroup::Callable:
					offsets.CallableGroupOffset = record * recordSize;
					offsets.CallableGroupSize = recordCount * recordSize;
					offsets.CallableGroupStride = recordSize;
					break;
				case ShaderBindingGroup::HitGroup:
					offsets.HitGroupOffset = record * recordSize;
					offsets.HitGroupSize = recordCount * recordSize;
					offsets.HitGroupStride = recordSize;
					break;
				default:
					std::unreachable();
				}

				// Write each record and its payload into the buffer.
				for (auto& currentRecord : filteredRecords)
				{
					// Get the shader group handle for the current record.
					std::memcpy(recordData.data(), getRecordIdentifier(currentRecord), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

					// Write the payload and map everything into the buffer.
					std::memcpy(recordData.data() + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, currentRecord->localData(), currentRecord->localDataSize());
					result->map(recordData.data(), recordSize, record++);
				}

				// Increment record counter to address for empty records required to comply with alignment rules.
				record += alignmentRecords[group];
			}
		}

		return result;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RayTracingPipeline::DirectX12RayTracingPipeline(const DirectX12Device& device, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, ShaderRecordCollection&& shaderRecords, UInt32 maxRecursionDepth, UInt32 maxPayloadSize, UInt32 maxAttributeSize, const String& name) :
	m_impl(makePimpl<DirectX12RayTracingPipelineImpl>(this, device, layout, shaderProgram, maxRecursionDepth, maxPayloadSize, maxAttributeSize, std::move(shaderRecords))), DirectX12PipelineState(nullptr)
{
	if (!name.empty())
		this->name() = name;

	m_impl->initialize();
}

DirectX12RayTracingPipeline::DirectX12RayTracingPipeline(const DirectX12Device& device, ShaderRecordCollection&& shaderRecords) noexcept :
	m_impl(makePimpl<DirectX12RayTracingPipelineImpl>(this, device, std::move(shaderRecords))), DirectX12PipelineState(nullptr)
{
}

DirectX12RayTracingPipeline::~DirectX12RayTracingPipeline() noexcept = default;

SharedPtr<const DirectX12ShaderProgram> DirectX12RayTracingPipeline::program() const noexcept
{
	return m_impl->m_program;
}

SharedPtr<const DirectX12PipelineLayout> DirectX12RayTracingPipeline::layout() const noexcept
{
	return m_impl->m_layout;
}

const ShaderRecordCollection& DirectX12RayTracingPipeline::shaderRecords() const noexcept
{
	return m_impl->m_shaderRecordCollection;
}

UInt32 DirectX12RayTracingPipeline::maxRecursionDepth() const noexcept
{
	return m_impl->m_maxRecursionDepth;
}

UInt32 DirectX12RayTracingPipeline::maxPayloadSize() const noexcept
{
	return m_impl->m_maxPayloadSize;
}

UInt32 DirectX12RayTracingPipeline::maxAttributeSize() const noexcept
{
	return m_impl->m_maxAttributeSize;
}

ComPtr<ID3D12StateObject> DirectX12RayTracingPipeline::stateObject() const noexcept
{
	return m_impl->m_pipelineState;
}

UniquePtr<IDirectX12Buffer> DirectX12RayTracingPipeline::allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups) const noexcept
{
	return m_impl->allocateShaderBindingTable(offsets, groups);
}

void DirectX12RayTracingPipeline::use(const DirectX12CommandBuffer& commandBuffer) const noexcept
{
	commandBuffer.handle()->SetPipelineState1(m_impl->m_pipelineState.Get());
	commandBuffer.handle()->SetComputeRootSignature(std::as_const(*m_impl->m_layout).handle().Get());
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

constexpr DirectX12RayTracingPipelineBuilder::DirectX12RayTracingPipelineBuilder(const DirectX12Device& device, ShaderRecordCollection&& shaderRecords, const String& name) :
	RayTracingPipelineBuilder(UniquePtr<DirectX12RayTracingPipeline>(new DirectX12RayTracingPipeline(device, std::move(shaderRecords))))
{
	this->instance()->name() = name;
}

constexpr DirectX12RayTracingPipelineBuilder::~DirectX12RayTracingPipelineBuilder() noexcept = default;

void DirectX12RayTracingPipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = m_state.pipelineLayout;
	instance->m_impl->m_maxRecursionDepth = m_state.maxRecursionDepth;
	instance->m_impl->m_maxPayloadSize = m_state.maxPayloadSize;
	instance->m_impl->m_maxAttributeSize = m_state.maxAttributeSize;
	instance->m_impl->initialize();
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)