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
	UniquePtr<VulkanPipelineLayout> m_layout;
	UniquePtr<VulkanCommandBuffer> m_commandBuffer;
	String m_name;

public:
	VulkanComputePipelineImpl(VulkanComputePipeline* parent, const String& name, UniquePtr<VulkanPipelineLayout>&& layout) :
		base(parent), m_name(name), m_layout(std::move(layout))
	{
	}

	VulkanComputePipelineImpl(VulkanComputePipeline* parent) :
		base(parent)
	{
	}

public:
	VkPipeline initialize()
	{
		LITEFX_TRACE(VULKAN_LOG, "Creating compute pipeline (\"{1}\") for layout {0}...", fmt::ptr(reinterpret_cast<void*>(m_layout.get())), m_name);
	
		// Get the device and create a command buffer.
		auto device = m_parent->getDevice();
		m_commandBuffer = m_parent->parent().computeQueue().createCommandBuffer(false);

		// Setup shader stages.
		auto modules = m_layout->program().modules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(reinterpret_cast<const void*>(&m_layout->program())), modules.size());

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
		raiseIfFailed<RuntimeException>(::vkCreateComputePipelines(device->handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create compute pipeline.");

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanComputePipeline::VulkanComputePipeline(const VulkanDevice& device, UniquePtr<VulkanPipelineLayout>&& layout, const String& name) :
	m_impl(makePimpl<VulkanComputePipelineImpl>(this, name, std::move(layout))), VulkanRuntimeObject(device, &device), VulkanPipelineState(VK_NULL_HANDLE)
{
	this->handle() = m_impl->initialize();
}

VulkanComputePipeline::VulkanComputePipeline(const VulkanDevice& device) noexcept :
	m_impl(makePimpl<VulkanComputePipelineImpl>(this)), VulkanRuntimeObject(device, &device), VulkanPipelineState(VK_NULL_HANDLE)
{
}

VulkanComputePipeline::~VulkanComputePipeline() noexcept
{
	::vkDestroyPipeline(this->getDevice()->handle(), this->handle(), nullptr);
}

const String& VulkanComputePipeline::name() const noexcept
{
	return m_impl->m_name;
}

const VulkanPipelineLayout& VulkanComputePipeline::layout() const noexcept 
{
	return *m_impl->m_layout;
}

void VulkanComputePipeline::bind(const VulkanDescriptorSet& descriptorSet) const
{
	::vkCmdBindDescriptorSets(std::as_const(*m_impl->m_commandBuffer).handle(), VK_PIPELINE_BIND_POINT_COMPUTE, std::as_const(*m_impl->m_layout).handle(), descriptorSet.parent().space(), 1, &descriptorSet.handle(), 0, nullptr);
}

void VulkanComputePipeline::use() const 
{
	m_impl->m_commandBuffer->begin();
	::vkCmdBindPipeline(std::as_const(*m_impl->m_commandBuffer).handle(), VK_PIPELINE_BIND_POINT_COMPUTE, this->handle());
}

void VulkanComputePipeline::dispatch(const Vector3u& threadCount) const noexcept
{
	::vkCmdDispatch(std::as_const(*m_impl->m_commandBuffer).handle(), threadCount.x(), threadCount.y(), threadCount.z());
}

void VulkanComputePipeline::submit(const bool& wait) const noexcept
{
	m_impl->m_commandBuffer->end(true, wait);
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanComputePipelineBuilder::VulkanComputePipelineBuilderImpl : public Implement<VulkanComputePipelineBuilder> {
public:
	friend class VulkanComputePipelineBuilderBuilder;
	friend class VulkanComputePipelineBuilder;

private:
	UniquePtr<VulkanPipelineLayout> m_layout;

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
	this->instance()->m_impl->m_name = name;
}

VulkanComputePipelineBuilder::~VulkanComputePipelineBuilder() noexcept = default;

UniquePtr<VulkanComputePipeline> VulkanComputePipelineBuilder::go()
{
	auto instance = this->instance(); 
	instance->m_impl->m_layout = std::move(m_impl->m_layout);
	instance->handle() = instance->m_impl->initialize();

	return ComputePipelineBuilder::go();
}

void VulkanComputePipelineBuilder::use(UniquePtr<VulkanPipelineLayout>&& layout)
{
#ifndef NDEBUG
	if (m_impl->m_layout != nullptr)
		LITEFX_WARNING(VULKAN_LOG, "Another pipeline layout has already been initialized and will be replaced. A pipeline can only have one pipeline layout.");
#endif

	m_impl->m_layout = std::move(layout);
}

VulkanComputePipelineLayoutBuilder VulkanComputePipelineBuilder::layout()
{
	return VulkanComputePipelineLayoutBuilder(*this);
}