#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipeline::VulkanRenderPipelineImpl : public Implement<VulkanRenderPipeline> {
public:
	friend class VulkanRenderPipelineBuilder;
	friend class VulkanRenderPipeline;

private:
	UniquePtr<VulkanRenderPipelineLayout> m_layout;
	SharedPtr<VulkanInputAssembler> m_inputAssembler;
	SharedPtr<VulkanRasterizer> m_rasterizer;
	Array<SharedPtr<IViewport>> m_viewports;
	Array<SharedPtr<IScissor>> m_scissors;
	UInt32 m_id;
	String m_name;
	Vector4f m_blendFactors;

public:
	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent, const UInt32& id, const String& name, UniquePtr<VulkanRenderPipelineLayout>&& layout, SharedPtr<VulkanInputAssembler>&& inputAssembler, SharedPtr<VulkanRasterizer>&& rasterizer, Array<SharedPtr<IViewport>>&& viewports, Array<SharedPtr<IScissor>>&& scissors) :
		base(parent), m_id(id), m_name(name), m_layout(std::move(layout)), m_inputAssembler(std::move(inputAssembler)), m_rasterizer(std::move(rasterizer)), m_viewports(std::move(viewports)), m_scissors(std::move(scissors))
	{
	}

	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent) :
		base(parent)
	{
	}

public:
	VkPipeline initialize()
	{
		LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline {1} (\"{2}\") for layout {0}...", fmt::ptr(reinterpret_cast<void*>(m_layout.get())), m_id, m_name);
		
		// Get the device.
		auto device = m_parent->getDevice();

		// Setup rasterizer state.
		auto& rasterizer = std::as_const(*m_rasterizer.get());
		VkPipelineRasterizationStateCreateInfo rasterizerState = {};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = ::getPolygonMode(rasterizer.polygonMode());
		rasterizerState.lineWidth = rasterizer.lineWidth();
		rasterizerState.cullMode = ::getCullMode(rasterizer.cullMode());
		rasterizerState.frontFace = rasterizer.cullOrder() == CullOrder::ClockWise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizerState.depthBiasEnable = rasterizer.depthStencilState().depthBias().Enable;
		rasterizerState.depthBiasClamp = rasterizer.depthStencilState().depthBias().Clamp;
		rasterizerState.depthBiasConstantFactor = rasterizer.depthStencilState().depthBias().ConstantFactor;
		rasterizerState.depthBiasSlopeFactor = rasterizer.depthStencilState().depthBias().SlopeFactor;

		LITEFX_TRACE(VULKAN_LOG, "Rasterizer state: {{ PolygonMode: {0}, CullMode: {1}, CullOrder: {2}, LineWidth: {3} }}", rasterizer.polygonMode(), rasterizer.cullMode(), rasterizer.cullOrder(), rasterizer.lineWidth());
		
		if (rasterizer.depthStencilState().depthBias().Enable)
			LITEFX_TRACE(VULKAN_LOG, "\tRasterizer depth bias: {{ Clamp: {0}, ConstantFactor: {1}, SlopeFactor: {2} }}", rasterizer.depthStencilState().depthBias().Clamp, rasterizer.depthStencilState().depthBias().ConstantFactor, rasterizer.depthStencilState().depthBias().SlopeFactor);
		else
			LITEFX_TRACE(VULKAN_LOG, "\tRasterizer depth bias disabled.");

		// Setup input assembler state.
		VkPipelineVertexInputStateCreateInfo inputState = {};
		inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

		Array<VkVertexInputBindingDescription> vertexInputBindings;
		Array<VkVertexInputAttributeDescription> vertexInputAttributes;

		LITEFX_TRACE(VULKAN_LOG, "Input assembler state: {{ PrimitiveTopology: {0} }}", m_inputAssembler->topology());

		// Set primitive topology.
		inputAssembly.topology = ::getPrimitiveTopology(m_inputAssembler->topology());
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Parse vertex input descriptors.
		auto vertexLayouts = m_inputAssembler->vertexBufferLayouts();

		std::ranges::for_each(vertexLayouts, [&, l = 0](const IVertexBufferLayout* layout) mutable {
			auto bufferAttributes = layout->attributes();
			auto bindingPoint = layout->binding();

			LITEFX_TRACE(VULKAN_LOG, "Defining vertex buffer layout {0}/{1} {{ Attributes: {2}, Size: {3} bytes, Binding: {4} }}...", ++l, vertexLayouts.size(), bufferAttributes.size(), layout->elementSize(), bindingPoint);

			VkVertexInputBindingDescription binding = {};
			binding.binding = bindingPoint;
			binding.stride = static_cast<UInt32>(layout->elementSize());
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			Array<VkVertexInputAttributeDescription> currentAttributes = bufferAttributes |
				std::views::transform([&bufferAttributes, &bindingPoint, i = 0](const BufferAttribute* attribute) mutable {
					LITEFX_TRACE(VULKAN_LOG, "\tAttribute {0}/{1}: {{ Location: {2}, Offset: {3}, Format: {4} }}", ++i, bufferAttributes.size(), attribute->location(), attribute->offset(), attribute->format());

					VkVertexInputAttributeDescription descriptor{};
					descriptor.binding = bindingPoint;
					descriptor.location = attribute->location();
					descriptor.offset = attribute->offset();
					descriptor.format = ::getFormat(attribute->format());

					return descriptor;
				}) |
				ranges::to<Array<VkVertexInputAttributeDescription>>();

			vertexInputAttributes.insert(std::end(vertexInputAttributes), std::begin(currentAttributes), std::end(currentAttributes));
			vertexInputBindings.push_back(binding);
		});

		// Define vertex input state.
		inputState.vertexBindingDescriptionCount = static_cast<UInt32>(vertexInputBindings.size());
		inputState.pVertexBindingDescriptions = vertexInputBindings.data();
		inputState.vertexAttributeDescriptionCount = static_cast<UInt32>(vertexInputAttributes.size());
		inputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		// Setup viewport state.
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = static_cast<UInt32>(m_viewports.size());
		viewportState.scissorCount = static_cast<UInt32>(m_scissors.size());

		// Setup dynamic state.
		Array<VkDynamicState> dynamicStates { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR, VkDynamicState::VK_DYNAMIC_STATE_LINE_WIDTH, VkDynamicState::VK_DYNAMIC_STATE_BLEND_CONSTANTS };
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStates.data();
		dynamicState.dynamicStateCount = static_cast<UInt32>(dynamicStates.size());

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
		Array<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
		std::ranges::for_each(m_parent->parent().renderTargets(), [&colorBlendAttachments](const RenderTarget& renderTarget) {
			if (renderTarget.type() == RenderTargetType::DepthStencil)
				return;

			colorBlendAttachments.push_back(VkPipelineColorBlendAttachmentState {
				.blendEnable = renderTarget.blendState().Enable,
				.srcColorBlendFactor = ::getBlendFactor(renderTarget.blendState().SourceColor),
				.dstColorBlendFactor = ::getBlendFactor(renderTarget.blendState().DestinationColor),
				.colorBlendOp = ::getBlendOperation(renderTarget.blendState().ColorOperation),
				.srcAlphaBlendFactor = ::getBlendFactor(renderTarget.blendState().SourceAlpha),
				.dstAlphaBlendFactor = ::getBlendFactor(renderTarget.blendState().DestinationAlpha),
				.alphaBlendOp = ::getBlendOperation(renderTarget.blendState().AlphaOperation),
				.colorWriteMask = static_cast<VkColorComponentFlags>(renderTarget.blendState().WriteMask)
			});
		});

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = static_cast<UInt32>(colorBlendAttachments.size());
		colorBlending.pAttachments = colorBlendAttachments.data();
		colorBlending.blendConstants[0] = m_blendFactors.x();
		colorBlending.blendConstants[1] = m_blendFactors.y();
		colorBlending.blendConstants[2] = m_blendFactors.z();
		colorBlending.blendConstants[3] = m_blendFactors.w();

		// Setup depth/stencil state.
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.depthTestEnable = rasterizer.depthStencilState().depthState().Enable;
		depthStencilState.depthWriteEnable = rasterizer.depthStencilState().depthState().Write;
		depthStencilState.depthCompareOp = ::getCompareOp(rasterizer.depthStencilState().depthState().Operation);
		depthStencilState.stencilTestEnable = rasterizer.depthStencilState().stencilState().Enable;
		depthStencilState.front.compareMask = rasterizer.depthStencilState().stencilState().ReadMask;
		depthStencilState.front.writeMask = rasterizer.depthStencilState().stencilState().WriteMask;
		depthStencilState.front.compareOp = ::getCompareOp(rasterizer.depthStencilState().stencilState().FrontFace.Operation);
		depthStencilState.front.failOp = ::getStencilOp(rasterizer.depthStencilState().stencilState().FrontFace.StencilFailOp);
		depthStencilState.front.passOp = ::getStencilOp(rasterizer.depthStencilState().stencilState().FrontFace.StencilPassOp);
		depthStencilState.front.depthFailOp = ::getStencilOp(rasterizer.depthStencilState().stencilState().FrontFace.DepthFailOp);
		depthStencilState.back.compareMask = rasterizer.depthStencilState().stencilState().ReadMask;
		depthStencilState.back.writeMask = rasterizer.depthStencilState().stencilState().WriteMask;
		depthStencilState.back.compareOp = ::getCompareOp(rasterizer.depthStencilState().stencilState().BackFace.Operation);
		depthStencilState.back.failOp = ::getStencilOp(rasterizer.depthStencilState().stencilState().BackFace.StencilFailOp);
		depthStencilState.back.passOp = ::getStencilOp(rasterizer.depthStencilState().stencilState().BackFace.StencilPassOp);
		depthStencilState.back.depthFailOp = ::getStencilOp(rasterizer.depthStencilState().stencilState().BackFace.DepthFailOp);

		// Setup shader stages.
		auto modules = m_layout->program().modules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(reinterpret_cast<const void*>(&m_layout->program())), modules.size());

		Array<VkPipelineShaderStageCreateInfo> shaderStages = modules |
			std::views::transform([](const VulkanShaderModule* shaderModule) { return shaderModule->shaderStageDefinition(); }) |
			ranges::to<Array<VkPipelineShaderStageCreateInfo>>();

		// Setup pipeline state.
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pVertexInputState = &inputState;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizerState;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = std::as_const(*m_layout.get()).handle();
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.stageCount = modules.size();
		pipelineInfo.pStages = shaderStages.data();

		// Setup render pass state.
		pipelineInfo.renderPass = m_parent->parent().handle();
		pipelineInfo.subpass = 0;

		VkPipeline pipeline;
		raiseIfFailed<RuntimeException>(::vkCreateGraphicsPipelines(device->handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create render pipeline.");

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipeline::VulkanRenderPipeline(const VulkanRenderPass& renderPass, const UInt32& id, UniquePtr<VulkanRenderPipelineLayout>&& layout, SharedPtr<VulkanInputAssembler>&& inputAssembler, SharedPtr<VulkanRasterizer>&& rasterizer, Array<SharedPtr<IViewport>>&& viewports, Array<SharedPtr<IScissor>>&& scissors, const String& name) :
	m_impl(makePimpl<VulkanRenderPipelineImpl>(this, id, name, std::move(layout), std::move(inputAssembler), std::move(rasterizer), std::move(viewports), std::move(scissors))), VulkanRuntimeObject<VulkanRenderPass>(renderPass, renderPass.getDevice()), Resource<VkPipeline>(VK_NULL_HANDLE)
{
	this->handle() = m_impl->initialize();
}

VulkanRenderPipeline::VulkanRenderPipeline(const VulkanRenderPass& renderPass) noexcept : 
	m_impl(makePimpl<VulkanRenderPipelineImpl>(this)), VulkanRuntimeObject<VulkanRenderPass>(renderPass, renderPass.getDevice()), Resource<VkPipeline>(VK_NULL_HANDLE)
{
}

VulkanRenderPipeline::~VulkanRenderPipeline() noexcept
{
	::vkDestroyPipeline(this->getDevice()->handle(), this->handle(), nullptr);
}

const String& VulkanRenderPipeline::name() const noexcept
{
	return m_impl->m_name;
}

const UInt32& VulkanRenderPipeline::id() const noexcept
{
	return m_impl->m_id;
}

const VulkanRenderPipelineLayout& VulkanRenderPipeline::layout() const noexcept 
{
	return *m_impl->m_layout;
}

SharedPtr<VulkanInputAssembler> VulkanRenderPipeline::inputAssembler() const noexcept 
{
	return m_impl->m_inputAssembler;
}

SharedPtr<IRasterizer> VulkanRenderPipeline::rasterizer() const noexcept 
{
	return m_impl->m_rasterizer;
}

Array<const IViewport*> VulkanRenderPipeline::viewports() const noexcept 
{
	return m_impl->m_viewports |
		std::views::transform([](const SharedPtr<IViewport>& viewport) { return viewport.get(); }) |
		ranges::to<Array<const IViewport*>>();
}

Array<const IScissor*> VulkanRenderPipeline::scissors() const noexcept 
{
	return m_impl->m_scissors |
		std::views::transform([](const SharedPtr<IScissor>& scissor) { return scissor.get(); }) |
		ranges::to<Array<const IScissor*>>();
}

Vector4f& VulkanRenderPipeline::blendFactors() const noexcept
{
	return m_impl->m_blendFactors;
}

void VulkanRenderPipeline::bind(const IVulkanVertexBuffer& buffer) const 
{
	constexpr VkDeviceSize offsets[] = { 0 };
	::vkCmdBindVertexBuffers(this->parent().activeFrameBuffer().commandBuffer().handle(), buffer.binding(), 1, &buffer.handle(), offsets);
}

void VulkanRenderPipeline::bind(const IVulkanIndexBuffer& buffer) const 
{
	::vkCmdBindIndexBuffer(this->parent().activeFrameBuffer().commandBuffer().handle(), buffer.handle(), 0, buffer.layout().indexType() == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void VulkanRenderPipeline::bind(const VulkanDescriptorSet& descriptorSet) const
{
	::vkCmdBindDescriptorSets(this->parent().activeFrameBuffer().commandBuffer().handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::as_const(*m_impl->m_layout).handle(), descriptorSet.parent().space(), 1, &descriptorSet.handle(), 0, nullptr);
}

void VulkanRenderPipeline::use() const 
{
	auto viewports = m_impl->m_viewports |
		std::views::transform([](const auto& viewport) { return VkViewport{.x = viewport->getRectangle().x(), .y = viewport->getRectangle().y(), .width = viewport->getRectangle().width(), .height = viewport->getRectangle().height(), .minDepth = viewport->getMinDepth(), .maxDepth = viewport->getMaxDepth()}; }) |
		ranges::to<Array<VkViewport>>();
	
	auto scissors = m_impl->m_scissors |
		std::views::transform([](const auto& scissor) { return VkRect2D{VkOffset2D{.x = static_cast<Int32>(scissor->getRectangle().x()), .y = static_cast<Int32>(scissor->getRectangle().y())}, VkExtent2D{.width = static_cast<UInt32>(scissor->getRectangle().width()), .height = static_cast<UInt32>(scissor->getRectangle().height())} };}) |
		ranges::to<Array<VkRect2D>>();

	Float blendFactor[4] = { m_impl->m_blendFactors.x(), m_impl->m_blendFactors.y(), m_impl->m_blendFactors.z(), m_impl->m_blendFactors.w() };

	// Bind the pipeline and setup the dynamic state.
	auto commandBuffer = this->parent().activeFrameBuffer().commandBuffer().handle();
	::vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->handle());
	::vkCmdSetViewport(commandBuffer, 0, static_cast<UInt32>(viewports.size()), viewports.data());
	::vkCmdSetScissor(commandBuffer, 0, static_cast<UInt32>(scissors.size()), scissors.data());
	::vkCmdSetLineWidth(commandBuffer, std::as_const(*m_impl->m_rasterizer).lineWidth());
	::vkCmdSetBlendConstants(commandBuffer, blendFactor);
}

void VulkanRenderPipeline::draw(const UInt32& vertices, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const
{
	::vkCmdDraw(this->parent().activeFrameBuffer().commandBuffer().handle(), vertices, instances, firstVertex, firstInstance);
}

void VulkanRenderPipeline::drawIndexed(const UInt32& indices, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const
{
	::vkCmdDrawIndexed(this->parent().activeFrameBuffer().commandBuffer().handle(), indices, instances, firstIndex, vertexOffset, firstInstance);
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilderImpl : public Implement<VulkanRenderPipelineBuilder> {
public:
	friend class VulkanRenderPipelineBuilderBuilder;
	friend class VulkanRenderPipelineBuilder;

private:
	UniquePtr<VulkanRenderPipelineLayout> m_layout;
	SharedPtr<VulkanInputAssembler> m_inputAssembler;
	SharedPtr<VulkanRasterizer> m_rasterizer;
	Array<SharedPtr<IViewport>> m_viewports;
	Array<SharedPtr<IScissor>> m_scissors;

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
	instance->handle() = instance->m_impl->initialize();

	return RenderPipelineBuilder::go();
}

void VulkanRenderPipelineBuilder::use(UniquePtr<VulkanRenderPipelineLayout>&& layout)
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