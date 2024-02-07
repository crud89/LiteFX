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
	UInt64 m_renderPassResetEventToken;
	Dictionary<const VulkanRenderPass*, UInt64> m_dependencyResetEventTokens;
	Array<Array<UniquePtr<VulkanDescriptorSet>>> m_inputAttachmentBindings;


public:
	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent, const VulkanRenderPass& renderPass, bool alphaToCoverage, SharedPtr<VulkanPipelineLayout> layout, SharedPtr<VulkanShaderProgram> shaderProgram, SharedPtr<VulkanInputAssembler> inputAssembler, SharedPtr<VulkanRasterizer> rasterizer) :
		base(parent), m_renderPass(renderPass), m_alphaToCoverage(alphaToCoverage), m_layout(layout), m_program(shaderProgram), m_inputAssembler(inputAssembler), m_rasterizer(rasterizer)
	{
		this->initializeEventHandlers();
	}

	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent, const VulkanRenderPass& renderPass) :
		base(parent), m_renderPass(renderPass)
	{
		this->initializeEventHandlers();
	}

public:
	void initializeEventHandlers()
	{
		// Listen to parent render pass reset event, in case the number of frame buffers changes.
		m_renderPassResetEventToken = m_renderPass.reseted.add(std::bind(&VulkanRenderPipelineImpl::onRenderPassReset, this, std::placeholders::_1, std::placeholders::_2));

		// Listen to the parent render pass dependency reset events, in case their render area changes, in which case we have to reset the input attachment bindings.
		std::ranges::for_each(m_renderPass.inputAttachments(), [&](const VulkanRenderPassDependency& dependency) {
			if (!m_dependencyResetEventTokens.contains(dependency.inputAttachmentSource()))
			{
				// Register listener and add the source to the pass list.
				auto pass = dependency.inputAttachmentSource();
				m_dependencyResetEventTokens[pass] = pass->reseted.add(std::bind(&VulkanRenderPipelineImpl::onDependencyReset, this, std::placeholders::_1, std::placeholders::_2));
			}
		});
	}

	VkPipeline initialize()
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
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(reinterpret_cast<const void*>(m_program.get())), modules.size());

		Array<VkPipelineShaderStageCreateInfo> shaderStages = modules |
			std::views::transform([](const VulkanShaderModule* shaderModule) { return shaderModule->shaderStageDefinition(); }) |
			std::ranges::to<Array<VkPipelineShaderStageCreateInfo>>();

		// Setup the pipeline.
		auto pipeline = this->initializeGraphicsPipeline(dynamicState, shaderStages);

#ifndef NDEBUG
		m_renderPass.device().setDebugName(*reinterpret_cast<const UInt64*>(&pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, m_parent->name());
#endif
		// Setup input attachment bindings.
		this->initializeInputAttachmentBindings();

		// Return the pipeline instance.
		return pipeline;

	}

	VkPipeline initializeGraphicsPipeline(const VkPipelineDynamicStateCreateInfo& dynamicState, const LiteFX::Array<VkPipelineShaderStageCreateInfo>& shaderStages)
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
		VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };

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

		// Setup pipeline state.
		VkGraphicsPipelineCreateInfo pipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
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
			.renderPass = m_renderPass.handle(),
			.subpass = 0
		};

		VkPipeline pipeline;
		raiseIfFailed(::vkCreateGraphicsPipelines(m_renderPass.device().handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Unable to create render pipeline.");

		return pipeline;
	}

	void initializeInputAttachmentBindings()
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

		// Allocate the input attachment bindings.
		this->allocateInputAttachmentBindings(descriptorsPerSet | std::views::keys);

		// Bind all input attachments.
		this->bindInputAttachments();
	}

	void allocateInputAttachmentBindings(const std::ranges::input_range auto& descriptorSets) requires 
		std::is_convertible_v<std::ranges::range_value_t<decltype(descriptorSets)>, UInt32>
	{
		// Allocate the bindings array.
		auto backBuffers = m_renderPass.frameBuffers().size();
		m_inputAttachmentBindings.resize(backBuffers);

		// Initialize the descriptor set bindings.
		std::ranges::for_each(m_inputAttachmentBindings, [this, &descriptorSets](auto& inputBindings) {
			// Allocate each descriptor set.
			for (UInt32 descriptorSet : descriptorSets)
				inputBindings.push_back(std::move(m_layout->descriptorSet(descriptorSet).allocate()));
		});
	}

	void bindInputAttachments()
	{
		// Iterate the dependencies and update the binding for each one on every back buffer descriptor set.
		std::ranges::for_each(m_renderPass.inputAttachments(), [this](const VulkanRenderPassDependency& dependency) {
			// Find the input attachment for the binding.
			for (UInt32 backBuffer{ 0 }; auto& bindings : m_inputAttachmentBindings)
			{
				for (auto& binding : bindings)
				{
					if (binding->layout().space() == dependency.binding().Space)
					{
						// Attach the image from the right frame buffer to the descriptor set.
						binding->attach(dependency.binding().Register, dependency.inputAttachmentSource()->frameBuffer(backBuffer++).image(dependency.renderTarget().location()));
						break;
					}
				}

				// We should never reach here.
			}
		});
	}

	void onRenderPassReset(const void* sender, const IRenderPass::ResetEventArgs& eventArgs)
	{
		// If frame buffer count changed, reset number of descriptor sets and re-bind them.
		if (eventArgs.frameBuffers() != static_cast<UInt32>(m_inputAttachmentBindings.size())) [[unlikely]]
		{
			// First, clear all the descriptor sets.
			m_inputAttachmentBindings.clear();

			// Get all descriptor sets that bind input attachments.
			auto descriptorSets = m_renderPass.inputAttachments() | std::views::transform([](auto& dependency) { return dependency.binding().Space; }) | std::ranges::to<Array<UInt32>>();
			std::ranges::sort(descriptorSets);
			descriptorSets = std::ranges::unique(descriptorSets) | std::ranges::to<Array<UInt32>>();

			// Re-allocate the descriptor sets.
			this->allocateInputAttachmentBindings(descriptorSets);

			// Bind all input attachments.
			this->bindInputAttachments();
		}
	}

	void onDependencyReset(const void* sender, const IRenderPass::ResetEventArgs& eventArgs)
	{
		// Re-bind all input attachments, but not if the frame buffer count has changed (this is handled if the parent render pass back buffer count is changed).
		if (eventArgs.frameBuffers() == static_cast<UInt32>(m_inputAttachmentBindings.size())) [[likely]]
			this->bindInputAttachments();
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

	// Bind all the input attachments for the parent render pass.
	std::ranges::for_each(m_impl->m_inputAttachmentBindings[m_impl->m_renderPass.activeBackBuffer()], [this, &commandBuffer](const auto& descriptorSet) { commandBuffer.bind(*descriptorSet, *this); });
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