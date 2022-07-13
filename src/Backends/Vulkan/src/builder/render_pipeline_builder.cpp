#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilderImpl : public Implement<VulkanRenderPipelineBuilder> {
public:
	friend class VulkanRenderPipelineBuilderBuilder;
	friend class VulkanRenderPipelineBuilder;

private:
	UniquePtr<VulkanPipelineLayout> m_layout;
	SharedPtr<VulkanInputAssembler> m_inputAssembler;
	SharedPtr<VulkanRasterizer> m_rasterizer;
	Array<SharedPtr<IViewport>> m_viewports;
	Array<SharedPtr<IScissor>> m_scissors;
	bool m_alphaToCoverage{ false };

public:
	VulkanRenderPipelineBuilderImpl(VulkanRenderPipelineBuilder* parent) :
		base(parent)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilder(const VulkanRenderPass& renderPass, const UInt32& id, const String& name) :
	m_impl(makePimpl<VulkanRenderPipelineBuilderImpl>(this)), RenderPipelineBuilder(UniquePtr<VulkanRenderPipeline>(new VulkanRenderPipeline(renderPass)))
{
	this->instance()->m_impl->m_id = id;
	this->instance()->m_impl->m_name = name;
}

VulkanRenderPipelineBuilder::~VulkanRenderPipelineBuilder() noexcept = default;

UniquePtr<VulkanRenderPipeline> VulkanRenderPipelineBuilder::go()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = std::move(m_impl->m_layout);
	instance->m_impl->m_inputAssembler = std::move(m_impl->m_inputAssembler);
	instance->m_impl->m_rasterizer = std::move(m_impl->m_rasterizer);
	instance->m_impl->m_viewports = std::move(m_impl->m_viewports);
	instance->m_impl->m_scissors = std::move(m_impl->m_scissors);
	instance->m_impl->m_alphaToCoverage = std::move(m_impl->m_alphaToCoverage);
	instance->handle() = instance->m_impl->initialize();

	return RenderPipelineBuilder::go();
}

void VulkanRenderPipelineBuilder::use(UniquePtr<VulkanPipelineLayout>&& layout)
{
#ifndef NDEBUG
	if (m_impl->m_layout != nullptr)
		LITEFX_WARNING(VULKAN_LOG, "Another pipeline layout has already been initialized and will be replaced. A pipeline can only have one pipeline layout.");
#endif

	m_impl->m_layout = std::move(layout);
}

void VulkanRenderPipelineBuilder::use(SharedPtr<IRasterizer> rasterizer)
{
#ifndef NDEBUG
	if (m_impl->m_rasterizer != nullptr)
		LITEFX_WARNING(VULKAN_LOG, "Another rasterizer has already been initialized and will be replaced. A pipeline can only have one rasterizer.");
#endif

	auto vulkanRasterizer = std::dynamic_pointer_cast<VulkanRasterizer>(rasterizer);

	if (vulkanRasterizer == nullptr)
		throw InvalidArgumentException("The provided rasterizer must be a Vulkan rasterizer instance.");

	m_impl->m_rasterizer = vulkanRasterizer;
}

void VulkanRenderPipelineBuilder::use(SharedPtr<VulkanInputAssembler> inputAssembler)
{
#ifndef NDEBUG
	if (m_impl->m_inputAssembler != nullptr)
		LITEFX_WARNING(VULKAN_LOG, "Another input assembler has already been initialized and will be replaced. A pipeline can only have one input assembler.");
#endif

	m_impl->m_inputAssembler = inputAssembler;
}

void VulkanRenderPipelineBuilder::use(SharedPtr<IViewport> viewport)
{
	m_impl->m_viewports.push_back(viewport);
}

void VulkanRenderPipelineBuilder::use(SharedPtr<IScissor> scissor)
{
	m_impl->m_scissors.push_back(scissor);
}

VulkanRenderPipelineBuilder& VulkanRenderPipelineBuilder::enableAlphaToCoverage(const bool& enable)
{
	m_impl->m_alphaToCoverage = enable;
	return *this;
}

VulkanRenderPipelineLayoutBuilder VulkanRenderPipelineBuilder::layout()
{
	return VulkanRenderPipelineLayoutBuilder(*this);
}

VulkanRasterizerBuilder VulkanRenderPipelineBuilder::rasterizer()
{
	return VulkanRasterizerBuilder(*this);
}

VulkanInputAssemblerBuilder VulkanRenderPipelineBuilder::inputAssembler()
{
	return VulkanInputAssemblerBuilder(*this);
}

VulkanRenderPipelineBuilder& VulkanRenderPipelineBuilder::withRasterizer(SharedPtr<IRasterizer> rasterizer)
{
	this->use(std::move(rasterizer));
	return *this;
}

VulkanRenderPipelineBuilder& VulkanRenderPipelineBuilder::withInputAssembler(SharedPtr<VulkanInputAssembler> inputAssembler)
{
	this->use(std::move(inputAssembler));
	return *this;
}

VulkanRenderPipelineBuilder& VulkanRenderPipelineBuilder::withViewport(SharedPtr<IViewport> viewport)
{
	this->use(std::move(viewport));
	return *this;
}

VulkanRenderPipelineBuilder& VulkanRenderPipelineBuilder::withScissor(SharedPtr<IScissor> scissor)
{
	this->use(std::move(scissor));
	return *this;
}