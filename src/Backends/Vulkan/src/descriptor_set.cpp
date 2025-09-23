#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
extern PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSize;
extern PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffset;
extern PFN_vkGetDescriptorEXT vkGetDescriptor;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------
class VulkanDescriptorSet::VulkanDescriptorSetImpl {
public:
    friend class VulkanDescriptorSet;

private:
    Dictionary<UInt32, VkImageView> m_imageViews{};
    SharedPtr<const VulkanDescriptorSetLayout> m_layout;
    Array<Byte> m_descriptorBuffer;
    UInt32 m_unboundedArraySize, m_offset{ 0 }, m_heapSize{ 0 };

public:
    VulkanDescriptorSetImpl(const VulkanDescriptorSetLayout& layout, Array<Byte>&& buffer) :
        m_layout(layout.shared_from_this()), m_unboundedArraySize(0)
    {
        m_descriptorBuffer = std::move(buffer);
    }

    VulkanDescriptorSetImpl(const VulkanDescriptorSetLayout& layout, UInt32 unboundedArraySize) :
        m_layout(layout.shared_from_this()), m_unboundedArraySize(unboundedArraySize)
    {
        // Allocate the descriptor set binding buffer.
        VkDeviceSize descriptorSetSize;

        if (!layout.containsUnboundedArray())
            vkGetDescriptorSetLayoutSize(m_layout->device().handle(), m_layout->handle(), &descriptorSetSize);
        else
        {
            // If the layout contains an unbounded array size, the actual required address space needs to be computed. We exploit the guarantee that unbounded arrays are always put last in 
            // the descriptor set, so we compute the offset, descriptor size and from this the total amount of required memory.
            // First, we need to lookup the binding for the unbounded array. If we cannot match any binding here, we've conceptually messed up somewhere earlier, in which case no error handling
            // could save us, so we gently ignore the chance of not matching anything.
            auto descriptorLayout = std::ranges::find_if(layout.descriptors(), [](auto& layout) { return layout.descriptors() == -1; });
            vkGetDescriptorSetLayoutBindingOffset(layout.device().handle(), layout.handle(), descriptorLayout->binding(), &descriptorSetSize);
            descriptorSetSize += unboundedArraySize * layout.device().descriptorSize(descriptorLayout->descriptorType());
        }

        m_descriptorBuffer.resize(static_cast<size_t>(descriptorSetSize), 0_b);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, Array<Byte>&& buffer) :
    m_impl(layout, std::forward<Array<Byte>>(buffer))
{
    layout.device().allocateGlobalDescriptors(*this, m_impl->m_offset, m_impl->m_heapSize);
}

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, UInt32 unboundedArraySize) :
    m_impl(layout, unboundedArraySize)
{
    layout.device().allocateGlobalDescriptors(*this, m_impl->m_offset, m_impl->m_heapSize);
}

VulkanDescriptorSet::~VulkanDescriptorSet() noexcept
{
    const auto& device = m_impl->m_layout->device();

    for (auto& imageView : m_impl->m_imageViews)
        ::vkDestroyImageView(device.handle(), imageView.second, nullptr);

    m_impl->m_layout->free(*this);
}

const VulkanDescriptorSetLayout& VulkanDescriptorSet::layout() const noexcept
{
    return *m_impl->m_layout;
}

Array<Byte>&& VulkanDescriptorSet::releaseBuffer() const noexcept
{
    return std::move(m_impl->m_descriptorBuffer);
}

Span<const Byte> VulkanDescriptorSet::descriptorBuffer() const noexcept
{
    return m_impl->m_descriptorBuffer;
}

UInt32 VulkanDescriptorSet::globalHeapOffset() const noexcept
{
    return m_impl->m_offset;
}

UInt32 VulkanDescriptorSet::globalHeapAddressRange() const noexcept
{
    return m_impl->m_heapSize;
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor) const
{
    UInt32 elementCount = elements > 0 ? elements : buffer.elements() - bufferElement;

    if (bufferElement + elementCount > buffer.elements()) [[unlikely]]
        LITEFX_WARNING(VULKAN_LOG, "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), elementCount, bufferElement);

    // Find the descriptor.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
        return;
    }

    // Check if the descriptor type is valid for the requested operation.
    if (descriptorLayout->descriptorType() != DescriptorType::Buffer && 
        descriptorLayout->descriptorType() != DescriptorType::ConstantBuffer &&
        descriptorLayout->descriptorType() != DescriptorType::RWBuffer && 
        descriptorLayout->descriptorType() != DescriptorType::ByteAddressBuffer &&
        descriptorLayout->descriptorType() != DescriptorType::RWByteAddressBuffer && 
        descriptorLayout->descriptorType() != DescriptorType::StructuredBuffer &&
        descriptorLayout->descriptorType() != DescriptorType::RWStructuredBuffer) [[unlikely]]
        throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to a buffer descriptor.", binding);

    // Check if all elements can be bound to a bounded array.
    if (descriptorLayout->descriptors() > 1 && descriptorLayout->descriptors() < (firstDescriptor + elementCount)) [[unlikely]]
        throw ArgumentOutOfRangeException("elements", "The descriptor layout can only bind up to {0} descriptors at binding {3}, however the request was to bind {1} descriptors starting at {2}.", descriptorLayout->descriptors(), elementCount, firstDescriptor, binding);

    // Acquire the binding offset.
    auto descriptorOffset = static_cast<VkDeviceSize>(m_impl->m_layout->getDescriptorOffset(binding));

    // Offset to first array index. Arrays are tightly packed, so we simply add the descriptor size for each element.
    size_t descriptorSize = m_impl->m_layout->device().descriptorSize(descriptorLayout->descriptorType());

    // Update the descriptor each element.
    for (UInt32 i{ 0 }; i < elementCount; ++i)
    {
        // Create the address info object.
        VkDescriptorAddressInfoEXT addressInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
            .address = buffer.virtualAddress() + (bufferElement + i) * buffer.alignedElementSize(),
            .range = buffer.alignedElementSize(),
            .format = VK_FORMAT_UNDEFINED
        };

        // Setup the descriptor info.
        VkDescriptorGetInfoEXT descriptorInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };

        // TODO: Texel buffers need a format, which is currently not supported.
        switch (descriptorLayout->descriptorType())
        {
        case DescriptorType::ConstantBuffer:
            descriptorInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorInfo.data.pUniformBuffer = &addressInfo;
            break;
        case DescriptorType::Buffer:
            descriptorInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            descriptorInfo.data.pUniformTexelBuffer = &addressInfo;
            addressInfo.format = VK_FORMAT_R8G8B8A8_UINT;
            break;
        case DescriptorType::RWBuffer:
            descriptorInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            descriptorInfo.data.pStorageTexelBuffer = &addressInfo;
            addressInfo.format = VK_FORMAT_R8G8B8A8_UINT;
            break;
        case DescriptorType::ByteAddressBuffer:
        case DescriptorType::RWByteAddressBuffer:
        case DescriptorType::StructuredBuffer:
        case DescriptorType::RWStructuredBuffer:
            descriptorInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorInfo.data.pStorageBuffer = &addressInfo;
            break;
        }

        // Create the descriptor in the descriptor buffer.
        vkGetDescriptor(m_impl->m_layout->device().handle(), &descriptorInfo, descriptorSize, m_impl->m_descriptorBuffer.data());
    }

    // Update the invalidated range on the global descriptor heap.
    m_impl->m_layout->device().updateGlobalDescriptors(*this, binding, firstDescriptor, elementCount);
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanImage& texture, UInt32 descriptor, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
{
    throw;

    //// Find the descriptor.
    //auto descriptors = m_impl->m_layout->descriptors();
    //auto match = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    //if (match == descriptors.end()) [[unlikely]]
    //{
    //    LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
    //    return;
    //}
    //
    //const auto& layout = *match;

    //VkDescriptorImageInfo imageInfo{ };
    //VkWriteDescriptorSet descriptorWrite{ };
    //descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //descriptorWrite.dstSet = this->handle();
    //descriptorWrite.dstBinding = binding;
    //descriptorWrite.dstArrayElement = descriptor;
    //descriptorWrite.descriptorCount = 1;
    //descriptorWrite.pImageInfo = &imageInfo;

    //switch (layout.descriptorType())
    //{
    //case DescriptorType::Texture:
    //    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    //    break;
    //case DescriptorType::RWTexture:
    //    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    //    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    //    break;
    //case DescriptorType::InputAttachment:
    //    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    //    break;
    //default: [[unlikely]]
    //    throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to a texture descriptor.", binding);
    //}

    //// Remove the image view, if there is one bound to the current descriptor.
    //if (m_impl->m_imageViews.contains(binding))
    //{
    //    ::vkDestroyImageView(m_impl->m_layout->device().handle(), m_impl->m_imageViews[binding], nullptr);
    //    m_impl->m_imageViews.erase(binding);
    //}
    //
    //// Create a new image view.
    //const UInt32 numLevels = levels == 0 ? texture.levels() - firstLevel : levels;
    //const UInt32 numLayers = layers == 0 ? texture.layers() - firstLayer : layers;

    //VkImageViewCreateInfo imageViewDesc = {
    //    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    //    .pNext = nullptr,
    //    .image = texture.handle(),
    //    .viewType = Vk::getImageViewType(texture.dimensions(), numLayers), // TODO: What if we want to bind an array with one layer only, though?!... `DescriptorLayout` should get an "isArray" property.
    //    .format = Vk::getFormat(texture.format()),
    //    .components = VkComponentMapping {
    //        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
    //        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
    //        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
    //        .a = VK_COMPONENT_SWIZZLE_IDENTITY
    //    },
    //    .subresourceRange = VkImageSubresourceRange {
    //        .baseMipLevel = firstLevel,
    //        .levelCount = numLevels,
    //        .baseArrayLayer = firstLayer,
    //        .layerCount = numLayers
    //    }
    //};

    //if (!::hasDepth(texture.format()) && !::hasStencil(texture.format()))
    //    imageViewDesc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //else
    //{
    //    // TODO: This probably wont work, instead we need separate views here. Maybe we could add a "plane" parameter that addresses the depth/stencil view.
    //    if (::hasDepth(texture.format()))
    //        imageViewDesc.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;

    //    if (::hasStencil(texture.format()))
    //        imageViewDesc.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    //}

    //VkImageView imageView{};
    //raiseIfFailed(::vkCreateImageView(m_impl->m_layout->device().handle(), &imageViewDesc, nullptr, &imageView), "Unable to create image view.");
    //m_impl->m_imageViews[binding] = imageView;
    //imageInfo.imageView = imageView;

    //::vkUpdateDescriptorSets(m_impl->m_layout->device().handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanSampler& sampler, UInt32 descriptor) const
{
    throw;

    //// Find the descriptor.
    //auto descriptors = m_impl->m_layout->descriptors();
    //auto descriptorLayout = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    //if (descriptorLayout == descriptors.end()) [[unlikely]]
    //{
    //    LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
    //    return;
    //}

    //// Check if the descriptor type is valid for the requested operation.
    //if (descriptorLayout->descriptorType() != DescriptorType::Sampler) [[unlikely]]
    //    throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to a sampler descriptor.", binding);

    //// Check if all elements can be bound to a bounded array.
    //if (descriptorLayout->descriptors() > 1 && descriptorLayout->descriptors() <= descriptor) [[unlikely]]
    //    throw ArgumentOutOfRangeException("descriptor", "The descriptor layout can only bind up to {0} descriptors at binding {3}, however the request was to bind {1} descriptors starting at {2}.", descriptorLayout->descriptors(), 1, descriptor, binding);

    //// Acquire the binding offset.
    //auto descriptorOffset = ...;
    ////vkGetDescriptorSetLayoutBindingOffset(m_impl->m_layout->device().handle(), m_impl->m_layout->handle(), binding, &descriptorOffset);

    //// Offset to first array index. Arrays are tightly packed, so we simply add the descriptor size for each element.
    //size_t descriptorSize = m_impl->m_layout->device().descriptorSize(descriptorLayout->descriptorType());

    //// Setup the descriptor info.
    //VkDescriptorGetInfoEXT descriptorInfo = { 
    //    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
    //    .type = VK_DESCRIPTOR_TYPE_SAMPLER,
    //    .data = { .pSampler = &sampler.handle() }
    //};

    //// Create the descriptor in the descriptor buffer.
    //vkGetDescriptor(m_impl->m_layout->device().handle(), &descriptorInfo, descriptorSize, m_impl->m_descriptorBuffer.get());
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanAccelerationStructure& accelerationStructure, UInt32 descriptor) const
{
    throw;

    //// Check if the acceleration structure has been initialized.
    //if (accelerationStructure.buffer() == nullptr || accelerationStructure.handle() == VK_NULL_HANDLE) [[unlikely]]
    //    throw InvalidArgumentException("accelerationStructure", "The acceleration structure buffer has not yet been allocated.");

    //// Find the descriptor.
    //auto descriptors = m_impl->m_layout->descriptors();
    //auto descriptorLayout = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    //if (descriptorLayout == descriptors.end()) [[unlikely]]
    //{
    //    LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
    //    return;
    //}

    //// Check if the descriptor type is valid for the requested operation.
    //if (descriptorLayout->descriptorType() != DescriptorType::AccelerationStructure) [[unlikely]]
    //    throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to an acceleration structure descriptor.", binding);

    //// Check if all elements can be bound to a bounded array.
    //if (descriptorLayout->descriptors() > 1 && descriptorLayout->descriptors() <= descriptor) [[unlikely]]
    //    throw ArgumentOutOfRangeException("descriptor", "The descriptor layout can only bind up to {0} descriptors at binding {3}, however the request was to bind {1} descriptors starting at {2}.", descriptorLayout->descriptors(), 1, descriptor, binding);

    //// Acquire the binding offset.
    //auto descriptorOffset = ...;
    ////vkGetDescriptorSetLayoutBindingOffset(m_impl->m_layout->device().handle(), m_impl->m_layout->handle(), binding, &descriptorOffset);

    //// Offset to first array index. Arrays are tightly packed, so we simply add the descriptor size for each element.
    //size_t descriptorSize = m_impl->m_layout->device().descriptorSize(descriptorLayout->descriptorType());

    //// Setup the descriptor info.
    //VkDescriptorGetInfoEXT descriptorInfo = { 
    //    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
    //    .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    //    .data = { .accelerationStructure = accelerationStructure.buffer()->virtualAddress() }
    //};

    //// Create the descriptor in the descriptor buffer.
    //vkGetDescriptor(m_impl->m_layout->device().handle(), &descriptorInfo, descriptorSize, m_impl->m_descriptorBuffer.get());
}