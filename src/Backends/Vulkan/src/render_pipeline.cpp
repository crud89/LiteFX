#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipeline::VulkanRenderPipelineImpl : public Implement<VulkanRenderPipeline> {
public:
	friend class VulkanRenderPipelineBuilder;
	friend class VulkanRenderPipeline;

private:
	SharedPtr<VulkanPipelineLayout> m_layout;
	SharedPtr<VulkanShaderProgram> m_program;
	SharedPtr<VulkanInputAssembler> m_inputAssembler;
	SharedPtr<VulkanRasterizer> m_rasterizer;
	bool m_alphaToCoverage{ false };
	const VulkanRenderPass& m_renderPass;

public:
	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent, const VulkanRenderPass& renderPass, bool alphaToCoverage, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram, SharedPtr<VulkanInputAssembler> inputAssembler, SharedPtr<VulkanRasterizer> rasterizer) :
		base(parent), m_renderPass(renderPass), m_alphaToCoverage(alphaToCoverage), m_layout(layout), m_program(shaderProgram), m_inputAssembler(inputAssembler), m_rasterizer(rasterizer)
	{
	}

	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent, const VulkanRenderPass& renderPass) :
		base(parent), m_renderPass(renderPass)
	{
	}

public:
	VkPipeline initialize()
	{
		LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline \"{1}\" for layout {0}...", fmt::ptr(reinterpret_cast<void*>(m_layout.get())), m_parent->name());
		
		// Get the device.
		const auto& device = m_renderPass.device();

		// Setup rasterizer state.
		auto& rasterizer = std::as_const(*m_rasterizer.get());
		VkPipelineRasterizationStateCreateInfo rasterizerState = {};
		rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerState.depthClampEnable = VK_FALSE;
		rasterizerState.rasterizerDiscardEnable = VK_FALSE;
		rasterizerState.polygonMode = Vk::getPolygonMode(rasterizer.polygonMode());
		rasterizerState.lineWidth = rasterizer.lineWidth();
		rasterizerState.cullMode = Vk::getCullMode(rasterizer.cullMode());
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
		inputAssembly.topology = Vk::getPrimitiveTopology(m_inputAssembler->topology());
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Parse vertex input descriptors.
		auto vertexLayouts = m_inputAssembler->vertexBufferLayouts();

		std::ranges::for_each(vertexLayouts, [&, l = 0](const IVertexBufferLayout* layout) mutable {
			auto bufferAttributes = layout->attributes() | std::ranges::to<std::vector>();
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
					descriptor.format = Vk::getFormat(attribute->format());

					return descriptor;
				}) | std::ranges::to<Array<VkVertexInputAttributeDescription>>();

			vertexInputAttributes.insert(std::end(vertexInputAttributes), std::begin(currentAttributes), std::end(currentAttributes));
			vertexInputBindings.push_back(binding);
		});

		// Define vertex input state.
		inputState.vertexBindingDescriptionCount = static_cast<UInt32>(vertexInputBindings.size());
		inputState.pVertexBindingDescriptions = vertexInputBindings.data();
		inputState.vertexAttributeDescriptionCount = static_cast<UInt32>(vertexInputAttributes.size());
		inputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		// Setup viewport state (still required, even if all viewports and scissors are dynamic).
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		// Setup dynamic state.
		Array<VkDynamicState> dynamicStates { 
			VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT, 
			VkDynamicState::VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT, 
			VkDynamicState::VK_DYNAMIC_STATE_LINE_WIDTH, 
			VkDynamicState::VK_DYNAMIC_STATE_BLEND_CONSTANTS,
			VkDynamicState::VK_DYNAMIC_STATE_STENCIL_REFERENCE
		};
		
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStates.data();
		dynamicState.dynamicStateCount = static_cast<UInt32>(dynamicStates.size());

		// Setup multisampling state.
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = Vk::getSamples(m_renderPass.multiSamplingLevel());
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = m_alphaToCoverage;
		multisampling.alphaToOneEnable = VK_FALSE;

		// Setup color blend state.
		Array<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
		std::ranges::for_each(m_renderPass.renderTargets(), [&colorBlendAttachments](const RenderTarget& renderTarget) {
			if (renderTarget.type() == RenderTargetType::DepthStencil)
				return;

			colorBlendAttachments.push_back(VkPipelineColorBlendAttachmentState {
				.blendEnable = renderTarget.blendState().Enable,
				.srcColorBlendFactor = Vk::getBlendFactor(renderTarget.blendState().SourceColor),
				.dstColorBlendFactor = Vk::getBlendFactor(renderTarget.blendState().DestinationColor),
				.colorBlendOp = Vk::getBlendOperation(renderTarget.blendState().ColorOperation),
				.srcAlphaBlendFactor = Vk::getBlendFactor(renderTarget.blendState().SourceAlpha),
				.dstAlphaBlendFactor = Vk::getBlendFactor(renderTarget.blendState().DestinationAlpha),
				.alphaBlendOp = Vk::getBlendOperation(renderTarget.blendState().AlphaOperation),
				.colorWriteMask = static_cast<VkColorComponentFlags>(renderTarget.blendState().WriteMask)
			});
		});

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = static_cast<UInt32>(colorBlendAttachments.size());
		colorBlending.pAttachments = colorBlendAttachments.data();

		// Setup depth/stencil state.
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.depthTestEnable = rasterizer.depthStencilState().depthState().Enable;
		depthStencilState.depthWriteEnable = rasterizer.depthStencilState().depthState().Write;
		depthStencilState.depthCompareOp = Vk::getCompareOp(rasterizer.depthStencilState().depthState().Operation);
		depthStencilState.stencilTestEnable = rasterizer.depthStencilState().stencilState().Enable;
		depthStencilState.front.compareMask = rasterizer.depthStencilState().stencilState().ReadMask;
		depthStencilState.front.writeMask = rasterizer.depthStencilState().stencilState().WriteMask;
		depthStencilState.front.compareOp = Vk::getCompareOp(rasterizer.depthStencilState().stencilState().FrontFace.Operation);
		depthStencilState.front.failOp = Vk::getStencilOp(rasterizer.depthStencilState().stencilState().FrontFace.StencilFailOp);
		depthStencilState.front.passOp = Vk::getStencilOp(rasterizer.depthStencilState().stencilState().FrontFace.StencilPassOp);
		depthStencilState.front.depthFailOp = Vk::getStencilOp(rasterizer.depthStencilState().stencilState().FrontFace.DepthFailOp);
		depthStencilState.back.compareMask = rasterizer.depthStencilState().stencilState().ReadMask;
		depthStencilState.back.writeMask = rasterizer.depthStencilState().stencilState().WriteMask;
		depthStencilState.back.compareOp = Vk::getCompareOp(rasterizer.depthStencilState().stencilState().BackFace.Operation);
		depthStencilState.back.failOp = Vk::getStencilOp(rasterizer.depthStencilState().stencilState().BackFace.StencilFailOp);
		depthStencilState.back.passOp = Vk::getStencilOp(rasterizer.depthStencilState().stencilState().BackFace.StencilPassOp);
		depthStencilState.back.depthFailOp = Vk::getStencilOp(rasterizer.depthStencilState().stencilState().BackFace.DepthFailOp);

		// Setup shader stages.
		auto modules = m_program->modules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(reinterpret_cast<const void*>(m_program.get())), modules.size());

		Array<VkPipelineShaderStageCreateInfo> shaderStages = modules |
			std::views::transform([](const VulkanShaderModule* shaderModule) { return shaderModule->shaderStageDefinition(); }) |
			std::ranges::to<Array<VkPipelineShaderStageCreateInfo>>();

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
		pipelineInfo.renderPass = m_renderPass.handle();
		pipelineInfo.subpass = 0;

		VkPipeline pipeline;
		raiseIfFailed(::vkCreateGraphicsPipelines(m_renderPass.device().handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create render pipeline.");

#ifndef NDEBUG
		m_renderPass.device().setDebugName(*reinterpret_cast<const UInt64*>(&pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, m_parent->name());
#endif

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipeline::VulkanRenderPipeline(const VulkanRenderPass& renderPass, SharedPtr<VulkanShaderProgram> shaderProgram, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanInputAssembler> inputAssembler, SharedPtr<VulkanRasterizer> rasterizer, bool enableAlphaToCoverage, const String& name) :
	m_impl(makePimpl<VulkanRenderPipelineImpl>(this, renderPass, enableAlphaToCoverage, layout, shaderProgram, inputAssembler, rasterizer)), VulkanPipelineState(VK_NULL_HANDLE)
{
	this->handle() = m_impl->initialize();

	if (!name.empty())
		this->name() = name;
}

VulkanRenderPipeline::VulkanRenderPipeline(const VulkanRenderPass& renderPass, const String& name) noexcept :
	m_impl(makePimpl<VulkanRenderPipelineImpl>(this, renderPass)), VulkanPipelineState(VK_NULL_HANDLE)
{
	if (!name.empty())
		this->name() = name;
}

VulkanRenderPipeline::~VulkanRenderPipeline() noexcept
{
	::vkDestroyPipeline(m_impl->m_renderPass.device().handle(), this->handle(), nullptr);
}

SharedPtr<const VulkanShaderProgram> VulkanRenderPipeline::program() const noexcept
{
	return m_impl->m_program;
}

SharedPtr<const VulkanPipelineLayout> VulkanRenderPipeline::layout() const noexcept 
{
	return m_impl->m_layout;
}

SharedPtr<VulkanInputAssembler> VulkanRenderPipeline::inputAssembler() const noexcept 
{
	return m_impl->m_inputAssembler;
}

SharedPtr<VulkanRasterizer> VulkanRenderPipeline::rasterizer() const noexcept
{
	return m_impl->m_rasterizer;
}

bool VulkanRenderPipeline::alphaToCoverage() const noexcept
{
	return m_impl->m_alphaToCoverage;
}

void VulkanRenderPipeline::use(const VulkanCommandBuffer& commandBuffer) const noexcept
{
	::vkCmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->handle());

	// Set the line width (in case it has been changed). Currently we do not expose an command buffer interface for this, since this is mostly unsupported anyway and has no D3D12 counter-part.
	::vkCmdSetLineWidth(commandBuffer.handle(), std::as_const(*m_impl->m_rasterizer).lineWidth());
}

void VulkanRenderPipeline::bind(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet) const noexcept
{
	::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::as_const(*m_impl->m_layout).handle(), descriptorSet.layout().space(), 1, &descriptorSet.handle(), 0, nullptr);
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

constexpr VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilder(const VulkanRenderPass& renderPass, const String& name) :
	RenderPipelineBuilder(UniquePtr<VulkanRenderPipeline>(new VulkanRenderPipeline(renderPass)))
{
	this->instance()->name() = name;
}

constexpr VulkanRenderPipelineBuilder::~VulkanRenderPipelineBuilder() noexcept = default;

void VulkanRenderPipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = m_state.pipelineLayout;
	instance->m_impl->m_program = m_state.shaderProgram;
	instance->m_impl->m_inputAssembler = m_state.inputAssembler;
	instance->m_impl->m_rasterizer = m_state.rasterizer;
	instance->m_impl->m_alphaToCoverage = m_state.enableAlphaToCoverage;
	instance->handle() = instance->m_impl->initialize();
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)