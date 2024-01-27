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
	UInt32 m_maxRecursionDepth { 10 };
	const DirectX12Device& m_device;

public:
	DirectX12RayTracingPipelineImpl(DirectX12RayTracingPipeline* parent, const DirectX12Device& device, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, UInt32 maxRecursionDepth, ShaderRecordCollection&& shaderRecords) :
		base(parent), m_device(device), m_layout(layout), m_program(shaderProgram), m_shaderRecordCollection(std::move(shaderRecords)), m_maxRecursionDepth(maxRecursionDepth)
	{
	}

	DirectX12RayTracingPipelineImpl(DirectX12RayTracingPipeline* parent, const DirectX12Device& device, ShaderRecordCollection&& shaderRecords) :
		base(parent), m_device(device), m_shaderRecordCollection(std::move(shaderRecords))
	{
		m_program = std::dynamic_pointer_cast<const DirectX12ShaderProgram>(m_shaderRecordCollection.program());
	}

public:
	ComPtr<ID3D12PipelineState> initialize()
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
				HitGroupData hitGroup { .Name = Widen(std::format("HitGroup_{0}", i++)) };

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
				case DescriptorType::ConstantBuffer:    binding.Type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break; // descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, range->descriptors(), range->binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); break;
				case DescriptorType::AccelerationStructure:
				case DescriptorType::Buffer:
				case DescriptorType::StructuredBuffer:
				case DescriptorType::ByteAddressBuffer:
				case DescriptorType::Texture:           binding.Type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break; // descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, range->descriptors(), range->binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); break;
				case DescriptorType::RWBuffer:
				case DescriptorType::RWStructuredBuffer:
				case DescriptorType::RWByteAddressBuffer:
				case DescriptorType::RWTexture:         binding.Type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break; // descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, range->descriptors(), range->binding(), space, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND); break;
				case DescriptorType::Sampler:			throw RuntimeException("Shader-local samplers are not supported.");
				case DescriptorType::InputAttachment:   throw RuntimeException("Shader-local input attachments are not supported.");
			}

			if (!rootSignatures.contains(binding))
			{
				// Create a new root signature.
				CD3DX12_DESCRIPTOR_RANGE1 descriptorRange(binding.Type, 1, binding.BindingPoint.Register, binding.BindingPoint.Space);
				CD3DX12_ROOT_PARAMETER1 rootParameter;
				rootParameter.InitAsDescriptorTable(1, &descriptorRange, D3D12_SHADER_VISIBILITY_ALL);

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
		
		// Define local root signature associations.
		// TODO: Check if the subobject association works as inteted.
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

		throw;

		// Define the payload and attribute sizes.
		// NOTE: Attributes (hit payloads passed between shaders) are set to the default maximum for now, as we currently have no way to determine it (e.g., from 
		//       shader reflection) it and the limit is not that wasteful anyway.
		// TODO: Callable shaders don't actually count towards the payload size, but we include them anyway for now, as we allow putting their payloads into the
		//       same SBT later.
		auto payloadSize = std::ranges::max(m_shaderRecordCollection.shaderRecords() | std::views::transform([](auto& record) { return record->payloadSize(); }));

		D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {
			.MaxPayloadSizeInBytes = static_cast<UInt32>(payloadSize),
			.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES
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
		
		throw;
//		// Define the pipeline state.
//		D3D12_RAYTRACING_PIPELINE_STATE_DESC pipelineStateDescription = {};
//
//		std::ranges::for_each(modules, [&, i = 0](const DirectX12ShaderModule* shaderModule) mutable {
//			LITEFX_TRACE(DIRECTX12_LOG, "\tModule {0}/{1} (\"{2}\") state: {{ Type: {3}, EntryPoint: {4} }}", ++i, modules.size(), shaderModule->fileName(), shaderModule->type(), shaderModule->entryPoint());
//
//			switch (shaderModule->type())
//			{
//			case ShaderStage::RayTracing:
//				pipelineStateDescription.CS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
//				pipelineStateDescription.CS.BytecodeLength = shaderModule->handle()->GetBufferSize();
//				break;
//			default:
//				throw InvalidArgumentException("shaderModule", "Trying to bind shader to unsupported shader stage '{0}'.", shaderModule->type());
//			}
//		});
//
//		// Create a pipeline state description.
//		pipelineStateDescription.pRootSignature = std::as_const(*m_layout).handle().Get();
//
//		// Create the pipeline state instance.
//		ComPtr<ID3D12PipelineState> pipelineState;
//		raiseIfFailed(m_device.handle()->CreateRayTracingPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState)), "Unable to create raytracing pipeline state.");
//
//#ifndef NDEBUG
//		pipelineState->SetName(Widen(m_parent->name()).c_str());
//#endif
//
//		return pipelineState;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RayTracingPipeline::DirectX12RayTracingPipeline(const DirectX12Device& device, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, ShaderRecordCollection&& shaderRecords, UInt32 maxRecursionDepth, const String& name) :
	m_impl(makePimpl<DirectX12RayTracingPipelineImpl>(this, device, layout, shaderProgram, maxRecursionDepth, std::move(shaderRecords))), DirectX12PipelineState(nullptr)
{
	if (!name.empty())
		this->name() = name;

	this->handle() = m_impl->initialize();
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

UniquePtr<IDirectX12Buffer> DirectX12RayTracingPipeline::allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups) const noexcept
{
	throw;
}

void DirectX12RayTracingPipeline::use(const DirectX12CommandBuffer& commandBuffer) const noexcept
{
	//commandBuffer.handle()->SetPipelineState(this->handle().Get());
	//commandBuffer.handle()->SetRayTracingRootSignature(std::as_const(*m_impl->m_layout).handle().Get());
	throw;
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
	instance->handle() = instance->m_impl->initialize();
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)