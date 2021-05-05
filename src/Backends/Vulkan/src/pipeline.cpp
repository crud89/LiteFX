#include <litefx/backends/vulkan.hpp>
#include "buffer.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipeline::VulkanRenderPipelineImpl : public Implement<VulkanRenderPipeline> {
public:
	friend class VulkanRenderPipelineBuilder;
	friend class VulkanRenderPipeline;

private:
	const VulkanRenderPass& m_renderPass;
	const VulkanRenderPipelineLayout* m_vkLayout;
	UniquePtr<IRenderPipelineLayout> m_layout;
	SharedPtr<IInputAssembler> m_inputAssembler;
	SharedPtr<IRasterizer> m_rasterizer;
	Array<SharedPtr<IViewport>> m_viewports;
	Array<SharedPtr<IScissor>> m_scissors;
	const UInt32 m_id;
	const String m_name;

public:
	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent, const VulkanRenderPass& renderPass, const UInt32& id, const String& name) :
		base(parent), m_renderPass(renderPass), m_id(id), m_name(name)
	{
	}

public:
	VkPipeline initialize(UniquePtr<IRenderPipelineLayout>&& layout, SharedPtr<IInputAssembler>&& inputAssembler, SharedPtr<IRasterizer>&& rasterizer, Array<SharedPtr<IViewport>>&& viewports, Array<SharedPtr<IScissor>>&& scissors)
	{
		if (inputAssembler == nullptr)
			throw ArgumentNotInitializedException("The input assembler must be initialized.");

		if (rasterizer == nullptr)
			throw ArgumentNotInitializedException("The rasterizer must be initialized.");

		auto pipelineLayout = dynamic_cast<const VulkanRenderPipelineLayout*>(layout.get());

		if (pipelineLayout == nullptr)
			throw InvalidArgumentException("The pipeline layout is not a valid Vulkan pipeline layout instance.");

		auto program = pipelineLayout->getProgram();

		if (program == nullptr)
			throw InvalidArgumentException("The pipeline shader program must be initialized.");

		LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline for layout {0}...", fmt::ptr(pipelineLayout));
		
		// Get the device.
		auto device = m_parent->getDevice();

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
			LITEFX_TRACE(VULKAN_LOG, "\tRasterizer depth bias: {{ Clamp: {0}, ConstantFactor: {1}, SlopeFactor: {2} }}", rasterizer->getDepthBiasClamp(), rasterizer->getDepthBiasConstantFactor(), rasterizer->getDepthBiasSlopeFactor());
		else
			LITEFX_TRACE(VULKAN_LOG, "\tRasterizer depth bias disabled.");

		// Setup input assembler state.
		VkPipelineVertexInputStateCreateInfo inputState = {};
		inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

		Array<VkVertexInputBindingDescription> vertexInputBindings;
		Array<VkVertexInputAttributeDescription> vertexInputAttributes;

		LITEFX_TRACE(VULKAN_LOG, "Input assembler state: {{ PrimitiveTopology: {0} }}", inputAssembler->getTopology());

		// Set primitive topology.
		inputAssembly.topology = getPrimitiveTopology(inputAssembler->getTopology());
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Parse vertex input descriptors.
		auto vertexLayouts = inputAssembler->getVertexBufferLayouts();

		std::for_each(std::begin(vertexLayouts), std::end(vertexLayouts), [&, l = 0](const IVertexBufferLayout* layout) mutable {
			auto bufferAttributes = layout->getAttributes();
			auto bindingPoint = layout->getBinding();

			LITEFX_TRACE(VULKAN_LOG, "Defining vertex buffer layout {0}/{1} {{ Attributes: {2}, Size: {3} bytes, Binding: {4} }}...", ++l, vertexLayouts.size(), bufferAttributes.size(), layout->getElementSize(), bindingPoint);

			VkVertexInputBindingDescription binding = {};
			binding.binding = bindingPoint;
			binding.stride = static_cast<UInt32>(layout->getElementSize());
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			Array<VkVertexInputAttributeDescription> currentAttributes(bufferAttributes.size());

			std::generate(std::begin(currentAttributes), std::end(currentAttributes), [&, i = 0]() mutable {
				auto attribute = bufferAttributes[i++];

				LITEFX_TRACE(VULKAN_LOG, "\tAttribute {0}/{1}: {{ Location: {2}, Offset: {3}, Format: {4} }}", i, bufferAttributes.size(), attribute->getLocation(), attribute->getOffset(), attribute->getFormat());

				VkVertexInputAttributeDescription descriptor{};
				descriptor.binding = bindingPoint;
				descriptor.location = attribute->getLocation();
				descriptor.offset = attribute->getOffset();
				descriptor.format = getFormat(attribute->getFormat());

				return descriptor;
			});

			vertexInputAttributes.insert(std::end(vertexInputAttributes), std::begin(currentAttributes), std::end(currentAttributes));
			vertexInputBindings.push_back(binding);
		});

		// Define vertex input state.
		inputState.vertexBindingDescriptionCount = static_cast<UInt32>(vertexInputBindings.size());
		inputState.pVertexBindingDescriptions = vertexInputBindings.data();
		inputState.vertexAttributeDescriptionCount = static_cast<UInt32>(vertexInputAttributes.size());
		inputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		// Setup viewport state.
		VkPipelineViewportStateCreateInfo viewportState;
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = static_cast<UInt32>(viewports.size());
		viewportState.scissorCount = static_cast<UInt32>(scissors.size());

		// Setup dynamic state.
		VkDynamicState dynamicStates[] = { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStates;
		dynamicState.dynamicStateCount = 2;

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
		// TODO: Add blend parameters to render target.
		auto targets = m_renderPass.getTargets();
		auto colorAttachments = std::count_if(std::begin(targets), std::end(targets), [](const auto& target) { return target->getType() != RenderTargetType::Depth; });
		
		Array<VkPipelineColorBlendAttachmentState> colorBlendAttachments(colorAttachments);
		std::generate(std::begin(colorBlendAttachments), std::end(colorBlendAttachments), []() {
			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};

			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;

			return colorBlendAttachment;
		});

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = static_cast<UInt32>(colorBlendAttachments.size());
		colorBlending.pAttachments = colorBlendAttachments.data();
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		// Setup depth/stencil state.
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;		// TODO: From depth/stencil state.
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;		// TODO: From depth/stencil state.
		depthStencilState.depthWriteEnable = std::any_of(std::begin(targets), std::end(targets), [](const auto& t) { return t->getType() == RenderTargetType::Depth; });
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

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
		pipelineInfo.layout = pipelineLayout->handle();
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// Setup shader stages.
		auto modules = program->getModules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(program), modules.size());

		Array<VkPipelineShaderStageCreateInfo> shaderStages(modules.size());

		std::generate(std::begin(shaderStages), std::end(shaderStages), [&, i = 0]() mutable {
			auto module = dynamic_cast<const VulkanShaderModule*>(modules[i++]);

			if (module == nullptr)
				throw std::invalid_argument("The provided shader module is not a valid Vulkan shader.");

			LITEFX_TRACE(VULKAN_LOG, "\tModule {0}/{1} (\"{2}\") state: {{ Type: {3}, EntryPoint: {4} }}", i, modules.size(), module->getFileName(), module->getType(), module->getEntryPoint());
			
			return module->getShaderStageDefinition();
		});

		pipelineInfo.stageCount = modules.size();
		pipelineInfo.pStages = shaderStages.data();

		// Setup render pass state.
		pipelineInfo.renderPass = m_renderPass.handle();
		pipelineInfo.subpass = 0;

		VkPipeline pipeline;
		auto result = ::vkCreateGraphicsPipelines(device->handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

		if (result != VK_SUCCESS)
			throw std::runtime_error(fmt::format("Unable to create render pipeline: {0}", result));

		m_layout = std::move(layout);
		m_vkLayout = pipelineLayout;
		m_inputAssembler = std::move(inputAssembler);
		m_rasterizer = std::move(rasterizer);
		m_viewports = std::move(viewports);
		m_scissors = std::move(scissors);

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipeline::VulkanRenderPipeline(const VulkanRenderPass& renderPass, const UInt32& id, const String& name) :
	m_impl(makePimpl<VulkanRenderPipelineImpl>(this, renderPass, id, name)), VulkanRuntimeObject(renderPass.getDevice()), IResource<VkPipeline>(nullptr)
{
}

VulkanRenderPipeline::~VulkanRenderPipeline() noexcept
{
	if (this->isInitialized())
		::vkDestroyPipeline(this->getDevice()->handle(), this->handle(), nullptr);
}

bool VulkanRenderPipeline::isInitialized() const noexcept
{
	return this->handle() != nullptr;
}

const IRenderPass& VulkanRenderPipeline::renderPass() const noexcept
{
	return m_impl->m_renderPass;
}

const String& VulkanRenderPipeline::name() const noexcept
{
	return m_impl->m_name;
}

const UInt32& VulkanRenderPipeline::id() const noexcept
{
	return m_impl->m_id;
}

void VulkanRenderPipeline::initialize(UniquePtr<IRenderPipelineLayout>&& layout, SharedPtr<IInputAssembler> inputAssembler, SharedPtr<IRasterizer> rasterizer, Array<SharedPtr<IViewport>>&& viewports, Array<SharedPtr<IScissor>>&& scissors)
{
	if (this->isInitialized())
		throw RuntimeException("The render pipeline already has been initialized.");

	this->handle() = m_impl->initialize(std::move(layout), std::move(inputAssembler), std::move(rasterizer), std::move(viewports), std::move(scissors));
}

const IRenderPipelineLayout* VulkanRenderPipeline::getLayout() const noexcept
{
	return m_impl->m_layout.get();
}

SharedPtr<IInputAssembler> VulkanRenderPipeline::getInputAssembler() const noexcept
{
	return m_impl->m_inputAssembler;
}

SharedPtr<IRasterizer> VulkanRenderPipeline::getRasterizer() const noexcept
{
	return m_impl->m_rasterizer;
}

Array<const IViewport*> VulkanRenderPipeline::getViewports() const noexcept
{
	Array<const IViewport*> viewports(m_impl->m_viewports.size());
	std::generate(std::begin(viewports), std::end(viewports), [&, i = 0]() mutable { return m_impl->m_viewports[i++].get(); });

	return viewports;
}

Array<const IScissor*> VulkanRenderPipeline::getScissors() const noexcept
{
	Array<const IScissor*> scissors(m_impl->m_scissors.size());
	std::generate(std::begin(scissors), std::end(scissors), [&, i = 0]() mutable { return m_impl->m_scissors[i++].get(); });

	return scissors;
}

void VulkanRenderPipeline::bind(const IVertexBuffer* buffer) const
{
	auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);
	auto commandBuffer = m_impl->m_renderPass.getVkCommandBuffer();

	if (resource == nullptr)
		throw std::invalid_argument("The provided vertex buffer is not a valid Vulkan buffer.");

	// Depending on the type, bind the buffer accordingly.
	constexpr VkDeviceSize offsets[] = { 0 };

	::vkCmdBindVertexBuffers(commandBuffer->handle(), 0, 1, &resource->handle(), offsets);
}

void VulkanRenderPipeline::bind(const IIndexBuffer* buffer) const
{
	auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);
	auto commandBuffer = m_impl->m_renderPass.getVkCommandBuffer();

	if (resource == nullptr)
		throw std::invalid_argument("The provided index buffer is not a valid Vulkan buffer.");

	::vkCmdBindIndexBuffer(commandBuffer->handle(), resource->handle(), 0, buffer->getLayout()->getIndexType() == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void VulkanRenderPipeline::bind(const IDescriptorSet* descriptorSet) const
{
	auto resource = dynamic_cast<const VulkanDescriptorSet*>(descriptorSet);
	auto commandBuffer = m_impl->m_renderPass.getVkCommandBuffer();

	if (resource == nullptr)
		throw std::invalid_argument("The provided descriptor set is not a valid Vulkan descriptor set.");

	VkDescriptorSet descriptorSets[] = { resource->getHandle(m_impl->m_renderPass.getCurrentBackBuffer()) };

	// TODO: Synchronize with possible update calls on this command buffer, first.
	::vkCmdBindDescriptorSets(commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_impl->m_vkLayout->handle(), descriptorSet->getDescriptorSetLayout()->getSetId(), 1, descriptorSets, 0, nullptr);
}

void VulkanRenderPipeline::use() const
{
	// TODO: Make this more efficient by buffering the viewports and scissors (and only updating with each `use` call).
	Array<VkViewport> viewports(m_impl->m_viewports.size());
	Array<VkRect2D> scissors(m_impl->m_scissors.size());
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

	std::for_each(std::begin(m_impl->m_viewports), std::end(m_impl->m_viewports), [i = 0, &viewports](const auto& viewport) mutable {
		viewports[i].x = viewport->getRectangle().x();
		viewports[i].y = viewport->getRectangle().y();
		viewports[i].width = viewport->getRectangle().width();
		viewports[i].height = viewport->getRectangle().height();
		viewports[i].minDepth = viewport->getMinDepth();
		viewports[i].maxDepth = viewport->getMaxDepth();

		i++;
	});

	std::for_each(std::begin(m_impl->m_scissors), std::end(m_impl->m_scissors), [i = 0, &scissors](const auto& scissor) mutable {
		scissors[i].offset = { static_cast<Int32>(scissor->getRectangle().x()), static_cast<Int32>(scissor->getRectangle().y()) };
		scissors[i].extent = { static_cast<UInt32>(scissor->getRectangle().width()), static_cast<UInt32>(scissor->getRectangle().height()) };

		i++;
	});

	::vkCmdBindPipeline(m_impl->m_renderPass.getVkCommandBuffer()->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, this->handle());
	::vkCmdSetViewport(m_impl->m_renderPass.getVkCommandBuffer()->handle(), 0, static_cast<UInt32>(viewports.size()), viewports.data());
	::vkCmdSetScissor(m_impl->m_renderPass.getVkCommandBuffer()->handle(), 0, static_cast<UInt32>(scissors.size()), scissors.data());
}

UniquePtr<IVertexBuffer> VulkanRenderPipeline::makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding) const
{
	return this->getDevice()->createVertexBuffer(m_impl->m_inputAssembler->getVertexBufferLayout(binding), usage, elements);
}

UniquePtr<IIndexBuffer> VulkanRenderPipeline::makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const
{
	return this->getDevice()->createIndexBuffer(m_impl->m_inputAssembler->getIndexBufferLayout(), usage, elements);
}

UniquePtr<IDescriptorSet> VulkanRenderPipeline::makeBufferPool(const UInt32& setId) const
{
	auto layouts = m_impl->m_layout->getDescriptorSetLayouts();
	auto match = std::find_if(std::begin(layouts), std::end(layouts), [&](const IDescriptorSetLayout* layout) { return layout->getSetId() == setId; });

	if (match == layouts.end())
		throw std::invalid_argument("The requested buffer set is not defined.");

	return (*match)->createBufferPool();
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilderImpl : public Implement<VulkanRenderPipelineBuilder> {
public:
	friend class VulkanRenderPipelineBuilderBuilder;
	friend class VulkanRenderPipelineBuilder;

private:
	UniquePtr<IRenderPipelineLayout> m_layout;
	SharedPtr<IInputAssembler> m_inputAssembler;
	SharedPtr<IRasterizer> m_rasterizer;
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

VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilder(VulkanRenderPassBuilder& parent, UniquePtr<VulkanRenderPipeline>&& instance) :
	RenderPipelineBuilder(parent, std::move(instance)), m_impl(makePimpl<VulkanRenderPipelineBuilderImpl>(this))
{
}

VulkanRenderPipelineBuilder::~VulkanRenderPipelineBuilder() noexcept = default;

VulkanRenderPassBuilder& VulkanRenderPipelineBuilder::go()
{
	this->instance()->initialize(std::move(m_impl->m_layout), std::move(m_impl->m_inputAssembler), std::move(m_impl->m_rasterizer), std::move(m_impl->m_viewports), std::move(m_impl->m_scissors));

	return RenderPipelineBuilder::go();
}

void VulkanRenderPipelineBuilder::use(UniquePtr<IRenderPipelineLayout>&& layout)
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

	m_impl->m_rasterizer = rasterizer;
}

void VulkanRenderPipelineBuilder::use(SharedPtr<IInputAssembler> inputAssembler)
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
	return this->make<VulkanRenderPipelineLayout>();
}

VulkanRasterizerBuilder VulkanRenderPipelineBuilder::rasterizer()
{
	return this->make<VulkanRasterizer>();
}

VulkanInputAssemblerBuilder VulkanRenderPipelineBuilder::inputAssembler()
{
	return this->make<VulkanInputAssembler>();
}

VulkanRenderPipelineBuilder& VulkanRenderPipelineBuilder::withRasterizer(SharedPtr<IRasterizer> rasterizer)
{
	this->use(std::move(rasterizer));
	return *this;
}

VulkanRenderPipelineBuilder& VulkanRenderPipelineBuilder::withInputAssembler(SharedPtr<IInputAssembler> inputAssembler)
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