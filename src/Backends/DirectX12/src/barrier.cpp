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
	DirectX12BarrierImpl(DirectX12Barrier* parent, const PipelineStage& syncBefore, const PipelineStage& syncAfter) :
		base(parent), m_syncBefore(syncBefore), m_syncAfter(syncAfter)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Barrier::DirectX12Barrier(const PipelineStage& syncBefore, const PipelineStage& syncAfter) noexcept :
	m_impl(makePimpl<DirectX12BarrierImpl>(this, syncBefore, syncAfter))
{
}

DirectX12Barrier::DirectX12Barrier() noexcept :
	DirectX12Barrier(PipelineStage::None, PipelineStage::None)
{
}

DirectX12Barrier::~DirectX12Barrier() noexcept = default;

const PipelineStage& DirectX12Barrier::syncBefore() const noexcept
{
	return m_impl->m_syncBefore;
}

PipelineStage& DirectX12Barrier::syncBefore() noexcept
{
	return m_impl->m_syncBefore;
}

const PipelineStage& DirectX12Barrier::syncAfter() const noexcept
{
	return m_impl->m_syncAfter;
}

PipelineStage& DirectX12Barrier::syncAfter() noexcept
{
	return m_impl->m_syncAfter;
}

void DirectX12Barrier::wait(const ResourceAccess& accessBefore, const ResourceAccess& accessAfter) noexcept
{
	m_impl->m_globalBarriers.push_back({ accessBefore, accessAfter });
}

void DirectX12Barrier::transition(IDirectX12Buffer& buffer, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter)
{
	m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES });
}

void DirectX12Barrier::transition(IDirectX12Buffer& buffer, const UInt32& element, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter)
{
	m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, element });
}

void DirectX12Barrier::transition(IDirectX12Image& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout)
{
	m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, 0, image.levels(), 0, image.layers(), 0 });
}

void DirectX12Barrier::transition(IDirectX12Image& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout)
{
	m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, fromLayout, toLayout, 0, image.levels(), 0, image.layers(), 0 });
}

void DirectX12Barrier::transition(IDirectX12Image& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout)
{
	m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, level, levels, layer, layers, plane });
}

void DirectX12Barrier::transition(IDirectX12Image& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout)
{
	m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, fromLayout, toLayout, level, levels, layer, layers, plane });
}

void DirectX12Barrier::execute(const DirectX12CommandBuffer& commandBuffer) const
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

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Sub-builder definitions.
// ------------------------------------------------------------------------------------------------

class DirectX12BarrierBuilder::DirectX12SecondStageBarrierBuilder : public DirectX12BarrierBuilder::SecondStageBuilder {
private:
	DirectX12BarrierBuilder& m_parent;

public:
	DirectX12SecondStageBarrierBuilder(DirectX12BarrierBuilder& parent) :
		m_parent(parent)
	{
	}

public:
	virtual DirectX12BarrierBuilder& toContinueWith(const PipelineStage& stage) override;
};

class DirectX12BarrierBuilder::DirectX12GlobalBarrierBuilder : public DirectX12BarrierBuilder::GlobalBarrierBuilder {
private:
	DirectX12BarrierBuilder& m_parent;

public:
	ResourceAccess m_accessAfter;

public:
	DirectX12GlobalBarrierBuilder(DirectX12BarrierBuilder& parent) :
		m_parent(parent)
	{
	}

public:
	virtual DirectX12BarrierBuilder& untilFinishedWith(const ResourceAccess& access) override;
};

class DirectX12BarrierBuilder::DirectX12BufferBarrierBuilder : public DirectX12BarrierBuilder::BufferBarrierBuilder {
private:
	DirectX12BarrierBuilder& m_parent;

public:
	IBuffer* m_buffer;
	UInt32 m_subresource;
	ResourceAccess m_accessAfter;

public:
	DirectX12BufferBarrierBuilder(DirectX12BarrierBuilder& parent) :
		m_parent(parent)
	{
	}

public:
	virtual DirectX12BarrierBuilder& untilFinishedWith(const ResourceAccess& access) override;
};

class DirectX12BarrierBuilder::DirectX12ImageBarrierBuilder : public DirectX12BarrierBuilder::ImageBarrierBuilder {
private:
	DirectX12BarrierBuilder& m_parent;

public:
	DirectX12ImageBarrierBuilder(DirectX12BarrierBuilder& parent) :
		m_parent(parent)
	{
	}

public:
	virtual ImageLayoutBarrierBuilder& transitionLayout(const ImageLayout& layout) override;
	virtual DirectX12ImageBarrierBuilder& subresource(const UInt32& level, const UInt32& levels, const UInt32& layer = 0, const UInt32& layers = 1, const UInt32& plane = 0) override;
};

class DirectX12BarrierBuilder::DirectX12ImageLayoutBarrierBuilder : public DirectX12BarrierBuilder::ImageLayoutBarrierBuilder {
public:
	friend class DirectX12BarrierBuilder;
	friend class DirectX12BarrierBuilder::DirectX12ImageBarrierBuilder;

private:
	DirectX12BarrierBuilder& m_parent;
	UInt32 m_level, m_levels;
	UInt32 m_layer, m_layers;
	UInt32 m_plane;
	ImageLayout m_layout;
	IImage* m_image;
	ResourceAccess m_accessAfter;

public:
	DirectX12ImageLayoutBarrierBuilder(DirectX12BarrierBuilder& parent) :
		m_parent(parent), m_level(0), m_levels(0), m_layer(0), m_layers(0), m_plane(0), m_layout(ImageLayout::Common)
	{
	}

private:
	void setSubresource(const UInt32& level = 0, const UInt32& levels = 0, const UInt32& layer = 0, const UInt32& layers = 0, const UInt32& plane = 0) noexcept {
		m_level = level;
		m_levels = levels;
		m_layer = layer;
		m_layers = layers;
		m_plane = plane;
	}

public:
	virtual DirectX12BarrierBuilder& whenFinishedWith(const ResourceAccess& access) override;
};

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12BarrierBuilder::DirectX12BarrierBuilderImpl : public Implement<DirectX12BarrierBuilder> {
public:
	friend class DirectX12BarrierBuilder;
	friend class DirectX12BarrierBuilder::DirectX12SecondStageBarrierBuilder;
	friend class DirectX12BarrierBuilder::DirectX12GlobalBarrierBuilder;
	friend class DirectX12BarrierBuilder::DirectX12BufferBarrierBuilder;
	friend class DirectX12BarrierBuilder::DirectX12ImageBarrierBuilder;
	friend class DirectX12BarrierBuilder::DirectX12ImageLayoutBarrierBuilder;

private:
	Optional<PipelineStage> m_syncBefore, m_syncAfter;
	DirectX12BarrierBuilder::DirectX12SecondStageBarrierBuilder m_secondStageBuilder;
	DirectX12BarrierBuilder::DirectX12GlobalBarrierBuilder      m_globalBuilder;
	DirectX12BarrierBuilder::DirectX12BufferBarrierBuilder      m_bufferBuilder;
	DirectX12BarrierBuilder::DirectX12ImageBarrierBuilder       m_imageBuilder;
	DirectX12BarrierBuilder::DirectX12ImageLayoutBarrierBuilder m_imageLayoutBuilder;

public:
	DirectX12BarrierBuilderImpl(DirectX12BarrierBuilder* parent) :
		base(parent), m_secondStageBuilder(*parent), m_globalBuilder(*parent), m_bufferBuilder(*parent), m_imageBuilder(*parent), m_imageLayoutBuilder(*parent)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Sub-builder implementations.
// ------------------------------------------------------------------------------------------------

DirectX12BarrierBuilder& DirectX12BarrierBuilder::DirectX12SecondStageBarrierBuilder::toContinueWith(const PipelineStage& stage)
{
	m_parent.m_impl->m_syncAfter = stage;
	return m_parent;
}

DirectX12BarrierBuilder& DirectX12BarrierBuilder::DirectX12GlobalBarrierBuilder::untilFinishedWith(const ResourceAccess& access)
{
	m_parent.instance()->wait(access, m_accessAfter);
	return m_parent;
}

DirectX12BarrierBuilder& DirectX12BarrierBuilder::DirectX12BufferBarrierBuilder::untilFinishedWith(const ResourceAccess& access)
{
	m_parent.instance()->transition(*m_buffer, m_subresource, access, m_accessAfter);
	return m_parent;
}

DirectX12BarrierBuilder::ImageLayoutBarrierBuilder& DirectX12BarrierBuilder::DirectX12ImageBarrierBuilder::transitionLayout(const ImageLayout& layout)
{
	m_parent.m_impl->m_imageLayoutBuilder.m_layout = layout;
	return m_parent.m_impl->m_imageLayoutBuilder;
}

DirectX12BarrierBuilder::DirectX12ImageBarrierBuilder& DirectX12BarrierBuilder::DirectX12ImageBarrierBuilder::subresource(const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane)
{
	m_parent.m_impl->m_imageLayoutBuilder.setSubresource(level, levels, layer, layers, plane);
	return *this;
}

DirectX12BarrierBuilder& DirectX12BarrierBuilder::DirectX12ImageLayoutBarrierBuilder::whenFinishedWith(const ResourceAccess& access)
{
	auto levels = m_levels > 0 ? m_levels : m_image->levels() - m_level;
	auto layers = m_layers > 0 ? m_layers : m_image->layers() - m_layer;
	m_parent.instance()->transition(*m_image, m_level, levels, m_layer, layers, m_plane, access, m_accessAfter, m_layout);
	return m_parent;
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12BarrierBuilder::DirectX12BarrierBuilder() :
	m_impl(makePimpl<DirectX12BarrierBuilderImpl>(this)), BarrierBuilder(std::move(UniquePtr<DirectX12Barrier>(new DirectX12Barrier())))
{
}

DirectX12BarrierBuilder::~DirectX12BarrierBuilder() noexcept = default;

void DirectX12BarrierBuilder::build()
{
	if (!m_impl->m_syncBefore.has_value() || !m_impl->m_syncAfter.has_value())
		throw RuntimeException("A pipeline requires a synchronization scope. Make sure to call `waitFor` to define it.");

	this->instance()->syncBefore() = m_impl->m_syncBefore.value();
	this->instance()->syncAfter() = m_impl->m_syncAfter.value();
}

DirectX12BarrierBuilder::SecondStageBuilder& DirectX12BarrierBuilder::waitFor(const PipelineStage& stage)
{
	m_impl->m_syncBefore = stage;
	return m_impl->m_secondStageBuilder;
}

DirectX12BarrierBuilder::GlobalBarrierBuilder& DirectX12BarrierBuilder::blockAccessTo(const ResourceAccess& access)
{
	m_impl->m_globalBuilder.m_accessAfter = access;
	return m_impl->m_globalBuilder;
}

DirectX12BarrierBuilder::BufferBarrierBuilder& DirectX12BarrierBuilder::blockAccessTo(IBuffer& buffer, const ResourceAccess& access)
{
	m_impl->m_bufferBuilder.m_buffer = &buffer;
	m_impl->m_bufferBuilder.m_accessAfter = access;
	m_impl->m_bufferBuilder.m_subresource = 0;
	return m_impl->m_bufferBuilder;
}

DirectX12BarrierBuilder::BufferBarrierBuilder& DirectX12BarrierBuilder::blockAccessTo(IBuffer& buffer, const UInt32 subresource, const ResourceAccess& access)
{
	m_impl->m_bufferBuilder.m_buffer = &buffer;
	m_impl->m_bufferBuilder.m_accessAfter = access;
	m_impl->m_bufferBuilder.m_subresource = subresource;
	return m_impl->m_bufferBuilder;
}

DirectX12BarrierBuilder::ImageBarrierBuilder& DirectX12BarrierBuilder::blockAccessTo(IImage& image, const ResourceAccess& access)
{
	m_impl->m_imageLayoutBuilder.m_image = &image;
	m_impl->m_imageLayoutBuilder.m_accessAfter = access;
	m_impl->m_imageLayoutBuilder.setSubresource();
	return m_impl->m_imageBuilder;
}
#endif // defined(BUILD_DEFINE_BUILDERS)