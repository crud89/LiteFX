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
    ComPtr<ID3D12DescriptorHeap> m_renderTargetHeap;
    UInt32 m_renderTargetDescriptorSize;
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


        // Create descriptor heap for RTVs.
        D3D12_DESCRIPTOR_HEAP_DESC renderTargetHeapDesc = {};
        renderTargetHeapDesc.NumDescriptors = static_cast<UInt32>(m_parent->parent().renderTargets().size());
        renderTargetHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateDescriptorHeap(&renderTargetHeapDesc, IID_PPV_ARGS(&m_renderTargetHeap)), "Unable to create render target descriptor heap.");
        m_renderTargetDescriptorSize = m_parent->getDevice()->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewDescriptor(m_renderTargetHeap->GetCPUDescriptorHandleForHeapStart());

        // Initialize the output attachments from render targets of the parent render pass.
        // NOTE: Again, we assume, that the parent render pass provides the render targets in an sorted manner.
        std::ranges::for_each(m_parent->parent().renderTargets(), [&, i = 0](const RenderTarget& renderTarget) mutable {
            if (renderTarget.location() != i++) [[unlikely]]
                LITEFX_WARNING(DIRECTX12_LOG, "Remapped render target from location {0} to location {1}. Please make sure that the render targets are sorted within the render pass and do not have any gaps in their location mappings.", renderTarget.location(), i - 1);
            
            const IDirectX12Image* renderTargetView;

            if (renderTarget.type() == RenderTargetType::Present)
            {
                // If the render target is a present target, acquire an image view from the swap chain.
                auto swapChainImages = m_parent->getDevice()->swapChain().images();
                renderTargetView = swapChainImages[m_bufferIndex];
            }
            else
            {
                // Create an image view for the render target.
                auto image = m_parent->getDevice()->factory().createAttachment(renderTarget.format(), m_size, renderTarget.samples());
                renderTargetView = image.get();
                m_outputAttachments.push_back(std::move(image));
            }

            m_parent->getDevice()->handle()->CreateRenderTargetView(renderTargetView->handle().Get(), nullptr, renderTargetViewDescriptor);
            m_renderTargetViews.push_back(renderTargetView);
        });

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