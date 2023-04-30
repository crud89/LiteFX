#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

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
	const VulkanDevice& m_device;

public:
	VulkanComputePipelineImpl(VulkanComputePipeline* parent, const VulkanDevice& device, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram) :
		base(parent), m_device(device), m_layout(layout), m_program(shaderProgram)
	{
	}

	VulkanComputePipelineImpl(VulkanComputePipeline* parent, const VulkanDevice& device) :
		base(parent), m_device(device)
	{
	}

public:
	VkPipeline initialize()
	{
		LITEFX_TRACE(VULKAN_LOG, "Creating compute pipeline (\"{1}\") for layout {0}...", fmt::ptr(reinterpret_cast<void*>(m_layout.get())), m_parent->name());
	
		// Setup shader stages.
		auto modules = m_program->modules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(reinterpret_cast<const void*>(m_program.get())), modules.size());

		if (modules.size() > 1)
			throw RuntimeException("Only one shader module must be bound to a compute pipeline.");

		Array<VkPipelineShaderStageCreateInfo> shaderStages = modules |
			std::views::transform([](const VulkanShaderModule* shaderModule) { return shaderModule->shaderStageDefinition(); }) |
			std::ranges::to<Array<VkPipelineShaderStageCreateInfo>>();

		// Setup pipeline state.
		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = std::as_const(*m_layout.get()).handle();
		pipelineInfo.stage = shaderStages.front();

		VkPipeline pipeline;
		raiseIfFailed<RuntimeException>(::vkCreateComputePipelines(m_device.handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create compute pipeline.");

#ifndef NDEBUG
		m_device.setDebugName(*reinterpret_cast<const UInt64*>(&pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, m_parent->name());
#endif

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipeline::VulkanComputePipeline(const VulkanDevice& device, SharedPtr<VulkanShaderProgram> shaderProgram, SharedPtr<VulkanPipelineLayout> layout, const String& name) :
	m_impl(makePimpl<VulkanComputePipelineImpl>(this, device, layout, shaderProgram)), VulkanPipelineState(VK_NULL_HANDLE)
{
	if (!name.empty())
		this->name() = name;

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

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanComputePipelineBuilder::VulkanComputePipelineBuilderImpl : public Implement<VulkanComputePipelineBuilder> {
public:
	friend class VulkanComputePipelineBuilder;

private:
	SharedPtr<VulkanShaderProgram> m_program;
	SharedPtr<VulkanPipelineLayout> m_layout;

public:
	VulkanComputePipelineBuilderImpl(VulkanComputePipelineBuilder* parent) :
		base(parent)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipelineBuilder::VulkanComputePipelineBuilder(const VulkanDevice& device, const String& name) :
	m_impl(makePimpl<VulkanComputePipelineBuilderImpl>(this)), ComputePipelineBuilder(UniquePtr<VulkanComputePipeline>(new VulkanComputePipeline(device)))
{
	this->instance()->name() = name;
}

VulkanComputePipelineBuilder::~VulkanComputePipelineBuilder() noexcept = default;

void VulkanComputePipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = std::move(m_impl->m_layout);
	instance->m_impl->m_program = std::move(m_impl->m_program);
	instance->handle() = instance->m_impl->initialize();
}

VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::shaderProgram(SharedPtr<VulkanShaderProgram> program)
{
#ifndef NDEBUG
	if (m_impl->m_layout != nullptr)
		LITEFX_WARNING(VULKAN_LOG, "Another shader program has already been initialized and will be replaced. A pipeline can only have one shader program.");
#endif

	m_impl->m_program = program;
	return *this;
}

VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::layout(SharedPtr<VulkanPipelineLayout> layout)
{
#ifndef NDEBUG
	if (m_impl->m_layout != nullptr)
		LITEFX_WARNING(VULKAN_LOG, "Another pipeline layout has already been initialized and will be replaced. A pipeline can only have one pipeline layout.");
#endif

	m_impl->m_layout = layout;
	return *this;
}
#endif // defined(BUILD_DEFINE_BUILDERS)