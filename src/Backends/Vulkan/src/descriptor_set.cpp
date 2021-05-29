#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, VkDescriptorSet descriptorSet) :
    VulkanRuntimeObject<VulkanDescriptorSetLayout>(layout, layout.getDevice()), Resource<VkDescriptorSet>(descriptorSet)
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

UniquePtr<IVulkanSampler> VulkanDescriptorSet::makeSampler(const UInt32& binding, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) const
{
    return this->getDevice()->factory().createSampler(this->parent().layout(binding), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
}

void VulkanDescriptorSet::update(const IVulkanConstantBuffer& buffer, const UInt32& bufferElement) const noexcept
{
    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = buffer.binding();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;    // = elements;

    size_t alignedSize = static_cast<size_t>(buffer.elementSize());
    size_t alignment = 0;

    switch (buffer.layout().descriptorType())
    {
    default:
        LITEFX_WARNING(VULKAN_LOG, "The constant buffer is bound to a descriptor with an unsupported buffer type: {0}. Descriptor will be treated as uniform buffer.", buffer.layout().descriptorType());
        [[fallthrough]];
    case DescriptorType::Uniform: 
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        alignment = this->getDevice()->adapter().getLimits().minUniformBufferOffsetAlignment;
        break;
    case DescriptorType::Storage: 
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        alignment = this->getDevice()->adapter().getLimits().minStorageBufferOffsetAlignment;
        break;
    }

    if (alignment > 0)
        alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);

    // Create buffer info.
    VkDescriptorBufferInfo bufferInfo{ };
    bufferInfo.buffer = buffer.handle();
    bufferInfo.range = alignedSize;    // * elements;
    bufferInfo.offset = static_cast<size_t>(bufferElement) * alignedSize;

    // Set the buffer info and update the descriptor set.
    descriptorWrite.pBufferInfo = &bufferInfo;
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