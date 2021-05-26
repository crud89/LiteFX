#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanFrameBuffer::VulkanFrameBufferImpl : public Implement<VulkanFrameBuffer> {
public:
	friend class VulkanFrameBuffer;

private:
    Array<UniquePtr<IVulkanImage>> m_outputAttachments;
    Array<const IVulkanImage*> m_renderTargetViews;
	UniquePtr<VulkanCommandBuffer> m_commandBuffer;
	Size2d m_size;
	VkSemaphore m_semaphore;
    UInt32 m_bufferIndex;

public:
    VulkanFrameBufferImpl(VulkanFrameBuffer* parent, const UInt32& bufferIndex) :
        base(parent), m_bufferIndex(bufferIndex)
	{
		// Initialize the semaphore.
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		raiseIfFailed<RuntimeException>(::vkCreateSemaphore(m_parent->getDevice()->handle(), &semaphoreInfo, nullptr, &m_semaphore), "Unable to create swap semaphore on frame buffer.");
	}

	~VulkanFrameBufferImpl()
	{
		::vkDestroySemaphore(m_parent->getDevice()->handle(), m_semaphore, nullptr);
	}

public:
	VkFramebuffer initialize(const Size2d& size)
	{
        // Retrieve the image views for the input and output attachments.
        Array<VkImageView> attachmentViews;

        // Start with the input attachments.
        // NOTE: We assume, that the parent render pass provides the attachments in an sorted manner.
        std::ranges::for_each(m_parent->parent().inputAttachments(), [&, i = 0](const VulkanInputAttachmentMapping& inputAttachment) mutable {
            if (inputAttachment.location() != i) [[unlikely]]
                LITEFX_WARNING(VULKAN_LOG, "Remapped input attachment from location {0} to location {1}. Please make sure that the input attachments are sorted within the render pass and do not have any gaps in their location mappings.", inputAttachment.location(), i);

            if (inputAttachment.renderTarget().type() == RenderTargetType::Present)
                throw InvalidArgumentException("The input attachment mapped to location {0} is a present target, which cannot be used as input attachment.", i);

            // Store the image view from the source frame buffer.
            attachmentViews.push_back(inputAttachment.inputAttachmentSource().frameBuffer(m_bufferIndex).image(i++).imageView());
        });

        // Initialize the output attachments from render targets of the parent render pass.
        // NOTE: Again, we assume, that the parent render pass provides the render targets in an sorted manner.
        std::ranges::for_each(m_parent->parent().renderTargets(), [&, i = 0](const RenderTarget& renderTarget) mutable {
            if (renderTarget.location() != i++) [[unlikely]]
                LITEFX_WARNING(VULKAN_LOG, "Remapped render target from location {0} to location {1}. Please make sure that the render targets are sorted within the render pass and do not have any gaps in their location mappings.", renderTarget.location(), i - 1);

            if (renderTarget.type() == RenderTargetType::Present)
            {
                // If the render target is a present target, acquire an image view from the swap chain.
                auto swapChainImages = m_parent->getDevice()->swapChain().images();
                auto image = swapChainImages[m_bufferIndex];
                m_renderTargetViews.push_back(image);
                attachmentViews.push_back(image->imageView());
            }
            else
            {
                // Create an image view for the render target.
                auto image = m_parent->getDevice()->factory().createAttachment(renderTarget.format(), size, renderTarget.samples());
                attachmentViews.push_back(image->imageView());
                m_renderTargetViews.push_back(image.get());
                m_outputAttachments.push_back(std::move(image));
            }
        });

        // Allocate the frame buffer.
        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = m_parent->parent().handle();
        frameBufferInfo.attachmentCount = static_cast<UInt32>(attachmentViews.size());
        frameBufferInfo.pAttachments = attachmentViews.data();
        frameBufferInfo.width = size.width();
        frameBufferInfo.height = size.height();
        frameBufferInfo.layers = 1;

        VkFramebuffer frameBuffer;
        raiseIfFailed<RuntimeException>(::vkCreateFramebuffer(m_parent->getDevice()->handle(), &frameBufferInfo, nullptr, &frameBuffer), "Unable to create frame buffer from swap chain frame.");

        return frameBuffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanFrameBuffer::VulkanFrameBuffer(const VulkanRenderPass& renderPass, const UInt32& bufferIndex, const Size2d& renderArea) :
	m_impl(makePimpl<VulkanFrameBufferImpl>(this, bufferIndex)), VulkanRuntimeObject<VulkanRenderPass>(renderPass, renderPass.getDevice()), IResource(nullptr)
{
    this->resize(renderArea);
}

VulkanFrameBuffer::~VulkanFrameBuffer() noexcept = default;

const VkSemaphore& VulkanFrameBuffer::semaphore() const noexcept
{
	return m_impl->m_semaphore;
}

const UInt32& VulkanFrameBuffer::bufferIndex() const noexcept
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

const VulkanCommandBuffer& VulkanFrameBuffer::commandBuffer() const noexcept
{
	return *m_impl->m_commandBuffer;
}

Array<const IVulkanImage*> VulkanFrameBuffer::images() const noexcept
{
    return m_impl->m_renderTargetViews;
}

const IVulkanImage& VulkanFrameBuffer::image(const UInt32& location) const
{
    if (location >= m_impl->m_renderTargetViews.size())
        throw ArgumentOutOfRangeException("No render target is mapped to location {0}.", location);

    return *m_impl->m_renderTargetViews[location];
}

void VulkanFrameBuffer::resize(const Size2d& renderArea)
{
    this->handle() = m_impl->initialize(renderArea);
}