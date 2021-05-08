#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanSampler::VulkanSamplerImpl : public Implement<VulkanSampler> {
public:
	friend class VulkanSampler;

public:
	VulkanSamplerImpl(VulkanSampler* parent) : base(parent) { }

private:
	VkFilter getFilterMode(const FilterMode& mode)
	{
		switch (mode)
		{
		case FilterMode::Linear: return VK_FILTER_LINEAR;
		case FilterMode::Nearest: return VK_FILTER_NEAREST;
		default: throw std::invalid_argument("Invalid filter mode.");
		}
	}

	VkSamplerMipmapMode getMipMapMode(const MipMapMode& mode)
	{
		switch (mode)
		{
		case MipMapMode::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case MipMapMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		default: throw std::invalid_argument("Invalid mip map mode.");
		}
	}

	VkSamplerAddressMode getBorderMode(const BorderMode& mode)
	{
		switch (mode)
		{
		case BorderMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case BorderMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case BorderMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case BorderMode::RepeatMirrored: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case BorderMode::ClampToEdgeMirrored: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		default: throw std::invalid_argument("Invalid border mode.");
		}
	}

public:
	VkSampler initialize()
	{
		VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = getFilterMode(m_parent->getMagnifyingFilter());
		samplerInfo.minFilter = getFilterMode(m_parent->getMinifyingFilter());
		samplerInfo.addressModeU = getBorderMode(m_parent->getBorderModeU());
		samplerInfo.addressModeV = getBorderMode(m_parent->getBorderModeV());
		samplerInfo.addressModeW = getBorderMode(m_parent->getBorderModeW());
		samplerInfo.anisotropyEnable = m_parent->getAnisotropy() > 0.f ? VK_TRUE : VK_FALSE;
		samplerInfo.maxAnisotropy = m_parent->getAnisotropy();
		samplerInfo.mipmapMode = getMipMapMode(m_parent->getMipMapMode());
		samplerInfo.mipLodBias = m_parent->getMipMapBias();
		samplerInfo.minLod = m_parent->getMinLOD();
		samplerInfo.maxLod = m_parent->getMaxLOD();

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		VkSampler sampler;

		if (::vkCreateSampler(m_parent->getDevice()->handle(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			throw std::runtime_error("Unable to create sampler.");
		
		return sampler;
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanSampler::VulkanSampler(const VulkanDevice& device, const IDescriptorLayout* layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
	VulkanRuntimeObject<VulkanDevice>(device, &device), Sampler(layout, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy), m_impl(makePimpl<VulkanSamplerImpl>(this)), IResource(nullptr)
{
	this->handle() = m_impl->initialize();
}

VulkanSampler::~VulkanSampler() noexcept
{
	::vkDestroySampler(this->getDevice()->handle(), this->handle(), nullptr);
}