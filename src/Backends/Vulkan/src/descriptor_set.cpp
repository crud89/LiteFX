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
    const VulkanDescriptorSetLayout& m_layout;

public:
    VulkanDescriptorSetImpl(VulkanDescriptorSet* parent, const VulkanDescriptorSetLayout& layout) :
        base(parent), m_layout(layout)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, VkDescriptorSet descriptorSet) :
    m_impl(makePimpl<VulkanDescriptorSetImpl>(this, layout)), Resource<VkDescriptorSet>(descriptorSet)
{
    if (descriptorSet == VK_NULL_HANDLE)
        throw ArgumentNotInitializedException("descriptorSet", "The descriptor set handle must be initialized.");
}

VulkanDescriptorSet::~VulkanDescriptorSet() noexcept
{
    for (auto& bufferView : m_impl->m_bufferViews)
        ::vkDestroyBufferView(m_impl->m_layout.device().handle(), bufferView.second, nullptr);

    for (auto& imageView: m_impl->m_imageViews)
        ::vkDestroyImageView(m_impl->m_layout.device().handle(), imageView.second, nullptr);

    m_impl->m_layout.free(*this);
}

const VulkanDescriptorSetLayout& VulkanDescriptorSet::layout() const noexcept
{
    return m_impl->m_layout;
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor) const
{
    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = firstDescriptor;
    descriptorWrite.descriptorCount = 1;

    auto& descriptorLayout = m_impl->m_layout.descriptor(binding);
    Array<VkDescriptorBufferInfo> bufferInfos;
    UInt32 elementCount = elements > 0 ? elements : buffer.elements() - bufferElement;

    switch (descriptorLayout.descriptorType())
    {
    case DescriptorType::ConstantBuffer:
    {
        descriptorWrite.descriptorCount = elementCount;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        bufferInfos.resize(elementCount);
        std::ranges::generate(bufferInfos, [&buffer, &bufferElement, i = 0]() mutable {
            return VkDescriptorBufferInfo {
                .buffer = buffer.handle(),
                .offset = buffer.alignedElementSize() * static_cast<size_t>(bufferElement + i++),
                .range = buffer.elementSize()
            };
        });

        descriptorWrite.pBufferInfo = bufferInfos.data();
        break;
    }
    case DescriptorType::StructuredBuffer:
    case DescriptorType::RWStructuredBuffer:
    case DescriptorType::ByteAddressBuffer:
    case DescriptorType::RWByteAddressBuffer:
    {
        descriptorWrite.descriptorCount = elementCount;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        bufferInfos.resize(elementCount);
        std::ranges::generate(bufferInfos, [&buffer, &bufferElement, i = 0]() mutable {
            return VkDescriptorBufferInfo {
                .buffer = buffer.handle(),
                .offset = buffer.alignedElementSize() * static_cast<size_t>(bufferElement + i++),
                .range = buffer.elementSize()
            };
        });

        descriptorWrite.pBufferInfo = bufferInfos.data();
        break;
    }
    case DescriptorType::Buffer:
    {
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;

        VkBufferViewCreateInfo bufferViewDesc {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .buffer = buffer.handle(),
            .format = VK_FORMAT_UNDEFINED,
            .offset = buffer.alignedElementSize() * bufferElement,     // TODO: Handle alignment properly, as texel buffers do not need to be aligned (afaik).
            .range = buffer.alignedElementSize() * elementCount
        };

        VkBufferView bufferView;
        raiseIfFailed(::vkCreateBufferView(m_impl->m_layout.device().handle(), &bufferViewDesc, nullptr, &bufferView), "Unable to create buffer view.");
        m_impl->m_bufferViews[binding] = bufferView;

        descriptorWrite.pTexelBufferView = &bufferView;
        break;
    }
    case DescriptorType::RWBuffer:
    {
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

        VkBufferViewCreateInfo bufferViewDesc {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .buffer = buffer.handle(),
            .format = VK_FORMAT_UNDEFINED,
            .offset = buffer.alignedElementSize() * bufferElement,     // TODO: Handle alignment properly, as texel buffers do not need to be aligned (afaik).
            .range = buffer.alignedElementSize() * elementCount
        };

        VkBufferView bufferView;
        raiseIfFailed(::vkCreateBufferView(m_impl->m_layout.device().handle(), &bufferViewDesc, nullptr, &bufferView), "Unable to create buffer view.");
        m_impl->m_bufferViews[binding] = bufferView;

        descriptorWrite.pTexelBufferView = &bufferView;
        break;
    }
    default: [[unlikely]]
        throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to a buffer descriptor.", binding);
    }

    // Remove the buffer view, if there is one bound to the current descriptor.
    if (m_impl->m_bufferViews.contains(binding))
    {
        ::vkDestroyBufferView(m_impl->m_layout.device().handle(), m_impl->m_bufferViews[binding], nullptr);
        m_impl->m_bufferViews.erase(binding);
    }

    // Update the descriptor set.
    ::vkUpdateDescriptorSets(m_impl->m_layout.device().handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanImage& texture, UInt32 descriptor, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
{
    VkDescriptorImageInfo imageInfo{ };
    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->handle();
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = descriptor;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    const auto& layout = m_impl->m_layout.descriptor(binding);

    switch (layout.descriptorType())
    {
    case DescriptorType::Texture:
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        break;
    case DescriptorType::RWTexture:
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        break;
    default: [[unlikely]]
        throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to a texture descriptor.", binding);
    }

    // Remove the image view, if there is one bound to the current descriptor.
    if (m_impl->m_imageViews.contains(binding))
    {
        ::vkDestroyImageView(m_impl->m_layout.device().handle(), m_impl->m_imageViews[binding], nullptr);
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
    raiseIfFailed(::vkCreateImageView(m_impl->m_layout.device().handle(), &imageViewDesc, nullptr, &imageView), "Unable to create image view.");
    m_impl->m_imageViews[binding] = imageView;
    imageInfo.imageView = imageView;

    ::vkUpdateDescriptorSets(m_impl->m_layout.device().handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanSampler& sampler, UInt32 descriptor) const
{
    const auto& layout = m_impl->m_layout.descriptor(binding);

    if (layout.descriptorType() != DescriptorType::Sampler) [[unlikely]]
        throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to a sampler descriptor.", binding);

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

    ::vkUpdateDescriptorSets(m_impl->m_layout.device().handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanAccelerationStructure& accelerationStructure, UInt32 descriptor) const
{
    const auto& layout = m_impl->m_layout.descriptor(binding);

    if (layout.descriptorType() != DescriptorType::AccelerationStructure) [[unlikely]]
        throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to an acceleration structure descriptor.", binding);

    if (accelerationStructure.buffer() == nullptr || accelerationStructure.handle() == VK_NULL_HANDLE) [[unlikely]]
        throw InvalidArgumentException("accelerationStructure", "The acceleration structure buffer has not yet been allocated.");

    VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureInfo = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
        .accelerationStructureCount = 1,
        .pAccelerationStructures = &accelerationStructure.handle()
    };
    
    VkWriteDescriptorSet descriptorWrite = { 
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = &accelerationStructureInfo,
        .dstSet = this->handle(),
        .dstBinding = binding,
        .dstArrayElement = descriptor,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
    };

    ::vkUpdateDescriptorSets(m_impl->m_layout.device().handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::attach(UInt32 binding, const IVulkanImage& image) const
{
    const auto& layout = m_impl->m_layout.descriptor(binding);

    if (layout.descriptorType() != DescriptorType::InputAttachment) [[unlikely]]
        throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to a input attachment descriptor.", binding);

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

    ::vkUpdateDescriptorSets(m_impl->m_layout.device().handle(), 1, &descriptorWrite, 0, nullptr);
}