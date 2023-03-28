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
    Array<const IVulkanImage*> m_renderTargetViews;
	Array<SharedPtr<VulkanCommandBuffer>> m_commandBuffers;
	Size2d m_size;
	VkSemaphore m_semaphore;
    UInt32 m_bufferIndex;
    UInt64 m_lastFence{ 0 };

public:
    VulkanFrameBufferImpl(VulkanFrameBuffer* parent, const VulkanRenderPass& renderPass, const UInt32& bufferIndex, const Size2d& renderArea, const UInt32& commandBuffers) :
        base(parent), m_bufferIndex(bufferIndex), m_size(renderArea), m_renderPass(renderPass)
	{
        const auto& device = m_renderPass.device();

		// Initialize the semaphore.
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		raiseIfFailed<RuntimeException>(::vkCreateSemaphore(device.handle(), &semaphoreInfo, nullptr, &m_semaphore), "Unable to create swap semaphore on frame buffer.");

        // Retrieve a command buffer from the graphics queue.
        m_commandBuffers.resize(commandBuffers);
        std::ranges::generate(m_commandBuffers, [this, &device]() { return device.graphicsQueue().createCommandBuffer(true, false); });
	}

	~VulkanFrameBufferImpl()
	{
		::vkDestroySemaphore(m_renderPass.device().handle(), m_semaphore, nullptr);
	}

public:
	VkFramebuffer initialize()
	{
        // Clear earlier images.
        m_renderTargetViews.clear();
        m_outputAttachments.clear();

        // Retrieve the image views for the input and output attachments.
        Array<VkImageView> attachmentViews;

        // Start with the input attachments.
        // NOTE: We assume, that the parent render pass provides the attachments in an sorted manner.
        std::ranges::for_each(m_renderPass.inputAttachments(), [&, i = 0](const VulkanInputAttachmentMapping& inputAttachment) mutable {
            if (inputAttachment.location() != i) [[unlikely]]
                LITEFX_WARNING(VULKAN_LOG, "Remapped input attachment from location {0} to location {1}. Please make sure that the input attachments are sorted within the render pass and do not have any gaps in their location mappings.", inputAttachment.location(), i);

            if (inputAttachment.renderTarget().type() == RenderTargetType::Present)
                throw InvalidArgumentException("The input attachment mapped to location {0} is a present target, which cannot be used as input attachment.", i);

            if (inputAttachment.inputAttachmentSource() == nullptr)
                throw InvalidArgumentException("The input attachment mapped to location {0} has no initialized source.", i);

            // Store the image view from the source frame buffer.
            attachmentViews.push_back(inputAttachment.inputAttachmentSource()->frameBuffer(m_bufferIndex).image(i++).imageView());
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
                auto swapChainImages = m_renderPass.device().swapChain().images();
                auto image = swapChainImages[m_bufferIndex];
                m_renderTargetViews.push_back(image);
                attachmentViews.push_back(image->imageView());
            }
            else
            {
                // Create an image view for the render target.
                auto image = m_renderPass.device().factory().createAttachment(renderTarget.format(), m_size, samples);
                attachmentViews.push_back(image->imageView());
                m_renderTargetViews.push_back(image.get());
                m_outputAttachments.push_back(std::move(image));
            }
        });

        // If we have a present target and multi sampling is enabled, make sure to add a view for the resolve attachment.
        if (samples > MultiSamplingLevel::x1 && std::ranges::any_of(m_renderPass.renderTargets(), [](const RenderTarget& renderTarget) { return renderTarget.type() == RenderTargetType::Present; }))
        {
            auto swapChainImages = m_renderPass.device().swapChain().images();
            auto image = swapChainImages[m_bufferIndex];
            m_renderTargetViews.push_back(image);
            attachmentViews.push_back(image->imageView());
        }

        // Allocate the frame buffer.
        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = m_renderPass.handle();
        frameBufferInfo.attachmentCount = static_cast<UInt32>(attachmentViews.size());
        frameBufferInfo.pAttachments = attachmentViews.data();
        frameBufferInfo.width = m_size.width();
        frameBufferInfo.height = m_size.height();
        frameBufferInfo.layers = 1;

        VkFramebuffer frameBuffer;
        raiseIfFailed<RuntimeException>(::vkCreateFramebuffer(m_renderPass.device().handle(), &frameBufferInfo, nullptr, &frameBuffer), "Unable to create frame buffer from swap chain frame.");

        return frameBuffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanFrameBuffer::VulkanFrameBuffer(const VulkanRenderPass& renderPass, const UInt32& bufferIndex, const Size2d& renderArea, const UInt32& commandBuffers) :
	m_impl(makePimpl<VulkanFrameBufferImpl>(this, renderPass, bufferIndex, renderArea, commandBuffers)), Resource<VkFramebuffer>(VK_NULL_HANDLE)
{
    this->handle() = m_impl->initialize();
}

VulkanFrameBuffer::~VulkanFrameBuffer() noexcept
{
    ::vkDestroyFramebuffer(m_impl->m_renderPass.device().handle(), this->handle(), nullptr);
}

const VkSemaphore& VulkanFrameBuffer::semaphore() const noexcept
{
    return m_impl->m_semaphore;
}

UInt64& VulkanFrameBuffer::lastFence() const noexcept
{
    return m_impl->m_lastFence;
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

SharedPtr<const VulkanCommandBuffer> VulkanFrameBuffer::commandBuffer(const UInt32& index) const
{
    if (index >= static_cast<UInt32>(m_impl->m_commandBuffers.size())) [[unlikely]]
        throw ArgumentOutOfRangeException("No command buffer with index {1} is stored in the frame buffer. The frame buffer only contains {0} command buffers.", m_impl->m_commandBuffers.size(), index);

	return m_impl->m_commandBuffers[index];
}

Array<SharedPtr<const VulkanCommandBuffer>> VulkanFrameBuffer::commandBuffers() const noexcept
{
    return m_impl->m_commandBuffers | ranges::to<Array<SharedPtr<const VulkanCommandBuffer>>>();
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
    // Destroy the old frame buffer.
    ::vkDestroyFramebuffer(m_impl->m_renderPass.device().handle(), this->handle(), nullptr);

    // Reset the size and re-initialize the frame buffer.
    m_impl->m_size = renderArea;
    this->handle() = m_impl->initialize();
}