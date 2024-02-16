#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanFrameBuffer::VulkanFrameBufferImpl : public Implement<VulkanFrameBuffer> {
public:
	friend class VulkanFrameBuffer;

private:
    Array<UniquePtr<IVulkanImage>> m_images;
    Array<VkImageView> m_imageViews;
    Dictionary<UInt64, IVulkanImage*> m_mappedRenderTargets;
	Size2d m_size;
    const VulkanDevice& m_device;

public:
    VulkanFrameBufferImpl(VulkanFrameBuffer* parent, const VulkanDevice& device, const Size2d& renderArea) :
        base(parent), m_device(device), m_size(renderArea)
	{
	}

    ~VulkanFrameBufferImpl()
    {
        this->cleanup();
    }

public:
    void cleanup()
    {
        for (auto& view : m_imageViews)
            ::vkDestroyImageView(m_device.handle(), view, nullptr);

        m_imageViews.clear();
    }

	void initialize()
	{
        // Define a factory callback for an image view.
        auto getImageView = [&](const UniquePtr<IVulkanImage>& image) -> VkImageView {
            VkImageViewCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .image = std::as_const(*image).handle(),
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = Vk::getFormat(image->format()),
                .components = VkComponentMapping {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange = VkImageSubresourceRange {
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            if (!::hasDepth(image->format()) && !::hasStencil(image->format()))
                createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            else
            {
                if (::hasDepth(image->format()))
                    createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
                if (::hasStencil(image->format()))
                    createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }

            VkImageView imageView;
            raiseIfFailed(::vkCreateImageView(m_device.handle(), &createInfo, nullptr, &imageView), "Unable to create image view.");
            return imageView;
        };


        // Create the image views for each image.
        m_imageViews = m_images | std::views::transform(getImageView) | std::ranges::to<Array<VkImageView>>();
	}

    void resize(const Size2d& renderArea)
    {
        // TODO: Resize/Re-allocate all images.
        m_size = renderArea;

        // Recreate all resources.
        auto images = m_images |
            std::views::transform([&](const UniquePtr<IVulkanImage>& image) { return m_device.factory().createTexture(image->name(), image->format(), renderArea, image->dimensions(), image->levels(), image->layers(), image->samples(), image->usage()); }) |
            std::views::as_rvalue | std::ranges::to<Array<UniquePtr<IVulkanImage>>>();
        m_images = std::move(images);

        // Re-initialize to update heaps and descriptors.
        this->initialize();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanFrameBuffer::VulkanFrameBuffer(const VulkanDevice& device, const Size2d& renderArea) :
	m_impl(makePimpl<VulkanFrameBufferImpl>(this, device, renderArea))
{
}

VulkanFrameBuffer::~VulkanFrameBuffer() noexcept = default;

const Size2d& VulkanFrameBuffer::size() const noexcept
{
	return m_impl->m_size;
}

size_t VulkanFrameBuffer::getWidth() const noexcept
{
	return m_impl->m_size.width();
}

size_t VulkanFrameBuffer::getHeight() const noexcept
{
	return m_impl->m_size.height();
}

void VulkanFrameBuffer::mapRenderTarget(const RenderTarget& renderTarget, UInt32 index)
{
    if (index >= m_impl->m_images.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("index", 0u, static_cast<UInt32>(m_impl->m_images.size()), index, "The frame buffer does not contain an image at index {0}.", index);

    m_impl->m_mappedRenderTargets[renderTarget.identifier()] = m_impl->m_images[index].get();
}

void VulkanFrameBuffer::unmapRenderTarget(const RenderTarget& renderTarget) noexcept
{
    m_impl->m_mappedRenderTargets.erase(renderTarget.identifier());
}

SharedPtr<const VulkanCommandBuffer> VulkanFrameBuffer::commandBuffer(UInt32 index) const
{
    throw;
}

Enumerable<SharedPtr<const VulkanCommandBuffer>> VulkanFrameBuffer::commandBuffers() const noexcept
{
    throw;
}

Enumerable<const IVulkanImage*> VulkanFrameBuffer::images() const noexcept
{
    return m_impl->m_images | std::views::transform([](auto& image) { return image.get(); });
}

const IVulkanImage& VulkanFrameBuffer::image(UInt32 index) const
{
    if (index >= m_impl->m_images.size())
        throw ArgumentOutOfRangeException("index", 0u, static_cast<UInt32>(m_impl->m_images.size()), index, "The frame buffer does not contain an image at index {0}.", index);

    return *m_impl->m_images[index];
}

const IVulkanImage& VulkanFrameBuffer::image(const RenderTarget& renderTarget) const
{
    if (!m_impl->m_mappedRenderTargets.contains(renderTarget.identifier())) [[unlikely]]
        throw InvalidArgumentException("renderTarget", "The frame buffer does not map an image to the provided render target \"{0}\".", renderTarget.name());

    return *m_impl->m_mappedRenderTargets[renderTarget.identifier()];
}

void VulkanFrameBuffer::addImage(const String& name, Format format, MultiSamplingLevel samples, ResourceUsage usage)
{
    // Add a new image.
    m_impl->m_images.push_back(std::move(m_impl->m_device.factory().createTexture(name, format, m_impl->m_size, ImageDimensions::DIM_2, 1u, 1u, samples, usage)));

    // Re-initialize to reset descriptor heaps and allocate descriptors.
    m_impl->initialize();
}

void VulkanFrameBuffer::resize(const Size2d& renderArea)
{
    // Reset the size and re-initialize the frame buffer.
    m_impl->resize(renderArea);
}