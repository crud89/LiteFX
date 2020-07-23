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
	const VulkanRenderPass* m_renderPass;
	const VulkanCommandBuffer* m_commandBuffer;
	const VulkanRenderPipelineLayout* m_pipelineLayout;

public:
	VulkanRenderPipelineImpl(VulkanRenderPipeline* parent) : base(parent) { }

private:
	void cleanup()
	{
		::vkDestroyPipeline(m_parent->getDevice()->handle(), m_parent->handle(), nullptr);
	}

public:
	VkPipeline initialize()
	{
		m_pipelineLayout = dynamic_cast<const VulkanRenderPipelineLayout*>(m_parent->getLayout());
		m_renderPass = dynamic_cast<const VulkanRenderPass*>(m_parent->getRenderPass());
		m_commandBuffer = dynamic_cast<const VulkanCommandBuffer*>(m_renderPass->getCommandBuffer());

		if (m_pipelineLayout == nullptr)
			throw std::invalid_argument("The pipeline layout is not a valid Vulkan pipeline layout instance.");

		if (m_renderPass == nullptr)
			throw std::invalid_argument("The render pass is not a valid Vulkan render pass instance.");

		if (m_commandBuffer == nullptr)
			throw std::invalid_argument("The command buffer is not a valid Vulkan command buffer instance.");

		LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline for layout {0}...", fmt::ptr(m_pipelineLayout));

		// Get the device.
		auto device = m_parent->getDevice();

		// Request configuration interface.
		auto rasterizer = m_pipelineLayout->getRasterizer();
		auto inputAssembler = m_pipelineLayout->getInputAssembler();
		auto views = m_pipelineLayout->getViewports();
		auto program = m_pipelineLayout->getProgram();

		if (rasterizer == nullptr)
			throw std::invalid_argument("The pipeline layout does not contain a rasterizer.");

		if (program == nullptr)
			throw std::invalid_argument("The pipeline shader program must be initialized.");

		if (inputAssembler == nullptr)
			throw std::invalid_argument("The input assembler must be initialized.");

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
		pipelineInfo.layout = m_pipelineLayout->handle();
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// Setup shader stages.
		auto modules = program->getModules();
		LITEFX_TRACE(VULKAN_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(program), modules.size());

		Array<VkPipelineShaderStageCreateInfo> shaderStages(modules.size());

		std::generate(std::begin(shaderStages), std::end(shaderStages), [&, i = 0]() mutable {
			auto module = dynamic_cast<const VulkanShaderModule*>(modules[i++]);

			if (module == nullptr)
				throw std::invalid_argument("The provided shader module is not a valid Vulkan shader.");

			LITEFX_TRACE(VULKAN_LOG, "\tModule {0}/{1} (\"{4}\") state: {{ Type: {2}, EntryPoint: {3} }}", i, modules.size(), module->getType(), module->getEntryPoint(), module->getFileName());
			
			return module->getShaderStageDefinition();
		});

		pipelineInfo.stageCount = modules.size();
		pipelineInfo.pStages = shaderStages.data();

		// Setup render pass state.
		pipelineInfo.renderPass = m_renderPass->handle();
		pipelineInfo.subpass = 0;

		VkPipeline pipeline;
		auto result = ::vkCreateGraphicsPipelines(device->handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

		if (result != VK_SUCCESS)
			throw std::runtime_error(fmt::format("Unable to create render pipeline: {0}", result));

		return pipeline;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipeline::VulkanRenderPipeline(const IGraphicsDevice* device) : VulkanRenderPipeline(dynamic_cast<const VulkanDevice*>(device)) { }

VulkanRenderPipeline::VulkanRenderPipeline(const VulkanDevice* device) :
	m_impl(makePimpl<VulkanRenderPipelineImpl>(this)), VulkanRuntimeObject(device), RenderPipeline(device), IResource<VkPipeline>(nullptr)
{
}

VulkanRenderPipeline::VulkanRenderPipeline(UniquePtr<IRenderPipelineLayout>&& layout, const VulkanDevice* device) :
	m_impl(makePimpl<VulkanRenderPipelineImpl>(this)), VulkanRuntimeObject(device), RenderPipeline(device, std::move(layout)), IResource<VkPipeline>(nullptr)
{
	this->handle() = m_impl->initialize();
}

VulkanRenderPipeline::~VulkanRenderPipeline() noexcept
{
	m_impl->cleanup();
}

void VulkanRenderPipeline::reset()
{
	m_impl->cleanup();

	this->getRenderPass()->reset();
	this->handle() = m_impl->initialize();
}

void VulkanRenderPipeline::bind(const IVertexBuffer* buffer) const
{
	auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);

	if (resource == nullptr)
		throw std::invalid_argument("The provided vertex buffer is not a valid Vulkan buffer.");

	// Depending on the type, bind the buffer accordingly.
	constexpr VkDeviceSize offsets[] = { 0 };

	::vkCmdBindVertexBuffers(m_impl->m_commandBuffer->handle(), 0, 1, &resource->handle(), offsets);
}

void VulkanRenderPipeline::bind(const IIndexBuffer* buffer) const
{
	auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);

	if (resource == nullptr)
		throw std::invalid_argument("The provided index buffer is not a valid Vulkan buffer.");

	::vkCmdBindIndexBuffer(m_impl->m_commandBuffer->handle(), resource->handle(), 0, buffer->getLayout()->getIndexType() == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void VulkanRenderPipeline::bind(const IDescriptorSet* b) const
{
	auto pool = dynamic_cast<const VulkanBufferPool*>(b);

	if (pool == nullptr)
		throw std::invalid_argument("The provided buffer pool is not a valid Vulkan buffer pool.");

	auto renderPass = m_impl->m_renderPass;
	auto commandBuffer = m_impl->m_commandBuffer;
	auto pipelineLayout = m_impl->m_pipelineLayout;
	auto bufferSet = pool->getDescriptorSetLayout()->getSetId();
	VkDescriptorSet descriptorSets[] = { pool->getDescriptorSet() };

	// TODO: Synchronize with possible update calls on this command buffer, first.
	::vkCmdBindDescriptorSets(commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->handle(), bufferSet, 1, descriptorSets, 0, nullptr);
}

UniquePtr<IVertexBuffer> VulkanRenderPipeline::makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding) const
{
	return this->getDevice()->createVertexBuffer(this->getLayout()->getInputAssembler()->getVertexBufferLayout(binding), usage, elements);
}

UniquePtr<IIndexBuffer> VulkanRenderPipeline::makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const
{
	return this->getDevice()->createIndexBuffer(this->getLayout()->getInputAssembler()->getIndexBufferLayout(), usage, elements);
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilderImpl : public Implement<VulkanRenderPipelineBuilder> {
public:
	friend class VulkanRenderPipelineBuilder;

private:
	UniquePtr<IRenderPass> m_renderPass;
	UniquePtr<IRenderPipelineLayout> m_layout;

public:
	VulkanRenderPipelineBuilderImpl(VulkanRenderPipelineBuilder* parent) : base(parent) { }
	VulkanRenderPipelineBuilderImpl() noexcept = default;
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineBuilder::VulkanRenderPipelineBuilder(UniquePtr<VulkanRenderPipeline>&& instance) :
	RenderPipelineBuilder(std::move(instance)), m_impl(makePimpl<VulkanRenderPipelineBuilderImpl>(this))
{
}

VulkanRenderPipelineBuilder::~VulkanRenderPipelineBuilder() noexcept = default;

UniquePtr<VulkanRenderPipeline> VulkanRenderPipelineBuilder::go()
{
	auto instance = this->instance();

	instance->use(std::move(m_impl->m_layout));
	instance->use(std::move(m_impl->m_renderPass));

	instance->handle() = instance->m_impl->initialize();

	return RenderPipelineBuilder::go();
}

void VulkanRenderPipelineBuilder::use(UniquePtr<IRenderPipelineLayout> && layout)
{
    if (layout == nullptr)
        throw std::invalid_argument("The layout must be initialized.");

	m_impl->m_layout = std::move(layout);
}

void VulkanRenderPipelineBuilder::use(UniquePtr<IRenderPass>&& renderPass)
{
	if (renderPass == nullptr)
		throw std::invalid_argument("The render pass must be initialized.");

	m_impl->m_renderPass = std::move(renderPass);
}