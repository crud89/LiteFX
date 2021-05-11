#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanFrameBuffer::VulkanFrameBufferImpl : public Implement<VulkanFrameBuffer> {
public:
	friend class VulkanFrameBuffer;

private:
	Array<RenderTarget> m_renderTargets;
	UniquePtr<VulkanCommandBuffer> m_commandBuffer;
	Size2d m_size;
	VkSemaphore m_semaphore;

public:
	VulkanFrameBufferImpl(VulkanFrameBuffer* parent) : 
		base(parent) 
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
	VkFramebuffer initialize(const Size2d& size, const Array<RenderTarget>& renderTargets)
	{
        Array<VkImageView> attachmentViews;
        Array<UniquePtr<IImage>> attachmentImages;

        std::for_each(std::begin(dependencyTargets), std::end(dependencyTargets), [&, a = 0](const auto& target) mutable {
            auto inputAttachmentImage = dynamic_cast<const IVulkanImage*>(m_dependency->m_impl->m_attachmentImages[i][a++].get());

            if (inputAttachmentImage == nullptr)
                throw std::invalid_argument("An input attachment of the render pass dependency is not a valid Vulkan image.");

            attachmentViews.push_back(inputAttachmentImage->getImageView());
        });

        std::for_each(std::begin(m_targets), std::end(m_targets), [&, a = attachmentViews.size()](const auto& target) mutable {
            if (m_presentAttachment.has_value() && a++ == m_presentAttachment->attachment)
            {
                // Acquire an image view from the swap chain.
                auto swapChainImage = dynamic_cast<const IVulkanImage*>(frames[i]);

                if (swapChainImage == nullptr)
                    throw std::invalid_argument("A frame of the provided swap chain is not a valid Vulkan texture.");

                attachmentViews.push_back(swapChainImage->getImageView());
            }
            else
            {
                // Create an image view for the render target.
                auto image = this->makeImageView(target.get());
                attachmentViews.push_back(dynamic_cast<const IVulkanImage*>(image.get())->getImageView());
                attachmentImages.push_back(std::move(image));
            }
        });

        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = renderPass;
        frameBufferInfo.attachmentCount = static_cast<UInt32>(attachmentViews.size());
        frameBufferInfo.pAttachments = attachmentViews.data();
        frameBufferInfo.width = m_parent->getDevice()->swapChain().getBufferSize().width();
        frameBufferInfo.height = m_parent->getDevice()->swapChain().getBufferSize().height();
        frameBufferInfo.layers = 1;

        VkFramebuffer frameBuffer;

        if (::vkCreateFramebuffer(m_parent->getDevice()->handle(), &frameBufferInfo, nullptr, &frameBuffer) != VK_SUCCESS)
            throw std::runtime_error("Unable to create frame buffer from swap chain frame.");

        m_attachmentImages[i++] = std::move(attachmentImages);

        return frameBuffer;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanFrameBuffer::VulkanFrameBuffer(const VulkanRenderPass& renderPass, const Size2d& size, const Array<RenderTarget>& renderTargets) :
	m_impl(makePimpl<VulkanFrameBufferImpl>(this)), VulkanRuntimeObject<VulkanRenderPass>(renderPass, renderPass.getDevice()), IResource(nullptr)
{
	this->handle() = m_impl->initialize(size, renderTargets);
}

VulkanFrameBuffer::~VulkanFrameBuffer() noexcept = default;

const VkSemaphore& VulkanFrameBuffer::semaphore() const noexcept
{
	return m_impl->m_semaphore;
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

Array<std::reference_wrapper<const RenderTarget>> VulkanFrameBuffer::renderTargets() const noexcept
{
	return m_impl->m_renderTargets | 
		std::views::transform([](const RenderTarget& renderTarget) { return std::ref(renderTarget); }) |
		ranges::to<Array<std::reference_wrapper<const RenderTarget>>>();
}

const VulkanCommandBuffer& VulkanFrameBuffer::commandBuffer() const noexcept
{
	return *m_impl->m_commandBuffer;
}

bool VulkanFrameBuffer::hasPresentTarget() const noexcept
{
	return std::ranges::any_of(m_impl->m_renderTargets, [](const auto& renderTarget) { return renderTarget.type() == RenderTargetType::Present; });
}

void VulkanFrameBuffer::resize(const Size2d& newSize, UniquePtr<IVulkanImage>&& presentImage)
{
	throw;
}