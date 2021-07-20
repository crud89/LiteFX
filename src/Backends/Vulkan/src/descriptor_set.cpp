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

void VulkanDescriptorSet::update(const UInt32& binding, const IVulkanBuffer& buffer, const UInt32& bufferElement, const UInt32& elements, const UInt32& firstDescriptor) const
{
    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = firstDescriptor;
    descriptorWrite.descriptorCount = elements;

    switch (this->parent().descriptor(binding).descriptorType())
    {
    case DescriptorType::Uniform:
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
    case DescriptorType::Storage:
    case DescriptorType::WritableStorage:
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        break;
    case DescriptorType::Buffer:
    case DescriptorType::WritableBuffer:
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        break;
    default: [[unlikely]]
        throw InvalidArgumentException("Invalid descriptor type. The binding {0} does not point to a buffer descriptor.", binding);
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

void VulkanDescriptorSet::update(const UInt32& binding, const IVulkanTexture& texture, const UInt32& descriptor) const
{
    VkDescriptorImageInfo imageInfo{ };
    imageInfo.imageView = texture.imageView();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = descriptor;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    const auto& layout = this->parent().descriptor(binding);

    switch (layout.descriptorType())
    {
    case DescriptorType::Texture:
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        break;
    case DescriptorType::WritableTexture:
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        break;
    default: [[unlikely]]
        throw InvalidArgumentException("Invalid descriptor type. The binding {0} does not point to a texture descriptor.", binding);
    }

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(const UInt32& binding, const IVulkanSampler& sampler, const UInt32& descriptor) const
{
    const auto& layout = this->parent().descriptor(binding);

    if (layout.descriptorType() != DescriptorType::Sampler) [[unlikely]]
        throw InvalidArgumentException("Invalid descriptor type. The binding {0} does not point to a sampler descriptor.", binding);

    VkDescriptorImageInfo imageInfo{ };
    imageInfo.sampler = sampler.handle();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = descriptor;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::attach(const UInt32& binding, const IVulkanImage& image) const
{
    const auto& layout = this->parent().descriptor(binding);

    if (layout.descriptorType() != DescriptorType::InputAttachment) [[unlikely]]
        throw InvalidArgumentException("Invalid descriptor type. The binding {0} does not point to a input attachment descriptor.", binding);

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