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
	MultiSamplingLevel m_samples{ MultiSamplingLevel::x1 };
	const VulkanRenderPass& m_renderPass;
	UniquePtr<IVulkanSampler> m_inputAttachmentSampler;
	Dictionary<const IFrameBuffer*, Array<UniquePtr<VulkanDescriptorSet>>> m_inputAttachmentBindings;
	Dictionary<const IFrameBuffer*, size_t> m_frameBufferResizeTokens, m_frameBufferReleaseTokens;
	mutable std::mutex m_usageMutex;


public:
	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent, const VulkanRenderPass& renderPass, bool alphaToCoverage, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram, SharedPtr<VulkanInputAssembler> inputAssembler, SharedPtr<VulkanRasterizer> rasterizer) :
		base(parent), m_layout(layout), m_program(shaderProgram), m_inputAssembler(inputAssembler), m_rasterizer(rasterizer), m_alphaToCoverage(alphaToCoverage), m_renderPass(renderPass)
	{
		if (renderPass.inputAttachmentSamplerBinding().has_value())
			m_inputAttachmentSampler = m_renderPass.device().factory().createSampler();
	}

	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent, const VulkanRenderPass& renderPass) :
		base(parent), m_renderPass(renderPass)
	{
		if (renderPass.inputAttachmentSamplerBinding().has_value())
			m_inputAttachmentSampler = m_renderPass.device().factory().createSampler();
	}

	~VulkanRenderPipelineImpl()
	{
		// Stop listening to frame buffer events.
		for (auto [frameBuffer, token] : m_frameBufferResizeTokens)
			frameBuffer->resized -= token;

		for (auto [frameBuffer, token] : m_frameBufferReleaseTokens)
			frameBuffer->released -= token;
	}

public:
	VkPipeline initialize(MultiSamplingLevel samples)
	{
		// Get the shader modules.
		auto modules = m_program->modules();

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

		// Setup shader stages.
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", reinterpret_cast<void*>(m_program.get()), modules.size());

		Array<VkPipelineShaderStageCreateInfo> shaderStages = modules |
			std::views::transform([](const VulkanShaderModule* shaderModule) { return shaderModule->shaderStageDefinition(); }) |
			std::ranges::to<Array<VkPipelineShaderStageCreateInfo>>();

		// Setup the pipeline.
		m_samples = samples;
		auto pipeline = this->initializeGraphicsPipeline(dynamicState, shaderStages);

#ifndef NDEBUG
		m_renderPass.device().setDebugName(*reinterpret_cast<const UInt64*>(&pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, m_parent->name());
#endif

		// Return the pipeline instance.
		return pipeline;

	}

	VkPipeline initializeGraphicsPipeline(const VkPipelineDynamicStateCreateInfo& dynamicState, const LiteFX::Array<VkPipelineShaderStageCreateInfo>& shaderStages)
	{
		LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline \"{1}\" for layout {0}...", reinterpret_cast<void*>(m_layout.get()), m_parent->name());

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
		VkPipelineVertexInputStateCreateInfo inputState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
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

#ifdef NDEBUG
			(void)l; // Required as [[maybe_unused]] is not supported in captures.
#else
			LITEFX_TRACE(VULKAN_LOG, "Defining vertex buffer layout {0}/{1} {{ Attributes: {2}, Size: {3} bytes, Binding: {4} }}...", ++l, vertexLayouts.size(), bufferAttributes.size(), layout->elementSize(), bindingPoint);
#endif

			VkVertexInputBindingDescription binding = {};
			binding.binding = bindingPoint;
			binding.stride = static_cast<UInt32>(layout->elementSize());
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			Array<VkVertexInputAttributeDescription> currentAttributes = bufferAttributes |
				std::views::transform([&bindingPoint, attributes = bufferAttributes.size(), i = 0] (const BufferAttribute* attribute) mutable {
#ifdef NDEBUG
					(void)attributes; // Required as [[maybe_unused]] is not supported in captures.
					(void)i;
#else
					LITEFX_TRACE(VULKAN_LOG, "\tAttribute {0}/{1}: {{ Location: {2}, Offset: {3}, Format: {4} }}", ++i, attributes, attribute->location(), attribute->offset(), attribute->format());
#endif

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
		VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };

		// Setup multisampling state.
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = Vk::getSamples(m_samples);
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

		// Setup rendering info for dynamic render pass.
		auto renderTargets = m_renderPass.renderTargets();
		auto formats = renderTargets |
			std::views::filter([](auto& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; }) |
			std::views::transform([](auto& renderTarget) { return Vk::getFormat(renderTarget.format()); }) |
			std::ranges::to<Array<VkFormat>>();
		auto depthStencilFormats = renderTargets |
			std::views::filter([](auto& renderTarget) { return renderTarget.type() == RenderTargetType::DepthStencil; }) |
			std::views::transform([](auto& renderTarget) { return renderTarget.format(); }) |
			std::ranges::to<Array<Format>>();
		auto depthFormat = depthStencilFormats.size() > 0 && ::hasDepth(depthStencilFormats.front()) ? Vk::getFormat(depthStencilFormats.front()) : VK_FORMAT_UNDEFINED;
		auto stencilFormat = depthStencilFormats.size() > 0 && ::hasStencil(depthStencilFormats.front()) ? Vk::getFormat(depthStencilFormats.front()) : VK_FORMAT_UNDEFINED;

		VkPipelineRenderingCreateInfo renderingInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = static_cast<UInt32>(formats.size()),
			.pColorAttachmentFormats = formats.data(),
			.depthAttachmentFormat = depthFormat,
			.stencilAttachmentFormat = stencilFormat
		};

		// Setup pipeline state.
		VkGraphicsPipelineCreateInfo pipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &renderingInfo,
			.stageCount = static_cast<UInt32>(shaderStages.size()),
			.pStages = shaderStages.data(),
			.pVertexInputState = &inputState,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizerState,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depthStencilState,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = std::as_const(*m_layout.get()).handle(),
			.subpass = 0
		};

		VkPipeline pipeline;
		raiseIfFailed(::vkCreateGraphicsPipelines(m_renderPass.device().handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create render pipeline.");

		return pipeline;
	}

	void initializeInputAttachmentBindings(const VulkanFrameBuffer& frameBuffer)
	{
		// Find out how many descriptor sets there are within the input attachments and which descriptors are bound.
		Dictionary<UInt32, Array<UInt32>> descriptorsPerSet;
		std::ranges::for_each(m_renderPass.inputAttachments(), [&descriptorsPerSet](auto& dependency) { descriptorsPerSet[dependency.binding().Space].push_back(dependency.binding().Register); });

		// Validate the descriptor sets, so that no descriptors are bound twice all descriptor sets are fully bound.
		for (auto& [set, descriptors] : descriptorsPerSet)
		{
			// Sort and check if there are duplicates.
			std::ranges::sort(descriptors);

			if (std::ranges::adjacent_find(descriptors) != descriptors.end()) [[unlikely]]
				throw RuntimeException("The descriptor set {0} has input attachment mappings that point to the same descriptor.", set);

			// Check if all descriptors in the set are mapped.
			auto& layout = m_layout->descriptorSet(set);

			for (auto& descriptor : layout.descriptors())
			{
				if (std::ranges::find(descriptors, descriptor->binding()) == descriptors.end()) [[unlikely]]
				{
					LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} is not fully mapped by the provided input attachments for the render pass.", set);
					break;
				}
			}
		}

		// Don't forget the sampler.
		if (m_renderPass.inputAttachmentSamplerBinding().has_value())
		{
			auto& samplerBinding = m_renderPass.inputAttachmentSamplerBinding().value();
			auto layouts = m_layout->descriptorSets();

			if (auto samplerSet = std::ranges::find_if(layouts, [&](auto set) { return set->space() == samplerBinding.Space; }); samplerSet != layouts.end())
			{
				if (descriptorsPerSet.contains(samplerBinding.Space)) [[unlikely]]
					throw RuntimeException("The input attachment sampler is defined in a descriptor set that contains input attachment descriptors. Samplers must be defined within their own space.");

				// Store the descriptor so it gets bound.
				descriptorsPerSet[samplerBinding.Space].push_back(samplerBinding.Register);
			}
		}

		// Allocate the input attachment bindings.
		this->allocateInputAttachmentBindings(descriptorsPerSet | std::views::keys, frameBuffer);
	}

	void allocateInputAttachmentBindings(const std::ranges::input_range auto& descriptorSets, const VulkanFrameBuffer& frameBuffer) requires 
		std::is_convertible_v<std::ranges::range_value_t<decltype(descriptorSets)>, UInt32>
	{
		// Allocate the bindings array.
		auto interfacePointer = static_cast<const IFrameBuffer*>(&frameBuffer);
		auto& bindings = m_inputAttachmentBindings[interfacePointer];

		// Initialize the descriptor set bindings.
		bindings.append_range(descriptorSets | std::views::transform([this](UInt32 set) { return std::move(m_layout->descriptorSet(set).allocate()); }));

		// Listen to frame buffer events and update the bindings or remove the sets (on release).
		m_frameBufferResizeTokens[interfacePointer] = frameBuffer.resized.add(std::bind(&VulkanRenderPipelineImpl::onFrameBufferResize, this, std::placeholders::_1, std::placeholders::_2));
		m_frameBufferReleaseTokens[interfacePointer] = frameBuffer.released.add(std::bind(&VulkanRenderPipelineImpl::onFrameBufferRelease, this, std::placeholders::_1, std::placeholders::_2));
	}

	void updateInputAttachmentBindings(const VulkanFrameBuffer& frameBuffer)
	{
		// Get the interface pointer and obtain the descriptor sets for the input attachments.
		auto interfacePointer = static_cast<const IFrameBuffer*>(&frameBuffer);
		auto& bindings = m_inputAttachmentBindings.at(interfacePointer);

		// Iterate the dependencies and update the binding for each one.
		std::ranges::for_each(m_renderPass.inputAttachments(), [&](auto& dependency) {
			for (auto& binding : bindings)
			{
				if (binding->layout().space() == dependency.binding().Space)
				{
					// Resolve the image and update the binding.
					auto& image = frameBuffer[dependency.renderTarget()];

					if (image.samples() != m_samples) [[unlikely]]
						LITEFX_WARNING(VULKAN_LOG, "The image multi sampling level {0} does not match the render pipeline multi sampling state {1}.", image.samples(), m_samples);

					// Attach the image from the right frame buffer to the descriptor set.
					binding->update(dependency.binding().Register, image);
					break;
				}
			}
		});

		// If there's a sampler, bind it too.
		if (m_renderPass.inputAttachmentSamplerBinding().has_value())
		{
			for (auto& binding : bindings)
			{
				if (binding->layout().space() == m_renderPass.inputAttachmentSamplerBinding().value().Space)
				{
					binding->update(m_renderPass.inputAttachmentSamplerBinding().value().Register, *m_inputAttachmentSampler);
					break;
				}
			}
		}
	}

	void bindInputAttachments(const VulkanCommandBuffer& commandBuffer)
	{
		// If this is the first time, the current frame buffer is bound to the render pass, we need to allocate descriptors for it.
		auto& frameBuffer = m_renderPass.activeFrameBuffer();
		auto interfacePointer = static_cast<const IFrameBuffer*>(&frameBuffer);

		if (!m_inputAttachmentBindings.contains(interfacePointer))
		{
			// Allocate and update input attachment bindings.
			this->initializeInputAttachmentBindings(frameBuffer);
			this->updateInputAttachmentBindings(frameBuffer);
		}

		// Bind the input attachment sets.
		//commandBuffer.bind(m_inputAttachmentBindings.at(interfacePointer) | std::views::transform([](auto& set) { return set.get(); }));
		auto descriptorSets = m_inputAttachmentBindings.at(interfacePointer) | std::views::transform([](auto& set) { return set.get(); }) | std::ranges::to<Array<const VulkanDescriptorSet*>>();
		m_parent->bind(commandBuffer, descriptorSets);
	}

	void onFrameBufferResize(const void* sender, IFrameBuffer::ResizeEventArgs args)
	{
		// Update the descriptors in the descriptor sets.
		// NOTE: No slicing here, as the event is always triggered by the frame buffer instance.
		auto frameBuffer = reinterpret_cast<const VulkanFrameBuffer*>(sender);
		this->updateInputAttachmentBindings(*frameBuffer);
	}

	void onFrameBufferRelease(const void* sender, IFrameBuffer::ReleasedEventArgs args)
	{
		// Get the frame buffer pointer.
		auto interfacePointer = reinterpret_cast<const IFrameBuffer*>(sender);

		// Release the descriptor sets.
		m_inputAttachmentBindings.erase(interfacePointer);

		// Release the tokens.
		m_frameBufferReleaseTokens.erase(interfacePointer);
		m_frameBufferResizeTokens.erase(interfacePointer);
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipeline::VulkanRenderPipeline(const VulkanRenderPass& renderPass, SharedPtr<VulkanShaderProgram> shaderProgram, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanInputAssembler> inputAssembler, SharedPtr<VulkanRasterizer> rasterizer, MultiSamplingLevel samples, bool enableAlphaToCoverage, const String& name) :
	VulkanPipelineState(VK_NULL_HANDLE), m_impl(makePimpl<VulkanRenderPipelineImpl>(this, renderPass, enableAlphaToCoverage, layout, shaderProgram, inputAssembler, rasterizer))
{
	this->handle() = m_impl->initialize(samples);

	if (!name.empty())
		this->name() = name;
}

VulkanRenderPipeline::VulkanRenderPipeline(const VulkanRenderPass& renderPass, const String& name) noexcept :
	VulkanPipelineState(VK_NULL_HANDLE), m_impl(makePimpl<VulkanRenderPipelineImpl>(this, renderPass))
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

MultiSamplingLevel VulkanRenderPipeline::samples() const noexcept
{
	return m_impl->m_samples;
}

void VulkanRenderPipeline::updateSamples(MultiSamplingLevel samples)
{
	// Release all frame buffer bindings.
	m_impl->m_inputAttachmentBindings.clear();

	// Release current pipeline state.
	::vkDestroyPipeline(m_impl->m_renderPass.device().handle(), this->handle(), nullptr);

	// Rebuild the pipeline.
	this->handle() = m_impl->initialize(samples);
}

void VulkanRenderPipeline::use(const VulkanCommandBuffer& commandBuffer) const noexcept
{
	::vkCmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->handle());

	// Set the line width (in case it has been changed). Currently we do not expose an command buffer interface for this, since this is mostly unsupported anyway and has no D3D12 counter-part.
	::vkCmdSetLineWidth(commandBuffer.handle(), std::as_const(*m_impl->m_rasterizer).lineWidth());

	// NOTE: The same pipeline can be used from multiple multi-threaded command buffers, in which case we need to prevent multiple threads 
	//       from attempting to initialize the bindings on first use.
	std::lock_guard<std::mutex> lock(m_impl->m_usageMutex);

	// Bind the input attachments and the input attachment sampler.
	m_impl->bindInputAttachments(commandBuffer);
}

void VulkanRenderPipeline::bind(const VulkanCommandBuffer& commandBuffer, Span<const VulkanDescriptorSet*> descriptorSets) const noexcept
{
	// Filter out uninitialized sets.
	auto sets = descriptorSets | std::views::filter([](auto set) { return set != nullptr; }) | std::ranges::to<Array<const VulkanDescriptorSet*>>();

	if (sets.empty()) [[unlikely]]
		return; // Nothing to do on empty sets.
	else if (sets.size() == 1)
		::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::as_const(*m_impl->m_layout).handle(), sets.front()->layout().space(), 1, &sets.front()->handle(), 0, nullptr);
	else
	{
		// Sort the descriptor sets by space, as we might be able to pass the sets more efficiently if they are sorted and continuous.
		std::ranges::sort(sets, [](auto lhs, auto rhs) { return lhs->layout().space() > rhs->layout().space(); });

		// In a sorted range, last - (first - 1) equals the size of the range only if there are no duplicates and no gaps.
		auto startSpace = sets.back()->layout().space();

		if (startSpace - (sets.front()->layout().space() - 1) != static_cast<UInt32>(sets.size()))
			std::ranges::for_each(sets, [&](auto set) { ::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::as_const(*m_impl->m_layout).handle(), set->layout().space(), 1, &set->handle(), 0, nullptr); });
		else
		{
			// Obtain the handles and bind the sets.
			auto handles = sets | std::views::transform([](auto set) { return set->handle(); }) | std::ranges::to<Array<VkDescriptorSet>>();
			::vkCmdBindDescriptorSets(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::as_const(*m_impl->m_layout).handle(), startSpace, static_cast<UInt32>(handles.size()), handles.data(), 0, nullptr);
		}
	}
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilder(const VulkanRenderPass& renderPass, const String& name) :
	RenderPipelineBuilder(UniquePtr<VulkanRenderPipeline>(new VulkanRenderPipeline(renderPass)))
{
	this->instance()->name() = name;
}

VulkanRenderPipelineBuilder::~VulkanRenderPipelineBuilder() noexcept = default;

void VulkanRenderPipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = m_state.pipelineLayout;
	instance->m_impl->m_program = m_state.shaderProgram;
	instance->m_impl->m_inputAssembler = m_state.inputAssembler;
	instance->m_impl->m_rasterizer = m_state.rasterizer;
	instance->m_impl->m_alphaToCoverage = m_state.enableAlphaToCoverage;
	instance->handle() = instance->m_impl->initialize(m_state.samples);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)