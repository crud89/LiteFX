#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------
class VulkanDescriptorSet::VulkanDescriptorSetImpl : public Implement<VulkanDescriptorSet> {
public:
    friend class VulkanDescriptorSet;

private:
    Dictionary<UInt32, VkBufferView> m_bufferViews;
    Dictionary<UInt32, VkImageView> m_imageViews;

public:
    VulkanDescriptorSetImpl(VulkanDescriptorSet* parent) :
        base(parent) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, VkDescriptorSet descriptorSet) :
    m_impl(makePimpl<VulkanDescriptorSetImpl>(this)), VulkanRuntimeObject<VulkanDescriptorSetLayout>(layout, layout.getDevice()), Resource<VkDescriptorSet>(descriptorSet)
{
    if (descriptorSet == VK_NULL_HANDLE)
        throw ArgumentNotInitializedException("The descriptor set handle must be initialized.");
}

VulkanDescriptorSet::~VulkanDescriptorSet() noexcept
{
    for (auto& bufferView : m_impl->m_bufferViews)
        ::vkDestroyBufferView(this->getDevice()->handle(), bufferView.second, nullptr);

    for (auto& imageView: m_impl->m_imageViews)
        ::vkDestroyImageView(this->getDevice()->handle(), imageView.second, nullptr);

    this->parent().free(*this);
}

void VulkanDescriptorSet::update(const UInt32& binding, const IVulkanBuffer& buffer, const UInt32& bufferElement, const UInt32& elements, const UInt32& firstDescriptor) const
{
    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = firstDescriptor;
    descriptorWrite.descriptorCount = 1;

    auto& descriptorLayout = this->parent().descriptor(binding);

    switch (descriptorLayout.descriptorType())
    {
    case DescriptorType::Uniform:
        descriptorWrite.descriptorCount = elements;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
    case DescriptorType::Storage:
    case DescriptorType::WritableStorage:
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        break;
    case DescriptorType::Buffer:
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        break;
    case DescriptorType::WritableBuffer:
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        break;
    default: [[unlikely]]
        throw InvalidArgumentException("Invalid descriptor type. The binding {0} does not point to a buffer descriptor.", binding);
    }

    // Remove the buffer view, if there is one bound to the current descriptor.
    if (m_impl->m_bufferViews.contains(binding))
    {
        ::vkDestroyBufferView(this->getDevice()->handle(), m_impl->m_bufferViews[binding], nullptr);
        m_impl->m_bufferViews.erase(binding);
    }

    // Create a buffer view for (writable) buffers.
    Array<VkDescriptorBufferInfo> bufferInfos;
    
    if (descriptorLayout.descriptorType() == DescriptorType::Uniform)
    {
        bufferInfos.resize(elements);
        std::ranges::generate(bufferInfos, [&buffer, &bufferElement, i = 0]() mutable {
            VkDescriptorBufferInfo bufferInfo{ };

            bufferInfo.buffer = buffer.handle();
            bufferInfo.range = buffer.elementSize();
            bufferInfo.offset = buffer.alignedElementSize() * static_cast<size_t>(bufferElement + i++);

            return bufferInfo;
        });

        descriptorWrite.pBufferInfo = bufferInfos.data();
    }
    else if (descriptorLayout.descriptorType() == DescriptorType::Storage || descriptorLayout.descriptorType() == DescriptorType::WritableStorage)
    {
        bufferInfos.push_back(VkDescriptorBufferInfo {
            .buffer = buffer.handle(),
            .offset = buffer.alignedElementSize() * static_cast<size_t>(bufferElement),
            .range = buffer.alignedElementSize() * static_cast<size_t>(elements)
        });

        descriptorWrite.pBufferInfo = bufferInfos.data();
    }
    else if (descriptorLayout.descriptorType() == DescriptorType::Buffer || descriptorLayout.descriptorType() == DescriptorType::WritableBuffer)
    {
        VkBufferViewCreateInfo bufferViewDesc{
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .buffer = buffer.handle(),
            .format = VK_FORMAT_UNDEFINED,
            .offset = buffer.alignedElementSize() * bufferElement,     // TODO: Handle alignment properly, as texel buffers do not need to be aligned (afaik).
            .range = buffer.alignedElementSize() * elements
        };

        VkBufferView bufferView;
        raiseIfFailed<RuntimeException>(::vkCreateBufferView(this->getDevice()->handle(), &bufferViewDesc, nullptr, &bufferView), "Unable to create buffer view.");
        m_impl->m_bufferViews[binding] = bufferView;
        descriptorWrite.pTexelBufferView = &bufferView;
    }

    // Update the descriptor set.
    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(const UInt32& binding, const IVulkanImage& texture, const UInt32& descriptor, const UInt32& firstLevel, const UInt32& levels, const UInt32& firstLayer, const UInt32& layers) const
{
    VkDescriptorImageInfo imageInfo{ };
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

    // Remove the image view, if there is one bound to the current descriptor.
    if (m_impl->m_imageViews.contains(binding))
    {
        ::vkDestroyImageView(this->getDevice()->handle(), m_impl->m_imageViews[binding], nullptr);
        m_impl->m_imageViews.erase(binding);
    }
    
    // Create a new image view.
    const UInt32 numLevels = levels == 0 ? texture.levels() - firstLevel : levels;
    const UInt32 numLayers = layers == 0 ? texture.layers() - firstLayer : layers;

    VkImageViewCreateInfo imageViewDesc = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .image = texture.handle(),
        .viewType = Vk::getImageViewType(texture.dimensions(), texture.layers()),
        .format = Vk::getFormat(texture.format()),
        .components = VkComponentMapping {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = VkImageSubresourceRange {
            .baseMipLevel = firstLevel,
            .levelCount = numLevels,
            .baseArrayLayer = firstLayer,
            .layerCount = numLayers
        }
    };

    if (!::hasDepth(texture.format()) && !::hasStencil(texture.format()))
        imageViewDesc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    else if (::hasDepth(texture.format()))
        imageViewDesc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else if (::hasStencil(texture.format()))
        imageViewDesc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

    VkImageView imageView;
    raiseIfFailed<RuntimeException>(::vkCreateImageView(this->getDevice()->handle(), &imageViewDesc, nullptr, &imageView), "Unable to create image view.");
    m_impl->m_imageViews[binding] = imageView;
    imageInfo.imageView = imageView;

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