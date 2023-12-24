#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanCommandBuffer::VulkanCommandBufferImpl : public Implement<VulkanCommandBuffer> {
public:
	friend class VulkanCommandBuffer;

private:
	const VulkanQueue& m_queue;
	bool m_recording{ false }, m_secondary{ false };
	VkCommandPool m_commandPool;
	Array<SharedPtr<const IStateResource>> m_sharedResources;
	const VulkanPipelineState* m_lastPipeline = nullptr;

public:
	VulkanCommandBufferImpl(VulkanCommandBuffer* parent, const VulkanQueue& queue, bool primary) :
		base(parent), m_queue(queue), m_secondary(!primary)
	{
	}

	~VulkanCommandBufferImpl() 
	{
		this->release();
	}

public:
	void release() 
	{
		::vkFreeCommandBuffers(m_queue.device().handle(), m_commandPool, 1, &m_parent->handle());
		::vkDestroyCommandPool(m_queue.device().handle(), m_commandPool, nullptr);
	}

	VkCommandBuffer initialize()
	{
		// Create command pool.
		VkCommandPoolCreateInfo poolInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = m_queue.familyId(),
		};

		// Primary command buffers are frequently reset and re-allocated, whilst secondary command buffers must be recorded once and never reset.
		if (!m_secondary)
			poolInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		raiseIfFailed(::vkCreateCommandPool(m_queue.device().handle(), &poolInfo, nullptr, &m_commandPool), "Unable to create command pool.");

		// Create the command buffer.
		VkCommandBufferAllocateInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_commandPool,
			.level = m_secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		VkCommandBuffer buffer;
		raiseIfFailed(::vkAllocateCommandBuffers(m_queue.device().handle(), &bufferInfo, &buffer), "Unable to allocate command buffer.");

		return buffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanQueue& queue, bool begin, bool primary) :
	m_impl(makePimpl<VulkanCommandBufferImpl>(this, queue, primary)), Resource<VkCommandBuffer>(nullptr)
{
	this->handle() = m_impl->initialize();

	if (begin)
		this->begin();
}

VulkanCommandBuffer::~VulkanCommandBuffer() noexcept = default;

void VulkanCommandBuffer::begin() const
{
	// Set the buffer into recording state.
	VkCommandBufferBeginInfo beginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	raiseIfFailed(::vkBeginCommandBuffer(this->handle(), &beginInfo), "Unable to begin command recording.");
	
	m_impl->m_recording = true;
}

void VulkanCommandBuffer::begin(const VulkanRenderPass& renderPass) const noexcept
{
	// Create an inheritance info for the parent buffer.
	VkCommandBufferInheritanceInfo inheritanceInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		.pNext = nullptr,
		.renderPass = renderPass.handle(),
		.subpass = 0,
		.framebuffer = renderPass.activeFrameBuffer().handle(),
		.occlusionQueryEnable = false
	};

	// Set the buffer into recording state.
	VkCommandBufferBeginInfo beginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		.pInheritanceInfo = &inheritanceInfo
	};

	raiseIfFailed(::vkBeginCommandBuffer(this->handle(), &beginInfo), "Unable to begin command recording.");

	m_impl->m_recording = true;
}

void VulkanCommandBuffer::end() const
{
	// End recording.
	if (m_impl->m_recording)
		raiseIfFailed(::vkEndCommandBuffer(this->handle()), "Unable to stop command recording.");

	m_impl->m_recording = false;
}

bool VulkanCommandBuffer::isSecondary() const noexcept
{
	return m_impl->m_secondary;
}

void VulkanCommandBuffer::setViewports(Span<const IViewport*> viewports) const noexcept
{
	auto vps = viewports |
		std::views::transform([](const auto& viewport) { return VkViewport{ .x = viewport->getRectangle().x(), .y = viewport->getRectangle().y(), .width = viewport->getRectangle().width(), .height = viewport->getRectangle().height(), .minDepth = viewport->getMinDepth(), .maxDepth = viewport->getMaxDepth() }; }) |
		std::ranges::to<Array<VkViewport>>();

	::vkCmdSetViewportWithCount(this->handle(), static_cast<UInt32>(vps.size()), vps.data());
}

void VulkanCommandBuffer::setViewports(const IViewport* viewport) const noexcept
{
	auto vp = VkViewport{ .x = viewport->getRectangle().x(), .y = viewport->getRectangle().y(), .width = viewport->getRectangle().width(), .height = viewport->getRectangle().height(), .minDepth = viewport->getMinDepth(), .maxDepth = viewport->getMaxDepth() };
	::vkCmdSetViewportWithCount(this->handle(), 1, &vp);
}

void VulkanCommandBuffer::setScissors(Span<const IScissor*> scissors) const noexcept
{
	auto scs = scissors |
		std::views::transform([](const auto& scissor) { return VkRect2D{ { .x = static_cast<Int32>(scissor->getRectangle().x()), .y = static_cast<Int32>(scissor->getRectangle().y())}, { .width = static_cast<UInt32>(scissor->getRectangle().width()), .height = static_cast<UInt32>(scissor->getRectangle().height())} }; }) |
		std::ranges::to<Array<VkRect2D>>();

	::vkCmdSetScissorWithCount(this->handle(), static_cast<UInt32>(scs.size()), scs.data());
}

void VulkanCommandBuffer::setScissors(const IScissor* scissor) const noexcept
{
	auto s = VkRect2D{ { .x = static_cast<Int32>(scissor->getRectangle().x()), .y = static_cast<Int32>(scissor->getRectangle().y())},  { .width = static_cast<UInt32>(scissor->getRectangle().width()), .height = static_cast<UInt32>(scissor->getRectangle().height())} };
	::vkCmdSetScissorWithCount(this->handle(), 1, &s);
}

void VulkanCommandBuffer::setBlendFactors(const Vector4f& blendFactors) const noexcept
{
	::vkCmdSetBlendConstants(this->handle(), blendFactors.elements());
}

void VulkanCommandBuffer::setStencilRef(UInt32 stencilRef) const noexcept
{
	::vkCmdSetStencilReference(this->handle(), VK_STENCIL_FACE_FRONT_AND_BACK, stencilRef);
}

UInt64 VulkanCommandBuffer::submit() const 
{
	if (this->isSecondary())
		throw RuntimeException("A secondary command buffer cannot be directly submitted to a command queue and must be executed on a primary command buffer instead.");

	return m_impl->m_queue.submit(this->shared_from_this());
}

void VulkanCommandBuffer::generateMipMaps(IVulkanImage& image) noexcept
{
	VulkanBarrier startBarrier(PipelineStage::None, PipelineStage::Transfer);
	startBarrier.transition(image, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
	this->barrier(startBarrier);

	for (UInt32 layer(0); layer < image.layers(); ++layer)
	{
		Int32 mipWidth = static_cast<Int32>(image.extent().width());
		Int32 mipHeight = static_cast<Int32>(image.extent().height());
		Int32 mipDepth = static_cast<Int32>(image.extent().depth());

		for (UInt32 level(1); level < image.levels(); ++level)
		{
			VulkanBarrier subBarrier(PipelineStage::Transfer, PipelineStage::Transfer);
			subBarrier.transition(image, level - 1, 1, layer, 1, 0, ResourceAccess::TransferWrite, ResourceAccess::TransferRead, ImageLayout::CopySource);
			this->barrier(subBarrier);

			// Blit the image of the previous level into the current level.
			VkImageBlit blit {
				.srcSubresource = VkImageSubresourceLayers {
					.aspectMask = image.aspectMask(),
					.mipLevel = level - 1,
					.baseArrayLayer = layer,
					.layerCount = 1
				},
				.dstSubresource = VkImageSubresourceLayers {
					.aspectMask = image.aspectMask(),
					.mipLevel = level,
					.baseArrayLayer = layer,
					.layerCount = 1
				}
			};

			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, mipDepth };
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1 };

			::vkCmdBlitImage(this->handle(), std::as_const(image).handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, std::as_const(image).handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			// Compute the new size.
			mipWidth = std::max(mipWidth / 2, 1);
			mipHeight = std::max(mipHeight / 2, 1);
			mipDepth = std::max(mipDepth / 2, 1);
		}

		VulkanBarrier subBarrier(PipelineStage::Transfer, PipelineStage::Transfer);
		subBarrier.transition(image, image.levels() - 1, 1, layer, 1, 0, ResourceAccess::TransferWrite, ResourceAccess::TransferRead, ImageLayout::CopySource);
		subBarrier.transition(image, 0, 1, layer, 1, 0, ResourceAccess::TransferWrite, ResourceAccess::TransferRead, ImageLayout::CopySource);
		this->barrier(subBarrier);
	}

	VulkanBarrier endBarrier(PipelineStage::Transfer, PipelineStage::All);
	endBarrier.transition(image, ResourceAccess::TransferRead | ResourceAccess::TransferWrite, ResourceAccess::ShaderRead, ImageLayout::ShaderResource);
	this->barrier(endBarrier);
}

void VulkanCommandBuffer::barrier(const VulkanBarrier& barrier) const noexcept
{
	barrier.execute(*this);
}

void VulkanCommandBuffer::transfer(IVulkanBuffer& source, IVulkanBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);

	VkBufferCopy copyInfo {
		.srcOffset = sourceElement * source.alignedElementSize(),
		.dstOffset = targetElement * target.alignedElementSize(),
		.size      = elements      * source.alignedElementSize()
	};

	::vkCmdCopyBuffer(this->handle(), std::as_const(source).handle(), std::as_const(target).handle(), 1, &copyInfo);
}

void VulkanCommandBuffer::transfer(IVulkanBuffer& source, IVulkanImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < firstSubresource + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target image has only {0} sub-resources, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, firstSubresource);

	// Create a copy command and add it to the command buffer.
	VulkanBarrier barrier(PipelineStage::None, PipelineStage::Transfer);

	Array<VkBufferImageCopy> copyInfos(elements);
	std::ranges::generate(copyInfos, [&, this, i = firstSubresource]() mutable {
		UInt32 subresource = i++, layer = 0, level = 0, plane = 0;
		target.resolveSubresource(subresource, plane, layer, level);

		if (static_cast<const IImage&>(target).layout(subresource) != ImageLayout::CopyDestination)
			barrier.transition(target, level, 1, layer, 1, plane, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::CopyDestination);

		return VkBufferImageCopy {
			.bufferOffset = source.alignedElementSize() * sourceElement,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = VkImageSubresourceLayers {
				.aspectMask = target.aspectMask(plane),
				.mipLevel = level,
				.baseArrayLayer = layer,
				.layerCount = 1
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { static_cast<UInt32>(target.extent().width()), static_cast<UInt32>(target.extent().height()), static_cast<UInt32>(target.extent().depth()) }
		};
	});

	this->barrier(barrier);
	::vkCmdCopyBufferToImage(this->handle(), std::as_const(source).handle(), std::as_const(target).handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<UInt32>(copyInfos.size()), copyInfos.data());
}

void VulkanCommandBuffer::transfer(IVulkanImage& source, IVulkanImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const
{
	if (source.elements() < sourceSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, sourceSubresource);

	if (target.elements() < targetSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resources {2} has been requested.", target.elements(), subresources, targetSubresource);

	// Create a copy command and add it to the command buffer.
	VulkanBarrier barrier(PipelineStage::None, PipelineStage::Transfer);

	Array<VkImageCopy> copyInfos(subresources);
	std::ranges::generate(copyInfos, [&, this, i = 0]() mutable {
		UInt32 sourceRsc = sourceSubresource + i, sourceLayer = 0, sourceLevel = 0, sourcePlane = 0;
		UInt32 targetRsc = targetSubresource + i, targetLayer = 0, targetLevel = 0, targetPlane = 0;
		source.resolveSubresource(sourceRsc, sourceLayer, sourceLevel, sourcePlane);
		target.resolveSubresource(targetRsc, targetLayer, targetLevel, targetPlane);
		i++;

		//if (static_cast<const IImage&>(source).layout(sourceSubresource) != ImageLayout::CopySource)
		//	barrier.transition(source, sourceLayer, 1, sourceLevel, 1, sourcePlane, ResourceAccess::None, ResourceAccess::TransferRead, ImageLayout::CopySource);

		if (static_cast<const IImage&>(target).layout(targetSubresource) != ImageLayout::CopyDestination)
			barrier.transition(target, targetLayer, 1, targetLevel, 1, targetPlane, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::CopyDestination);

		return VkImageCopy {
			.srcSubresource = VkImageSubresourceLayers {
				.aspectMask = source.aspectMask(sourcePlane),
				.mipLevel = sourceLevel,
				.baseArrayLayer = sourceLayer,
				.layerCount = 1
			},
			.srcOffset = { 0, 0, 0 },
			.dstSubresource = VkImageSubresourceLayers {
				.aspectMask = target.aspectMask(targetPlane),
				.mipLevel = targetLevel,
				.baseArrayLayer = targetLayer,
				.layerCount = 1
			},
			.dstOffset = { 0, 0, 0 },
			.extent = { static_cast<UInt32>(source.extent().width()), static_cast<UInt32>(source.extent().height()), static_cast<UInt32>(source.extent().depth()) }
		};
	});

	this->barrier(barrier);
	::vkCmdCopyImage(this->handle(), std::as_const(source).handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, std::as_const(target).handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<UInt32>(copyInfos.size()), copyInfos.data());
}

void VulkanCommandBuffer::transfer(IVulkanImage& source, IVulkanBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const
{
	if (source.elements() < firstSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, firstSubresource);

	if (target.elements() <= targetElement + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), subresources, targetElement);
	
	// Create a copy command and add it to the command buffer.
	Array<VkBufferImageCopy> copyInfos(subresources);
	std::ranges::generate(copyInfos, [&, this, i = targetElement]() mutable {
		UInt32 subresource = i++, layer = 0, level = 0, plane = 0;
		source.resolveSubresource(subresource, plane, layer, level);

		return VkBufferImageCopy {
			.bufferOffset = target.alignedElementSize() * subresource,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = VkImageSubresourceLayers {
				.aspectMask = source.aspectMask(plane),
				.mipLevel = level,
				.baseArrayLayer = layer,
				.layerCount = 1
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { static_cast<UInt32>(source.extent().width()), static_cast<UInt32>(source.extent().height()), static_cast<UInt32>(source.extent().depth()) }
		};
	});

	::vkCmdCopyImageToBuffer(this->handle(), std::as_const(source).handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, std::as_const(target).handle(), static_cast<UInt32>(copyInfos.size()), copyInfos.data());
}

void VulkanCommandBuffer::transfer(SharedPtr<IVulkanBuffer> source, IVulkanBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const
{
	this->transfer(*source, target, sourceElement, targetElement, elements);
	m_impl->m_sharedResources.push_back(source);
}

void VulkanCommandBuffer::transfer(SharedPtr<IVulkanBuffer> source, IVulkanImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const
{
	this->transfer(*source, target, sourceElement, firstSubresource, elements);
	m_impl->m_sharedResources.push_back(source);
}

void VulkanCommandBuffer::transfer(SharedPtr<IVulkanImage> source, IVulkanImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const
{
	this->transfer(*source, target, sourceSubresource, targetSubresource, subresources);
	m_impl->m_sharedResources.push_back(source);
}

void VulkanCommandBuffer::transfer(SharedPtr<IVulkanImage> source, IVulkanBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const
{
	this->transfer(*source, target, firstSubresource, targetElement, subresources);
	m_impl->m_sharedResources.push_back(source);
}

void VulkanCommandBuffer::use(const VulkanPipelineState& pipeline) const noexcept
{
	m_impl->m_lastPipeline = &pipeline;
	pipeline.use(*this);
}

void VulkanCommandBuffer::bind(const VulkanDescriptorSet& descriptorSet) const
{
	if (m_impl->m_lastPipeline) [[likely]]
		m_impl->m_lastPipeline->bind(*this, descriptorSet);
	else
		throw RuntimeException("No pipeline has been used on the command buffer before attempting to bind the descriptor set.");
}

void VulkanCommandBuffer::bind(const VulkanDescriptorSet& descriptorSet, const VulkanPipelineState& pipeline) const noexcept
{
	pipeline.bind(*this, descriptorSet);
}

void VulkanCommandBuffer::bind(const IVulkanVertexBuffer& buffer) const noexcept
{
	constexpr VkDeviceSize offsets[] = { 0 };
	::vkCmdBindVertexBuffers(this->handle(), buffer.layout().binding(), 1, &buffer.handle(), offsets);
}

void VulkanCommandBuffer::bind(const IVulkanIndexBuffer& buffer) const noexcept
{
	::vkCmdBindIndexBuffer(this->handle(), buffer.handle(), 0, buffer.layout().indexType() == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBuffer::dispatch(const Vector3u& threadCount) const noexcept
{
	::vkCmdDispatch(this->handle(), threadCount.x(), threadCount.y(), threadCount.z());
}

void VulkanCommandBuffer::draw(UInt32 vertices, UInt32 instances, UInt32 firstVertex, UInt32 firstInstance) const noexcept
{
	::vkCmdDraw(this->handle(), vertices, instances, firstVertex, firstInstance);
}

void VulkanCommandBuffer::drawIndexed(UInt32 indices, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const noexcept
{
	::vkCmdDrawIndexed(this->handle(), indices, instances, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::pushConstants(const VulkanPushConstantsLayout& layout, const void* const memory) const noexcept
{
	std::ranges::for_each(layout.ranges(), [this, &layout, &memory](const VulkanPushConstantsRange* range) { ::vkCmdPushConstants(this->handle(), layout.pipelineLayout().handle(), static_cast<VkShaderStageFlags>(Vk::getShaderStage(range->stage())), range->offset(), range->size(), memory); });
}

void VulkanCommandBuffer::writeTimingEvent(SharedPtr<const TimingEvent> timingEvent) const
{
	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	::vkCmdWriteTimestamp(this->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_impl->m_queue.device().swapChain().timestampQueryPool(), timingEvent->queryId());
}

void VulkanCommandBuffer::execute(SharedPtr<const VulkanCommandBuffer> commandBuffer) const
{
	::vkCmdExecuteCommands(this->handle(), 1, &commandBuffer->handle());
}

void VulkanCommandBuffer::execute(Enumerable<SharedPtr<const VulkanCommandBuffer>> commandBuffers) const
{
	auto secondaryHandles = commandBuffers | 
		std::views::transform([](auto commandBuffer) { return commandBuffer->handle(); }) | 
		std::ranges::to<Array<VkCommandBuffer>>();

	::vkCmdExecuteCommands(this->handle(), static_cast<UInt32>(secondaryHandles.size()), secondaryHandles.data());
}

void VulkanCommandBuffer::releaseSharedState() const
{
	m_impl->m_sharedResources.clear();
}