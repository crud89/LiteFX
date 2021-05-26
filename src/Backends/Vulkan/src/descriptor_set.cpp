#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, VkDescriptorSet descriptorSet) :
    VulkanRuntimeObject<VulkanDescriptorSetLayout>(layout, layout.getDevice()), IResource<VkDescriptorPool>(descriptorSet)
{
    if (descriptorSet == nullptr)
        throw ArgumentNotInitializedException("The descriptor set handle must be initialized.");
}

VulkanDescriptorSet::~VulkanDescriptorSet() noexcept
{
    this->parent().free(*this);
}

UniquePtr<IVulkanConstantBuffer> VulkanDescriptorSet::makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements) const
{
    return this->getDevice()->factory().createConstantBuffer(this->parent().layout(binding), usage, elements);
}

UniquePtr<IVulkanTexture> VulkanDescriptorSet::makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const
{
    return this->getDevice()->factory().createTexture(this->parent().layout(binding), format, size, levels, samples);
}

UniquePtr<IVulkanSampler> VulkanDescriptorSet::makeSampler(const UInt32& binding, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const
{
    return this->getDevice()->factory().createSampler(this->parent().layout(binding), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
}

void VulkanDescriptorSet::update(const IVulkanConstantBuffer& buffer, const UInt32& bufferElement) const noexcept
{
    VkDescriptorBufferInfo bufferInfo{ };
    bufferInfo.buffer = buffer.handle();
    bufferInfo.range = buffer.elementSize();    // * elements;
    bufferInfo.offset = bufferElement * buffer.elementSize();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = buffer.binding();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;    // = elements;
    descriptorWrite.pBufferInfo = &bufferInfo;

    switch (buffer.layout().descriptorType())
    {
    case DescriptorType::Uniform: descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
    case DescriptorType::Storage: descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
    default: throw std::runtime_error("Unsupported buffer type.");
    }

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(const IVulkanTexture& texture) const noexcept
{
    VkDescriptorImageInfo imageInfo{ };
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.imageView();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = texture.binding();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(const IVulkanSampler& sampler) const noexcept
{
    VkDescriptorImageInfo imageInfo{ };
    imageInfo.sampler = sampler.handle();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = sampler.binding();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::attach(const UInt32& binding, const IVulkanImage& image) const noexcept
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = image.imageView();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}