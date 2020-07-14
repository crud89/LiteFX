#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTexture::VulkanTextureImpl : public Implement<VulkanTexture> {
public:
	friend class VulkanTexture;

private:
	VkImageView m_view;
	Format m_format;
	Size2d m_size;

public:
	VulkanTextureImpl(VulkanTexture* parent, const Format& format = Format::B8G8R8A8_UNORM_SRGB, const Size2d& size = Size2d(0)) :
		base(parent), m_format(format), m_size(size), m_view(nullptr) { }

	~VulkanTextureImpl() noexcept 
	{
		::vkDestroyImageView(m_parent->getDevice()->handle(), m_view, nullptr);
	}

public:
	void initialize()
	{
		m_view = m_parent->getDevice()->vkCreateImageView(m_parent->handle(), m_format);
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTexture::VulkanTexture(const VulkanDevice* device, VkImage image, const Format& format, const Size2d& size) :
	IResource(image), RuntimeObject(device), m_impl(makePimpl<VulkanTextureImpl>(this, format, size))
{
	if (image == nullptr)
		throw std::invalid_argument("The argument `image` is not initialized.");

	m_impl->initialize();
}

VulkanTexture::~VulkanTexture() noexcept = default;

Size2d VulkanTexture::getSize() const noexcept
{
	return m_impl->m_size;
}

Format VulkanTexture::getFormat() const noexcept
{
	return m_impl->m_format;
}

VkImageView VulkanTexture::getView() const noexcept
{
	return m_impl->m_view;
}