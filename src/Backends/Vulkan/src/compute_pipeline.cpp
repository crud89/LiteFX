#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanComputePipeline::VulkanComputePipelineImpl {
public:
	friend class VulkanComputePipelineBuilder;
	friend class VulkanComputePipeline;

private:
	SharedPtr<const VulkanDevice> m_device;
	SharedPtr<VulkanPipelineLayout> m_layout;
	SharedPtr<VulkanShaderProgram> m_program;

public:
	VulkanComputePipelineImpl(const VulkanDevice& device, const SharedPtr<VulkanPipelineLayout>& layout, const SharedPtr<VulkanShaderProgram>& shaderProgram) :
		m_device(device.shared_from_this()), m_layout(layout), m_program(shaderProgram)
	{
	}

	VulkanComputePipelineImpl(const VulkanDevice& device) :
		m_device(device.shared_from_this())
	{
	}

public:
	VkPipeline initialize([[maybe_unused]] const VulkanComputePipeline& parent)
	{
		LITEFX_TRACE(VULKAN_LOG, "Creating compute pipeline (\"{1}\") for layout {0}...", static_cast<void*>(m_layout.get()), parent.name());
	
		// Setup shader stages.
		auto& modules = m_program->modules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", static_cast<void*>(m_program.get()), modules.size());

		if (modules.size() > 1)
			throw RuntimeException("Only one shader module must be bound to a compute pipeline.");

		Array<VkPipelineShaderStageCreateInfo> shaderStages = modules 
			| std::views::transform([](const auto& shaderModule) { return shaderModule->shaderStageDefinition(); }) 
			| std::ranges::to<Array<VkPipelineShaderStageCreateInfo>>();

		// Setup pipeline state.
		VkComputePipelineCreateInfo pipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
			.stage = shaderStages.front(),
			.layout = std::as_const(*m_layout.get()).handle()
		};

		VkPipeline pipeline{};
		raiseIfFailed(::vkCreateComputePipelines(m_device->handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create compute pipeline.");

#ifndef NDEBUG
		m_device->setDebugName(pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, parent.name());
#endif

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipeline::VulkanComputePipeline(const VulkanDevice& device, const SharedPtr<VulkanPipelineLayout>& layout, const SharedPtr<VulkanShaderProgram>& shaderProgram, const String& name) :
	VulkanPipelineState(VK_NULL_HANDLE), m_impl(device, layout, shaderProgram)
{
	if (!name.empty())
		this->name() = name;

	this->handle() = m_impl->initialize(*this);
}

VulkanComputePipeline::VulkanComputePipeline(const VulkanDevice& device) noexcept :
	VulkanPipelineState(VK_NULL_HANDLE), m_impl(device)
{
}

VulkanComputePipeline::VulkanComputePipeline(VulkanComputePipeline&&) noexcept = default;
VulkanComputePipeline& VulkanComputePipeline::operator=(VulkanComputePipeline&&) noexcept = default;

VulkanComputePipeline::~VulkanComputePipeline() noexcept
{
	::vkDestroyPipeline(m_impl->m_device->handle(), this->handle(), nullptr);
}

SharedPtr<const VulkanShaderProgram> VulkanComputePipeline::program() const noexcept
{
	return m_impl->m_program;
}

SharedPtr<const VulkanPipelineLayout> VulkanComputePipeline::layout() const noexcept 
{
	return m_impl->m_layout;
}

void VulkanComputePipeline::use(const VulkanCommandBuffer& commandBuffer) const
{
	::vkCmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, this->handle());
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipelineBuilder::VulkanComputePipelineBuilder(const VulkanDevice& device, const String& name) :
	ComputePipelineBuilder(UniquePtr<VulkanComputePipeline>(new VulkanComputePipeline(device)))
{
	this->instance()->name() = name;
}

VulkanComputePipelineBuilder::~VulkanComputePipelineBuilder() noexcept = default;

void VulkanComputePipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = this->state().pipelineLayout;
	instance->m_impl->m_program = this->state().shaderProgram;
	instance->handle() = instance->m_impl->initialize(*instance);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)