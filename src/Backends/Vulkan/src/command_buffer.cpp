#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
extern PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructure;
extern PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructures;
extern PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructure;
extern PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructure;
extern PFN_vkCmdTraceRaysKHR vkCmdTraceRays;
extern PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasks;
extern PFN_vkCmdDrawMeshTasksIndirectEXT vkCmdDrawMeshTasksIndirect;
extern PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCount;
extern PFN_vkCmdPushDataEXT vkCmdPushData;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanCommandBuffer::VulkanCommandBufferImpl {
public:
	friend class VulkanCommandBuffer;

private:
	bool m_recording{ false }, m_secondary{ false };
	const VulkanPipelineState* m_lastPipeline = nullptr;
	WeakPtr<const VulkanQueue> m_queue;
	WeakPtr<const VulkanDevice> m_device;
	bool m_canBindDescriptorHeaps = false;
	TrackedResources m_trackedResources{};
	UniquePtr<IVulkanCommandAllocatorHandle> m_allocatorHandle{};

public:
	VulkanCommandBufferImpl(const VulkanQueue& queue, bool primary) :
		m_secondary(!primary), m_queue(queue.weak_from_this()), m_device(queue.device()), 
		m_canBindDescriptorHeaps(LITEFX_FLAG_IS_SET(queue.type(), QueueType::Compute) || LITEFX_FLAG_IS_SET(queue.type(), QueueType::Graphics))
	{
	}

public:
	void begin(const VulkanCommandBuffer& commandBuffer, const VulkanRenderPass* renderPass = nullptr)
	{
		// Set the buffer into recording state.
		VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = m_secondary ? 0 : VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		// Secondary command buffers always require an inheritance info. Those continuing a render pass also inherit its rendering state.
		VkCommandBufferInheritanceInfo inheritanceInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
		VkCommandBufferInheritanceRenderingInfo renderingInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO };
		Array<VkFormat> colorFormats{};

		if (m_secondary)
			beginInfo.pInheritanceInfo = &inheritanceInfo;
		
		// If a render pass is provided, gather formats from the render targets and attachments.
		if (renderPass != nullptr)
		{
			auto frameBuffer = renderPass->activeFrameBuffer();

			if (frameBuffer == nullptr) [[unlikely]]
				throw RuntimeException("Cannot begin a secondary command buffer on an inactive render pass.");

			auto renderTargets = renderPass->renderTargets();
			colorFormats = renderTargets |
				std::views::filter([](auto& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; }) |
				std::views::transform([](auto& renderTarget) { return Vk::getFormat(renderTarget.format()); }) |
				std::ranges::to<Array<VkFormat>>();
			auto depthStencilFormats = renderTargets |
				std::views::filter([](auto& renderTarget) { return renderTarget.type() == RenderTargetType::DepthStencil; }) |
				std::views::transform([](auto& renderTarget) { return renderTarget.format(); }) |
				std::ranges::to<Array<Format>>();
			auto depthFormat = depthStencilFormats.size() > 0 && ::hasDepth(depthStencilFormats.front()) ? Vk::getFormat(depthStencilFormats.front()) : VK_FORMAT_UNDEFINED;
			auto stencilFormat = depthStencilFormats.size() > 0 && ::hasStencil(depthStencilFormats.front()) ? Vk::getFormat(depthStencilFormats.front()) : VK_FORMAT_UNDEFINED;

			// Get the multi sampling level.
			auto samples = renderTargets |
				std::views::transform([&frameBuffer](auto& renderTarget) { return Vk::getSamples(frameBuffer->image(renderTarget).samples()); }) |
				std::ranges::to<Array<VkSampleCountFlagBits>>();

			if (std::ranges::adjacent_find(samples, std::not_equal_to{ }) != samples.end()) [[unlikely]]
				throw RuntimeException("All render targets of the current render pass must use the same multi-sampling level.");

			renderingInfo.viewMask = renderPass->viewMask();
			renderingInfo.colorAttachmentCount = static_cast<UInt32>(colorFormats.size());
			renderingInfo.pColorAttachmentFormats = colorFormats.data();
			renderingInfo.depthAttachmentFormat = depthFormat;
			renderingInfo.stencilAttachmentFormat = stencilFormat;
			renderingInfo.rasterizationSamples = samples.front();

			inheritanceInfo.pNext = &renderingInfo;
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		}

		// Begin the command buffer.
		raiseIfFailed(::vkBeginCommandBuffer(m_allocatorHandle->commandBuffer(), &beginInfo), "Unable to begin command recording.");

		// Bind global descriptor heaps.
		bindDescriptorHeaps(commandBuffer);
		m_recording = true;
	}

	void end()
	{
		if (m_recording)
			raiseIfFailed(::vkEndCommandBuffer(m_allocatorHandle->commandBuffer()), "Unable to stop command recording.");

		m_recording = false;
	}

	inline void buildAccelerationStructure(const VulkanCommandBuffer& commandBuffer, VulkanBottomLevelAccelerationStructure& blas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset, bool update)
	{
		if (!m_recording)
			throw RuntimeException("Command buffers that have ended cannot record any more commands.");

		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Unable to build acceleration structure from a released device instance.");

		if (scratchBuffer == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("scratchBuffer");

		// Create new acceleration structure handle.
		UInt64 size{}, scratchSize{};
		device->computeAccelerationStructureSizes(blas, size, scratchSize, update);
		VkAccelerationStructureKHR handle{};

		VkAccelerationStructureCreateInfoKHR info = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.buffer = buffer.handle(),
			.offset = offset,
			.size = size,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
		};

		raiseIfFailed(::vkCreateAccelerationStructure(device->handle(), &info, nullptr, &handle), "Unable to update acceleration structure handle.");

		auto buildInfo = blas.buildInfo();
		auto descriptions = buildInfo | std::views::values | std::ranges::to<Array<VkAccelerationStructureGeometryKHR>>();
		auto ranges = buildInfo | std::views::keys |
			std::views::transform([](UInt32 primitives) { return VkAccelerationStructureBuildRangeInfoKHR { .primitiveCount = primitives }; }) | 
			std::ranges::to<Array<VkAccelerationStructureBuildRangeInfoKHR>>();
		auto rangePointer = ranges.data();

		VkAccelerationStructureBuildGeometryInfoKHR inputs = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			.flags = std::bit_cast<VkBuildAccelerationStructureFlagsKHR>(blas.flags()),
			.mode = update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = update ? blas.handle() : 0ull,
			.dstAccelerationStructure = handle,
			.geometryCount = static_cast<UInt32>(descriptions.size()),
			.pGeometries = descriptions.data(),
			.scratchData = {
			    .deviceAddress = scratchBuffer->virtualAddress() 
			}
		};

		::vkCmdBuildAccelerationStructures(commandBuffer.handle(), 1, &inputs, &rangePointer);

		// Store the acceleration structure handle.
		blas.updateState(device.get(), handle);

		// Store the scratch buffer.
		m_trackedResources.Resources.push_back(scratchBuffer);
	}

	inline void buildAccelerationStructure(const VulkanCommandBuffer& commandBuffer, VulkanTopLevelAccelerationStructure& tlas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset, bool update)
	{
		if (!m_recording)
			throw RuntimeException("Command buffers that have ended cannot record any more commands.");

		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Unable to build acceleration structure from a released device instance.");

		if (scratchBuffer == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("scratchBuffer");

		// Create a buffer to store the instance data.
		auto buildInfo = tlas.buildInfo();
		auto instanceBuffer = device->factory().createBuffer(BufferType::Storage, ResourceHeap::Dynamic, sizeof(VkAccelerationStructureInstanceKHR) * buildInfo.size(), 1, ResourceUsage::AccelerationStructureBuildInput);

		// Map the instance buffer.
		instanceBuffer->map(buildInfo.data(), sizeof(VkAccelerationStructureInstanceKHR) * buildInfo.size());

		VkAccelerationStructureBuildRangeInfoKHR ranges { static_cast<UInt32>(tlas.instances().size()) };
		auto rangePointer = &ranges;

		// Create new acceleration structure handle.
		UInt64 size{}, scratchSize{};
		device->computeAccelerationStructureSizes(tlas, size, scratchSize, update);
		VkAccelerationStructureKHR handle{};

		VkAccelerationStructureCreateInfoKHR info = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.buffer = buffer.handle(),
			.offset = offset,
			.size = size,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
		};

		raiseIfFailed(::vkCreateAccelerationStructure(device->handle(), &info, nullptr, &handle), "Unable to update acceleration structure handle.");

		// Setup TLAS bindings.
		VkAccelerationStructureGeometryInstancesDataKHR instanceInfo = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.arrayOfPointers = false,
			.data = {
				.deviceAddress = instanceBuffer->virtualAddress()
			}
		};

		VkAccelerationStructureGeometryKHR geometryInfo = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
			.geometry = {
				.instances = instanceInfo
			}
		};

		VkAccelerationStructureBuildGeometryInfoKHR inputs = {
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			.flags = std::bit_cast<VkBuildAccelerationStructureFlagsKHR>(tlas.flags()),
			.mode = update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.srcAccelerationStructure = update ? tlas.handle() : 0ull,
			.dstAccelerationStructure = handle,
			.geometryCount = 1u,
			.pGeometries = &geometryInfo,
			.scratchData = {
				.deviceAddress = scratchBuffer->virtualAddress()
			}
		};

		::vkCmdBuildAccelerationStructures(commandBuffer.handle(), 1, &inputs, &rangePointer);

		// Store the acceleration structure handle.
		tlas.updateState(device.get(), handle);

		// Store the scratch buffer.
		m_trackedResources.Resources.emplace_back(instanceBuffer);
		m_trackedResources.Resources.emplace_back(scratchBuffer);
	}

	inline void bindDescriptorHeaps(const VulkanCommandBuffer& parent)
	{
		// Bind the global descriptor heaps.
		if (m_canBindDescriptorHeaps)
		{
			auto device = m_device.lock();

			if (device != nullptr)
				device->bindGlobalDescriptorHeaps(parent);
			else [[unlikely]]
				throw RuntimeException("Cannot bind descriptor heaps on a released device instance.");
		}
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanQueue& queue, bool primary) :
	Resource<VkCommandBuffer>(nullptr), m_impl(queue, primary)
{
	m_impl->m_allocatorHandle = queue.acquireCommandAllocator(!primary);
	this->handle() = m_impl->m_allocatorHandle->commandBuffer();
	m_impl->begin(*this);
}

VulkanCommandBuffer::VulkanCommandBuffer(const VulkanQueue& queue, const VulkanRenderPass& renderPass) :
	Resource<VkCommandBuffer>(nullptr), m_impl(queue, false)
{
	m_impl->m_allocatorHandle = queue.acquireCommandAllocator(true);
	this->handle() = m_impl->m_allocatorHandle->commandBuffer();
	m_impl->begin(*this, &renderPass);
}

void VulkanCommandBuffer::end() const 
{
	m_impl->end();
}

SharedPtr<const VulkanQueue> VulkanCommandBuffer::queue() const noexcept
{
	return m_impl->m_queue.lock();
}

const ICommandBuffer::TrackedResources& VulkanCommandBuffer::trackedResources() const noexcept 
{
	return m_impl->m_trackedResources;
}

void VulkanCommandBuffer::track(SharedPtr<const IBuffer> buffer) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers may only start resource tracking if they are currently recording.");

	if (buffer != nullptr)
		m_impl->m_trackedResources.Resources.push_back(buffer);
}

void VulkanCommandBuffer::track(SharedPtr<const IImage> image) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers may only start resource tracking if they are currently recording.");

	if (image != nullptr)
		m_impl->m_trackedResources.Resources.push_back(image);
}

void VulkanCommandBuffer::track(SharedPtr<const ISampler> sampler) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers may only start resource tracking if they are currently recording.");

	if (sampler != nullptr)
		m_impl->m_trackedResources.Resources.push_back(sampler);
}

void VulkanCommandBuffer::track(UniquePtr<const IDescriptorSet>&& descriptorSet) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers may only start resource tracking if they are currently recording.");

	if (descriptorSet != nullptr)
		m_impl->m_trackedResources.DescriptorSets.push_back(std::move(descriptorSet));
}

void VulkanCommandBuffer::track(UniquePtr<const IDescriptorRange>&& descriptorRange) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers may only start resource tracking if they are currently recording.");

	if (descriptorRange != nullptr)
		m_impl->m_trackedResources.DescriptorRanges.push_back(std::move(descriptorRange));
}

bool VulkanCommandBuffer::secondary() const noexcept
{
	return m_impl->m_secondary;
}

bool VulkanCommandBuffer::recording() const noexcept
{
	return m_impl->m_recording;
}

void VulkanCommandBuffer::setViewports(Span<const IViewport*> viewports) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	auto vps = viewports |
		std::views::transform([](const auto& viewport) { return VkViewport{ .x = viewport->getRectangle().x(), .y = viewport->getRectangle().y(), .width = viewport->getRectangle().width(), .height = viewport->getRectangle().height(), .minDepth = viewport->getMinDepth(), .maxDepth = viewport->getMaxDepth() }; }) |
		std::ranges::to<Array<VkViewport>>();

	::vkCmdSetViewportWithCount(this->handle(), static_cast<UInt32>(vps.size()), vps.data());
}

void VulkanCommandBuffer::setViewports(const IViewport* viewport) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	auto vp = VkViewport{ .x = viewport->getRectangle().x(), .y = viewport->getRectangle().y(), .width = viewport->getRectangle().width(), .height = viewport->getRectangle().height(), .minDepth = viewport->getMinDepth(), .maxDepth = viewport->getMaxDepth() };
	::vkCmdSetViewportWithCount(this->handle(), 1, &vp);
}

void VulkanCommandBuffer::setScissors(Span<const IScissor*> scissors) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	auto scs = scissors |
		std::views::transform([](const auto& scissor) { return VkRect2D{ { .x = static_cast<Int32>(scissor->getRectangle().x()), .y = static_cast<Int32>(scissor->getRectangle().y())}, { .width = static_cast<UInt32>(scissor->getRectangle().width()), .height = static_cast<UInt32>(scissor->getRectangle().height())} }; }) |
		std::ranges::to<Array<VkRect2D>>();

	::vkCmdSetScissorWithCount(this->handle(), static_cast<UInt32>(scs.size()), scs.data());
}

void VulkanCommandBuffer::setScissors(const IScissor* scissor) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	auto s = VkRect2D{ { .x = static_cast<Int32>(scissor->getRectangle().x()), .y = static_cast<Int32>(scissor->getRectangle().y())},  { .width = static_cast<UInt32>(scissor->getRectangle().width()), .height = static_cast<UInt32>(scissor->getRectangle().height())} };
	::vkCmdSetScissorWithCount(this->handle(), 1, &s);
}

void VulkanCommandBuffer::setBlendFactors(const Vector4f& blendFactors) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdSetBlendConstants(this->handle(), blendFactors.elements());
}

void VulkanCommandBuffer::setStencilRef(UInt32 stencilRef) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdSetStencilReference(this->handle(), VK_STENCIL_FACE_FRONT_AND_BACK, stencilRef);
}

void VulkanCommandBuffer::setDepthBounds(Float minBounds, Float maxBounds) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdSetDepthBounds(this->handle(), minBounds, maxBounds);
}

UInt64 VulkanCommandBuffer::submit() const 
{
	auto queue = m_impl->m_queue.lock();

	if (queue == nullptr) [[unlikely]]
		throw RuntimeException("Unable to submit command buffer to a released device instance.");

	if (this->secondary())
		throw RuntimeException("A secondary command buffer cannot be directly submitted to a command queue and must be executed on a primary command buffer instead.");

	return queue->submit(this->shared_from_this());
}

UniquePtr<VulkanBarrier> VulkanCommandBuffer::makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot create barrier on a released device instance.");

	return device->makeBarrier(syncBefore, syncAfter);
}

void VulkanCommandBuffer::barrier(const VulkanBarrier& barrier) const noexcept
{
	assert(m_impl->m_recording);
	barrier.execute(*this);
}

void VulkanCommandBuffer::transfer(const IVulkanBuffer& source, const IVulkanBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

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

void VulkanCommandBuffer::transfer(const void* const data, size_t size, const IVulkanBuffer& target, UInt32 targetElement, UInt32 elements) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot create staging buffer on a released device instance.");

	auto stagingBuffer = device->factory().createBuffer(target.type(), ResourceHeap::Staging, target.elementSize(), elements);
	stagingBuffer->map(data, size, 0);

	this->transfer(stagingBuffer, target, 0, targetElement, elements);
}

void VulkanCommandBuffer::transfer(Span<const void* const> data, size_t elementSize, const IVulkanBuffer& target, UInt32 firstElement) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot create staging buffer on a released device instance.");

	auto elements = static_cast<UInt32>(data.size());
	auto stagingBuffer = device->factory().createBuffer(target.type(), ResourceHeap::Staging, target.elementSize(), elements);
	stagingBuffer->map(data, elementSize, 0);

	this->transfer(stagingBuffer, target, 0, firstElement, elements);
}

void VulkanCommandBuffer::transfer(const IVulkanBuffer& source, const IVulkanImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	if (!LITEFX_FLAG_IS_SET(target.usage(), ResourceUsage::TransferDestination)) [[unlikely]]
		throw InvalidArgumentException("target", "The target image's usage does not allow for it to be used as a transfer destination.");

	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < firstSubresource + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target image has only {0} sub-resources, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, firstSubresource);

	Array<VkBufferImageCopy> copyInfos(elements);
	std::ranges::generate(copyInfos, [&, i = firstSubresource]() mutable {
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

	::vkCmdCopyBufferToImage(this->handle(), std::as_const(source).handle(), std::as_const(target).handle(), VK_IMAGE_LAYOUT_GENERAL, static_cast<UInt32>(copyInfos.size()), copyInfos.data());
}

void VulkanCommandBuffer::transfer(const void* const data, size_t size, const IVulkanImage& target, UInt32 subresource) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot create staging buffer on a released device instance.");

	if (!LITEFX_FLAG_IS_SET(target.usage(), ResourceUsage::TransferDestination)) [[unlikely]]
		throw InvalidArgumentException("target", "The target image's usage does not allow for it to be used as a transfer destination.");

	auto stagingBuffer = device->factory().createBuffer(BufferType::Other, ResourceHeap::Staging, size);
	stagingBuffer->map(data, size, 0);

	this->transfer(stagingBuffer, target, 0, subresource, 1);
}

void VulkanCommandBuffer::transfer(Span<const void* const> data, size_t elementSize, const IVulkanImage& target, UInt32 firstSubresource, UInt32 subresources) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot create staging buffer on a released device instance.");

	if (!LITEFX_FLAG_IS_SET(target.usage(), ResourceUsage::TransferDestination)) [[unlikely]]
		throw InvalidArgumentException("target", "The target image's usage does not allow for it to be used as a transfer destination.");

	auto elements = static_cast<UInt32>(data.size());
	auto stagingBuffer = device->factory().createBuffer(BufferType::Other, ResourceHeap::Staging, elementSize, elements);
	stagingBuffer->map(data, elementSize, 0);

	this->transfer(stagingBuffer, target, 0, firstSubresource, subresources);
}

void VulkanCommandBuffer::transfer(const IVulkanImage& source, const IVulkanImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	if (!LITEFX_FLAG_IS_SET(source.usage(), ResourceUsage::TransferSource)) [[unlikely]]
		throw InvalidArgumentException("source", "The source image's usage does not allow for it to be used as a transfer source.");

	if (!LITEFX_FLAG_IS_SET(target.usage(), ResourceUsage::TransferDestination)) [[unlikely]]
		throw InvalidArgumentException("target", "The target image's usage does not allow for it to be used as a transfer destination.");

	if (source.elements() < sourceSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, sourceSubresource);

	if (target.elements() < targetSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resources {2} has been requested.", target.elements(), subresources, targetSubresource);

	Array<VkImageCopy> copyInfos(subresources);
	std::ranges::generate(copyInfos, [&, i = 0]() mutable {
		UInt32 sourceRsc = sourceSubresource + i, sourceLayer = 0, sourceLevel = 0, sourcePlane = 0;
		UInt32 targetRsc = targetSubresource + i, targetLayer = 0, targetLevel = 0, targetPlane = 0;
		source.resolveSubresource(sourceRsc, sourcePlane, sourceLayer, sourceLevel);
		target.resolveSubresource(targetRsc, targetPlane, targetLayer, targetLevel);
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

	::vkCmdCopyImage(this->handle(), std::as_const(source).handle(), VK_IMAGE_LAYOUT_GENERAL, std::as_const(target).handle(), VK_IMAGE_LAYOUT_GENERAL, static_cast<UInt32>(copyInfos.size()), copyInfos.data());
}

void VulkanCommandBuffer::transfer(const IVulkanImage& source, const IVulkanBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	if (!LITEFX_FLAG_IS_SET(source.usage(), ResourceUsage::TransferSource)) [[unlikely]]
		throw InvalidArgumentException("source", "The source image's usage does not allow for it to be used as a transfer source.");

	if (source.elements() < firstSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, firstSubresource);

	if (target.elements() <= targetElement + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), subresources, targetElement);
	
	// Create a copy command and add it to the command buffer.
	Array<VkBufferImageCopy> copyInfos(subresources);
	std::ranges::generate(copyInfos, [&, i = targetElement]() mutable {
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

	::vkCmdCopyImageToBuffer(this->handle(), std::as_const(source).handle(), VK_IMAGE_LAYOUT_GENERAL, std::as_const(target).handle(), static_cast<UInt32>(copyInfos.size()), copyInfos.data());
}

void VulkanCommandBuffer::transfer(const SharedPtr<const IVulkanBuffer>& source, const IVulkanBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const
{
	this->transfer(*source, target, sourceElement, targetElement, elements);
	m_impl->m_trackedResources.Resources.push_back(source);
}

void VulkanCommandBuffer::transfer(const SharedPtr<const IVulkanBuffer>& source, const IVulkanImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const
{
	this->transfer(*source, target, sourceElement, firstSubresource, elements);
	m_impl->m_trackedResources.Resources.push_back(source);
}

void VulkanCommandBuffer::transfer(const SharedPtr<const IVulkanImage>& source, const IVulkanImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const
{
	this->transfer(*source, target, sourceSubresource, targetSubresource, subresources);
	m_impl->m_trackedResources.Resources.push_back(source);
}

void VulkanCommandBuffer::transfer(const SharedPtr<const IVulkanImage>& source, const IVulkanBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const
{
	this->transfer(*source, target, firstSubresource, targetElement, subresources);
	m_impl->m_trackedResources.Resources.push_back(source);
}

void VulkanCommandBuffer::use(const VulkanPipelineState& pipeline) const noexcept
{
	assert(m_impl->m_recording);
	m_impl->m_lastPipeline = &pipeline;
	pipeline.use(*this);
}

void VulkanCommandBuffer::bind(const VulkanDescriptorSet& descriptorSet) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot bind descriptor set on a released device instance.");

	if (m_impl->m_lastPipeline) [[likely]]
		device->bindDescriptorSet(*this, descriptorSet, *m_impl->m_lastPipeline);
	else
		throw RuntimeException("No pipeline has been used on the command buffer before attempting to bind the descriptor set.");
}

void VulkanCommandBuffer::bind(Span<const VulkanDescriptorSet*> descriptorSets) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot bind descriptor set on a released device instance.");

	if (m_impl->m_lastPipeline) [[likely]]
		std::ranges::for_each(descriptorSets | std::views::filter([](auto descriptorSet) { return descriptorSet != nullptr; }), [&](auto descriptorSet) { device->bindDescriptorSet(*this, *descriptorSet, *m_impl->m_lastPipeline); });
	else
		throw RuntimeException("No pipeline has been used on the command buffer before attempting to bind the descriptor set.");
}

void VulkanCommandBuffer::bind(const VulkanDescriptorSet& descriptorSet, const VulkanPipelineState& pipeline) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot bind descriptor set on a released device instance.");

	device->bindDescriptorSet(*this, descriptorSet, pipeline);
}

void VulkanCommandBuffer::bind(Span<const VulkanDescriptorSet*> descriptorSets, const VulkanPipelineState& pipeline) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot transfer buffers on a released device instance.");

	std::ranges::for_each(descriptorSets | std::views::filter([](auto descriptorSet) { return descriptorSet != nullptr; }), [&](auto descriptorSet) { device->bindDescriptorSet(*this, *descriptorSet, pipeline); });
}

void VulkanCommandBuffer::bind(const IVulkanVertexBuffer& buffer) const noexcept
{
	assert(m_impl->m_recording);
	constexpr VkDeviceSize offset { 0 };
	::vkCmdBindVertexBuffers(this->handle(), buffer.layout().binding(), 1, &buffer.handle(), &offset);
}

void VulkanCommandBuffer::bind(const IVulkanIndexBuffer& buffer) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdBindIndexBuffer(this->handle(), buffer.handle(), 0, buffer.layout().indexType() == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBuffer::dispatch(const Vector3u& threadCount) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDispatch(this->handle(), threadCount.x(), threadCount.y(), threadCount.z());
}

void VulkanCommandBuffer::dispatchIndirect(const IVulkanBuffer& batchBuffer, UInt32 /*batchCount*/, UInt64 offset) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDispatchIndirect(this->handle(), batchBuffer.handle(), offset);
}

void VulkanCommandBuffer::dispatchMesh(const Vector3u& threadCount) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDrawMeshTasks(this->handle(), threadCount.x(), threadCount.y(), threadCount.z());
}

void VulkanCommandBuffer::dispatchMeshIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDrawMeshTasksIndirect(this->handle(), batchBuffer.handle(), offset, batchCount, static_cast<UInt32>(batchBuffer.elementSize()));
}

void VulkanCommandBuffer::dispatchMeshIndirect(const IVulkanBuffer& batchBuffer, const IVulkanBuffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDrawMeshTasksIndirectCount(this->handle(), batchBuffer.handle(), offset, countBuffer.handle(), countOffset, std::min(maxBatches, static_cast<UInt32>(batchBuffer.alignedElementSize() / sizeof(IndirectDispatchBatch))), sizeof(IndirectDispatchBatch));
}

void VulkanCommandBuffer::draw(UInt32 vertices, UInt32 instances, UInt32 firstVertex, UInt32 firstInstance) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDraw(this->handle(), vertices, instances, firstVertex, firstInstance);
}

void VulkanCommandBuffer::drawIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDrawIndirect(this->handle(), batchBuffer.handle(), offset, batchCount, static_cast<UInt32>(batchBuffer.elementSize()));
}

void VulkanCommandBuffer::drawIndirect(const IVulkanBuffer& batchBuffer, const IVulkanBuffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDrawIndirectCount(this->handle(), batchBuffer.handle(), offset, countBuffer.handle(), countOffset, std::min(maxBatches, static_cast<UInt32>(batchBuffer.alignedElementSize() / sizeof(IndirectBatch))), sizeof(IndirectBatch));
}

void VulkanCommandBuffer::drawIndexed(UInt32 indices, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDrawIndexed(this->handle(), indices, instances, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::drawIndexedIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDrawIndexedIndirect(this->handle(), batchBuffer.handle(), offset, batchCount, static_cast<UInt32>(batchBuffer.elementSize()));
}

void VulkanCommandBuffer::drawIndexedIndirect(const IVulkanBuffer& batchBuffer, const IVulkanBuffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept
{
	assert(m_impl->m_recording);
	::vkCmdDrawIndexedIndirectCount(this->handle(), batchBuffer.handle(), offset, countBuffer.handle(), countOffset, std::min(maxBatches, static_cast<UInt32>(batchBuffer.alignedElementSize() / sizeof(IndirectIndexedBatch))), sizeof(IndirectIndexedBatch));
}

void VulkanCommandBuffer::pushConstants(const VulkanPushConstantsLayout& layout, const void* const memory) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	std::ranges::for_each(layout.ranges(), [&](const auto& range) {
		VkPushDataInfoEXT pushData { 
			.sType = VK_STRUCTURE_TYPE_PUSH_DATA_INFO_EXT, 
			.offset = range->offset(), 
			.data = { .address = memory, .size = range->size() } 
		};

		vkCmdPushData(this->handle(), &pushData);
	});
}

void VulkanCommandBuffer::writeTimingEvent(const SharedPtr<const TimingEvent>& timingEvent) const
{
	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot write timing event on a released device instance.");

	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	::vkCmdWriteTimestamp2(this->handle(), VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, device->swapChain().timestampQueryPool(), timingEvent->queryId());
}

void VulkanCommandBuffer::execute(const SharedPtr<const VulkanCommandBuffer>& commandBuffer) const
{
	if (!commandBuffer->secondary()) [[unlikely]]
		throw InvalidArgumentException("commandBuffer", "Unable to execute a primary command buffer on another command buffer.");

	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	// End and execute the command buffer.
	commandBuffer->end();
	::vkCmdExecuteCommands(this->handle(), 1, &commandBuffer->handle());
	m_impl->m_trackedResources.SecondaryCommandBuffers.push_back(commandBuffer);
}

void VulkanCommandBuffer::execute(Enumerable<SharedPtr<const VulkanCommandBuffer>> commandBuffers) const
{
	auto buffers = Enumerable<SharedPtr<const VulkanCommandBuffer>> | std::ranges::to<std::vector>();

	if (std::ranges::any_of(buffers, [](const auto& commandBuffer) { return !commandBuffer->secondary(); })) [[unlikely]]
		throw InvalidArgumentException("commandBuffers", "Unable to execute a primary command buffer on another command buffer. At least one of the provided command buffers is a primary command buffer.");

	if (!m_impl->m_recording)
		throw RuntimeException("Command buffers that have ended cannot record any more commands.");

	auto secondaryHandles = buffers
		| std::views::transform([](const SharedPtr<const VulkanCommandBuffer>& commandBuffer) { 
				// End the command buffer and return its handle.
				commandBuffer->end();
				return commandBuffer->handle(); 
			})
		| std::ranges::to<Array<VkCommandBuffer>>();

	::vkCmdExecuteCommands(this->handle(), static_cast<UInt32>(secondaryHandles.size()), secondaryHandles.data());
	m_impl->m_trackedResources.SecondaryCommandBuffers.append_range(buffers);
}

void VulkanCommandBuffer::releaseSharedState() const
{
	m_impl->m_trackedResources = {};
}

void VulkanCommandBuffer::buildAccelerationStructure(VulkanBottomLevelAccelerationStructure& blas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(*this, blas, scratchBuffer, buffer, offset, false);
}

void VulkanCommandBuffer::buildAccelerationStructure(VulkanTopLevelAccelerationStructure& tlas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(*this, tlas, scratchBuffer, buffer, offset, false);
}

void VulkanCommandBuffer::updateAccelerationStructure(VulkanBottomLevelAccelerationStructure& blas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(*this, blas, scratchBuffer, buffer, offset, true);
}

void VulkanCommandBuffer::updateAccelerationStructure(VulkanTopLevelAccelerationStructure& tlas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(*this, tlas, scratchBuffer, buffer, offset, true);
}

void VulkanCommandBuffer::copyAccelerationStructure(const VulkanBottomLevelAccelerationStructure& from, const VulkanBottomLevelAccelerationStructure& to, bool compress) const noexcept
{
	assert(m_impl->m_recording);

	VkCopyAccelerationStructureInfoKHR copyInfo = {
		.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR,
		.src = from.handle(),
		.dst = to.handle(),
		.mode = compress ? VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR : VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR
	};

	::vkCmdCopyAccelerationStructure(this->handle(), &copyInfo);
}

void VulkanCommandBuffer::copyAccelerationStructure(const VulkanTopLevelAccelerationStructure& from, const VulkanTopLevelAccelerationStructure& to, bool compress) const noexcept
{
	assert(m_impl->m_recording);

	VkCopyAccelerationStructureInfoKHR copyInfo = {
		.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR,
		.src = from.handle(),
		.dst = to.handle(),
		.mode = compress ? VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR : VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR
	};

	::vkCmdCopyAccelerationStructure(this->handle(), &copyInfo);
}

void VulkanCommandBuffer::traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IVulkanBuffer& rayGenerationShaderBindingTable, const IVulkanBuffer* missShaderBindingTable, const IVulkanBuffer* hitShaderBindingTable, const IVulkanBuffer* callableShaderBindingTable) const noexcept
{
	assert(m_impl->m_recording);

	VkStridedDeviceAddressRegionKHR raygen = {
		.deviceAddress = rayGenerationShaderBindingTable.virtualAddress() + offsets.RayGenerationGroupOffset,
		.stride = offsets.RayGenerationGroupStride,
		.size = offsets.RayGenerationGroupSize
	};

	VkStridedDeviceAddressRegionKHR miss { }, hit { }, callable { };

	if (missShaderBindingTable != nullptr)
	{
		miss.deviceAddress = missShaderBindingTable->virtualAddress() + offsets.MissGroupOffset;
		miss.stride = offsets.MissGroupStride;
		miss.size = offsets.MissGroupSize;
	}

	if (hitShaderBindingTable != nullptr)
	{
		hit.deviceAddress = hitShaderBindingTable->virtualAddress() + offsets.HitGroupOffset;
		hit.stride = offsets.HitGroupStride;
		hit.size = offsets.HitGroupSize;
	}

	if (callableShaderBindingTable != nullptr)
	{
		callable.deviceAddress = callableShaderBindingTable->virtualAddress() + offsets.CallableGroupOffset;
		callable.stride = offsets.CallableGroupStride;
		callable.size = offsets.CallableGroupSize;
	}

	::vkCmdTraceRays(this->handle(), &raygen, &miss, &hit, &callable, width, height, depth);
}