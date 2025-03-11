#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12CommandBuffer::DirectX12CommandBufferImpl {
public:
	friend class DirectX12CommandBuffer;

private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	bool m_recording{ false }, m_secondary{ false };
	WeakPtr<const DirectX12Queue> m_queue;
	WeakPtr<const DirectX12Device> m_device;
	Array<SharedPtr<const IStateResource>> m_sharedResources;
	const DirectX12PipelineState* m_lastPipeline = nullptr;
	ComPtr<ID3D12CommandSignature> m_dispatchSignature, m_drawSignature, m_drawIndexedSignature, m_dispatchMeshSignature;
	bool m_canBindDescriptorHeaps = false;

public:
	DirectX12CommandBufferImpl(const DirectX12Queue& queue) :
		m_queue(queue.weak_from_this()), m_device(queue.device()), m_canBindDescriptorHeaps(queue.type() == QueueType::Compute || queue.type() == QueueType::Graphics)
	{
	}

public:
	ComPtr<ID3D12GraphicsCommandList7> initialize(const DirectX12Queue& queue, bool begin, bool primary)
	{
		// Get the parent device instance.
		auto device = queue.device();
		
		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot initialize command queue on a released device instance.");

		// Store the command signatures for indirect drawing.
		device->indirectDrawSignatures(m_dispatchSignature, m_dispatchMeshSignature, m_drawSignature, m_drawIndexedSignature);

		// Create a command allocator.
		D3D12_COMMAND_LIST_TYPE type{};

		m_secondary = !primary;
		if (m_secondary)
			type = D3D12_COMMAND_LIST_TYPE_BUNDLE;
		else
		{
			switch (queue.type())
			{
			case QueueType::Compute: type = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
			case QueueType::Transfer: type = D3D12_COMMAND_LIST_TYPE_COPY; break;
			default:
			case QueueType::Graphics: type = D3D12_COMMAND_LIST_TYPE_DIRECT; break;
			}
		}

		raiseIfFailed(device->handle()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_commandAllocator)), "Unable to create command allocator for command buffer.");

		// Create the actual command list.
		ComPtr<ID3D12GraphicsCommandList7> commandList;

		m_recording = begin;

		if (begin)
			raiseIfFailed(device->handle()->CreateCommandList(0, type, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)), "Unable to create command list for command buffer.");
		else
			raiseIfFailed(device->handle()->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList)), "Unable to create command list for command buffer.");

		return commandList;
	}

	void reset(const DirectX12CommandBuffer& commandBuffer)
	{
		raiseIfFailed(m_commandAllocator->Reset(), "Unable to reset command allocator.");
		raiseIfFailed(commandBuffer.handle()->Reset(m_commandAllocator.Get(), nullptr), "Unable to reset command list.");
		m_recording = true;

		// If it was possible to reset the command buffer, we can also safely release shared resources from previous recordings.
		m_sharedResources.clear();
	}

	inline void bindDescriptorHeaps(const DirectX12CommandBuffer& commandBuffer)
	{
		if (m_canBindDescriptorHeaps)
		{
			auto device = m_device.lock();

			if (device == nullptr) [[unlikely]]
				throw RuntimeException("Cannot bind descriptor heaps on a released device instance.");

			device->bindGlobalDescriptorHeaps(commandBuffer);
		}
	}

	inline void buildAccelerationStructure(const DirectX12CommandBuffer& commandBuffer, DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset, bool update)
	{
		if (scratchBuffer == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("scratchBuffer");

		auto descriptions = blas.buildInfo();

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasDesc = {
			.DestAccelerationStructureData = buffer.virtualAddress() + offset,
			.Inputs = {
				.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
				.Flags = std::bit_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(blas.flags()),
				.NumDescs = static_cast<UInt32>(descriptions.size()),
				.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
				.pGeometryDescs = descriptions.data()
			},
			.SourceAccelerationStructureData = update ? blas.buffer()->virtualAddress() : 0ull,
			.ScratchAccelerationStructureData = scratchBuffer->virtualAddress()
		};

		if (update)
			blasDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

		// Build the acceleration structure.
		commandBuffer.handle()->BuildRaytracingAccelerationStructure(&blasDesc, 0, nullptr);

		// Store the scratch buffer.
		m_sharedResources.push_back(scratchBuffer);
	}

	inline void buildAccelerationStructure(const DirectX12CommandBuffer& commandBuffer, DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset, bool update)
	{
		// Check if the device is still valid.
		auto device = m_device.lock();

		if (device == nullptr) [[unlikely]]
			throw RuntimeException("Cannot build acceleration struction on a released device instance.");

		if (scratchBuffer == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("scratchBuffer");

		// Create a buffer to store the instance build info.
		auto buildInfo = tlas.buildInfo();
		auto instanceBuffer = device->factory().createBuffer(BufferType::Storage, ResourceHeap::Dynamic, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * buildInfo.size(), 1, ResourceUsage::AccelerationStructureBuildInput);

		// Map the instance buffer.
		instanceBuffer->map(buildInfo.data(), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * buildInfo.size());

		// Build the TLAS.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc = {
			.DestAccelerationStructureData = buffer.virtualAddress() + offset,
			.Inputs = {
				.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
				.Flags = std::bit_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(tlas.flags()),
				.NumDescs = static_cast<UInt32>(tlas.instances().size()),
				.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
				.InstanceDescs = instanceBuffer->virtualAddress()
			},
			.SourceAccelerationStructureData = update ? tlas.buffer()->virtualAddress() : 0ull,
			.ScratchAccelerationStructureData = scratchBuffer->virtualAddress()
		};

		if (update)
			tlasDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

		// Build the acceleration structure.
		commandBuffer.handle()->BuildRaytracingAccelerationStructure(&tlasDesc, 0, nullptr);

		// Store the scratch buffer.
		m_sharedResources.emplace_back(instanceBuffer);
		m_sharedResources.emplace_back(scratchBuffer);
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12CommandBuffer::DirectX12CommandBuffer(const DirectX12Queue& queue, bool begin, bool primary) :
	ComResource<ID3D12GraphicsCommandList7>(nullptr), m_impl(queue)
{
	this->handle() = m_impl->initialize(queue, begin, primary);

	if (begin)
		m_impl->bindDescriptorHeaps(*this);
}

DirectX12CommandBuffer::~DirectX12CommandBuffer() noexcept = default;

SharedPtr<const DirectX12Queue> DirectX12CommandBuffer::queue() const noexcept
{
	return m_impl->m_queue.lock();
}

void DirectX12CommandBuffer::begin() const
{
	// Reset the command buffer.
	m_impl->reset(*this);

	// Bind the descriptor heaps.
	m_impl->bindDescriptorHeaps(*this);
}

void DirectX12CommandBuffer::end() const
{
	// Close the command list, so that it does not longer record any commands.
	if (m_impl->m_recording)
		raiseIfFailed(this->handle()->Close(), "Unable to close command buffer for recording.");

	m_impl->m_recording = false;
}

bool DirectX12CommandBuffer::isSecondary() const noexcept
{
	return m_impl->m_secondary;
}

void DirectX12CommandBuffer::setViewports(Span<const IViewport*> viewports) const
{
	auto vps = viewports |
		std::views::transform([](const auto& viewport) { return CD3DX12_VIEWPORT(viewport->getRectangle().x(), viewport->getRectangle().y(), viewport->getRectangle().width(), viewport->getRectangle().height(), viewport->getMinDepth(), viewport->getMaxDepth()); }) |
		std::ranges::to<Array<D3D12_VIEWPORT>>();

	this->handle()->RSSetViewports(static_cast<UINT>(vps.size()), vps.data());
}

void DirectX12CommandBuffer::setViewports(const IViewport* viewport) const
{
	auto vp = CD3DX12_VIEWPORT(viewport->getRectangle().x(), viewport->getRectangle().y(), viewport->getRectangle().width(), viewport->getRectangle().height(), viewport->getMinDepth(), viewport->getMaxDepth());
	this->handle()->RSSetViewports(1, &vp);
}

void DirectX12CommandBuffer::setScissors(Span<const IScissor*> scissors) const
{
	auto scs = scissors |
		std::views::transform([](const auto& scissor) { return CD3DX12_RECT(static_cast<LONG>(scissor->getRectangle().x()), static_cast<LONG>(scissor->getRectangle().y()), static_cast<LONG>(scissor->getRectangle().width()), static_cast<LONG>(scissor->getRectangle().height())); }) |
		std::ranges::to<Array<D3D12_RECT>>();

	this->handle()->RSSetScissorRects(static_cast<UINT>(scs.size()), scs.data());
}

void DirectX12CommandBuffer::setScissors(const IScissor* scissor) const
{
	auto s = CD3DX12_RECT(static_cast<LONG>(scissor->getRectangle().x()), static_cast<LONG>(scissor->getRectangle().y()), static_cast<LONG>(scissor->getRectangle().width()), static_cast<LONG>(scissor->getRectangle().height()));
	this->handle()->RSSetScissorRects(1, &s);
}

void DirectX12CommandBuffer::setBlendFactors(const Vector4f& blendFactors) const noexcept
{
	this->handle()->OMSetBlendFactor(blendFactors.elements());
}

void DirectX12CommandBuffer::setStencilRef(UInt32 stencilRef) const noexcept
{
	this->handle()->OMSetStencilRef(stencilRef);
}

UInt64 DirectX12CommandBuffer::submit() const
{
	// Check if the queue is still valid.
	auto queue = m_impl->m_queue.lock();

	if (queue == nullptr) [[unlikely]]
		throw RuntimeException("Cannot submit command buffer to a released command queue.");

	if (this->isSecondary())
		throw RuntimeException("A secondary command buffer cannot be directly submitted to a command queue and must be executed on a primary command buffer instead.");

	return queue->submit(this->shared_from_this());
}

UniquePtr<DirectX12Barrier> DirectX12CommandBuffer::makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot create barrier on a released device instance.");

	return device->makeBarrier(syncBefore, syncAfter);
}

void DirectX12CommandBuffer::barrier(const DirectX12Barrier& barrier) const noexcept
{
	barrier.execute(*this);
}

void DirectX12CommandBuffer::transfer(const IDirectX12Buffer& source, const IDirectX12Buffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);

	this->handle()->CopyBufferRegion(std::as_const(target).handle().Get(), targetElement * target.alignedElementSize(), std::as_const(source).handle().Get(), sourceElement * source.alignedElementSize(), elements * source.alignedElementSize());
}

void DirectX12CommandBuffer::transfer(const void* const data, size_t size, const IDirectX12Buffer& target, UInt32 targetElement, UInt32 elements) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot create staging buffer on a released device instance.");

	auto stagingBuffer = device->factory().createBuffer(target.type(), ResourceHeap::Staging, target.elementSize(), elements);
	stagingBuffer->map(data, size, 0);

	this->transfer(stagingBuffer, target, 0, targetElement, elements);
}

void DirectX12CommandBuffer::transfer(Span<const void* const> data, size_t elementSize, const IDirectX12Buffer& target, UInt32 firstElement) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot create staging buffer on a released device instance.");

	auto elements = static_cast<UInt32>(data.size());
	auto stagingBuffer = device->factory().createBuffer(target.type(), ResourceHeap::Staging, target.elementSize(), elements);
	stagingBuffer->map(data, elementSize, 0);

	this->transfer(stagingBuffer, target, 0, firstElement, elements);
}

void DirectX12CommandBuffer::transfer(const IDirectX12Buffer& source, const IDirectX12Image& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot transfer buffers on a released device instance.");

	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < firstSubresource + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target image has only {0} sub-resources, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, firstSubresource);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& targetDesc = std::as_const(target).handle()->GetDesc();

	for (UInt32 sr(0); sr < elements; ++sr)
	{
		device->handle()->GetCopyableFootprints(&targetDesc, sourceElement + sr, 1, 0, &footprint, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(std::as_const(source).handle().Get(), footprint), targetLocation(std::as_const(target).handle().Get(), firstSubresource + sr);
		this->handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}

void DirectX12CommandBuffer::transfer(const void* const data, size_t size, const IDirectX12Image& target, UInt32 subresource) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot transfer buffers on a released device instance.");

	auto stagingBuffer = device->factory().createBuffer(BufferType::Other, ResourceHeap::Staging, size);
	stagingBuffer->map(data, size, 0);

	this->transfer(stagingBuffer, target, 0, subresource, 1);
}

void DirectX12CommandBuffer::transfer(Span<const void* const> data, size_t elementSize, const IDirectX12Image& target, UInt32 firstSubresource, UInt32 subresources) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot transfer buffers on a released device instance.");

	auto elements = static_cast<UInt32>(data.size());
	auto stagingBuffer = device->factory().createBuffer(BufferType::Other, ResourceHeap::Staging, elementSize, elements);
	stagingBuffer->map(data, elementSize, 0);

	this->transfer(stagingBuffer, target, 0, firstSubresource, subresources);
}

void DirectX12CommandBuffer::transfer(const IDirectX12Image& source, const IDirectX12Image& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const
{
	if (source.elements() < sourceSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, sourceSubresource);

	if (target.elements() < targetSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resources {2} has been requested.", target.elements(), subresources, targetSubresource);

	for (UInt32 sr(0); sr < subresources; ++sr)
	{
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(std::as_const(source).handle().Get(), sourceSubresource + sr), targetLocation(std::as_const(target).handle().Get(), targetSubresource + sr);
		this->handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}

void DirectX12CommandBuffer::transfer(const IDirectX12Image& source, const IDirectX12Buffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot transfer buffers on a released device instance.");

	if (source.elements() < firstSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, firstSubresource);

	if (target.elements() <= targetElement + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), subresources, targetElement);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& targetDesc = std::as_const(target).handle()->GetDesc();

	for (UInt32 sr(0); sr < subresources; ++sr)
	{
		device->handle()->GetCopyableFootprints(&targetDesc, firstSubresource + sr, 1, 0, &footprint, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(std::as_const(source).handle().Get(), footprint), targetLocation(std::as_const(target).handle().Get(), targetElement + sr);
		this->handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}

void DirectX12CommandBuffer::transfer(const SharedPtr<const IDirectX12Buffer>& source, const IDirectX12Buffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const
{
	this->transfer(*source, target, sourceElement, targetElement, elements);
	m_impl->m_sharedResources.push_back(source);
}

void DirectX12CommandBuffer::transfer(const SharedPtr<const IDirectX12Buffer>& source, const IDirectX12Image& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const
{
	this->transfer(*source, target, sourceElement, firstSubresource, elements);
	m_impl->m_sharedResources.push_back(source);
}

void DirectX12CommandBuffer::transfer(const SharedPtr<const IDirectX12Image>& source, const IDirectX12Image& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const
{
	this->transfer(*source, target, sourceSubresource, targetSubresource, subresources);
	m_impl->m_sharedResources.push_back(source);
}

void DirectX12CommandBuffer::transfer(const SharedPtr<const IDirectX12Image>& source, const IDirectX12Buffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const
{
	this->transfer(*source, target, firstSubresource, targetElement, subresources);
	m_impl->m_sharedResources.push_back(source);
}

void DirectX12CommandBuffer::use(const DirectX12PipelineState& pipeline) const noexcept
{
	m_impl->m_lastPipeline = &pipeline;
	pipeline.use(*this);
}

void DirectX12CommandBuffer::bind(const DirectX12DescriptorSet& descriptorSet) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot bind descriptor set on a released device instance.");

	if (m_impl->m_lastPipeline) [[likely]]
		device->bindDescriptorSet(*this, descriptorSet, *m_impl->m_lastPipeline);
	else
		throw RuntimeException("No pipeline has been used on the command buffer before attempting to bind the descriptor set.");
}

void DirectX12CommandBuffer::bind(Span<const DirectX12DescriptorSet*> descriptorSets) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot bind descriptor set on a released device instance.");

	if (m_impl->m_lastPipeline) [[likely]]
		std::ranges::for_each(descriptorSets | std::views::filter([](auto descriptorSet) { return descriptorSet != nullptr; }), [&](auto descriptorSet) { device->bindDescriptorSet(*this, *descriptorSet, *m_impl->m_lastPipeline); });
	else
		throw RuntimeException("No pipeline has been used on the command buffer before attempting to bind the descriptor set.");
}

void DirectX12CommandBuffer::bind(const DirectX12DescriptorSet& descriptorSet, const DirectX12PipelineState& pipeline) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot bind descriptor set on a released device instance.");

	device->bindDescriptorSet(*this, descriptorSet, pipeline);
}

void DirectX12CommandBuffer::bind(Span<const DirectX12DescriptorSet*> descriptorSets, const DirectX12PipelineState& pipeline) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot transfer buffers on a released device instance.");

	std::ranges::for_each(descriptorSets | std::views::filter([](auto descriptorSet) { return descriptorSet != nullptr; }), [&](auto descriptorSet) { device->bindDescriptorSet(*this, *descriptorSet, pipeline); });
}

void DirectX12CommandBuffer::bind(const IDirectX12VertexBuffer& buffer) const noexcept 
{
	this->handle()->IASetVertexBuffers(buffer.layout().binding(), 1, &buffer.view());
}

void DirectX12CommandBuffer::bind(const IDirectX12IndexBuffer& buffer) const noexcept 
{
	this->handle()->IASetIndexBuffer(&buffer.view());
}

void DirectX12CommandBuffer::dispatch(const Vector3u& threadCount) const noexcept
{
	this->handle()->Dispatch(threadCount.x(), threadCount.y(), threadCount.z());
}

void DirectX12CommandBuffer::dispatchIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept
{
	this->handle()->ExecuteIndirect(m_impl->m_dispatchSignature.Get(), batchCount, batchBuffer.handle().Get(), offset, nullptr, 0);
}

void DirectX12CommandBuffer::dispatchIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept
{
	this->handle()->ExecuteIndirect(m_impl->m_dispatchSignature.Get(), std::min(maxBatches, static_cast<UInt32>(batchBuffer.alignedElementSize() / sizeof(IndirectDispatchBatch))), batchBuffer.handle().Get(), offset, countBuffer.handle().Get(), countOffset);
}

void DirectX12CommandBuffer::dispatchMesh(const Vector3u& threadCount) const noexcept
{
	this->handle()->DispatchMesh(threadCount.x(), threadCount.y(), threadCount.z());
}

void DirectX12CommandBuffer::dispatchMeshIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept
{
	this->handle()->ExecuteIndirect(m_impl->m_dispatchMeshSignature.Get(), batchCount, batchBuffer.handle().Get(), offset, nullptr, 0);
}

void DirectX12CommandBuffer::dispatchMeshIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept
{
	this->handle()->ExecuteIndirect(m_impl->m_dispatchMeshSignature.Get(), std::min(maxBatches, static_cast<UInt32>(batchBuffer.alignedElementSize() / sizeof(IndirectDispatchBatch))), batchBuffer.handle().Get(), offset, countBuffer.handle().Get(), countOffset);
}

void DirectX12CommandBuffer::draw(UInt32 vertices, UInt32 instances, UInt32 firstVertex, UInt32 firstInstance) const noexcept
{
	this->handle()->DrawInstanced(vertices, instances, firstVertex, firstInstance);
}

void DirectX12CommandBuffer::drawIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept
{
	this->handle()->ExecuteIndirect(m_impl->m_drawSignature.Get(), batchCount, batchBuffer.handle().Get(), offset, nullptr, 0);
}

void DirectX12CommandBuffer::drawIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept
{
	this->handle()->ExecuteIndirect(m_impl->m_drawSignature.Get(), std::min(maxBatches, static_cast<UInt32>(batchBuffer.alignedElementSize() / sizeof(IndirectBatch))), batchBuffer.handle().Get(), offset, countBuffer.handle().Get(), countOffset);
}

void DirectX12CommandBuffer::drawIndexed(UInt32 indices, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const noexcept
{
	this->handle()->DrawIndexedInstanced(indices, instances, firstIndex, vertexOffset, firstInstance);
}

void DirectX12CommandBuffer::drawIndexedIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept
{
	this->handle()->ExecuteIndirect(m_impl->m_drawIndexedSignature.Get(), batchCount, batchBuffer.handle().Get(), offset, nullptr, 0);
}

void DirectX12CommandBuffer::drawIndexedIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept
{
	this->handle()->ExecuteIndirect(m_impl->m_drawIndexedSignature.Get(), std::min(maxBatches, static_cast<UInt32>(batchBuffer.alignedElementSize() / sizeof(IndirectIndexedBatch))), batchBuffer.handle().Get(), offset, countBuffer.handle().Get(), countOffset);
}

void DirectX12CommandBuffer::pushConstants(const DirectX12PushConstantsLayout& layout, const void* const memory) const noexcept
{
	if (!m_impl->m_lastPipeline) [[unlikely]]
		throw RuntimeException("No pipeline has been used on the command buffer before attempting to bind the push constants range.");

	const bool isGraphicsSet = dynamic_cast<const DirectX12RenderPipeline*>(m_impl->m_lastPipeline) != nullptr;

	std::ranges::for_each(layout.ranges(), [&](const auto& range) { 
		auto rootParameter = m_impl->m_lastPipeline->layout()->rootParameterIndex(*range);

		if (!rootParameter.has_value()) [[likely]]
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to set push constant range at register {} and space {}, as the parent pipeline was not defined with a push constant there.", range->binding(), range->space());
		else
		{
			if (isGraphicsSet)
				this->handle()->SetGraphicsRoot32BitConstants(*rootParameter, range->size() / sizeof(UInt32), static_cast<const char* const>(memory) + range->offset(), 0); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			else
				this->handle()->SetComputeRoot32BitConstants(*rootParameter, range->size() / sizeof(UInt32), static_cast<const char* const>(memory) + range->offset(), 0); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		}
	});
}

void DirectX12CommandBuffer::writeTimingEvent(const SharedPtr<const TimingEvent>& timingEvent) const
{
	// Check if the device is still valid.
	auto device = m_impl->m_device.lock();

	if (device == nullptr) [[unlikely]]
		throw RuntimeException("Cannot write timing event on a released device instance.");

	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	this->handle()->EndQuery(device->swapChain().timestampQueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP, timingEvent->queryId());
}

void DirectX12CommandBuffer::execute(const SharedPtr<const DirectX12CommandBuffer>& commandBuffer) const
{
	this->handle()->ExecuteBundle(commandBuffer->handle().Get());
}

void DirectX12CommandBuffer::execute(Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers) const
{
	std::ranges::for_each(commandBuffers, [this](const SharedPtr<const DirectX12CommandBuffer>& bundle) { this->handle()->ExecuteBundle(bundle->handle().Get()); });
}

void DirectX12CommandBuffer::releaseSharedState() const
{
	m_impl->m_sharedResources.clear();
}

void DirectX12CommandBuffer::buildAccelerationStructure(DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(*this, blas, scratchBuffer, buffer, offset, false);
}

void DirectX12CommandBuffer::buildAccelerationStructure(DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(*this, tlas, scratchBuffer, buffer, offset, false);
}

void DirectX12CommandBuffer::updateAccelerationStructure(DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(*this, blas, scratchBuffer, buffer, offset, true);
}

void DirectX12CommandBuffer::updateAccelerationStructure(DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(*this, tlas, scratchBuffer, buffer, offset, true);
}

void DirectX12CommandBuffer::copyAccelerationStructure(const DirectX12BottomLevelAccelerationStructure& from, const DirectX12BottomLevelAccelerationStructure& to, bool compress) const noexcept
{
	this->handle()->CopyRaytracingAccelerationStructure(to.buffer()->virtualAddress() + to.offset(), from.buffer()->virtualAddress() + from.offset(), compress ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_CLONE);
}

void DirectX12CommandBuffer::copyAccelerationStructure(const DirectX12TopLevelAccelerationStructure& from, const DirectX12TopLevelAccelerationStructure& to, bool compress) const noexcept
{
	this->handle()->CopyRaytracingAccelerationStructure(to.buffer()->virtualAddress() + to.offset(), from.buffer()->virtualAddress() + from.offset(), compress ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_CLONE);
}

void DirectX12CommandBuffer::traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IDirectX12Buffer& rayGenerationShaderBindingTable, const IDirectX12Buffer* missShaderBindingTable, const IDirectX12Buffer* hitShaderBindingTable, const IDirectX12Buffer* callableShaderBindingTable) const noexcept
{
	D3D12_DISPATCH_RAYS_DESC rayDesc = {
		.RayGenerationShaderRecord = {
			.StartAddress = rayGenerationShaderBindingTable.virtualAddress() + offsets.RayGenerationGroupOffset,
			.SizeInBytes = offsets.RayGenerationGroupSize
		},
		.Width = width,
		.Height = height,
		.Depth = depth
	};

	if (missShaderBindingTable != nullptr)
		rayDesc.MissShaderTable = {
			.StartAddress = missShaderBindingTable->virtualAddress() + offsets.MissGroupOffset,
			.SizeInBytes = offsets.MissGroupSize,
			.StrideInBytes = offsets.MissGroupStride,
		};

	if (hitShaderBindingTable != nullptr)
		rayDesc.HitGroupTable = {
			.StartAddress = hitShaderBindingTable->virtualAddress() + offsets.HitGroupOffset,
			.SizeInBytes = offsets.HitGroupSize,
			.StrideInBytes = offsets.HitGroupStride,
		};

	if (callableShaderBindingTable != nullptr)
		rayDesc.CallableShaderTable = {
			.StartAddress = callableShaderBindingTable->virtualAddress() + offsets.CallableGroupOffset,
			.SizeInBytes = offsets.CallableGroupSize,
			.StrideInBytes = offsets.CallableGroupStride,
		};

	this->handle()->DispatchRays(&rayDesc);
}