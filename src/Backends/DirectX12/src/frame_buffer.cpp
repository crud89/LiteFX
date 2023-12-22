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
    Array<SharedPtr<DirectX12CommandBuffer>> m_commandBuffers;
    ComPtr<ID3D12DescriptorHeap> m_renderTargetHeap, m_depthStencilHeap;
    UInt32 m_renderTargetDescriptorSize, m_depthStencilDescriptorSize;
    Size2d m_size;
    UInt32 m_bufferIndex;
    UInt64 m_lastFence{ 0 };
    const DirectX12RenderPass& m_renderPass;

public:
    DirectX12FrameBufferImpl(DirectX12FrameBuffer* parent, const DirectX12RenderPass& renderPass, UInt32 bufferIndex, const Size2d& renderArea, UInt32 commandBuffers) :
        base(parent), m_renderPass(renderPass), m_bufferIndex(bufferIndex), m_size(renderArea)
    {
        // Initialize the command buffers from the graphics queue.
        m_commandBuffers.resize(commandBuffers);
        std::ranges::generate(m_commandBuffers, [this]() { return m_renderPass.commandQueue().createCommandBuffer(false); });
    }

public:
    void initialize()
    {
        // Clear current render targets.
        m_outputAttachments.clear();
        m_renderTargetViews.clear();

        // Create descriptor heaps for RTVs and DSVs.
        UInt32 renderTargets = std::ranges::count_if(m_renderPass.renderTargets(), [](const RenderTarget& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; });
        UInt32 depthStencilTargets = static_cast<UInt32>(m_renderPass.renderTargets().size()) - renderTargets;

        if (depthStencilTargets > 1)
            LITEFX_WARNING(DIRECTX12_LOG, "There are {0} depth/stencil targets mapped on the frame buffer. This will probably not work as intended, since only one depth/stencil target can be bound to a render pass.", depthStencilTargets);

        D3D12_DESCRIPTOR_HEAP_DESC renderTargetHeapDesc = {};
        renderTargetHeapDesc.NumDescriptors = renderTargets;
        renderTargetHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        m_renderTargetDescriptorSize = m_renderPass.device().handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_DESCRIPTOR_HEAP_DESC depthStencilHeapDesc = {};
        depthStencilHeapDesc.NumDescriptors = depthStencilTargets;
        depthStencilHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        m_depthStencilDescriptorSize = m_renderPass.device().handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        raiseIfFailed<RuntimeException>(m_renderPass.device().handle()->CreateDescriptorHeap(&renderTargetHeapDesc, IID_PPV_ARGS(&m_renderTargetHeap)), "Unable to create render target descriptor heap.");
        CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewDescriptor(m_renderTargetHeap->GetCPUDescriptorHandleForHeapStart());
        raiseIfFailed<RuntimeException>(m_renderPass.device().handle()->CreateDescriptorHeap(&depthStencilHeapDesc, IID_PPV_ARGS(&m_depthStencilHeap)), "Unable to create depth/stencil descriptor heap.");
        CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilViewDescriptor(m_depthStencilHeap->GetCPUDescriptorHandleForHeapStart());

        // Initialize the output attachments from render targets of the parent render pass.
        // NOTE: Again, we assume, that the parent render pass provides the render targets in an sorted manner.
        std::ranges::for_each(m_renderPass.renderTargets(), [&, i = 0](const RenderTarget& renderTarget) mutable {
            if (renderTarget.location() != i++) [[unlikely]]
                LITEFX_WARNING(DIRECTX12_LOG, "Remapped render target from location {0} to location {1}. Please make sure that the render targets are sorted within the render pass and do not have any gaps in their location mappings.", renderTarget.location(), i - 1);

            // Check if the device supports the multi sampling level for the render target.
            auto samples = m_renderPass.multiSamplingLevel();

            if (m_renderPass.device().maximumMultiSamplingLevel(renderTarget.format()) < samples)
                throw InvalidArgumentException("Render target {0} with format {1} does not support {2} samples.", i, renderTarget.format(), std::to_underlying(samples));

            const IDirectX12Image* renderTargetView;

            if (renderTarget.type() == RenderTargetType::Present && samples == MultiSamplingLevel::x1)
            {
                // If the render target is a present target and should not be multi-sampled, acquire an image view directly from the swap chain.
                // NOTE: Multi-sampling back-buffers directly is not supported (see https://docs.microsoft.com/en-us/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_effect#remarks).
                renderTargetView = m_renderPass.device().swapChain().image(m_bufferIndex);
            }
            else
            {
                // Create an image view for the render target.
                // TODO: Pass the optimized clear value from the render target to the attachment. (May need to refactor `CreateAttachment` to accept the render target and a size). Then
                //       remove the warning from the info queue.
                auto image = m_renderPass.device().factory().createAttachment(renderTarget.format(), m_size, m_renderPass.multiSamplingLevel());
                renderTargetView = image.get();
                m_outputAttachments.push_back(std::move(image));
            }

            if (renderTarget.type() == RenderTargetType::DepthStencil)
            {
                D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = { };
                depthStencilViewDesc.Format = DX12::getFormat(renderTarget.format());
                depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
                depthStencilViewDesc.Texture2D = { .MipSlice = 0 };
                depthStencilViewDesc.ViewDimension = samples == MultiSamplingLevel::x1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS;

                m_renderPass.device().handle()->CreateDepthStencilView(renderTargetView->handle().Get(), &depthStencilViewDesc, depthStencilViewDescriptor);
                depthStencilViewDescriptor = depthStencilViewDescriptor.Offset(m_depthStencilDescriptorSize);
            }
            else
            {
                D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { };
                renderTargetViewDesc.Format = DX12::getFormat(renderTarget.format());
                renderTargetViewDesc.ViewDimension = samples == MultiSamplingLevel::x1 ? D3D12_RTV_DIMENSION_TEXTURE2D : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                renderTargetViewDesc.Texture2D = { .MipSlice = 0, .PlaneSlice = 0 };
                renderTargetViewDesc.Buffer = { .FirstElement = 0, .NumElements = 1 };

                m_renderPass.device().handle()->CreateRenderTargetView(renderTargetView->handle().Get(), &renderTargetViewDesc, renderTargetViewDescriptor);
                renderTargetViewDescriptor = renderTargetViewDescriptor.Offset(m_renderTargetDescriptorSize);
            }

            m_renderTargetViews.push_back(renderTargetView);
        });
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12FrameBuffer::DirectX12FrameBuffer(const DirectX12RenderPass& renderPass, UInt32 bufferIndex, const Size2d& renderArea, UInt32 commandBuffers) :
    m_impl(makePimpl<DirectX12FrameBufferImpl>(this, renderPass, bufferIndex, renderArea, commandBuffers))
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

UInt32 DirectX12FrameBuffer::renderTargetDescriptorSize() const noexcept
{
    return m_impl->m_renderTargetDescriptorSize;
}

UInt32 DirectX12FrameBuffer::depthStencilTargetDescriptorSize() const noexcept
{
    return m_impl->m_depthStencilDescriptorSize;
}

UInt64& DirectX12FrameBuffer::lastFence() const noexcept
{
    return m_impl->m_lastFence;
}

UInt32 DirectX12FrameBuffer::bufferIndex() const noexcept
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

SharedPtr<const DirectX12CommandBuffer> DirectX12FrameBuffer::commandBuffer(UInt32 index) const
{
    if (index >= static_cast<UInt32>(m_impl->m_commandBuffers.size())) [[unlikely]]
        throw ArgumentOutOfRangeException("No command buffer with index {1} is stored in the frame buffer. The frame buffer only contains {0} command buffers.", m_impl->m_commandBuffers.size(), index);

    return m_impl->m_commandBuffers[index];
}

Enumerable<SharedPtr<const DirectX12CommandBuffer>> DirectX12FrameBuffer::commandBuffers() const noexcept
{
    return m_impl->m_commandBuffers;
}

Enumerable<const IDirectX12Image*> DirectX12FrameBuffer::images() const noexcept
{
    return m_impl->m_renderTargetViews;
}

const IDirectX12Image& DirectX12FrameBuffer::image(UInt32 location) const
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