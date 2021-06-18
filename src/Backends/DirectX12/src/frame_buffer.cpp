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
    ComPtr<ID3D12DescriptorHeap> m_renderTargetHeap, m_depthStencilHeap;
    UInt32 m_renderTargetDescriptorSize, m_depthStencilDescriptorSize;
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
        // Clear current render targets.
        m_outputAttachments.clear();
        m_renderTargetViews.clear();

        // Create descriptor heaps for RTVs and DSVs.
        UInt32 renderTargets = std::ranges::count_if(m_parent->parent().renderTargets(), [](const RenderTarget& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; });
        UInt32 depthStencilTargets = static_cast<UInt32>(m_parent->parent().renderTargets().size()) - renderTargets;

        if (depthStencilTargets > 1)
            LITEFX_WARNING(DIRECTX12_LOG, "There are {0} depth/stencil targets mapped on the frame buffer. This will probably not work as intended, since only one depth/stencil target can be bound to a render pass.", depthStencilTargets);

        D3D12_DESCRIPTOR_HEAP_DESC renderTargetHeapDesc = {};
        renderTargetHeapDesc.NumDescriptors = renderTargets;
        renderTargetHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        m_renderTargetDescriptorSize = m_parent->getDevice()->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_DESCRIPTOR_HEAP_DESC depthStencilHeapDesc = {};
        depthStencilHeapDesc.NumDescriptors = depthStencilTargets;
        depthStencilHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        m_depthStencilDescriptorSize = m_parent->getDevice()->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateDescriptorHeap(&renderTargetHeapDesc, IID_PPV_ARGS(&m_renderTargetHeap)), "Unable to create render target descriptor heap.");
        CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewDescriptor(m_renderTargetHeap->GetCPUDescriptorHandleForHeapStart());
        raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateDescriptorHeap(&depthStencilHeapDesc, IID_PPV_ARGS(&m_depthStencilHeap)), "Unable to create depth/stencil descriptor heap.");
        CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilViewDescriptor(m_depthStencilHeap->GetCPUDescriptorHandleForHeapStart());

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
                // TODO: Pass the optimized clear value from the render target to the attachment. (May need to refactor `CreateAttachment` to accept the render target and a size).
                auto image = m_parent->getDevice()->factory().createAttachment(renderTarget.format(), m_size, renderTarget.samples());
                renderTargetView = image.get();
                m_outputAttachments.push_back(std::move(image));
            }

            if (renderTarget.type() == RenderTargetType::DepthStencil)
            {
                D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = { };
                depthStencilViewDesc.Format = ::getFormat(renderTarget.format());
                depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
                depthStencilViewDesc.Texture2D = { .MipSlice = 0 };
                depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

                m_parent->getDevice()->handle()->CreateDepthStencilView(renderTargetView->handle().Get(), &depthStencilViewDesc, depthStencilViewDescriptor);
                depthStencilViewDescriptor = depthStencilViewDescriptor.Offset(m_depthStencilDescriptorSize);
            }
            else
            {
                D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { };
                renderTargetViewDesc.Format = ::getFormat(renderTarget.format());
                renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                renderTargetViewDesc.Texture2D = { .MipSlice = 0, .PlaneSlice = 0 };
                renderTargetViewDesc.Buffer = { .FirstElement = 0, .NumElements = 1 };

                m_parent->getDevice()->handle()->CreateRenderTargetView(renderTargetView->handle().Get(), &renderTargetViewDesc, renderTargetViewDescriptor);
                renderTargetViewDescriptor = renderTargetViewDescriptor.Offset(m_renderTargetDescriptorSize);
            }

            m_renderTargetViews.push_back(renderTargetView);
        });
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

ID3D12DescriptorHeap* DirectX12FrameBuffer::renderTargetHeap() const noexcept
{
    return m_impl->m_renderTargetHeap.Get();
}

ID3D12DescriptorHeap* DirectX12FrameBuffer::depthStencilTargetHeap() const noexcept
{
    return m_impl->m_depthStencilHeap.Get();
}

const UInt32& DirectX12FrameBuffer::renderTargetDescriptorSize() const noexcept
{
    return m_impl->m_renderTargetDescriptorSize;
}

const UInt32& DirectX12FrameBuffer::depthStencilTargetDescriptorSize() const noexcept
{
    return m_impl->m_depthStencilDescriptorSize;
}

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