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
	UInt32 m_levels;
	MultiSamplingLevel m_samples;

public:
	VulkanTextureImpl(VulkanTexture* parent, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) :
		base(parent), m_format(format), m_size(size), m_levels(levels), m_samples(samples), m_view(nullptr) { }

	~VulkanTextureImpl() noexcept 
	{
		::vkDestroyImageView(m_parent->getDevice()->handle(), m_view, nullptr);
	}

public:
	void initialize()
	{
		VkImageViewCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_parent->handle();
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = ::getFormat(m_format);
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = m_levels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (::vkCreateImageView(m_parent->getDevice()->handle(), &createInfo, nullptr, &m_view) != VK_SUCCESS)
			throw std::runtime_error("Unable to create image view!");
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTexture::VulkanTexture(const VulkanDevice* device, VkImage image, const Format& format, const Size2d& size, const UInt32& binding, const UInt32& levels, const MultiSamplingLevel& samples) :
	IResource(image), VulkanRuntimeObject(device), Buffer(BufferType::Descriptor, size.width() * size.height(), ::getSize(format), binding), m_impl(makePimpl<VulkanTextureImpl>(this, format, size, levels, samples))
{
	if (image == nullptr)
		throw std::invalid_argument("The argument `image` is not initialized.");

	m_impl->initialize();
}

VulkanTexture::~VulkanTexture() noexcept = default;

Size2d VulkanTexture::getExtent() const noexcept
{
	return m_impl->m_size;
}

Format VulkanTexture::getFormat() const noexcept
{
	return m_impl->m_format;
}

MultiSamplingLevel VulkanTexture::getSamples() const noexcept
{
	return m_impl->m_samples;
}

const UInt32& VulkanTexture::getLevels() const noexcept
{
	return m_impl->m_levels;
}

void VulkanTexture::map(const void* const data, const size_t& size)
{
	throw std::runtime_error("No data can be mapped to this texture.");
}

void VulkanTexture::transfer(const ICommandQueue* commandQueue, IBuffer* target, const size_t& size, const size_t& offset, const size_t& targetOffset) const
{
	throw;
}

VkImageView VulkanTexture::getView() const noexcept
{
	return m_impl->m_view;
}