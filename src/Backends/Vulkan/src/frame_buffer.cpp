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
		throw;
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