#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12FrameBuffer::DirectX12FrameBufferImpl {
public:
    friend class DirectX12FrameBuffer;

private:
    Array<UniquePtr<IDirectX12Image>> m_images;
    ComPtr<ID3D12DescriptorHeap> m_renderTargetHeap, m_depthStencilHeap;
    Dictionary<UInt64, IDirectX12Image*> m_mappedRenderTargets;
    Dictionary<const IDirectX12Image*, D3D12_CPU_DESCRIPTOR_HANDLE> m_renderTargetHandles;
    UInt32 m_renderTargetDescriptorSize{}, m_depthStencilDescriptorSize{};
    Size2d m_size;
    WeakPtr<const DirectX12Device> m_device;

public:
    DirectX12FrameBufferImpl(const DirectX12Device& device, Size2d renderArea) :
        m_size(std::move(renderArea)), m_device(device.weak_from_this())
    {
    }

public:
    void initialize()
    {
        // Check if the device is still valid.
        auto device = m_device.lock();

        if (device == nullptr) [[unlikely]]
            throw RuntimeException("Cannot initialize frame buffer for a released device instance.");

        // Create descriptor heaps for RTVs and DSVs.
        UInt32 renderTargets = static_cast<UInt32>(std::ranges::count_if(m_images, [](const auto& image) { return !::hasDepth(image->format()) && !::hasStencil(image->format()); }));
        UInt32 depthStencilTargets = static_cast<UInt32>(m_images.size()) - renderTargets;

        D3D12_DESCRIPTOR_HEAP_DESC renderTargetHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = renderTargets
        };

        D3D12_DESCRIPTOR_HEAP_DESC depthStencilHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = depthStencilTargets
        };

        m_renderTargetDescriptorSize = device->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_depthStencilDescriptorSize = device->handle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        raiseIfFailed(device->handle()->CreateDescriptorHeap(&renderTargetHeapDesc, IID_PPV_ARGS(&m_renderTargetHeap)), "Unable to create render target descriptor heap.");
        CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewDescriptor(m_renderTargetHeap->GetCPUDescriptorHandleForHeapStart());
        raiseIfFailed(device->handle()->CreateDescriptorHeap(&depthStencilHeapDesc, IID_PPV_ARGS(&m_depthStencilHeap)), "Unable to create depth/stencil descriptor heap.");
        CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilViewDescriptor(m_depthStencilHeap->GetCPUDescriptorHandleForHeapStart());

        // Initialize the output attachments from render targets of the parent render pass.
        m_renderTargetHandles.clear();

        std::ranges::for_each(m_images, [&, i = 0, device](const UniquePtr<IDirectX12Image>& image) mutable {
            // Check if the device supports the multi sampling level for the render target.
            auto samples = image->samples();
            auto format = image->format();

            if (device->maximumMultiSamplingLevel(format) < samples)
                throw RuntimeException("The image {0} with format {1} does not support {2} samples.", i, format, std::to_underlying(samples));

            if (::hasDepth(format) || ::hasStencil(format))
            {
                D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {
                    .Format = DX12::getFormat(format),
                    .ViewDimension = samples == MultiSamplingLevel::x1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS,
                    .Texture2D = { .MipSlice = 0 }
                };

                device->handle()->CreateDepthStencilView(std::as_const(*image).handle().Get(), &depthStencilViewDesc, depthStencilViewDescriptor);
                m_renderTargetHandles[image.get()] = depthStencilViewDescriptor;
                depthStencilViewDescriptor = depthStencilViewDescriptor.Offset(static_cast<INT>(m_depthStencilDescriptorSize));
            }
            else
            {
                D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {
                    .Format = DX12::getFormat(format),
                    .ViewDimension = samples == MultiSamplingLevel::x1 ? D3D12_RTV_DIMENSION_TEXTURE2D : D3D12_RTV_DIMENSION_TEXTURE2DMS,
                    .Texture2D = { .MipSlice = 0, .PlaneSlice = 0 }
                };

                device->handle()->CreateRenderTargetView(std::as_const(*image).handle().Get(), &renderTargetViewDesc, renderTargetViewDescriptor);
                m_renderTargetHandles[image.get()] = renderTargetViewDescriptor;
                renderTargetViewDescriptor = renderTargetViewDescriptor.Offset(static_cast<INT>(m_renderTargetDescriptorSize));
            }
        });
    }

    void resize(const Size2d& renderArea)
    {
        // Check if the device is still valid.
        auto device = m_device.lock();

        if (device == nullptr) [[unlikely]]
            throw RuntimeException("Cannot resize frame buffer on a released device instance.");

        // Resize/Re-allocate all images.
        m_size = renderArea;

        // Recreate all resources.
        Dictionary<const IDirectX12Image*, IDirectX12Image*> imageReplacements;

        auto images = m_images |
            std::views::transform([&](const UniquePtr<IDirectX12Image>& image) { 
                auto newImage = device->factory().createTexture(image->name(), image->format(), renderArea, image->dimensions(), image->levels(), image->layers(), image->samples(), image->usage()); 
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
    StateResource(name), m_impl(device, renderArea)
{
    m_impl->initialize();
}

DirectX12FrameBuffer::DirectX12FrameBuffer(DirectX12FrameBuffer&&) noexcept = default;
DirectX12FrameBuffer& DirectX12FrameBuffer::operator=(DirectX12FrameBuffer&&) noexcept = default;
DirectX12FrameBuffer::~DirectX12FrameBuffer() noexcept = default;

D3D12_CPU_DESCRIPTOR_HANDLE DirectX12FrameBuffer::descriptorHandle(UInt32 imageIndex) const
{
    if (imageIndex >= m_impl->m_images.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("imageIndex", std::make_pair(0uz, m_impl->m_images.size()), static_cast<size_t>(imageIndex), "The frame buffer does not contain an image at index {0}.", imageIndex);

    return m_impl->m_renderTargetHandles.at(m_impl->m_images[imageIndex].get());
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectX12FrameBuffer::descriptorHandle(StringView imageName) const
{
    auto nameHash = hash(imageName);

    if (auto match = std::ranges::find_if(m_impl->m_images, [nameHash](UniquePtr<IDirectX12Image>& image) { return hash(image->name()) == nameHash; }); match != m_impl->m_images.end())
        return m_impl->m_renderTargetHandles.at(match->get());
    else
        throw InvalidArgumentException("imageName", "The frame buffer does not contain an image with the name \"{0}\".", imageName);
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectX12FrameBuffer::descriptorHandle(const RenderTarget& renderTarget) const
{
    if (!m_impl->m_mappedRenderTargets.contains(renderTarget.identifier())) [[unlikely]]
        throw InvalidArgumentException("renderTarget", "The frame buffer does not map an image to the provided render target \"{0}\".", renderTarget.name());

    return m_impl->m_renderTargetHandles.at(m_impl->m_mappedRenderTargets[renderTarget.identifier()]);
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
        throw ArgumentOutOfRangeException("index", std::make_pair(0uz, m_impl->m_images.size()), static_cast<size_t>(index), "The frame buffer does not contain an image at index {0}.", index);

    if (m_impl->m_images[index]->format() != renderTarget.format()) [[unlikely]]
        LITEFX_WARNING(DIRECTX12_LOG, "The render target format {0} does not match the image format {1} for image {2}.", renderTarget.format(), m_impl->m_images[index]->format(), index);

    m_impl->m_mappedRenderTargets[renderTarget.identifier()] = m_impl->m_images[index].get();
}

void DirectX12FrameBuffer::mapRenderTarget(const RenderTarget& renderTarget, StringView name)
{
    auto nameHash = hash(name);

    if (auto match = std::ranges::find_if(m_impl->m_images, [nameHash](UniquePtr<IDirectX12Image>& image) { return hash(image->name()) == nameHash; }); match != m_impl->m_images.end())
        this->mapRenderTarget(renderTarget, static_cast<UInt32>(std::ranges::distance(m_impl->m_images.begin(), match)));
    else
        throw InvalidArgumentException("name", "The frame buffer does not contain an image with the name \"{0}\".", name);
}

void DirectX12FrameBuffer::unmapRenderTarget(const RenderTarget& renderTarget) noexcept
{
    m_impl->m_mappedRenderTargets.erase(renderTarget.identifier());
}

Enumerable<const IDirectX12Image*> DirectX12FrameBuffer::images() const
{
    return m_impl->m_images | std::views::transform([](auto& image) { return image.get(); });
}

const IDirectX12Image& DirectX12FrameBuffer::image(UInt32 index) const
{
    if (index >= m_impl->m_images.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("index", std::make_pair(0uz, m_impl->m_images.size()), static_cast<size_t>(index), "The frame buffer does not contain an image at index {0}.", index);

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
    // Check if the device is still valid.
    auto device = m_impl->m_device.lock();

    if (device == nullptr) [[unlikely]]
        throw RuntimeException("Cannot add image to frame buffer of a released device instance.");

    // Check if there's already another image with the same name.
    auto nameHash = hash(name);

    if (auto match = std::ranges::find_if(m_impl->m_images, [nameHash](auto& image) { return hash(image->name()) == nameHash; }); match != m_impl->m_images.end()) [[unlikely]]
        throw InvalidArgumentException("name", "Another image with the name {0} does already exist within the frame buffer.", name);

    // Add a new image.
    m_impl->m_images.push_back(device->factory().createTexture(name, format, m_impl->m_size, ImageDimensions::DIM_2, 1u, 1u, samples, usage));
    
    // Re-initialize to reset descriptor heaps and allocate descriptors.
    m_impl->initialize();
}

void DirectX12FrameBuffer::addImage(const String& name, const RenderTarget& renderTarget, MultiSamplingLevel samples, ResourceUsage usage)
{
    // Check if the device is still valid.
    auto device = m_impl->m_device.lock();

    if (device == nullptr) [[unlikely]]
        throw RuntimeException("Cannot add image to frame buffer of a released device instance.");

    // Check if there's already another image with the same name.
    auto nameHash = hash(name);
    
    if (auto match = std::ranges::find_if(m_impl->m_images, [nameHash](auto& image) { return hash(image->name()) == nameHash; }); match != m_impl->m_images.end()) [[unlikely]]
        throw InvalidArgumentException("name", "Another image with the name {0} does already exist within the frame buffer.", name);

    // Add a new image.
    auto index = m_impl->m_images.size();
    m_impl->m_images.push_back(device->factory().createTexture(name, renderTarget.format(), m_impl->m_size, ImageDimensions::DIM_2, 1u, 1u, samples, usage));

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