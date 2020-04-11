#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTexture::VulkanTextureImpl {
private:
	Size2d m_size;

public:
	VulkanTextureImpl() noexcept = default;

public:
	void initialize(const VulkanTexture& parent, const Size2d& size)
	{
	}

public:
	Size2d getSize() const noexcept
	{
		return m_size;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

//VulkanTexture::VulkanTexture(VkImage image) :
//	IResource(image), m_impl(makePimpl<VulkanTextureImpl>())
//{
//	if (image == nullptr)
//		throw std::invalid_argument("The argument `image` is not initialized.");
//
//	m_impl->initialize(*this);
//}

VulkanTexture::~VulkanTexture() noexcept = default;

Size2d VulkanTexture::getSize() const noexcept
{
	return m_impl->getSize();
}