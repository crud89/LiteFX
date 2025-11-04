#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

using GlobalBarrier = Tuple<ResourceAccess, ResourceAccess>;
using BufferBarrier = Tuple<ResourceAccess, ResourceAccess, SharedPtr<const IDirectX12Buffer>, UInt32>;
using ImageBarrier  = Tuple<ResourceAccess, ResourceAccess, SharedPtr<const IDirectX12Image>, Optional<ImageLayout>, ImageLayout, UInt32, UInt32, UInt32, UInt32, UInt32>;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Barrier::DirectX12BarrierImpl {
public:
	friend class DirectX12Barrier;

private:
	PipelineStage m_syncBefore, m_syncAfter;
	Array<GlobalBarrier> m_globalBarriers{};
	Array<BufferBarrier> m_bufferBarriers{};
	Array<ImageBarrier> m_imageBarriers{};

public:
	DirectX12BarrierImpl(PipelineStage syncBefore, PipelineStage syncAfter) :
		m_syncBefore(syncBefore), m_syncAfter(syncAfter)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Barrier::DirectX12Barrier(PipelineStage syncBefore, PipelineStage syncAfter) noexcept :
	m_impl(syncBefore, syncAfter)
{
}

DirectX12Barrier::DirectX12Barrier() noexcept :
	DirectX12Barrier(PipelineStage::None, PipelineStage::None)
{
}

DirectX12Barrier::DirectX12Barrier(DirectX12Barrier&&) noexcept = default;
DirectX12Barrier::DirectX12Barrier(const DirectX12Barrier&) = default;
DirectX12Barrier& DirectX12Barrier::operator=(DirectX12Barrier&&) noexcept = default;
DirectX12Barrier& DirectX12Barrier::operator=(const DirectX12Barrier&) = default;
DirectX12Barrier::~DirectX12Barrier() noexcept = default;

PipelineStage DirectX12Barrier::syncBefore() const noexcept
{
	return m_impl->m_syncBefore;
}

PipelineStage& DirectX12Barrier::syncBefore() noexcept
{
	return m_impl->m_syncBefore;
}

PipelineStage DirectX12Barrier::syncAfter() const noexcept
{
	return m_impl->m_syncAfter;
}

PipelineStage& DirectX12Barrier::syncAfter() noexcept
{
	return m_impl->m_syncAfter;
}

void DirectX12Barrier::wait(ResourceAccess accessBefore, ResourceAccess accessAfter)
{
	m_impl->m_globalBarriers.emplace_back(accessBefore, accessAfter);
}

void DirectX12Barrier::transition(const IDirectX12Buffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter)
{
	m_impl->m_bufferBarriers.emplace_back(accessBefore, accessAfter, buffer.shared_from_this(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
}

void DirectX12Barrier::transition(const IDirectX12Buffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter)
{
	m_impl->m_bufferBarriers.emplace_back(accessBefore, accessAfter, buffer.shared_from_this(), element);
}

void DirectX12Barrier::transition(const IDirectX12Image& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout)
{
	m_impl->m_imageBarriers.emplace_back(accessBefore, accessAfter, image.shared_from_this(), std::nullopt, layout, 0, image.levels(), 0, image.layers(), 0);
}

void DirectX12Barrier::transition(const IDirectX12Image& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout)
{
	m_impl->m_imageBarriers.emplace_back(accessBefore, accessAfter, image.shared_from_this(), fromLayout, toLayout, 0, image.levels(), 0, image.layers(), 0);
}

void DirectX12Barrier::transition(const IDirectX12Image& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout)
{
	m_impl->m_imageBarriers.emplace_back(accessBefore, accessAfter, image.shared_from_this(), std::nullopt, layout, level, levels, layer, layers, plane);
}

void DirectX12Barrier::transition(const IDirectX12Image& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout)
{
	m_impl->m_imageBarriers.emplace_back(accessBefore, accessAfter, image.shared_from_this(), fromLayout, toLayout, level, levels, layer, layers, plane);
}

void DirectX12Barrier::execute(const DirectX12CommandBuffer& commandBuffer) const
{
	auto syncBefore = DX12::getPipelineStage(m_impl->m_syncBefore);
	auto syncAfter  = DX12::getPipelineStage(m_impl->m_syncAfter);

	// Global barriers.
	auto globalBarriers = m_impl->m_globalBarriers | std::views::transform([&syncBefore, &syncAfter](auto& barrier) { 
		auto [accessBefore, accessAfter] = barrier;
		return CD3DX12_GLOBAL_BARRIER(syncBefore, syncAfter, DX12::getResourceAccess(accessBefore), DX12::getResourceAccess(accessAfter));
	}) | std::ranges::to<Array<D3D12_GLOBAL_BARRIER>>();

	// Buffer barriers.
	auto bufferBarriers = m_impl->m_bufferBarriers | std::views::transform([&syncBefore, &syncAfter](auto& barrier) {
		// Special case: scratch buffers for building acceleration structures are blocked differently between APIs.
		auto [before, after, resource, element] = barrier;
		auto accessBefore = DX12::getResourceAccess(before);
		auto accessAfter = DX12::getResourceAccess(after);

		if (syncBefore == D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE && accessBefore == D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE)
			accessBefore = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
		if (syncAfter == D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE && accessAfter == D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE)
			accessAfter = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;

		return CD3DX12_BUFFER_BARRIER(syncBefore, syncAfter, accessBefore, accessAfter, std::as_const(*resource).handle().Get());
	}) | std::ranges::to<Array<D3D12_BUFFER_BARRIER>>();

	// Image barriers.
	auto imageBarriers = m_impl->m_imageBarriers | std::views::transform([&syncBefore, &syncAfter](auto& barrier) {
		auto [accessBefore, accessAfter, resource, currentLayout, targetLayout, firstLevel, levels, firstLayer, layers, plane] = barrier;

		// TODO: The image itself should expose a property (e.g., "inheritOnAlias"), that in D3D12 prevents the discard flag and in Vulkan adds the 
		//       VK_IMAGE_CREATE_ALIAS_BIT flag to the image. This should only be done for aliasing images though. For non-aliasing images, however, 
		//       this behavior emulates the Vulkan behavior. Without the flag, data inheritance is not possible in D3D12.
		D3D12_TEXTURE_BARRIER_FLAGS flags{ D3D12_TEXTURE_BARRIER_FLAG_NONE };

		if (currentLayout == ImageLayout::Undefined)
			flags = D3D12_TEXTURE_BARRIER_FLAG_DISCARD;

		return CD3DX12_TEXTURE_BARRIER(syncBefore, syncAfter, 
			DX12::getResourceAccess(accessBefore), DX12::getResourceAccess(accessAfter), 
			DX12::getImageLayout(currentLayout.value_or(ImageLayout::Undefined)), DX12::getImageLayout(targetLayout),
			std::as_const(*resource).handle().Get(), 
			CD3DX12_BARRIER_SUBRESOURCE_RANGE(firstLevel, levels, firstLayer, layers, plane, 1u), flags); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
	}) | std::ranges::to<Array<D3D12_TEXTURE_BARRIER>>();

	// Put all into a buffer group.
	Array<D3D12_BARRIER_GROUP> barrierGroups;

	if (!globalBarriers.empty())
		barrierGroups.push_back(CD3DX12_BARRIER_GROUP(static_cast<UINT32>(globalBarriers.size()), globalBarriers.data()));

	if (!bufferBarriers.empty())
		barrierGroups.push_back(CD3DX12_BARRIER_GROUP(static_cast<UINT32>(bufferBarriers.size()), bufferBarriers.data()));

	if (!imageBarriers.empty())
		barrierGroups.push_back(CD3DX12_BARRIER_GROUP(static_cast<UINT32>(imageBarriers.size()), imageBarriers.data()));

	if (!globalBarriers.empty() || !bufferBarriers.empty() || !imageBarriers.empty())
		commandBuffer.handle()->Barrier(static_cast<UINT32>(barrierGroups.size()), barrierGroups.data());
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12BarrierBuilder::DirectX12BarrierBuilder() :
	BarrierBuilder(UniquePtr<DirectX12Barrier>(new DirectX12Barrier()))
{
}

DirectX12BarrierBuilder::~DirectX12BarrierBuilder() noexcept = default;

void DirectX12BarrierBuilder::setupStages(PipelineStage waitFor, PipelineStage continueWith)
{
	this->instance()->syncBefore() = waitFor;
	this->instance()->syncAfter() = continueWith;
}

void DirectX12BarrierBuilder::setupGlobalBarrier(ResourceAccess before, ResourceAccess after)
{
	this->instance()->wait(before, after);
}

void DirectX12BarrierBuilder::setupBufferBarrier(IBuffer& buffer, ResourceAccess before, ResourceAccess after)
{
	this->instance()->transition(buffer, before, after);
}

void DirectX12BarrierBuilder::setupImageBarrier(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane)
{
	auto numLevels = levels > 0 ? levels : image.levels() - level;
	auto numLayers = layers > 0 ? layers : image.layers() - layer;
	this->instance()->transition(image, level, numLevels, layer, numLayers, plane, before, after, layout);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)