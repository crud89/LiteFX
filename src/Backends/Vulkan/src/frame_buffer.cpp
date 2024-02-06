#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanFrameBuffer::VulkanFrameBufferImpl : public Implement<VulkanFrameBuffer> {
public:
	friend class VulkanFrameBuffer;

private:
    const VulkanRenderPass& m_renderPass;
    Array<UniquePtr<IVulkanImage>> m_outputAttachments;
    Array<IVulkanImage*> m_renderTargetViews;
	Array<SharedPtr<VulkanCommandBuffer>> m_commandBuffers;
    Array<VkImageView> m_attachmentViews;
	Size2d m_size;
    UInt32 m_bufferIndex;
    UInt64 m_lastFence{ 0 };

public:
    VulkanFrameBufferImpl(VulkanFrameBuffer* parent, const VulkanRenderPass& renderPass, UInt32 bufferIndex, const Size2d& renderArea, UInt32 commandBuffers) :
        base(parent), m_bufferIndex(bufferIndex), m_size(renderArea), m_renderPass(renderPass)
	{
        // Retrieve a command buffer from the graphics queue.
        m_commandBuffers.resize(commandBuffers);
        std::ranges::generate(m_commandBuffers, [this]() { return m_renderPass.commandQueue().createCommandBuffer(false, true); });
	}

    ~VulkanFrameBufferImpl()
    {
        this->cleanup();
    }

public:
    void cleanup()
    {
        for (auto& view : m_attachmentViews)
            ::vkDestroyImageView(m_renderPass.device().handle(), view, nullptr);

        m_attachmentViews.clear();
    }

	VkFramebuffer initialize()
	{
        // Get a reference to the device, the parent render pass is created on.
        const auto& device = m_renderPass.device();

        // Define a factory callback for an image view.
        auto getImageView = [&](const IVulkanImage* image) -> VkImageView {
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
            raiseIfFailed(::vkCreateImageView(device.handle(), &createInfo, nullptr, &imageView), "Unable to create image view.");
            return imageView;
        };

        // Clear earlier images.
        m_renderTargetViews.clear();
        m_outputAttachments.clear();

        // Retrieve the image views for the input and output attachments. Start with the input attachments.
        // NOTE: We assume, that the parent render pass provides the attachments in an sorted manner.
        std::ranges::for_each(m_renderPass.inputAttachments(), [&, i = 0](const VulkanRenderPassDependency& inputAttachment) mutable {
            if (inputAttachment.location() != i) [[unlikely]]
                LITEFX_WARNING(VULKAN_LOG, "Remapped input attachment from location {0} to location {1}. Please make sure that the input attachments are sorted within the render pass and do not have any gaps in their location mappings.", inputAttachment.location(), i);

            if (inputAttachment.renderTarget().type() == RenderTargetType::Present)
                throw InvalidArgumentException("renderPass", "The input attachment mapped to location {0} is a present target, which cannot be used as input attachment.", i);

            if (inputAttachment.inputAttachmentSource() == nullptr)
                throw InvalidArgumentException("renderPass", "The input attachment mapped to location {0} has no initialized source.", i);

            // Store the image view from the source frame buffer.
            auto imageView = getImageView(&inputAttachment.inputAttachmentSource()->frameBuffer(m_bufferIndex).image(i++));
            m_attachmentViews.push_back(imageView);
        });

        // Initialize the output attachments from render targets of the parent render pass.
        // NOTE: Again, we assume, that the parent render pass provides the render targets in an sorted manner.
        auto samples = m_renderPass.multiSamplingLevel();

        std::ranges::for_each(m_renderPass.renderTargets(), [&, i = 0](const RenderTarget& renderTarget) mutable {
            if (renderTarget.location() != i++) [[unlikely]]
                LITEFX_WARNING(VULKAN_LOG, "Remapped render target from location {0} to location {1}. Please make sure that the render targets are sorted within the render pass and do not have any gaps in their location mappings.", renderTarget.location(), i - 1);

            if (renderTarget.type() == RenderTargetType::Present && samples == MultiSamplingLevel::x1)
            {
                // If the render target is a present target, acquire an image view from the swap chain.
                auto image = device.swapChain().image(m_bufferIndex);
                m_renderTargetViews.push_back(image);
                m_attachmentViews.push_back(getImageView(image));
            }
            else
            {
                // Create an image view for the render target.
                auto image = device.factory().createAttachment(renderTarget, m_size, samples);
                m_attachmentViews.push_back(getImageView(image.get()));
                m_renderTargetViews.push_back(image.get());
                m_outputAttachments.push_back(std::move(image));
            }
        });

        // If we have a present target and multi sampling is enabled, make sure to add a view for the resolve attachment.
        if (samples > MultiSamplingLevel::x1 && std::ranges::any_of(m_renderPass.renderTargets(), [](const RenderTarget& renderTarget) { return renderTarget.type() == RenderTargetType::Present; }))
        {
            auto image = device.swapChain().image(m_bufferIndex);
            m_renderTargetViews.push_back(image);
            m_attachmentViews.push_back(getImageView(image));
        }

        // Allocate the frame buffer.
        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = m_renderPass.handle();
        frameBufferInfo.attachmentCount = static_cast<UInt32>(m_attachmentViews.size());
        frameBufferInfo.pAttachments = m_attachmentViews.data();
        frameBufferInfo.width = m_size.width();
        frameBufferInfo.height = m_size.height();
        frameBufferInfo.layers = 1;

        VkFramebuffer frameBuffer;
        raiseIfFailed(::vkCreateFramebuffer(device.handle(), &frameBufferInfo, nullptr, &frameBuffer), "Unable to create frame buffer from swap chain frame.");

        return frameBuffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanFrameBuffer::VulkanFrameBuffer(const VulkanRenderPass& renderPass, UInt32 bufferIndex, const Size2d& renderArea, UInt32 commandBuffers) :
	m_impl(makePimpl<VulkanFrameBufferImpl>(this, renderPass, bufferIndex, renderArea, commandBuffers)), Resource<VkFramebuffer>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize();
}

VulkanFrameBuffer::~VulkanFrameBuffer() noexcept
{
    ::vkDestroyFramebuffer(m_impl->m_renderPass.device().handle(), this->handle(), nullptr);
}

UInt64& VulkanFrameBuffer::lastFence() noexcept
{
    return m_impl->m_lastFence;
}

UInt64 VulkanFrameBuffer::lastFence() const noexcept
{
    return m_impl->m_lastFence;
}

UInt32 VulkanFrameBuffer::bufferIndex() const noexcept
{
    return m_impl->m_bufferIndex;
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

SharedPtr<const VulkanCommandBuffer> VulkanFrameBuffer::commandBuffer(UInt32 index) const
{
    if (index >= static_cast<UInt32>(m_impl->m_commandBuffers.size())) [[unlikely]]
        throw ArgumentOutOfRangeException("index", 0u, static_cast<UInt32>(m_impl->m_commandBuffers.size()), index, "No command buffer with index {1} is stored in the frame buffer. The frame buffer only contains {0} command buffers.", m_impl->m_commandBuffers.size(), index);

	return m_impl->m_commandBuffers[index];
}

Enumerable<SharedPtr<const VulkanCommandBuffer>> VulkanFrameBuffer::commandBuffers() const noexcept
{
    return m_impl->m_commandBuffers;
}

Enumerable<IVulkanImage*> VulkanFrameBuffer::images() const noexcept
{
    return m_impl->m_renderTargetViews;
}

IVulkanImage& VulkanFrameBuffer::image(UInt32 location) const
{
    if (location >= m_impl->m_renderTargetViews.size())
        throw ArgumentOutOfRangeException("location", 0u, static_cast<UInt32>(m_impl->m_renderTargetViews.size()), location, "No render target is mapped to location {0}.", location);

    return *m_impl->m_renderTargetViews[location];
}

void VulkanFrameBuffer::resize(const Size2d& renderArea)
{
    // Destroy the old frame buffer.
    ::vkDestroyFramebuffer(m_impl->m_renderPass.device().handle(), this->handle(), nullptr);

    // Reset the size and re-initialize the frame buffer.
    m_impl->m_size = renderArea;
    m_impl->cleanup();
    this->handle() = m_impl->initialize();
}