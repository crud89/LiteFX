#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12FrameBuffer::DirectX12FrameBufferImpl : public Implement<DirectX12FrameBuffer> {
public:
    friend class DirectX12FrameBuffer;

private:
    Array<UniquePtr<IDirectX12Image>> m_images;
    ComPtr<ID3D12DescriptorHeap> m_renderTargetHeap, m_depthStencilHeap;
    Dictionary<UInt64, IDirectX12Image*> m_mappedRenderTargets;
    UInt32 m_renderTargetDescriptorSize, m_depthStencilDescriptorSize;
    Size2d m_size;
    const DirectX12Device& m_device;

public:
    DirectX12FrameBufferImpl(DirectX12FrameBuffer* parent, const DirectX12Device& device, const Size2d& renderArea) :
        base(parent), m_size(renderArea), m_device(device)
    {
    }

public:
    void initialize()
    {
        // Create descriptor heaps for RTVs and DSVs.
        UInt32 renderTargets = std::ranges::count_if(m_images, [](const auto& image) { return !::hasDepth(image->format()) && !::hasStencil(image->format()); });
        UInt32 depthStencilTargets = static_cast<UInt32>(m_images.size()) - renderTargets;

        D3D12_DESCRIPTOR_HEAP_DESC renderTargetHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = renderTargets
        };

        D3D12_DESCRIPTOR_HEAP_DESC depthStencilHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = depthStencilTargets
        };

        m_renderTargetDescriptorSize = m_device.handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_depthStencilDescriptorSize = m_device.handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        raiseIfFailed(m_device.handle()->CreateDescriptorHeap(&renderTargetHeapDesc, IID_PPV_ARGS(&m_renderTargetHeap)), "Unable to create render target descriptor heap.");
        CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewDescriptor(m_renderTargetHeap->GetCPUDescriptorHandleForHeapStart());
        raiseIfFailed(m_device.handle()->CreateDescriptorHeap(&depthStencilHeapDesc, IID_PPV_ARGS(&m_depthStencilHeap)), "Unable to create depth/stencil descriptor heap.");
        CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilViewDescriptor(m_depthStencilHeap->GetCPUDescriptorHandleForHeapStart());

        // Initialize the output attachments from render targets of the parent render pass.
        // NOTE: Again, we assume, that the parent render pass provides the render targets in an sorted manner.
        std::ranges::for_each(m_images, [&, i = 0](const UniquePtr<IDirectX12Image>& image) mutable {
            // Check if the device supports the multi sampling level for the render target.
            auto samples = image->samples();
            auto format = image->format();

            if (m_device.maximumMultiSamplingLevel(format) < samples)
                throw RuntimeException("The image {0} with format {1} does not support {2} samples.", i, format, std::to_underlying(samples));

            if (::hasDepth(format) || ::hasStencil(format))
            {
                D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {
                    .Format = DX12::getFormat(format),
                    .ViewDimension = samples == MultiSamplingLevel::x1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS,
                    .Texture2D = { .MipSlice = 0 }
                };

                m_device.handle()->CreateDepthStencilView(std::as_const(*image).handle().Get(), &depthStencilViewDesc, depthStencilViewDescriptor);
                depthStencilViewDescriptor = depthStencilViewDescriptor.Offset(m_depthStencilDescriptorSize);
            }
            else
            {
                D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {
                    .Format = DX12::getFormat(format),
                    .ViewDimension = samples == MultiSamplingLevel::x1 ? D3D12_RTV_DIMENSION_TEXTURE2D : D3D12_RTV_DIMENSION_TEXTURE2DMS,
                    .Texture2D = { .MipSlice = 0, .PlaneSlice = 0 }
                };

                m_device.handle()->CreateRenderTargetView(std::as_const(*image).handle().Get(), &renderTargetViewDesc, renderTargetViewDescriptor);
                renderTargetViewDescriptor = renderTargetViewDescriptor.Offset(m_renderTargetDescriptorSize);
            }
        });
    }

    void resize(const Size2d& renderArea)
    {
        // TODO: Resize/Re-allocate all images.
        m_size = renderArea;

        // Recreate all resources.
        Dictionary<const IDirectX12Image*, IDirectX12Image*> imageReplacements;

        auto images = m_images |
            std::views::transform([&](const UniquePtr<IDirectX12Image>& image) { 
                auto newImage = m_device.factory().createTexture(image->name(), image->format(), renderArea, image->dimensions(), image->levels(), image->layers(), image->samples(), image->usage()); 
                imageReplacements[image.get()] = newImage.get();
                return std::move(newImage);
            }) | std::views::as_rvalue | std::ranges::to<Array<UniquePtr<IDirectX12Image>>>();

        // Update the mappings.
        std::ranges::for_each(m_mappedRenderTargets | std::views::values, [&imageReplacements](auto& image) { image = imageReplacements[image]; });

        // Store the new images.
        m_images = std::move(images);

        // Re-initialize to update heaps and descriptors.
        this->initialize();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12FrameBuffer::DirectX12FrameBuffer(const DirectX12Device& device, const Size2d& renderArea, StringView name) :
    m_impl(makePimpl<DirectX12FrameBufferImpl>(this, device, renderArea)), StateResource(name)
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

void DirectX12FrameBuffer::mapRenderTarget(const RenderTarget& renderTarget, UInt32 index)
{
    if (index >= m_impl->m_images.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("index", 0u, static_cast<UInt32>(m_impl->m_images.size()), index, "The frame buffer does not contain an image at index {0}.", index);

    if (m_impl->m_images[index]->format() != renderTarget.format()) [[unlikely]]
        LITEFX_WARNING(DIRECTX12_LOG, "The render target format {0} does not match the image format {1} for image {2}.", renderTarget.format(), m_impl->m_images[index]->format(), index);

    m_impl->m_mappedRenderTargets[renderTarget.identifier()] = m_impl->m_images[index].get();
}

void DirectX12FrameBuffer::unmapRenderTarget(const RenderTarget& renderTarget) noexcept
{
    m_impl->m_mappedRenderTargets.erase(renderTarget.identifier());
}

Enumerable<const IDirectX12Image*> DirectX12FrameBuffer::images() const noexcept
{
    return m_impl->m_images | std::views::transform([](auto& image) { return image.get(); });
}

const IDirectX12Image& DirectX12FrameBuffer::image(UInt32 index) const
{
    if (index >= m_impl->m_images.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("index", 0u, static_cast<UInt32>(m_impl->m_images.size()), index, "The frame buffer does not contain an image at index {0}.", index);

    return *m_impl->m_images[index];
}

const IDirectX12Image& DirectX12FrameBuffer::image(const RenderTarget& renderTarget) const
{
    if (!m_impl->m_mappedRenderTargets.contains(renderTarget.identifier())) [[unlikely]]
        throw InvalidArgumentException("renderTarget", "The frame buffer does not map an image to the provided render target \"{0}\".", renderTarget.name());

    return *m_impl->m_mappedRenderTargets[renderTarget.identifier()];
}

const IDirectX12Image& DirectX12FrameBuffer::resolveImage(UInt64 hash) const
{
    if (!m_impl->m_mappedRenderTargets.contains(hash)) [[unlikely]]
        throw InvalidArgumentException("renderTarget", "The frame buffer does not map an image to the provided render target name hash \"0x{0:016X}\".", hash);

    return *m_impl->m_mappedRenderTargets[hash];
}

void DirectX12FrameBuffer::addImage(const String& name, Format format, MultiSamplingLevel samples, ResourceUsage usage)
{
    // Add a new image.
    m_impl->m_images.push_back(std::move(m_impl->m_device.factory().createTexture(name, format, m_impl->m_size, ImageDimensions::DIM_2, 1u, 1u, samples, usage)));
    
    // Re-initialize to reset descriptor heaps and allocate descriptors.
    m_impl->initialize();
}

void DirectX12FrameBuffer::addImage(const String& name, const RenderTarget& renderTarget, MultiSamplingLevel samples, ResourceUsage usage)
{
    // Add a new image.
    auto index = m_impl->m_images.size();
    m_impl->m_images.push_back(std::move(m_impl->m_device.factory().createTexture(name, renderTarget.format(), m_impl->m_size, ImageDimensions::DIM_2, 1u, 1u, samples, usage)));

    // Re-initialize to reset descriptor heaps and allocate descriptors.
    m_impl->initialize();

    // Map the render target to the image.
    this->mapRenderTarget(renderTarget, static_cast<UInt32>(index));
}

void DirectX12FrameBuffer::resize(const Size2d& renderArea)
{
    m_impl->resize(renderArea);
    this->resized(this, { renderArea });
}