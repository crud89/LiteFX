#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTexture::VulkanTextureImpl {
private:

public:
	VulkanTextureImpl() noexcept = default;

public:
	void initialize(const VulkanTexture& parent)
	{
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