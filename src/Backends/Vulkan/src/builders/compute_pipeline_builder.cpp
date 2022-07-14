#include <litefx/backends/vulkan_builders.cpp>

using namespace LiteFX::Rendering::Backends;

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