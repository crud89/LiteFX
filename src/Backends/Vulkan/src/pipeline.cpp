#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipeline::VulkanRenderPipelineImpl {
private:

public:
	VulkanRenderPipelineImpl() noexcept = default;
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipeline::VulkanRenderPipeline() noexcept :
	m_impl(makePimpl<VulkanRenderPipelineImpl>()), RenderPipeline(), IResource<VkPipeline>(nullptr)
{
}

VulkanRenderPipeline::~VulkanRenderPipeline() noexcept = default;

void VulkanRenderPipeline::create(UniquePtr<IRenderPipelineLayout>&& layout)
{
	if (layout == nullptr)
		throw std::invalid_argument("The argument `layout` must be initialized.");

	LITEFX_DEBUG(VULKAN_LOG, "Creating render pipeline for layout {0}...", fmt::ptr(layout.get()));

	auto& h = this->handle();

	if (h != nullptr)
	{
		LITEFX_WARNING(VULKAN_LOG, "The render pipeline is already created and will be rebuilt. Consider using `IRenderPipeline::rebuild` instead.");
		//this->rebuild(std::move(layout));
		throw;
	}
	else
	{
		// TODO: Create pipeline.
		//h = m_impl->initialize(*this, std::move(pipeline));
		throw;
	}
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilderImpl {
private:
	UniquePtr<IRenderPipelineLayout> m_layout;

public:
	VulkanRenderPipelineBuilderImpl() noexcept = default;

public:
	UniquePtr<IRenderPipelineLayout> useLayout() noexcept
	{
		return std::move(m_layout);
	}

	void setLayout(UniquePtr<IRenderPipelineLayout>&& layout)
	{
		m_layout = std::move(layout);
	}
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilder(UniquePtr<VulkanRenderPipeline>&& instance) noexcept :
	RenderPipelineBuilder(std::move(instance)), m_impl(makePimpl<VulkanRenderPipelineBuilderImpl>())
{
}

VulkanRenderPipelineBuilder::~VulkanRenderPipelineBuilder() noexcept = default;

UniquePtr<VulkanRenderPipeline> VulkanRenderPipelineBuilder::go()
{
	this->instance()->create(std::move(m_impl->useLayout()));
	return RenderPipelineBuilder::go();
}

void VulkanRenderPipelineBuilder::use(UniquePtr<IRenderPipelineLayout> && layout)
{
	m_impl->setLayout(std::move(layout));
}