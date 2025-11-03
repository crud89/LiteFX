#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanFrameBuffer::VulkanFrameBufferImpl {
public:
	friend class VulkanFrameBuffer;

private:
    Array<SharedPtr<const IVulkanImage>> m_images;
    Dictionary<SharedPtr<const IVulkanImage>, VkImageView> m_renderTargetHandles;
    Dictionary<UInt64, SharedPtr<const IVulkanImage>> m_mappedRenderTargets;
    WeakPtr<const VulkanDevice> m_device;
	Size2d m_size;
    Optional<allocation_callback_type> m_allocationCallback;

public:
    VulkanFrameBufferImpl(const VulkanDevice& device, Size2d renderArea, Optional<allocation_callback_type> allocationCallback = std::nullopt) :
        m_device(device.weak_from_this()), m_size(std::move(renderArea)), m_allocationCallback(allocationCallback)
	{
	}

    VulkanFrameBufferImpl(VulkanFrameBufferImpl&&) noexcept = default;
    VulkanFrameBufferImpl(const VulkanFrameBufferImpl&) = delete;
    VulkanFrameBufferImpl& operator=(VulkanFrameBufferImpl&&) noexcept = default;
    VulkanFrameBufferImpl& operator=(const VulkanFrameBufferImpl&) = delete;

    ~VulkanFrameBufferImpl() noexcept
    {
        // Check if the device is still valid.
        auto device = m_device.lock();

        if (device == nullptr) [[unlikely]]
            LITEFX_FATAL_ERROR(VULKAN_LOG, "Invalid attempt to release frame buffer after parent device.");
        else
            this->cleanup(*device);
    }

private:
    inline SharedPtr<const IVulkanImage> createImage(const VulkanDevice& device, UInt64 renderTargetId, Size2d size, ResourceUsage usage, Format format, MultiSamplingLevel samples, const String& name) const {
        if (this->m_allocationCallback.has_value())
        {
            auto image = m_allocationCallback.value()(renderTargetId, size, usage, format, samples, name);

            if (image != nullptr)
                return image;
        }

        return device.factory().createTexture(name, format, size, ImageDimensions::DIM_2, 1u, 1u, samples, usage);
    }

public:
    void cleanup(const VulkanDevice& device) noexcept
    {
        for (auto view : m_renderTargetHandles | std::views::values)
            ::vkDestroyImageView(device.handle(), view, nullptr);

        m_renderTargetHandles.clear();
    }

	void initialize()
	{
        // Check if the device is still valid.
        auto device = m_device.lock();

        if (device == nullptr) [[unlikely]]
            throw RuntimeException("Cannot allocate frame buffer from a released device instance.");

        // Define a factory callback for an image view.
        auto getImageView = [&](auto& image) -> std::pair<SharedPtr<const IVulkanImage>, VkImageView> {
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

            VkImageView imageView{};
            raiseIfFailed(::vkCreateImageView(device->handle(), &createInfo, nullptr, &imageView), "Unable to create image view.");
            return { image, imageView };
        };

        // Destroy the previous image views.
        this->cleanup(*device);

        // Create the image views for each image.
        m_renderTargetHandles = m_images | std::views::transform(getImageView) | std::ranges::to<Dictionary<SharedPtr<const IVulkanImage>, VkImageView>>();

#ifndef NDEBUG
        // Set debug names.
        std::ranges::for_each(m_images, [device](auto& image) {
            device->setDebugName(std::as_const(*image).handle(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image->name().c_str());
        });
#endif
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
        Dictionary<const IVulkanImage*, SharedPtr<const IVulkanImage>> imageReplacements;
        auto& queue = device->defaultQueue(QueueType::Graphics);
        auto commandBuffer = queue.createCommandBuffer(true);
        auto barrier = commandBuffer->makeBarrier(PipelineStage::None, PipelineStage::None);

        auto images = m_mappedRenderTargets |
            std::views::transform([&](const auto& resource) { 
                const auto& [renderTargetId, image] = resource;
                auto format = image->format();
                auto newImage = this->createImage(*device, renderTargetId, renderArea, image->usage(), format, image->samples(), image->name());
                imageReplacements[image.get()] = newImage;

                if (::hasDepth(format) || ::hasStencil(format))
                    barrier->transition(*newImage, ResourceAccess::None, ResourceAccess::None, ImageLayout::DepthRead);
                else
                    barrier->transition(*newImage, ResourceAccess::None, ResourceAccess::None, ImageLayout::ShaderResource);

                return newImage;
            }) | std::views::as_rvalue | std::ranges::to<Array<SharedPtr<const IVulkanImage>>>();

        // Transition the image layouts into their expected states.
        commandBuffer->barrier(*barrier);
        auto fence = queue.submit(commandBuffer);

        // Update the mappings.
        std::ranges::for_each(m_mappedRenderTargets | std::views::values, [&imageReplacements](auto& image) { image = imageReplacements.at(image.get()); });

        // Store the new images.
        m_images = std::move(images);

        // Re-initialize to update heaps and descriptors.
        this->initialize();

        // Wait for the fence to finish.
        queue.waitFor(fence);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanFrameBuffer::VulkanFrameBuffer(const VulkanDevice& device, const Size2d& renderArea, StringView name) :
    StateResource(name), m_impl(device, renderArea)
{
}

VulkanFrameBuffer::VulkanFrameBuffer(const VulkanDevice& device, const Size2d& renderArea, allocation_callback_type allocationCallback, StringView name) :
    StateResource(name), m_impl(device, renderArea, allocationCallback)
{
}

VulkanFrameBuffer::~VulkanFrameBuffer() noexcept = default;

VkImageView VulkanFrameBuffer::imageView(UInt32 imageIndex) const
{
    if (imageIndex >= m_impl->m_images.size()) [[unlikely]]
        throw ArgumentOutOfRangeException("imageIndex", std::make_pair(0uz, m_impl->m_images.size()), static_cast<size_t>(imageIndex), "The frame buffer does not contain an image at index {0}.", imageIndex);

    return m_impl->m_renderTargetHandles.at(m_impl->m_images[imageIndex]);
}

VkImageView VulkanFrameBuffer::imageView(StringView imageName) const
{
    auto nameHash = hash(imageName);

    if (auto match = std::ranges::find_if(m_impl->m_images, [nameHash](auto& image) { return hash(image->name()) == nameHash; }); match != m_impl->m_images.end())
        return m_impl->m_renderTargetHandles.at(*match);
    else
        throw InvalidArgumentException("imageName", "The frame buffer does not contain an image with the name \"{0}\".", imageName);
}

VkImageView VulkanFrameBuffer::imageView(const RenderTarget& renderTarget) const
{
    if (!m_impl->m_mappedRenderTargets.contains(renderTarget.identifier())) [[unlikely]]
        throw InvalidArgumentException("renderTarget", "The frame buffer does not map an image to the provided render target \"{0}\".", renderTarget.name());

    return m_impl->m_renderTargetHandles.at(m_impl->m_mappedRenderTargets[renderTarget.identifier()]);
}

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
        throw ArgumentOutOfRangeException("index", std::make_pair(0uz, m_impl->m_images.size()), static_cast<size_t>(index), "The frame buffer does not contain an image at index {0}.", index);

    if (m_impl->m_images[index]->format() != renderTarget.format()) [[unlikely]]
        LITEFX_WARNING(VULKAN_LOG, "The render target format {0} does not match the image format {1} for image {2}.", renderTarget.format(), m_impl->m_images[index]->format(), index);

    m_impl->m_mappedRenderTargets[renderTarget.identifier()] = m_impl->m_images[index];
}

void VulkanFrameBuffer::mapRenderTarget(const RenderTarget& renderTarget, StringView name)
{
    auto nameHash = hash(name);

    if (auto match = std::ranges::find_if(m_impl->m_images, [nameHash](auto& image) { return hash(image->name()) == nameHash; }); match != m_impl->m_images.end())
        this->mapRenderTarget(renderTarget, static_cast<UInt32>(std::ranges::distance(m_impl->m_images.begin(), match)));
    else
        throw InvalidArgumentException("name", "The frame buffer does not contain an image with the name \"{0}\".", name);
}

void VulkanFrameBuffer::unmapRenderTarget(const RenderTarget& renderTarget) noexcept
{
    m_impl->m_mappedRenderTargets.erase(renderTarget.identifier());
}

const Array<SharedPtr<const IVulkanImage>>& VulkanFrameBuffer::images() const
{
    return m_impl->m_images;
}

const IVulkanImage& VulkanFrameBuffer::image(UInt32 index) const
{
    if (index >= m_impl->m_images.size())
        throw ArgumentOutOfRangeException("index", std::make_pair(0uz, m_impl->m_images.size()), static_cast<size_t>(index), "The frame buffer does not contain an image at index {0}.", index);

    return *m_impl->m_images[index];
}

const IVulkanImage& VulkanFrameBuffer::image(const RenderTarget& renderTarget) const
{
    if (!m_impl->m_mappedRenderTargets.contains(renderTarget.identifier())) [[unlikely]]
        throw InvalidArgumentException("renderTarget", "The frame buffer does not map an image to the provided render target \"{0}\".", renderTarget.name());

    return *m_impl->m_mappedRenderTargets[renderTarget.identifier()];
}

const IVulkanImage& VulkanFrameBuffer::resolveImage(UInt64 hash) const
{
    if (!m_impl->m_mappedRenderTargets.contains(hash)) [[unlikely]]
        throw InvalidArgumentException("renderTarget", "The frame buffer does not map an image to the provided render target name hash \"0x{0:016X}\".", hash);

    return *m_impl->m_mappedRenderTargets[hash];
}

void VulkanFrameBuffer::addImage(const String& name, Format format, MultiSamplingLevel samples, ResourceUsage usage)
{
    // Check if the device is still valid.
    auto device = m_impl->m_device.lock();

    if (device == nullptr) [[unlikely]]
        throw RuntimeException("Cannot add image to frame buffer on a released device instance.");

    // Check if there's already another image with the same name.
    auto nameHash = hash(name);

    if (auto match = std::ranges::find_if(m_impl->m_images, [nameHash](auto& image) { return hash(image->name()) == nameHash; }); match != m_impl->m_images.end()) [[unlikely]]
        throw InvalidArgumentException("name", "Another image with the name {0} does already exist within the frame buffer.", name);

    // Add a new image...
    auto newImage = m_impl->createImage(*device, nameHash, m_impl->m_size, usage, format, samples, name);
    m_impl->m_images.push_back(newImage);

    // ... and make sure it is in the right layout.
    auto& queue = device->defaultQueue(QueueType::Graphics);
    auto commandBuffer = queue.createCommandBuffer(true);
    auto barrier = commandBuffer->makeBarrier(PipelineStage::None, PipelineStage::None);
    if (::hasDepth(format) || ::hasStencil(format))
        barrier->transition(*newImage, ResourceAccess::None, ResourceAccess::None, ImageLayout::DepthRead);
    else
        barrier->transition(*newImage, ResourceAccess::None, ResourceAccess::None, ImageLayout::ShaderResource);
    commandBuffer->barrier(*barrier);
    auto fence = queue.submit(commandBuffer);

    // Re-initialize to reset descriptor heaps and allocate descriptors.
    m_impl->initialize();

    // Wait for the fence to finish.
    queue.waitFor(fence);
}

void VulkanFrameBuffer::addImage(const String& name, const RenderTarget& renderTarget, MultiSamplingLevel samples, ResourceUsage usage)
{
    // Check if the device is still valid.
    auto device = m_impl->m_device.lock();

    if (device == nullptr) [[unlikely]]
        throw RuntimeException("Cannot add image to frame buffer on a released device instance.");

    // Check if there's already another image with the same name.
    auto nameHash = hash(name);

    if (auto match = std::ranges::find_if(m_impl->m_images, [nameHash](auto& image) { return hash(image->name()) == nameHash; }); match != m_impl->m_images.end()) [[unlikely]]
        throw InvalidArgumentException("name", "Another image with the name {0} does already exist within the frame buffer.", name);

    // Add a new image...
    auto index = m_impl->m_images.size();
    auto format = renderTarget.format();
    auto newImage = m_impl->createImage(*device, nameHash, m_impl->m_size, usage, format, samples, name);
    m_impl->m_images.push_back(newImage);

    // ... and make sure it is in the right layout.
    auto& queue = device->defaultQueue(QueueType::Graphics);
    auto commandBuffer = queue.createCommandBuffer(true);
    auto barrier = commandBuffer->makeBarrier(PipelineStage::None, PipelineStage::None);
    if (::hasDepth(format) || ::hasStencil(format))
        barrier->transition(*newImage, ResourceAccess::None, ResourceAccess::None, ImageLayout::DepthRead);
    else
        barrier->transition(*newImage, ResourceAccess::None, ResourceAccess::None, ImageLayout::ShaderResource);
    commandBuffer->barrier(*barrier);
    auto fence = queue.submit(commandBuffer);

    // Re-initialize to reset descriptor heaps and allocate descriptors.
    m_impl->initialize();

    // Map the render target to the image.
    this->mapRenderTarget(renderTarget, static_cast<UInt32>(index));

    // Wait for the fence to finish.
    queue.waitFor(fence);
}

void VulkanFrameBuffer::resize(const Size2d& renderArea)
{
    this->resizing(this, { renderArea });
    m_impl->resize(renderArea);
    this->resized(this, { renderArea });
}