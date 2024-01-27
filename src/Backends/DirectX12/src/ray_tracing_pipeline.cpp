#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

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
		};

		auto shaderModuleSubobjects = m_program->modules() | std::views::transform([](auto module) {
			ShaderModuleSubobjectData data = {
				.Name = Widen(module->fileName()),
				.EntryPoint = Widen(module->entryPoint())
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