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
	VkImage initialize(const VulkanTexture& parent)
	{
		return nullptr;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTexture::VulkanTexture() :
	IResource(nullptr), m_impl(makePimpl<VulkanTextureImpl>())
{
	this->handle() = m_impl->initialize(*this);
}

VulkanTexture::~VulkanTexture() noexcept = default;