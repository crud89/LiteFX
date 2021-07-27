#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

using BufferElement = std::tuple<IDirectX12Buffer&, UInt32, ResourceState, ResourceState>;
using ImageElement = std::tuple<IDirectX12Image&, UInt32, ResourceState, ResourceState>;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Barrier::DirectX12BarrierImpl : public Implement<DirectX12Barrier> {
public:
	friend class DirectX12Barrier;

private:
	Array<BufferElement> m_buffers;
	Array<ImageElement> m_images;
	Array<D3D12_RESOURCE_BARRIER> m_uavBarriers;

public:
	DirectX12BarrierImpl(DirectX12Barrier* parent) :
		base(parent)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Barrier::DirectX12Barrier() noexcept :
	m_impl(makePimpl<DirectX12BarrierImpl>(this))
{
}

DirectX12Barrier::~DirectX12Barrier() noexcept = default;

void DirectX12Barrier::transition(IDirectX12Buffer& buffer, const ResourceState& targetState)
{
	this->transition(buffer, buffer.state(0), targetState);
}

void DirectX12Barrier::transition(IDirectX12Buffer& buffer, const UInt32& element, const ResourceState& targetState)
{
	this->transition(buffer, buffer.state(element), element, targetState);
}

void DirectX12Barrier::transition(IDirectX12Buffer& buffer, const ResourceState& sourceState, const ResourceState& targetState)
{
	m_impl->m_buffers.push_back(BufferElement(buffer, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, sourceState, targetState));
}

void DirectX12Barrier::transition(IDirectX12Buffer& buffer, const ResourceState& sourceState, const UInt32& element, const ResourceState& targetState)
{
	m_impl->m_buffers.push_back(BufferElement(buffer, element, sourceState, targetState));
}

void DirectX12Barrier::transition(IDirectX12Image& image, const ResourceState& targetState)
{
	this->transition(image, image.state(0), targetState);
}

void DirectX12Barrier::transition(IDirectX12Image& image, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState)
{
	this->transition(image, image.state(image.getSubresourceId(level, layer, plane)), level, layer, plane, targetState);
}

void DirectX12Barrier::transition(IDirectX12Image& image, const ResourceState& sourceState, const ResourceState& targetState)
{
	m_impl->m_images.push_back(ImageElement(image, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, sourceState, targetState));
}

void DirectX12Barrier::transition(IDirectX12Image& image, const ResourceState& sourceState, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState)
{
	m_impl->m_images.push_back(ImageElement(image, image.getSubresourceId(level, layer, plane), sourceState, targetState));
}

void DirectX12Barrier::waitFor(const IDirectX12Buffer& buffer)
{
	m_impl->m_uavBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(buffer.handle().Get()));
}

void DirectX12Barrier::waitFor(const IDirectX12Image& image)
{
	m_impl->m_uavBarriers.push_back(CD3DX12_RESOURCE_BARRIER::UAV(image.handle().Get()));
}

void DirectX12Barrier::execute(const DirectX12CommandBuffer& commandBuffer, const D3D12_RESOURCE_BARRIER_FLAGS& flags) const noexcept
{
	Array<D3D12_RESOURCE_BARRIER> barriers(m_impl->m_buffers.size() + m_impl->m_images.size());

	std::ranges::generate(barriers, [this, &flags, &commandBuffer, i = 0]() mutable {
		auto element = i++;

		if (element < m_impl->m_buffers.size())
		{
			auto& bufferElement = m_impl->m_buffers[element];
			auto& buffer = std::get<0>(bufferElement);
			auto& subresource = std::get<1>(bufferElement);
			auto& targetState = std::get<3>(bufferElement);

			if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
				for (int r(0); r < buffer.elements(); ++r)
					buffer.state(r) = targetState;
			else
				buffer.state(subresource) = targetState;

			return CD3DX12_RESOURCE_BARRIER::Transition(std::as_const(buffer).handle().Get(), ::getResourceState(std::get<2>(bufferElement)), ::getResourceState(targetState), flags);
		}
		else
		{
			auto& imageElement = m_impl->m_images[element - m_impl->m_buffers.size()];
			auto& image = std::get<0>(imageElement);
			auto& subresource = std::get<1>(imageElement);
			auto& targetState = std::get<3>(imageElement);

			if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
				for (int r(0); r < image.elements(); ++r)
					image.state(r) = targetState;
			else
				image.state(subresource) = targetState;

			return CD3DX12_RESOURCE_BARRIER::Transition(std::as_const(image).handle().Get(), ::getResourceState(std::get<2>(imageElement)), ::getResourceState(targetState), flags);
		}
	});

	barriers.insert(barriers.end(), m_impl->m_uavBarriers.begin(), m_impl->m_uavBarriers.end());
	commandBuffer.handle()->ResourceBarrier(barriers.size(), barriers.data());
}

void DirectX12Barrier::executeInverse(const DirectX12CommandBuffer& commandBuffer, const D3D12_RESOURCE_BARRIER_FLAGS& flags) const noexcept
{
	Array<D3D12_RESOURCE_BARRIER> barriers(m_impl->m_buffers.size() + m_impl->m_images.size());

	std::ranges::generate(barriers, [this, &flags, &commandBuffer, i = 0]() mutable {
		auto element = i++;

		if (element < m_impl->m_buffers.size())
		{
			auto& bufferElement = m_impl->m_buffers[element];
			auto& buffer = std::get<0>(bufferElement);
			auto& subresource = std::get<1>(bufferElement);
			auto& targetState = std::get<2>(bufferElement);

			if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
				for (int r(0); r < buffer.elements(); ++r)
					buffer.state(r) = targetState;
			else
				buffer.state(subresource) = targetState;

			return CD3DX12_RESOURCE_BARRIER::Transition(std::as_const(buffer).handle().Get(), ::getResourceState(std::get<3>(bufferElement)), ::getResourceState(targetState), flags);
		}
		else
		{
			auto& imageElement = m_impl->m_images[element - m_impl->m_buffers.size()];
			auto& image = std::get<0>(imageElement);
			auto& subresource = std::get<1>(imageElement);
			auto& targetState = std::get<2>(imageElement);

			if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
				for (int r(0); r < image.elements(); ++r)
					image.state(r) = targetState;
			else
				image.state(subresource) = targetState;

			return CD3DX12_RESOURCE_BARRIER::Transition(std::as_const(image).handle().Get(), ::getResourceState(std::get<3>(imageElement)), ::getResourceState(targetState), flags);
		}
	});

	barriers.insert(barriers.end(), m_impl->m_uavBarriers.begin(), m_impl->m_uavBarriers.end());
	commandBuffer.handle()->ResourceBarrier(barriers.size(), barriers.data());
}