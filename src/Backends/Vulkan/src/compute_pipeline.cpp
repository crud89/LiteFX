#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanComputePipeline::VulkanComputePipelineImpl : public Implement<VulkanComputePipeline> {
public:
	friend class VulkanComputePipelineBuilder;
	friend class VulkanComputePipeline;

private:
	SharedPtr<VulkanPipelineLayout> m_layout;
	SharedPtr<VulkanShaderProgram> m_program;
	String m_name;
	const VulkanDevice& m_device;

public:
	VulkanComputePipelineImpl(VulkanComputePipeline* parent, const VulkanDevice& device, const String& name, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram) :
		base(parent), m_device(device), m_name(name), m_layout(layout), m_program(shaderProgram)
	{
	}

	VulkanComputePipelineImpl(VulkanComputePipeline* parent, const VulkanDevice& device) :
		base(parent), m_device(device)
	{
	}

public:
	VkPipeline initialize()
	{
		LITEFX_TRACE(VULKAN_LOG, "Creating compute pipeline (\"{1}\") for layout {0}...", fmt::ptr(reinterpret_cast<void*>(m_layout.get())), m_name);
	
		// Setup shader stages.
		auto modules = m_program->modules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(reinterpret_cast<const void*>(m_program.get())), modules.size());

		if (modules.size() > 1)
			throw RuntimeException("Only one shader module must be bound to a compute pipeline.");

		Array<VkPipelineShaderStageCreateInfo> shaderStages = modules |
			std::views::transform([](const VulkanShaderModule* shaderModule) { return shaderModule->shaderStageDefinition(); }) |
			ranges::to<Array<VkPipelineShaderStageCreateInfo>>();

		// Setup pipeline state.
		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = std::as_const(*m_layout.get()).handle();
		pipelineInfo.stage = shaderStages.front();

		VkPipeline pipeline;
		raiseIfFailed<RuntimeException>(::vkCreateComputePipelines(m_device.handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create compute pipeline.");

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipeline::VulkanComputePipeline(const VulkanDevice& device, SharedPtr<VulkanShaderProgram> shaderProgram, SharedPtr<VulkanPipelineLayout> layout, const String& name) :
	m_impl(makePimpl<VulkanComputePipelineImpl>(this, device, name, layout, shaderProgram)), VulkanPipelineState(VK_NULL_HANDLE)
{
	this->handle() = m_impl->initialize();
}

VulkanComputePipeline::VulkanComputePipeline(const VulkanDevice& device) noexcept :
	m_impl(makePimpl<VulkanComputePipelineImpl>(this, device)), VulkanPipelineState(VK_NULL_HANDLE)
{
}

VulkanComputePipeline::~VulkanComputePipeline() noexcept
{
	::vkDestroyPipeline(m_impl->m_device.handle(), this->handle(), nullptr);
}

const String& VulkanComputePipeline::name() const noexcept
{
	return m_impl->m_name;
}

SharedPtr<const VulkanShaderProgram> VulkanComputePipeline::program() const noexcept
{
	return m_impl->m_program;
}

SharedPtr<const VulkanPipelineLayout> VulkanComputePipeline::layout() const noexcept 
{
	return m_impl->m_layout;
}

void VulkanComputePipeline::use(const VulkanCommandBuffer& commandBuffer) const noexcept
{
	::vkCmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, this->handle());
}

void VulkanComputePipeline::bind(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet) const noexcept
{
	// DescriptorSet -> DescriptorSetLayout
	::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, std::as_const(*m_impl->m_layout).handle(), descriptorSet.layout().space(), 1, &descriptorSet.handle(), 0, nullptr);
}