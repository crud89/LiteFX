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
		LITEFX_TRACE(VULKAN_LOG, "Creating compute pipeline (\"{1}\") for layout {0}...", reinterpret_cast<void*>(m_layout.get()), m_parent->name());
	
		// Setup shader stages.
		auto modules = m_program->modules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", reinterpret_cast<void*>(m_program.get()), modules.size());

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
		raiseIfFailed(::vkCreateComputePipelines(m_device.handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create compute pipeline.");

#ifndef NDEBUG
		m_device.setDebugName(*reinterpret_cast<const UInt64*>(&pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, m_parent->name());
#endif

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipeline::VulkanComputePipeline(const VulkanDevice& device, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram, const String& name) :
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

void VulkanComputePipeline::bind(const VulkanCommandBuffer& commandBuffer, Span<const VulkanDescriptorSet*> descriptorSets) const noexcept
{
	// Filter out uninitialized sets.
	auto sets = descriptorSets | std::views::filter([](auto set) { return set != nullptr; }) | std::ranges::to<Array<const VulkanDescriptorSet*>>();

	if (sets.empty()) [[unlikely]]
		return; // Nothing to do on empty sets.
	else if (sets.size() == 1)
		::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, std::as_const(*m_impl->m_layout).handle(), sets.front()->layout().space(), 1, &sets.front()->handle(), 0, nullptr);
	else
	{
		// Sort the descriptor sets by space, as we might be able to pass the sets more efficiently if they are sorted and continuous.
		std::ranges::sort(sets, [](auto lhs, auto rhs) { return lhs->layout().space() > rhs->layout().space(); });

		// In a sorted range, last - (first - 1) equals the size of the range only if there are no duplicates and no gaps.
		auto startSpace = sets.back()->layout().space();

		if (startSpace - (sets.front()->layout().space() - 1) != static_cast<UInt32>(sets.size()))
			std::ranges::for_each(sets, [&](auto set) { ::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, std::as_const(*m_impl->m_layout).handle(), set->layout().space(), 1, &set->handle(), 0, nullptr); });
		else
		{
			// Obtain the handles and bind the sets.
			auto handles = sets | std::views::transform([](auto set) { return set->handle(); }) | std::ranges::to<Array<VkDescriptorSet>>();
			::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, std::as_const(*m_impl->m_layout).handle(), startSpace, static_cast<UInt32>(handles.size()), handles.data(), 0, nullptr);
		}
	}
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
	instance->m_impl->m_layout = m_state.pipelineLayout;
	instance->m_impl->m_program = m_state.shaderProgram;
	instance->handle() = instance->m_impl->initialize();
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)