#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTexture::VulkanTextureImpl {
private:
	const VulkanDevice* m_device;
	VkImageView m_view;
	Format m_format;
	Size2d m_size;

public:
	VulkanTextureImpl(const VulkanDevice* device, const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Size2d& size = Size2d(0)) noexcept :
		m_device(device), m_format(format), m_size(size), m_view(nullptr) { }

	~VulkanTextureImpl() noexcept {
		::vkDestroyImageView(m_device->handle(), m_view, nullptr);
	}

public:
	void initialize(const VulkanTexture& parent)
	{
		m_view = m_device->vkCreateImageView(parent.handle(), m_format);
	}

public:
	Size2d getSize() const noexcept
	{
		return m_size;
	}

	Format getFormat() const noexcept 
	{
		return m_format;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTexture::VulkanTexture(const VulkanDevice* device, VkImage image, const Format& format, const Size2d& size) :
	IResource(image), m_impl(makePimpl<VulkanTextureImpl>(device, format, size))
{
	if (image == nullptr)
		throw std::invalid_argument("The argument `image` is not initialized.");

	m_impl->initialize(*this);
}

VulkanTexture::~VulkanTexture() noexcept = default;

Size2d VulkanTexture::getSize() const noexcept
{
	return m_impl->getSize();
}

Format VulkanTexture::getFormat() const noexcept
{
	return m_impl->getFormat();
}