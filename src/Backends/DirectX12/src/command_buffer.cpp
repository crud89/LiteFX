#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12CommandBuffer::DirectX12CommandBufferImpl : public Implement<DirectX12CommandBuffer> {
public:
	friend class DirectX12CommandBuffer;

private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	bool m_recording{ false }, m_secondary{ false };
	const DirectX12Queue& m_queue;
	Array<SharedPtr<const IStateResource>> m_sharedResources;
	const DirectX12PipelineState* m_lastPipeline = nullptr;

public:
	DirectX12CommandBufferImpl(DirectX12CommandBuffer* parent, const DirectX12Queue& queue) :
		base(parent), m_queue(queue)
	{
	}

public:
	ComPtr<ID3D12GraphicsCommandList7> initialize(bool begin, bool primary)
	{
		// Create a command allocator.
		D3D12_COMMAND_LIST_TYPE type;

		if (m_secondary = !primary)
			type = D3D12_COMMAND_LIST_TYPE_BUNDLE;
		else
		{
			switch (m_queue.type())
			{
			case QueueType::Compute: type = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
			case QueueType::Transfer: type = D3D12_COMMAND_LIST_TYPE_COPY; break;
			default:
			case QueueType::Graphics: type = D3D12_COMMAND_LIST_TYPE_DIRECT; break;
			}
		}

		raiseIfFailed(m_queue.device().handle()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_commandAllocator)), "Unable to create command allocator for command buffer.");

		// Create the actual command list.
		ComPtr<ID3D12GraphicsCommandList7> commandList;

		if (m_recording = begin)
			raiseIfFailed(m_queue.device().handle()->CreateCommandList(0, type, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)), "Unable to create command list for command buffer.");
		else
			raiseIfFailed(m_queue.device().handle()->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList)), "Unable to create command list for command buffer.");

		return commandList;
	}

	void reset()
	{
		raiseIfFailed(m_commandAllocator->Reset(), "Unable to reset command allocator.");
		raiseIfFailed(m_parent->handle()->Reset(m_commandAllocator.Get(), nullptr), "Unable to reset command list.");
		m_recording = true;

		// If it was possible to reset the command buffer, we can also safely release shared resources from previous recordings.
		m_sharedResources.clear();
	}

	void bindDescriptorHeaps()
	{
		if (m_queue.type() == QueueType::Compute || m_queue.type() == QueueType::Graphics)
			m_queue.device().bindGlobalDescriptorHeaps(*m_parent);
	}

	inline void buildAccelerationStructure(DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset, bool update)
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
		m_parent->handle()->BuildRaytracingAccelerationStructure(&blasDesc, 0, nullptr);

		// Store the scratch buffer.
		m_sharedResources.push_back(scratchBuffer);
	}

	inline void buildAccelerationStructure(DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset, bool update)
	{
		if (scratchBuffer == nullptr) [[unlikely]]
			throw ArgumentNotInitializedException("scratchBuffer");

		// Create a buffer to store the instance build info.
		auto buildInfo = tlas.buildInfo();
		auto instanceBuffer = m_queue.device().factory().createBuffer(BufferType::Storage, ResourceHeap::Dynamic, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * buildInfo.size(), 1, ResourceUsage::AccelerationStructureBuildInput);

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
		m_parent->handle()->BuildRaytracingAccelerationStructure(&tlasDesc, 0, nullptr);

		// Store the scratch buffer.
		m_sharedResources.push_back(asShared(std::move(instanceBuffer)));
		m_sharedResources.push_back(scratchBuffer);
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12CommandBuffer::DirectX12CommandBuffer(const DirectX12Queue& queue, bool begin, bool primary) :
	m_impl(makePimpl<DirectX12CommandBufferImpl>(this, queue)), ComResource<ID3D12GraphicsCommandList7>(nullptr)
{
	this->handle() = m_impl->initialize(begin, primary);

	if (begin)
		m_impl->bindDescriptorHeaps();
}

DirectX12CommandBuffer::~DirectX12CommandBuffer() noexcept = default;

const ICommandQueue& DirectX12CommandBuffer::queue() const noexcept
{
	return m_impl->m_queue;
}

void DirectX12CommandBuffer::begin() const
{
	// Reset the command buffer.
	m_impl->reset();

	// Bind the descriptor heaps.
	m_impl->bindDescriptorHeaps();
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

void DirectX12CommandBuffer::setViewports(Span<const IViewport*> viewports) const noexcept
{
	auto vps = viewports |
		std::views::transform([](const auto& viewport) { return CD3DX12_VIEWPORT(viewport->getRectangle().x(), viewport->getRectangle().y(), viewport->getRectangle().width(), viewport->getRectangle().height(), viewport->getMinDepth(), viewport->getMaxDepth()); }) |
		std::ranges::to<Array<D3D12_VIEWPORT>>();

	this->handle()->RSSetViewports(vps.size(), vps.data());
}

void DirectX12CommandBuffer::setViewports(const IViewport* viewport) const noexcept
{
	auto vp = CD3DX12_VIEWPORT(viewport->getRectangle().x(), viewport->getRectangle().y(), viewport->getRectangle().width(), viewport->getRectangle().height(), viewport->getMinDepth(), viewport->getMaxDepth());
	this->handle()->RSSetViewports(1, &vp);
}

void DirectX12CommandBuffer::setScissors(Span<const IScissor*> scissors) const noexcept
{
	auto scs = scissors |
		std::views::transform([](const auto& scissor) { return CD3DX12_RECT(scissor->getRectangle().x(), scissor->getRectangle().y(), scissor->getRectangle().width(), scissor->getRectangle().height()); }) |
		std::ranges::to<Array<D3D12_RECT>>();

	this->handle()->RSSetScissorRects(scs.size(), scs.data());
}

void DirectX12CommandBuffer::setScissors(const IScissor* scissor) const noexcept
{
	auto s = CD3DX12_RECT(scissor->getRectangle().x(), scissor->getRectangle().y(), scissor->getRectangle().width(), scissor->getRectangle().height());
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
	if (this->isSecondary())
		throw RuntimeException("A secondary command buffer cannot be directly submitted to a command queue and must be executed on a primary command buffer instead.");

	return m_impl->m_queue.submit(this->shared_from_this());
}

void DirectX12CommandBuffer::generateMipMaps(IDirectX12Image& image) noexcept
{
	struct Parameters {
		Float sizeX;
		Float sizeY;
		Float sRGB;
		Float padding;
	};

	// Create the array of parameter data.
	Array<Parameters> parametersData(image.levels());

	std::ranges::generate(parametersData, [this, &image, i = 0]() mutable {
		auto level = i++;

		return Parameters {
			.sizeX = 1.f / static_cast<Float>(std::max<size_t>(image.extent(level).width(), 1)),
			.sizeY = 1.f / static_cast<Float>(std::max<size_t>(image.extent(level).height(), 1)),
			.sRGB = DX12::isSRGB(image.format()) ? 1.f : 0.f
		};
	});

	auto parametersBlock = parametersData |
		std::views::transform([](const Parameters& parameters) { return reinterpret_cast<const void*>(&parameters); }) |
		std::ranges::to<Array<const void*>>();

	// Set the active pipeline state.
	auto& pipeline = m_impl->m_queue.device().blitPipeline();
	this->use(pipeline);

	// Create and bind the parameters.
	const auto& resourceBindingsLayout = pipeline.layout()->descriptorSet(0);
	auto resourceBindings = resourceBindingsLayout.allocateMultiple(image.levels() * image.layers());
	const auto& parametersLayout = resourceBindingsLayout.descriptor(0);
	auto parameters = m_impl->m_queue.device().factory().createBuffer(parametersLayout.type(), ResourceHeap::Dynamic, parametersLayout.elementSize(), image.levels());
	parameters->map(parametersBlock, sizeof(Parameters));

	// Create and bind the sampler.
	const auto& samplerBindingsLayout = pipeline.layout()->descriptorSet(1);
	auto samplerBindings = samplerBindingsLayout.allocate();
	auto sampler = m_impl->m_queue.device().factory().createSampler(FilterMode::Linear, FilterMode::Linear, BorderMode::ClampToEdge, BorderMode::ClampToEdge, BorderMode::ClampToEdge);
	samplerBindings->update(0, *sampler);
	this->bind(*samplerBindings, pipeline);

	// Transition the texture into a read/write state.
	DirectX12Barrier startBarrier(PipelineStage::None, PipelineStage::Compute);
	startBarrier.transition(image, ResourceAccess::None, ResourceAccess::ShaderReadWrite, ImageLayout::Undefined, ImageLayout::ReadWrite);
	this->barrier(startBarrier);
	auto resource = resourceBindings.begin();

	for (int l(0); l < image.layers(); ++l, ++resource)
	{
		auto size = image.extent();

		for (UInt32 i(1); i < image.levels(); ++i, size /= 2)
		{
			// Update the invocation parameters.
			(*resource)->update(parametersLayout.binding(), *parameters, i, 1);

			// Bind the previous mip map level to the SRV at binding point 1.
			(*resource)->update(1, image, 0, i - 1, 1, l, 1);

			// Bind the current level to the UAV at binding point 2.
			(*resource)->update(2, image, 0, i, 1, l, 1);

			// Dispatch the pipeline.
			this->bind(*(*resource), pipeline);
			this->dispatch({ std::max<UInt32>(size.width() / 8, 1), std::max<UInt32>(size.height() / 8, 1), 1 });

			// Wait for all writes.
			DirectX12Barrier subBarrier(PipelineStage::Compute, PipelineStage::Compute);
			subBarrier.transition(image, i, 1, l, 1, 0, ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderRead, ImageLayout::ReadWrite, ImageLayout::ShaderResource);
			this->barrier(subBarrier);
			resource++;
		}

		// Original sub-resource also needs to be transitioned.
		DirectX12Barrier endBarrier(PipelineStage::Compute, PipelineStage::All);
		endBarrier.transition(image, 0, 1, l, 1, 0, ResourceAccess::ShaderReadWrite, ResourceAccess::ShaderRead, ImageLayout::ReadWrite, ImageLayout::ShaderResource);
		this->barrier(endBarrier);
	}
}

UniquePtr<DirectX12Barrier> DirectX12CommandBuffer::makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept
{
	return m_impl->m_queue.device().makeBarrier(syncBefore, syncAfter);
}

void DirectX12CommandBuffer::barrier(const DirectX12Barrier& barrier) const noexcept
{
	barrier.execute(*this);
}

void DirectX12CommandBuffer::transfer(IDirectX12Buffer& source, IDirectX12Buffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < targetElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, targetElement);

	this->handle()->CopyBufferRegion(std::as_const(target).handle().Get(), targetElement * target.alignedElementSize(), std::as_const(source).handle().Get(), sourceElement * source.alignedElementSize(), elements * source.alignedElementSize());
}

void DirectX12CommandBuffer::transfer(const void* const data, size_t size, IDirectX12Buffer& target, UInt32 targetElement, UInt32 elements) const
{
	auto alignment = target.elementAlignment();
	auto elementSize = target.elementSize();
	auto alignedSize = target.alignedElementSize();

	auto stagingBuffer = asShared(std::move(m_impl->m_queue.device().factory().createBuffer(target.type(), ResourceHeap::Staging, target.elementSize(), elements)));
	stagingBuffer->map(data, size, 0);

	this->transfer(stagingBuffer, target, 0, targetElement, elements);
}

void DirectX12CommandBuffer::transfer(Span<const void* const> data, size_t elementSize, IDirectX12Buffer& target, UInt32 firstElement) const
{
	auto elements = static_cast<UInt32>(data.size());
	auto stagingBuffer = asShared(std::move(m_impl->m_queue.device().factory().createBuffer(target.type(), ResourceHeap::Staging, target.elementSize(), elements)));
	stagingBuffer->map(data, elementSize, 0);

	this->transfer(stagingBuffer, target, 0, firstElement, elements);
}

void DirectX12CommandBuffer::transfer(IDirectX12Buffer& source, IDirectX12Image& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const
{
	if (source.elements() < sourceElement + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", source.elements(), elements, sourceElement);

	if (target.elements() < firstSubresource + elements) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target image has only {0} sub-resources, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), elements, firstSubresource);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& targetDesc = std::as_const(target).handle()->GetDesc();

	for (int sr(0); sr < elements; ++sr)
	{
		m_impl->m_queue.device().handle()->GetCopyableFootprints(&targetDesc, sourceElement + sr, 1, 0, &footprint, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(std::as_const(source).handle().Get(), footprint), targetLocation(std::as_const(target).handle().Get(), firstSubresource + sr);
		this->handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}

void DirectX12CommandBuffer::transfer(const void* const data, size_t size, IDirectX12Image& target, UInt32 subresource) const
{
	auto stagingBuffer = asShared(std::move(m_impl->m_queue.device().factory().createBuffer(BufferType::Other, ResourceHeap::Staging, size)));
	stagingBuffer->map(data, size, 0);

	this->transfer(stagingBuffer, target, 0, subresource, 1);
}

void DirectX12CommandBuffer::transfer(Span<const void* const> data, size_t elementSize, IDirectX12Image& target, UInt32 firstSubresource, UInt32 subresources) const
{
	auto elements = static_cast<UInt32>(data.size());
	auto stagingBuffer = asShared(std::move(m_impl->m_queue.device().factory().createBuffer(BufferType::Other, ResourceHeap::Staging, elementSize, elements)));
	stagingBuffer->map(data, elementSize, 0);

	this->transfer(stagingBuffer, target, 0, firstSubresource, subresources);
}

void DirectX12CommandBuffer::transfer(IDirectX12Image& source, IDirectX12Image& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const
{
	if (source.elements() < sourceSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, sourceSubresource);

	if (target.elements() < targetSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resources {2} has been requested.", target.elements(), subresources, targetSubresource);

	for (int sr(0); sr < subresources; ++sr)
	{
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(std::as_const(source).handle().Get(), sourceSubresource + sr), targetLocation(std::as_const(target).handle().Get(), targetSubresource + sr);
		this->handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}

void DirectX12CommandBuffer::transfer(IDirectX12Image& source, IDirectX12Buffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const
{
	if (source.elements() < firstSubresource + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("sourceElement", "The source image has only {0} sub-resources, but a transfer for {1} sub-resources starting from sub-resource {2} has been requested.", source.elements(), subresources, firstSubresource);

	if (target.elements() <= targetElement + subresources) [[unlikely]]
		throw ArgumentOutOfRangeException("targetElement", "The target buffer has only {0} elements, but a transfer for {1} elements starting from element {2} has been requested.", target.elements(), subresources, targetElement);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	const auto& targetDesc = std::as_const(target).handle()->GetDesc();

	for (int sr(0); sr < subresources; ++sr)
	{
		m_impl->m_queue.device().handle()->GetCopyableFootprints(&targetDesc, firstSubresource + sr, 1, 0, &footprint, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION sourceLocation(std::as_const(source).handle().Get(), footprint), targetLocation(std::as_const(target).handle().Get(), targetElement + sr);
		this->handle()->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}

void DirectX12CommandBuffer::transfer(SharedPtr<IDirectX12Buffer> source, IDirectX12Buffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const
{
	this->transfer(*source, target, sourceElement, targetElement, elements);
	m_impl->m_sharedResources.push_back(source);
}

void DirectX12CommandBuffer::transfer(SharedPtr<IDirectX12Buffer> source, IDirectX12Image& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const
{
	this->transfer(*source, target, sourceElement, firstSubresource, elements);
	m_impl->m_sharedResources.push_back(source);
}

void DirectX12CommandBuffer::transfer(SharedPtr<IDirectX12Image> source, IDirectX12Image& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const
{
	this->transfer(*source, target, sourceSubresource, targetSubresource, subresources);
	m_impl->m_sharedResources.push_back(source);
}

void DirectX12CommandBuffer::transfer(SharedPtr<IDirectX12Image> source, IDirectX12Buffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const
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
	if (m_impl->m_lastPipeline) [[likely]]
		m_impl->m_queue.device().bindDescriptorSet(*this, descriptorSet, *m_impl->m_lastPipeline);
	else
		throw RuntimeException("No pipeline has been used on the command buffer before attempting to bind the descriptor set.");
}

void DirectX12CommandBuffer::bind(Span<const DirectX12DescriptorSet*> descriptorSets) const
{
	if (m_impl->m_lastPipeline) [[likely]]
		std::ranges::for_each(descriptorSets | std::views::filter([](auto descriptorSet) { return descriptorSet != nullptr; }), [this](auto descriptorSet) { m_impl->m_queue.device().bindDescriptorSet(*this, *descriptorSet, *m_impl->m_lastPipeline); });
	else
		throw RuntimeException("No pipeline has been used on the command buffer before attempting to bind the descriptor set.");
}

void DirectX12CommandBuffer::bind(const DirectX12DescriptorSet& descriptorSet, const DirectX12PipelineState& pipeline) const noexcept
{
	m_impl->m_queue.device().bindDescriptorSet(*this, descriptorSet, pipeline);
}

void DirectX12CommandBuffer::bind(Span<const DirectX12DescriptorSet*> descriptorSets, const DirectX12PipelineState& pipeline) const noexcept
{
	std::ranges::for_each(descriptorSets | std::views::filter([](auto descriptorSet) { return descriptorSet != nullptr; }), [this](auto descriptorSet) { m_impl->m_queue.device().bindDescriptorSet(*this, *descriptorSet, *m_impl->m_lastPipeline); });
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

void DirectX12CommandBuffer::dispatchMesh(const Vector3u& threadCount) const noexcept
{
	this->handle()->DispatchMesh(threadCount.x(), threadCount.y(), threadCount.z());
}

void DirectX12CommandBuffer::draw(UInt32 vertices, UInt32 instances, UInt32 firstVertex, UInt32 firstInstance) const noexcept
{
	this->handle()->DrawInstanced(vertices, instances, firstVertex, firstInstance);
}

void DirectX12CommandBuffer::drawIndexed(UInt32 indices, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const noexcept
{
	this->handle()->DrawIndexedInstanced(indices, instances, firstIndex, vertexOffset, firstInstance);
}

void DirectX12CommandBuffer::pushConstants(const DirectX12PushConstantsLayout& layout, const void* const memory) const noexcept
{
	std::ranges::for_each(layout.ranges(), [this, &layout, &memory](const DirectX12PushConstantsRange* range) { this->handle()->SetGraphicsRoot32BitConstants(range->rootParameterIndex(), range->size() / 4, reinterpret_cast<const char* const>(memory) + range->offset(), 0); });
}

void DirectX12CommandBuffer::writeTimingEvent(SharedPtr<const TimingEvent> timingEvent) const
{
	if (timingEvent == nullptr) [[unlikely]]
		throw ArgumentNotInitializedException("timingEvent", "The timing event must be initialized.");

	this->handle()->EndQuery(m_impl->m_queue.device().swapChain().timestampQueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP, timingEvent->queryId());
}

void DirectX12CommandBuffer::execute(SharedPtr<const DirectX12CommandBuffer> commandBuffer) const
{
	this->handle()->ExecuteBundle(commandBuffer->handle().Get());
}

void DirectX12CommandBuffer::execute(Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers) const
{
	std::ranges::for_each(commandBuffers, [this](auto& bundle) { this->handle()->ExecuteBundle(bundle->handle().Get()); });
}

void DirectX12CommandBuffer::releaseSharedState() const
{
	m_impl->m_sharedResources.clear();
}

void DirectX12CommandBuffer::buildAccelerationStructure(DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(blas, scratchBuffer, buffer, offset, false);
}

void DirectX12CommandBuffer::buildAccelerationStructure(DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(tlas, scratchBuffer, buffer, offset, false);
}

void DirectX12CommandBuffer::updateAccelerationStructure(DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(blas, scratchBuffer, buffer, offset, true);
}

void DirectX12CommandBuffer::updateAccelerationStructure(DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset) const
{
	m_impl->buildAccelerationStructure(tlas, scratchBuffer, buffer, offset, true);
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