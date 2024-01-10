#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

using GlobalBarrier = Tuple<ResourceAccess, ResourceAccess>;
using BufferBarrier = Tuple<ResourceAccess, ResourceAccess, IDirectX12Buffer&, UInt32>;
using ImageBarrier  = Tuple<ResourceAccess, ResourceAccess, IDirectX12Image&, Optional<ImageLayout>, ImageLayout, UInt32, UInt32, UInt32, UInt32, UInt32>;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Barrier::DirectX12BarrierImpl : public Implement<DirectX12Barrier> {
public:
	friend class DirectX12Barrier;

private:
	PipelineStage m_syncBefore, m_syncAfter;
	Array<GlobalBarrier> m_globalBarriers;
	Array<BufferBarrier> m_bufferBarriers;
	Array<ImageBarrier> m_imageBarriers;

public:
	DirectX12BarrierImpl(DirectX12Barrier* parent, PipelineStage syncBefore, PipelineStage syncAfter) :
		base(parent), m_syncBefore(syncBefore), m_syncAfter(syncAfter)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

constexpr DirectX12Barrier::DirectX12Barrier(PipelineStage syncBefore, PipelineStage syncAfter) noexcept :
	m_impl(makePimpl<DirectX12BarrierImpl>(this, syncBefore, syncAfter))
{
}

constexpr DirectX12Barrier::DirectX12Barrier() noexcept :
	DirectX12Barrier(PipelineStage::None, PipelineStage::None)
{
}

constexpr DirectX12Barrier::~DirectX12Barrier() noexcept = default;

constexpr PipelineStage DirectX12Barrier::syncBefore() const noexcept
{
	return m_impl->m_syncBefore;
}

constexpr PipelineStage& DirectX12Barrier::syncBefore() noexcept
{
	return m_impl->m_syncBefore;
}

constexpr PipelineStage DirectX12Barrier::syncAfter() const noexcept
{
	return m_impl->m_syncAfter;
}

constexpr PipelineStage& DirectX12Barrier::syncAfter() noexcept
{
	return m_impl->m_syncAfter;
}

constexpr void DirectX12Barrier::wait(ResourceAccess accessBefore, ResourceAccess accessAfter) noexcept
{
	m_impl->m_globalBarriers.push_back({ accessBefore, accessAfter });
}

constexpr void DirectX12Barrier::transition(IDirectX12Buffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter)
{
	m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES });
}

constexpr void DirectX12Barrier::transition(IDirectX12Buffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter)
{
	m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, element });
}

constexpr void DirectX12Barrier::transition(IDirectX12Image& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout)
{
	m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, 0, image.levels(), 0, image.layers(), 0 });
}

constexpr void DirectX12Barrier::transition(IDirectX12Image& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout)
{
	m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, fromLayout, toLayout, 0, image.levels(), 0, image.layers(), 0 });
}

constexpr void DirectX12Barrier::transition(IDirectX12Image& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout)
{
	m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, level, levels, layer, layers, plane });
}

constexpr void DirectX12Barrier::transition(IDirectX12Image& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout)
{
	m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, fromLayout, toLayout, level, levels, layer, layers, plane });
}

void DirectX12Barrier::execute(const DirectX12CommandBuffer& commandBuffer) const noexcept
{
	auto syncBefore = DX12::getPipelineStage(m_impl->m_syncBefore);
	auto syncAfter  = DX12::getPipelineStage(m_impl->m_syncAfter);

	// Global barriers.
	auto globalBarriers = m_impl->m_globalBarriers | std::views::transform([this, &syncBefore, &syncAfter](auto& barrier) { 
		return CD3DX12_GLOBAL_BARRIER(syncBefore, syncAfter, DX12::getResourceAccess(std::get<0>(barrier)), DX12::getResourceAccess(std::get<1>(barrier)));
	}) | std::ranges::to<Array<D3D12_GLOBAL_BARRIER>>();

	// Buffer barriers.
	auto bufferBarriers = m_impl->m_bufferBarriers | std::views::transform([this, &syncBefore, &syncAfter](auto& barrier) {
		return CD3DX12_BUFFER_BARRIER(syncBefore, syncAfter, DX12::getResourceAccess(std::get<0>(barrier)), DX12::getResourceAccess(std::get<1>(barrier)), std::as_const(std::get<2>(barrier)).handle().Get());
	}) | std::ranges::to<Array<D3D12_BUFFER_BARRIER>>();

	// Image barriers.
	auto imageBarriers = m_impl->m_imageBarriers | std::views::transform([this, &syncBefore, &syncAfter](auto& barrier) {
		auto& image = std::get<2>(barrier);
		auto layout = image.layout(image.subresourceId(std::get<5>(barrier), std::get<7>(barrier), std::get<9>(barrier)));
		auto currentLayout = DX12::getImageLayout(std::get<3>(barrier).value_or(layout));
		auto targetLayout = DX12::getImageLayout(std::get<4>(barrier));

		for (auto layer = std::get<7>(barrier); layer < std::get<7>(barrier) + std::get<8>(barrier); layer++)
		{
			for (auto level = std::get<5>(barrier); level < std::get<5>(barrier) + std::get<6>(barrier); level++)
			{
				auto subresource = image.subresourceId(level, layer, std::get<9>(barrier));

				if (image.layout(subresource) != layout && currentLayout != D3D12_BARRIER_LAYOUT_UNDEFINED) [[unlikely]]
					throw RuntimeException("All sub-resources in a sub-resource range need to have the same initial layout.");
				else
					image.layout(subresource) = std::get<4>(barrier);
			}
		}

		return CD3DX12_TEXTURE_BARRIER(syncBefore, syncAfter, DX12::getResourceAccess(std::get<0>(barrier)), DX12::getResourceAccess(std::get<1>(barrier)), currentLayout, targetLayout, std::as_const(image).handle().Get(), 
			CD3DX12_BARRIER_SUBRESOURCE_RANGE(std::get<5>(barrier), std::get<6>(barrier), std::get<7>(barrier), std::get<8>(barrier), std::get<9>(barrier)));
	}) | std::ranges::to<Array<D3D12_TEXTURE_BARRIER>>();

	// Put all into a buffer group.
	Array<D3D12_BARRIER_GROUP> barrierGroups;

	if (!globalBarriers.empty())
		barrierGroups.push_back(CD3DX12_BARRIER_GROUP(globalBarriers.size(), globalBarriers.data()));

	if (!bufferBarriers.empty())
		barrierGroups.push_back(CD3DX12_BARRIER_GROUP(bufferBarriers.size(), bufferBarriers.data()));

	if (!imageBarriers.empty())
		barrierGroups.push_back(CD3DX12_BARRIER_GROUP(imageBarriers.size(), imageBarriers.data()));

	if (!globalBarriers.empty() || !bufferBarriers.empty() || !imageBarriers.empty())
		commandBuffer.handle()->Barrier(barrierGroups.size(), barrierGroups.data());
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

constexpr DirectX12BarrierBuilder::DirectX12BarrierBuilder() :
	BarrierBuilder(std::move(UniquePtr<DirectX12Barrier>(new DirectX12Barrier())))
{
}

constexpr DirectX12BarrierBuilder::~DirectX12BarrierBuilder() noexcept = default;

constexpr void DirectX12BarrierBuilder::setupStages(PipelineStage waitFor, PipelineStage continueWith)
{
	this->instance()->syncBefore() = waitFor;
	this->instance()->syncAfter() = continueWith;
}

constexpr void DirectX12BarrierBuilder::setupGlobalBarrier(ResourceAccess before, ResourceAccess after)
{
	this->instance()->wait(before, after);
}

constexpr void DirectX12BarrierBuilder::setupBufferBarrier(IBuffer& buffer, ResourceAccess before, ResourceAccess after)
{
	this->instance()->transition(buffer, before, after);
}

constexpr void DirectX12BarrierBuilder::setupImageBarrier(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane)
{
	auto numLevels = levels > 0 ? levels : image.levels() - level;
	auto numLayers = layers > 0 ? layers : image.layers() - layer;
	this->instance()->transition(image, level, numLevels, layer, numLayers, plane, before, after, layout);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)