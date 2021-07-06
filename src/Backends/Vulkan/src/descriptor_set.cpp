#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, VkDescriptorSet descriptorSet) :
    VulkanRuntimeObject<VulkanDescriptorSetLayout>(layout, layout.getDevice()), Resource<VkDescriptorSet>(descriptorSet)
{
    if (descriptorSet == VK_NULL_HANDLE)
        throw ArgumentNotInitializedException("The descriptor set handle must be initialized.");
}

VulkanDescriptorSet::~VulkanDescriptorSet() noexcept
{
    this->parent().free(*this);
}

UniquePtr<IVulkanConstantBuffer> VulkanDescriptorSet::makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements) const
{
    return this->getDevice()->factory().createConstantBuffer(this->parent().descriptor(binding), usage, elements);
}

UniquePtr<IVulkanTexture> VulkanDescriptorSet::makeTexture(const UInt32& binding, const Format& format, const Size3d& size, const ImageDimensions& dimensions, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples) const
{
    return this->getDevice()->factory().createTexture(this->parent().descriptor(binding), format, size, dimensions, levels, layers, samples);
}

UniquePtr<IVulkanSampler> VulkanDescriptorSet::makeSampler(const UInt32& binding, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) const
{
    return this->getDevice()->factory().createSampler(this->parent().descriptor(binding), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy);
}

void VulkanDescriptorSet::update(const IVulkanConstantBuffer& buffer, const UInt32& bufferElement, const UInt32& elements, const UInt32& firstDescriptor) const noexcept
{
    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = buffer.binding();
    descriptorWrite.dstArrayElement = firstDescriptor;
    descriptorWrite.descriptorCount = elements;

    switch (buffer.layout().descriptorType())
    {
    default:
        LITEFX_WARNING(VULKAN_LOG, "The constant buffer is bound to a descriptor with an unsupported buffer type: {0}. Descriptor will be treated as uniform buffer.", buffer.layout().descriptorType());
        [[fallthrough]];
    case DescriptorType::Uniform:
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
    case DescriptorType::Storage:
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        break;
    }

    // Create buffer info.
    Array<VkDescriptorBufferInfo> bufferInfos(elements);
    std::ranges::generate(bufferInfos, [&buffer, &bufferElement, i = 0]() mutable {
        VkDescriptorBufferInfo bufferInfo{ };

        bufferInfo.buffer = buffer.handle();
        bufferInfo.range = buffer.elementSize();
        bufferInfo.offset = buffer.alignedElementSize() * static_cast<size_t>(bufferElement + i++);

        return bufferInfo;
    });

    // Set the buffer info and update the descriptor set.
    descriptorWrite.pBufferInfo = bufferInfos.data();
    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(const IVulkanTexture& texture, const UInt32& descriptor) const noexcept
{
    VkDescriptorImageInfo imageInfo{ };
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.imageView();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = texture.binding();
    descriptorWrite.dstArrayElement = descriptor;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(const IVulkanSampler& sampler, const UInt32& descriptor) const noexcept
{
    VkDescriptorImageInfo imageInfo{ };
    imageInfo.sampler = sampler.handle();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = sampler.binding();
    descriptorWrite.dstArrayElement = descriptor;
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