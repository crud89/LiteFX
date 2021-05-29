#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12FrameBuffer::DirectX12FrameBufferImpl : public Implement<DirectX12FrameBuffer> {
public:
    friend class DirectX12FrameBuffer;

private:
    Array<UniquePtr<IDirectX12Image>> m_outputAttachments;
    Array<const IDirectX12Image*> m_renderTargetViews;
    UniquePtr<DirectX12CommandBuffer> m_commandBuffer;
    Size2d m_size;
    UInt32 m_bufferIndex;

public:
    DirectX12FrameBufferImpl(DirectX12FrameBuffer* parent, const UInt32& bufferIndex, const Size2d& renderArea) :
        base(parent), m_bufferIndex(bufferIndex), m_size(renderArea)
    {
        // Retrieve a command buffer from the graphics queue.
        m_commandBuffer = m_parent->getDevice()->graphicsQueue().createCommandBuffer(false);
    }

public:
    void initialize()
    {
        // TODO: Clear existing RTVs.
        // TODO: Create RTVs and receive RTVs from swap chain for present targets.
        throw;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12FrameBuffer::DirectX12FrameBuffer(const DirectX12RenderPass& renderPass, const UInt32& bufferIndex, const Size2d& renderArea) :
    m_impl(makePimpl<DirectX12FrameBufferImpl>(this, bufferIndex, renderArea)), DirectX12RuntimeObject<DirectX12RenderPass>(renderPass, renderPass.getDevice())
{
    m_impl->initialize();
}

DirectX12FrameBuffer::~DirectX12FrameBuffer() noexcept = default;

const UInt32& DirectX12FrameBuffer::bufferIndex() const noexcept
{
    return m_impl->m_bufferIndex;
}

const Size2d& DirectX12FrameBuffer::size() const noexcept
{
    return m_impl->m_size;
}

size_t DirectX12FrameBuffer::getWidth() const noexcept
{
    return m_impl->m_size.width();
}

size_t DirectX12FrameBuffer::getHeight() const noexcept
{
    return m_impl->m_size.height();
}

const DirectX12CommandBuffer& DirectX12FrameBuffer::commandBuffer() const noexcept
{
    return *m_impl->m_commandBuffer;
}

Array<const IDirectX12Image*> DirectX12FrameBuffer::images() const noexcept
{
    return m_impl->m_renderTargetViews;
}

const IDirectX12Image& DirectX12FrameBuffer::image(const UInt32& location) const
{
    if (location >= m_impl->m_renderTargetViews.size())
        throw ArgumentOutOfRangeException("No render target is mapped to location {0}.", location);

    return *m_impl->m_renderTargetViews[location];
}

void DirectX12FrameBuffer::resize(const Size2d& renderArea)
{
    // Reset the size and re-initialize the frame buffer.
    m_impl->m_size = renderArea;
    m_impl->initialize();
}