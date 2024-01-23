#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

extern PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelines;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRayTracingPipeline::VulkanRayTracingPipelineImpl : public Implement<VulkanRayTracingPipeline> {
public:
	friend class VulkanRayTracingPipelineBuilder;
	friend class VulkanRayTracingPipeline;

private:
	SharedPtr<VulkanPipelineLayout> m_layout;
	SharedPtr<const VulkanShaderProgram> m_program;
	ShaderRecordCollection m_shaderRecordCollection;
	const VulkanDevice& m_device;

public:
	VulkanRayTracingPipelineImpl(VulkanRayTracingPipeline* parent, const VulkanDevice& device, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram, ShaderRecordCollection&& shaderRecords) :
		base(parent), m_device(device), m_layout(layout), m_program(shaderProgram), m_shaderRecordCollection(std::move(shaderRecords))
	{
	}

	VulkanRayTracingPipelineImpl(VulkanRayTracingPipeline* parent, const VulkanDevice& device, ShaderRecordCollection&& shaderRecords) :
		base(parent), m_device(device), m_shaderRecordCollection(std::move(shaderRecords))
	{
		m_program = std::dynamic_pointer_cast<const VulkanShaderProgram>(m_shaderRecordCollection.program());
	}

public:
	VkPipeline initialize()
	{
		if (m_program == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("shaderProgram", "The shader program must be initialized.");
		if (m_layout == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("layout", "The pipeline layout must be initialized.");
		if (m_program != m_shaderRecordCollection.program()) [[unlikely]]
			throw InvalidArgumentException("shaderRecords", "The ray tracing pipeline shader program must be the same as used to build the shader record collection.");

		LITEFX_TRACE(VULKAN_LOG, "Creating ray-tracing pipeline (\"{1}\") for layout {0} (records: {2})...", fmt::ptr(reinterpret_cast<void*>(m_layout.get())), m_parent->name(), m_shaderRecordCollection.shaderRecords().size());
	
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

		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(reinterpret_cast<const void*>(m_program.get())), modules.size());

		if (modules.size() > 1)
			throw RuntimeException("Only one shader module must be bound to a ray-tracing pipeline.");

		Array<VkPipelineShaderStageCreateInfo> shaderStages = modules |
			std::views::transform([](const VulkanShaderModule* shaderModule) { return shaderModule->shaderStageDefinition(); }) |
			std::ranges::to<Array<VkPipelineShaderStageCreateInfo>>();

		throw;
//		// Setup pipeline state.
//		VkRayTracingPipelineCreateInfo pipelineInfo = {};
//		pipelineInfo.sType = VK_STRUCTURE_TYPE_RAYTRACING_PIPELINE_CREATE_INFO;
//		pipelineInfo.layout = std::as_const(*m_layout.get()).handle();
//		pipelineInfo.stage = shaderStages.front();
//
//		VkPipeline pipeline;
//		raiseIfFailed(::vkCreateRayTracingPipelines(m_device.handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create ray-tracing pipeline.");
//
//#ifndef NDEBUG
//		m_device.setDebugName(*reinterpret_cast<const UInt64*>(&pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, m_parent->name());
//#endif
//
//		return pipeline;
	}

	//VkPipeline initializeRayTracingPipeline(const VkPipelineDynamicStateCreateInfo& dynamicState, const LiteFX::Array<VkPipelineShaderStageCreateInfo>& shaderStages)
	//{
	//	LITEFX_TRACE(VULKAN_LOG, "Creating ray-tracing pipeline \"{1}\" for layout {0}...", fmt::ptr(reinterpret_cast<void*>(m_layout.get())), m_parent->name());

	//	VkRayTracingPipelineCreateInfoKHR pipelineInfo = {
	//		.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
	//		.stageCount = static_cast<UInt32>(shaderStages.size()),
	//		.pStages = shaderStages.data(),
	//		.groupCount = 0,		// TODO: This must receive the SBT.
	//		.pGroups = nullptr,
	//		.maxPipelineRayRecursionDepth = 10, // TODO: Make configurable.
	//		.pDynamicState = &dynamicState,
	//		.layout = std::as_const(*m_layout.get()).handle()
	//	};

	//	VkPipeline pipeline;
	//	raiseIfFailed(::vkCreateRayTracingPipelines(m_renderPass.device().handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create render pipeline.");

	//	return pipeline;
	//}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRayTracingPipeline::VulkanRayTracingPipeline(const VulkanDevice& device, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram, ShaderRecordCollection&& shaderRecords, const String& name) :
	m_impl(makePimpl<VulkanRayTracingPipelineImpl>(this, device, layout, shaderProgram, std::move(shaderRecords))), VulkanPipelineState(VK_NULL_HANDLE)
{
	if (!name.empty())
		this->name() = name;

	this->handle() = m_impl->initialize();
}

VulkanRayTracingPipeline::VulkanRayTracingPipeline(const VulkanDevice& device, ShaderRecordCollection&& shaderRecords) noexcept :
	m_impl(makePimpl<VulkanRayTracingPipelineImpl>(this, device, std::move(shaderRecords))), VulkanPipelineState(VK_NULL_HANDLE)
{
}

VulkanRayTracingPipeline::~VulkanRayTracingPipeline() noexcept
{
	::vkDestroyPipeline(m_impl->m_device.handle(), this->handle(), nullptr);
}

SharedPtr<const VulkanShaderProgram> VulkanRayTracingPipeline::program() const noexcept
{
	return m_impl->m_program;
}

SharedPtr<const VulkanPipelineLayout> VulkanRayTracingPipeline::layout() const noexcept 
{
	return m_impl->m_layout;
}

const ShaderRecordCollection& VulkanRayTracingPipeline::shaderRecords() const noexcept
{
	return m_impl->m_shaderRecordCollection;
}

void VulkanRayTracingPipeline::use(const VulkanCommandBuffer& commandBuffer) const noexcept
{
	::vkCmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, this->handle());
}

void VulkanRayTracingPipeline::bind(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet) const noexcept
{
	// DescriptorSet -> DescriptorSetLayout
	::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, std::as_const(*m_impl->m_layout).handle(), descriptorSet.layout().space(), 1, &descriptorSet.handle(), 0, nullptr);
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

constexpr VulkanRayTracingPipelineBuilder::VulkanRayTracingPipelineBuilder(const VulkanDevice& device, ShaderRecordCollection&& shaderRecords, const String& name) :
	RayTracingPipelineBuilder(UniquePtr<VulkanRayTracingPipeline>(new VulkanRayTracingPipeline(device, std::move(shaderRecords))))
{
	this->instance()->name() = name;
}

constexpr VulkanRayTracingPipelineBuilder::~VulkanRayTracingPipelineBuilder() noexcept = default;

void VulkanRayTracingPipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = m_state.pipelineLayout;
	instance->handle() = instance->m_impl->initialize();
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)