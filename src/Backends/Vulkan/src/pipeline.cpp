#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipeline::VulkanRenderPipelineImpl {
private:

public:
	VulkanRenderPipelineImpl() noexcept = default;

public:
	VkPipeline initialize(VulkanRenderPipeline& parent, const VulkanRenderPipelineLayout* layout)
	{
		if (layout == nullptr)
			throw std::invalid_argument("The pipeline layout must be initialized.");

		// Get the device.
		auto device = dynamic_cast<const VulkanDevice*>(parent.getDevice());

		if (device == nullptr)
			throw std::invalid_argument("The pipeline device is not a valid Vulkan device.");

		// Request configuration interface.
		auto rasterizer = layout->getRasterizer();
		auto inputAssembler = layout->getInputAssembler();
		auto views = layout->getViewports();
		auto program = parent.getProgram();

		if (rasterizer == nullptr)
			throw std::invalid_argument("The pipeline layout does not contain a rasterizer.");

		if (program == nullptr)
			throw std::invalid_argument("The pipeline shader program must be initialized.");		

		// Setup rasterizer state.
		VkPipelineRasterizationStateCreateInfo rasterizerState = {};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = getPolygonMode(rasterizer->getPolygonMode());
		rasterizerState.lineWidth = rasterizer->getLineWidth();
		rasterizerState.cullMode = getCullMode(rasterizer->getCullMode());
		rasterizerState.frontFace = rasterizer->getCullOrder() == CullOrder::ClockWise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = rasterizer->getDepthBiasEnabled();
		rasterizerState.depthBiasClamp = rasterizer->getDepthBiasClamp();
		rasterizerState.depthBiasConstantFactor = rasterizer->getDepthBiasConstantFactor();
		rasterizerState.depthBiasSlopeFactor = rasterizer->getDepthBiasSlopeFactor();

		LITEFX_TRACE(VULKAN_LOG, "Rasterizer state: {{ PolygonMode: {0}, CullMode: {1}, CullOrder: {2}, LineWidth: {3} }}", rasterizer->getPolygonMode(), rasterizer->getCullMode(), rasterizer->getCullOrder(), rasterizer->getLineWidth());
		
		if (rasterizerState.depthBiasEnable)
			LITEFX_TRACE(VULKAN_LOG, "Rasterizer depth bias: {{ Clamp: {0}, ConstantFactor: {1}, SlopeFactor: {2} }}", rasterizer->getDepthBiasClamp(), rasterizer->getDepthBiasConstantFactor(), rasterizer->getDepthBiasSlopeFactor());
		else
			LITEFX_TRACE(VULKAN_LOG, "Rasterizer depth bias disabled.");

		// Setup input assembler state.
		VkPipelineVertexInputStateCreateInfo inputState = {};
		inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

		if (inputAssembler != nullptr)
		{
			LITEFX_TRACE(VULKAN_LOG, "Input assembler state: {{ PrimitiveTopology: {0} }}", inputAssembler->getTopology());

			inputAssembly.topology = getPrimitiveTopology(inputAssembler->getTopology());
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			// TODO: Implement me!
			inputState.vertexBindingDescriptionCount = 0;
			inputState.vertexAttributeDescriptionCount = 0;
		}
		else
		{
			LITEFX_TRACE(VULKAN_LOG, "No input assembler provided.");
		}

		// Setup viewport state.
		Array<VkViewport> viewports;
		Array<VkRect2D> scissors;
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		for (auto v(0); v < views.size(); ++v)
		{
			auto view = views[v];

			if (view == nullptr)
				throw std::invalid_argument("The specified viewports must be initialized.");

			LITEFX_TRACE(VULKAN_LOG, "Viewport state {0}/{1}: {{ X: {2}, Y: {3}, Width: {4}, Height: {5}, Scissors: {6} }}", v + 1, views.size(), 
				view->getRectangle().x(), view->getRectangle().y(), view->getRectangle().width(), view->getRectangle().height(), view->getScissors().size());

			VkViewport viewport = {};
			viewport.x = view->getRectangle().x();
			viewport.y = view->getRectangle().y();
			viewport.width = view->getRectangle().width();
			viewport.height = view->getRectangle().height();
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			for each (auto& stencil in view->getScissors())
			{
				VkRect2D scissor = {};
				scissor.offset = { static_cast<Int32>(stencil.x()), static_cast<Int32>(stencil.y()) };
				scissor.extent = { static_cast<UInt32>(stencil.width()), static_cast<UInt32>(stencil.height()) };
				scissors.push_back(scissor);
			}

			viewports.push_back(viewport);
		}

		viewportState.viewportCount = static_cast<UInt32>(viewports.size());
		viewportState.pViewports = viewports.data();
		viewportState.scissorCount = static_cast<UInt32>(scissors.size());
		viewportState.pScissors = scissors.data();

		// Setup multisampling state.
		// TODO: Abstract me!
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		// Setup color blend state.
		// TODO: Abstract me!
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		// Setup pipeline state.
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pVertexInputState = &inputState;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = layout->handle();
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// Setup shader stages.
		auto modules = program->getModules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(program), modules.size());
		Array<VkPipelineShaderStageCreateInfo> shaderStages(modules.size());

		std::generate(std::begin(shaderStages), std::end(shaderStages), [&, i = 0]() mutable {
			auto module = dynamic_cast<const VulkanShaderModule*>(modules[i++]);

			if (module == nullptr)
				throw std::invalid_argument("The provided shader module is not a valid Vulkan shader.");

			LITEFX_TRACE(VULKAN_LOG, "Module {0}/{1} (\"{4}\") state: {{ Type: {2}, EntryPoint: {3}}}", i, modules.size(), module->getType(), module->getEntryPoint(), module->getFileName());
			
			return module->getShaderStageDefinition();
		});

		pipelineInfo.stageCount = modules.size();
		pipelineInfo.pStages = shaderStages.data();

		// Setup render pass state.
		// TODO: Implement render passes / sub-passes.
		//pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		VkPipeline pipeline;

		if (::vkCreateGraphicsPipelines(device->handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
			throw std::runtime_error("Unable to create render pipeline.");

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipeline::VulkanRenderPipeline(const IGraphicsDevice* device) :
	m_impl(makePimpl<VulkanRenderPipelineImpl>()), RenderPipeline(device), IResource<VkPipeline>(nullptr)
{
	if (device == nullptr)
		throw std::invalid_argument("The argument `device` must be initialized.");
}

VulkanRenderPipeline::VulkanRenderPipeline(UniquePtr<IRenderPipelineLayout>&& layout, const IGraphicsDevice* device) :
	m_impl(makePimpl<VulkanRenderPipelineImpl>()), RenderPipeline(device, std::move(layout)), IResource<VkPipeline>(nullptr)
{
	this->create();
}

VulkanRenderPipeline::~VulkanRenderPipeline() noexcept = default;

void VulkanRenderPipeline::create()
{
	auto layout = dynamic_cast<const VulkanRenderPipelineLayout*>(this->getLayout());
	LITEFX_DEBUG(VULKAN_LOG, "Creating render pipeline for layout {0}...", fmt::ptr(layout));

	auto& h = this->handle();

	if (h == nullptr)
		h = m_impl->initialize(*this, layout);
	else
	{
		LITEFX_WARNING(VULKAN_LOG, "The render pipeline is already created and hence will be rebuilt. Consider using `IRenderPipeline::rebuild` instead.");
		//this->rebuild(std::move(layout));
		throw;
	}
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilderImpl {
private:
	Array<UniquePtr<IRenderPass>> m_renderPasses;
	UniquePtr<IRenderPipelineLayout> m_layout;
	UniquePtr<IShaderProgram> m_program;

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

	UniquePtr<IShaderProgram> useProgram() noexcept
	{
		return std::move(m_program);
	}

	void setProgram(UniquePtr<IShaderProgram>&& program)
	{
		m_program = std::move(program);
	}

	Array<UniquePtr<IRenderPass>> useRenderPasses() noexcept
	{
		return std::move(m_renderPasses);
	}

	void addRenderPass(UniquePtr<IRenderPass>&& renderPass)
	{
		m_renderPasses.push_back(std::move(renderPass));
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
	this->instance()->use(std::move(m_impl->useProgram()));
	this->instance()->use(std::move(m_impl->useLayout()));
	this->instance()->use(std::move(m_impl->useRenderPasses()));

	this->instance()->create();

	return RenderPipelineBuilder::go();
}

void VulkanRenderPipelineBuilder::use(UniquePtr<IRenderPipelineLayout> && layout)
{
    if (layout == nullptr)
        throw std::invalid_argument("The layout must be initialized.");

	m_impl->setLayout(std::move(layout));
}

void VulkanRenderPipelineBuilder::use(UniquePtr<IShaderProgram>&& program)
{
	if (program == nullptr)
		throw std::invalid_argument("The program must be initialized.");

	m_impl->setProgram(std::move(program));
}

void VulkanRenderPipelineBuilder::use(UniquePtr<IRenderPass>&& renderPass)
{
	if (renderPass == nullptr)
		throw std::invalid_argument("The render pass must be initialized.");

	m_impl->addRenderPass(std::move(renderPass));
}