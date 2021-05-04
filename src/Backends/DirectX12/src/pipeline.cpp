#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPipeline::DirectX12RenderPipelineImpl : public Implement<DirectX12RenderPipeline> {
public:
	friend class DirectX12RenderPipelineBuilder;
	friend class DirectX12RenderPipeline;

private:
	const DirectX12RenderPass& m_renderPass;
	UniquePtr<IRenderPipelineLayout> m_layout;

public:
	DirectX12RenderPipelineImpl(DirectX12RenderPipeline* parent, const DirectX12RenderPass& renderPass) :
		base(parent), m_renderPass(renderPass)
	{
	}

private:
	void cleanup()
	{
		//if (m_parent->handle() != nullptr)
		//	::vkDestroyPipeline(m_parent->getDevice()->handle(), m_parent->handle(), nullptr);

		//m_parent->handle() = nullptr;
	}

public:
	ComPtr<ID3D12PipelineState> initialize()
	{
		// Request the device (must be initialized, otherwise the render pass instance is invalid).
		auto device = m_renderPass.getDevice()->handle();

		// Get the root signature.
		auto layout = dynamic_cast<const DirectX12RenderPipelineLayout*>(m_parent->getLayout());

		if (layout == nullptr)
			throw InvalidArgumentException("The layout of the render pipeline is not a valid DirectX12 render pipeline layout.");

		// Create a pipeline state description.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescription = {};
		pipelineStateDescription.pRootSignature = layout->handle().Get();
		//pipelineStateDescription.
		throw;

		// Create the pipeline state instance.
		ComPtr<ID3D12PipelineState> pipelineState;
		raiseIfFailed<RuntimeException>(device->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState)), "Unable to create render pipeline state.");

		return pipelineState;

		//auto pipelineLayout = dynamic_cast<const DirectX12RenderPipelineLayout*>(m_layout.get());

		//if (pipelineLayout == nullptr)
		//	throw std::invalid_argument("The pipeline layout is not a valid DirectX12 pipeline layout instance.");

		//LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline for layout {0}...", fmt::ptr(pipelineLayout));

		//// Get the device.
		//auto device = m_parent->getDevice();

		//// Request configuration interface.
		//auto rasterizer = pipelineLayout->getRasterizer();
		//auto inputAssembler = pipelineLayout->getInputAssembler();
		//auto views = pipelineLayout->getViewports();
		//auto program = pipelineLayout->getProgram();

		//if (rasterizer == nullptr)
		//	throw std::invalid_argument("The pipeline layout does not contain a rasterizer.");

		//if (program == nullptr)
		//	throw std::invalid_argument("The pipeline shader program must be initialized.");

		//if (inputAssembler == nullptr)
		//	throw std::invalid_argument("The input assembler must be initialized.");

		//// Setup rasterizer state.
		//VkPipelineRasterizationStateCreateInfo rasterizerState = {};
		//rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		//rasterizerState.depthClampEnable = VK_FALSE;
		//rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		//rasterizerState.polygonMode = getPolygonMode(rasterizer->getPolygonMode());
		//rasterizerState.lineWidth = rasterizer->getLineWidth();
		//rasterizerState.cullMode = getCullMode(rasterizer->getCullMode());
		//rasterizerState.frontFace = rasterizer->getCullOrder() == CullOrder::ClockWise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
		//rasterizerState.depthBiasEnable = rasterizer->getDepthBiasEnabled();
		//rasterizerState.depthBiasClamp = rasterizer->getDepthBiasClamp();
		//rasterizerState.depthBiasConstantFactor = rasterizer->getDepthBiasConstantFactor();
		//rasterizerState.depthBiasSlopeFactor = rasterizer->getDepthBiasSlopeFactor();

		//LITEFX_TRACE(VULKAN_LOG, "Rasterizer state: {{ PolygonMode: {0}, CullMode: {1}, CullOrder: {2}, LineWidth: {3} }}", rasterizer->getPolygonMode(), rasterizer->getCullMode(), rasterizer->getCullOrder(), rasterizer->getLineWidth());

		//if (rasterizerState.depthBiasEnable)
		//	LITEFX_TRACE(VULKAN_LOG, "\tRasterizer depth bias: {{ Clamp: {0}, ConstantFactor: {1}, SlopeFactor: {2} }}", rasterizer->getDepthBiasClamp(), rasterizer->getDepthBiasConstantFactor(), rasterizer->getDepthBiasSlopeFactor());
		//else
		//	LITEFX_TRACE(VULKAN_LOG, "\tRasterizer depth bias disabled.");

		//// Setup input assembler state.
		//VkPipelineVertexInputStateCreateInfo inputState = {};
		//inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		//VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		//inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

		//Array<VkVertexInputBindingDescription> vertexInputBindings;
		//Array<VkVertexInputAttributeDescription> vertexInputAttributes;

		//LITEFX_TRACE(VULKAN_LOG, "Input assembler state: {{ PrimitiveTopology: {0} }}", inputAssembler->getTopology());

		//// Set primitive topology.
		//inputAssembly.topology = getPrimitiveTopology(inputAssembler->getTopology());
		//inputAssembly.primitiveRestartEnable = VK_FALSE;

		//// Parse vertex input descriptors.
		//auto vertexLayouts = inputAssembler->getVertexBufferLayouts();

		//std::for_each(std::begin(vertexLayouts), std::end(vertexLayouts), [&, l = 0](const IVertexBufferLayout* layout) mutable {
		//	auto bufferAttributes = layout->getAttributes();
		//	auto bindingPoint = layout->getBinding();

		//	LITEFX_TRACE(VULKAN_LOG, "Defining vertex buffer layout {0}/{1} {{ Attributes: {2}, Size: {3} bytes, Binding: {4} }}...", ++l, vertexLayouts.size(), bufferAttributes.size(), layout->getElementSize(), bindingPoint);

		//	VkVertexInputBindingDescription binding = {};
		//	binding.binding = bindingPoint;
		//	binding.stride = static_cast<UInt32>(layout->getElementSize());
		//	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		//	Array<VkVertexInputAttributeDescription> currentAttributes(bufferAttributes.size());

		//	std::generate(std::begin(currentAttributes), std::end(currentAttributes), [&, i = 0]() mutable {
		//		auto attribute = bufferAttributes[i++];

		//		LITEFX_TRACE(VULKAN_LOG, "\tAttribute {0}/{1}: {{ Location: {2}, Offset: {3}, Format: {4} }}", i, bufferAttributes.size(), attribute->getLocation(), attribute->getOffset(), attribute->getFormat());

		//		VkVertexInputAttributeDescription descriptor{};
		//		descriptor.binding = bindingPoint;
		//		descriptor.location = attribute->getLocation();
		//		descriptor.offset = attribute->getOffset();
		//		descriptor.format = getFormat(attribute->getFormat());

		//		return descriptor;
		//	});

		//	vertexInputAttributes.insert(std::end(vertexInputAttributes), std::begin(currentAttributes), std::end(currentAttributes));
		//	vertexInputBindings.push_back(binding);
		//});

		//// Define vertex input state.
		//inputState.vertexBindingDescriptionCount = static_cast<UInt32>(vertexInputBindings.size());
		//inputState.pVertexBindingDescriptions = vertexInputBindings.data();
		//inputState.vertexAttributeDescriptionCount = static_cast<UInt32>(vertexInputAttributes.size());
		//inputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		//// Setup viewport state.
		//Array<VkViewport> viewports;
		//Array<VkRect2D> scissors;
		//VkPipelineViewportStateCreateInfo viewportState = {};
		//viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		//for (auto v(0); v < views.size(); ++v)
		//{
		//	auto view = views[v];

		//	if (view == nullptr)
		//		throw std::invalid_argument("The specified viewports must be initialized.");

		//	LITEFX_TRACE(VULKAN_LOG, "Viewport state {0}/{1}: {{ X: {2}, Y: {3}, Width: {4}, Height: {5}, Scissors: {6} }}", v + 1, views.size(),
		//		view->getRectangle().x(), view->getRectangle().y(), view->getRectangle().width(), view->getRectangle().height(), view->getScissors().size());

		//	VkViewport viewport = {};
		//	viewport.x = view->getRectangle().x();
		//	viewport.y = view->getRectangle().y();
		//	viewport.width = view->getRectangle().width();
		//	viewport.height = view->getRectangle().height();
		//	viewport.minDepth = 0.0f;
		//	viewport.maxDepth = 1.0f;

		//	for each (auto & stencil in view->getScissors())
		//	{
		//		VkRect2D scissor = {};
		//		scissor.offset = { static_cast<Int32>(stencil.x()), static_cast<Int32>(stencil.y()) };
		//		scissor.extent = { static_cast<UInt32>(stencil.width()), static_cast<UInt32>(stencil.height()) };
		//		scissors.push_back(scissor);
		//	}

		//	viewports.push_back(viewport);
		//}

		//viewportState.viewportCount = static_cast<UInt32>(viewports.size());
		//viewportState.pViewports = viewports.data();
		//viewportState.scissorCount = static_cast<UInt32>(scissors.size());
		//viewportState.pScissors = scissors.data();

		//// Setup multisampling state.
		//// TODO: Abstract me!
		//VkPipelineMultisampleStateCreateInfo multisampling = {};
		//multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		//multisampling.sampleShadingEnable = VK_FALSE;
		//multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		//multisampling.minSampleShading = 1.0f;
		//multisampling.pSampleMask = nullptr;
		//multisampling.alphaToCoverageEnable = VK_FALSE;
		//multisampling.alphaToOneEnable = VK_FALSE;

		//// Setup color blend state.
		//// TODO: Add blend parameters to render target.
		//auto targets = renderPass.getTargets();
		//auto colorAttachments = std::count_if(std::begin(targets), std::end(targets), [](const auto& target) { return target->getType() != RenderTargetType::Depth; });

		//Array<VkPipelineColorBlendAttachmentState> colorBlendAttachments(colorAttachments);
		//std::generate(std::begin(colorBlendAttachments), std::end(colorBlendAttachments), []() {
		//	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};

		//	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		//	colorBlendAttachment.blendEnable = VK_FALSE;

		//	return colorBlendAttachment;
		//});

		//VkPipelineColorBlendStateCreateInfo colorBlending = {};
		//colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		//colorBlending.logicOpEnable = VK_FALSE;
		//colorBlending.logicOp = VK_LOGIC_OP_COPY;
		//colorBlending.attachmentCount = static_cast<UInt32>(colorBlendAttachments.size());
		//colorBlending.pAttachments = colorBlendAttachments.data();
		//colorBlending.blendConstants[0] = 0.0f;
		//colorBlending.blendConstants[1] = 0.0f;
		//colorBlending.blendConstants[2] = 0.0f;
		//colorBlending.blendConstants[3] = 0.0f;

		//// Setup depth/stencil state.
		//VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		//depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		//depthStencilState.depthTestEnable = pipelineLayout->getDepthTest();
		//depthStencilState.depthBoundsTestEnable = VK_FALSE;
		//depthStencilState.stencilTestEnable = pipelineLayout->getStencilTest();
		//depthStencilState.depthWriteEnable = std::any_of(std::begin(targets), std::end(targets), [](const auto& t) { return t->getType() == RenderTargetType::Depth; });
		//depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

		//// Setup pipeline state.
		//VkGraphicsPipelineCreateInfo pipelineInfo = {};
		//pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		//pipelineInfo.pVertexInputState = &inputState;
		//pipelineInfo.pInputAssemblyState = &inputAssembly;
		//pipelineInfo.pViewportState = &viewportState;
		//pipelineInfo.pRasterizationState = &rasterizerState;
		//pipelineInfo.pMultisampleState = &multisampling;
		//pipelineInfo.pColorBlendState = &colorBlending;
		//pipelineInfo.pDepthStencilState = &depthStencilState;
		//pipelineInfo.layout = pipelineLayout->handle();
		//pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		//// Setup shader stages.
		//auto modules = program->getModules();
		//LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(program), modules.size());

		//Array<VkPipelineShaderStageCreateInfo> shaderStages(modules.size());

		//std::generate(std::begin(shaderStages), std::end(shaderStages), [&, i = 0]() mutable {
		//	auto module = dynamic_cast<const DirectX12ShaderModule*>(modules[i++]);

		//	if (module == nullptr)
		//		throw std::invalid_argument("The provided shader module is not a valid DirectX12 shader.");

		//	LITEFX_TRACE(VULKAN_LOG, "\tModule {0}/{1} (\"{2}\") state: {{ Type: {3}, EntryPoint: {4} }}", i, modules.size(), module->getFileName(), module->getType(), module->getEntryPoint());

		//	return module->getShaderStageDefinition();
		//});

		//pipelineInfo.stageCount = modules.size();
		//pipelineInfo.pStages = shaderStages.data();

		//// Setup render pass state.
		//pipelineInfo.renderPass = renderPass.handle();
		//pipelineInfo.subpass = 0;

		//VkPipeline pipeline;
		//auto result = ::vkCreateGraphicsPipelines(device->handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

		//if (result != VK_SUCCESS)
		//	throw std::runtime_error(fmt::format("Unable to create render pipeline: {0}", result));

		//return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipeline::DirectX12RenderPipeline(const DirectX12RenderPass& renderPass) :
	m_impl(makePimpl<DirectX12RenderPipelineImpl>(this, renderPass)), IComResource<ID3D12PipelineState>(nullptr)
{
}

DirectX12RenderPipeline::~DirectX12RenderPipeline() noexcept
{
	m_impl->cleanup();
}

const IRenderPipelineLayout* DirectX12RenderPipeline::getLayout() const noexcept
{
	return m_impl->m_layout.get();
}

IRenderPipelineLayout* DirectX12RenderPipeline::getLayout() noexcept
{
	return m_impl->m_layout.get();
}

void DirectX12RenderPipeline::setLayout(UniquePtr<IRenderPipelineLayout>&& layout)
{
	if (m_impl->m_layout != nullptr)
		throw RuntimeException("The pipeline layout for this pipeline is already defined and cannot be replaced. Create a new pipeline instead.");

	if (layout == nullptr)
		throw ArgumentNotInitializedException("The pipeline layout must be initialized.");

	m_impl->m_layout = std::move(layout);
	this->handle() = m_impl->initialize();
}

const IRenderPass& DirectX12RenderPipeline::renderPass() const noexcept 
{
	return m_impl->m_renderPass;
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

void DirectX12RenderPipelineBuilder::use(UniquePtr<IRenderPipelineLayout>&& layout)
{
	this->instance()->setLayout(std::move(layout));
}

DirectX12RenderPipelineLayoutBuilder DirectX12RenderPipelineBuilder::defineLayout()
{
	return this->make<DirectX12RenderPipelineLayout>();
}