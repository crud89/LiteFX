#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanCommandBuffer::VulkanCommandBufferImpl : public Implement<VulkanCommandBuffer> {
public:
	friend class VulkanCommandBuffer;

private:
	bool m_recording{ false };
	Optional<VkCommandPool> m_commandPool;

public:
	VulkanCommandBufferImpl(VulkanCommandBuffer* parent) :
		base(parent)
	{
	}

public:
	VkCommandBuffer initialize(const bool& primary)
	{
		// Secondary command buffers have their own command pool.
		if (!primary)
		{
			VkCommandPoolCreateInfo poolInfo = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
				.queueFamilyIndex = m_parent->parent().queueId()
			};

			VkCommandPool commandPool;
			raiseIfFailed<RuntimeException>(::vkCreateCommandPool(m_parent->getDevice()->handle(), &poolInfo, nullptr, &commandPool), "Unable to create command pool.");
			m_commandPool = commandPool;
		}

		// Create the command buffer.
		VkCommandBufferAllocateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		bufferInfo.commandPool = primary ? m_parent->parent().commandPool() : m_commandPool.value();
		bufferInfo.commandBufferCount = 1;

		VkCommandBuffer buffer;
		raiseIfFailed<RuntimeException>(::vkAllocateCommandBuffers(m_parent->getDevice()->handle(), &bufferInfo, &buffer), "Unable to allocate command buffer.");

		return buffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanQueue& queue, const bool& begin, const bool& primary) :
	m_impl(makePimpl<VulkanCommandBufferImpl>(this)), VulkanRuntimeObject<VulkanQueue>(queue, queue.getDevice()), Resource<VkCommandBuffer>(nullptr)
{
	if (!queue.isBound())
		throw InvalidArgumentException("You must bind the queue before creating a command buffer from it.");

	this->handle() = m_impl->initialize(primary);

	if (begin)
		this->begin();
}

VulkanCommandBuffer::~VulkanCommandBuffer() noexcept
{
	if (!m_impl->m_commandPool.has_value())
		::vkFreeCommandBuffers(this->getDevice()->handle(), this->parent().commandPool(), 1, &this->handle());
	else
	{
		::vkFreeCommandBuffers(this->getDevice()->handle(), m_impl->m_commandPool.value(), 1, &this->handle());
		::vkDestroyCommandPool(this->getDevice()->handle(), m_impl->m_commandPool.value(), nullptr);
	}
}

void VulkanCommandBuffer::begin() const
{
	// Set the buffer into recording state.
	VkCommandBufferBeginInfo beginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	raiseIfFailed<RuntimeException>(::vkBeginCommandBuffer(this->handle(), &beginInfo), "Unable to begin command recording.");
	
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

	raiseIfFailed<RuntimeException>(::vkBeginCommandBuffer(this->handle(), &beginInfo), "Unable to begin command recording.");

	m_impl->m_recording = true;
}

void VulkanCommandBuffer::end() const
{
	// End recording.
	if (m_impl->m_recording)
		raiseIfFailed<RuntimeException>(::vkEndCommandBuffer(this->handle()), "Unable to stop command recording.");

	m_impl->m_recording = false;
}

void VulkanCommandBuffer::generateMipMaps(IVulkanImage& image) noexcept
{
	// Use a native barrier for improved performance (we update it for each level).
	VkImageMemoryBarrier barrier {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = std::as_const(image).handle(),
		.subresourceRange = VkImageSubresourceRange { .aspectMask = image.aspectMask() }
	};

	auto layout = ::getImageLayout(image.state(0));

	for (UInt32 layer(0); layer < image.layers(); ++layer)
	{
		Int32 mipWidth = static_cast<Int32>(image.extent().width());
		Int32 mipHeight = static_cast<Int32>(image.extent().height());
		Int32 mipDepth = static_cast<Int32>(image.extent().depth());

		for (UInt32 level(1); level < image.levels(); ++level)
		{
			// Transition the previous level to transfer source.
			barrier.subresourceRange.aspectMask = image.aspectMask();
			barrier.subresourceRange.baseArrayLayer = layer;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.baseMipLevel = level - 1;
			barrier.subresourceRange.levelCount = 1;
			barrier.oldLayout = layout;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			::vkCmdPipelineBarrier(this->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			// Blit the image of the previous level into the current level.
			VkImageBlit blit{
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
	}

	// Finally, transition all the levels back to the original layout.
	barrier.subresourceRange.aspectMask = image.aspectMask();
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = image.layers();
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = image.levels();
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = layout;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	::vkCmdPipelineBarrier(this->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanCommandBuffer::barrier(const VulkanBarrier& barrier, const bool& invert) const noexcept
{
	if (invert)
		barrier.executeInverse(*this);
	else
		barrier.execute(*this);
}

void VulkanCommandBuffer::transfer(const IVulkanBuffer& source, const IVulkanBuffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);
	
	VkBufferCopy copyInfo {
		.srcOffset = sourceElement * source.alignedElementSize(),
		.dstOffset = targetElement * target.alignedElementSize(),
		.size      = elements      * source.alignedElementSize()
	};

	::vkCmdCopyBuffer(this->handle(), source.handle(), target.handle(), 1, &copyInfo);
}

void VulkanCommandBuffer::transfer(const IVulkanBuffer& source, const IVulkanImage& target, const UInt32& sourceElement, const UInt32& firstSubresource, const UInt32& elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < firstSubresource + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("The target image has only {0} sub-resources, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, firstSubresource);
	
	// Create a copy command and add it to the command buffer.
	Array<VkBufferImageCopy> copyInfos(elements);

	std::ranges::generate(copyInfos, [&, this, i = firstSubresource]() mutable {
		UInt32 subresource = i++, layer = 0, level = 0, plane = 0;
		target.resolveSubresource(subresource, plane, layer, level);

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

	::vkCmdCopyBufferToImage(this->handle(), source.handle(), target.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<UInt32>(copyInfos.size()), copyInfos.data());
}

void VulkanCommandBuffer::transfer(const IVulkanImage& source, const IVulkanImage& target, const UInt32& sourceSubresource, const UInt32& targetSubresource, const UInt32& subresources) const
{
	if (source.elements() < sourceSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, sourceSubresource);

	if (target.elements() < targetSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The target image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resources {2} has been requested.", target.elements(), subresources, targetSubresource);

	// Create a copy command and add it to the command buffer.
	Array<VkImageCopy> copyInfos(subresources);

	std::ranges::generate(copyInfos, [&, this, i = 0]() mutable {
		UInt32 sourceRsc = sourceSubresource + i, sourceLayer = 0, sourceLevel = 0, sourcePlane = 0;
		UInt32 targetRsc = sourceSubresource + i, targetLayer = 0, targetLevel = 0, targetPlane = 0;
		source.resolveSubresource(sourceRsc, sourceLayer, sourceLevel, sourcePlane);
		target.resolveSubresource(targetRsc, targetLayer, targetLevel, targetPlane);
		i++;

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

	::vkCmdCopyImage(this->handle(), source.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<UInt32>(copyInfos.size()), copyInfos.data());
}

void VulkanCommandBuffer::transfer(const IVulkanImage& source, const IVulkanBuffer& target, const UInt32& firstSubresource, const UInt32& targetElement, const UInt32& subresources) const
{
	if (source.elements() < firstSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, firstSubresource);

	if (target.elements() <= targetElement + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), subresources, targetElement);
	
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

	::vkCmdCopyImageToBuffer(this->handle(), source.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target.handle(), static_cast<UInt32>(copyInfos.size()), copyInfos.data());
}

void VulkanCommandBuffer::use(const VulkanPipelineState& pipeline) const noexcept
{
	pipeline.use(*this);
}

void VulkanCommandBuffer::bind(const VulkanDescriptorSet& descriptorSet) const noexcept
{
	descriptorSet.parent().parent().parent().bind(*this, descriptorSet);
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

void VulkanCommandBuffer::draw(const UInt32& vertices, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const noexcept
{
	::vkCmdDraw(this->handle(), vertices, instances, firstVertex, firstInstance);
}

void VulkanCommandBuffer::drawIndexed(const UInt32& indices, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const noexcept
{
	::vkCmdDrawIndexed(this->handle(), indices, instances, firstIndex, vertexOffset, firstInstance);
}