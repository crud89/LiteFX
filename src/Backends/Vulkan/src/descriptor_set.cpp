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
    Array<Byte> m_descriptorBuffer{};
    UInt32 m_unboundedArraySize, m_offset{ 0 }, m_heapSize{ 0 };

public:
    VulkanDescriptorSetImpl(const VulkanDescriptorSetLayout& layout, Array<Byte>&& buffer) :
        m_layout(layout.shared_from_this()), m_descriptorBuffer(std::move(buffer)), m_unboundedArraySize(0)
    {
    }

    VulkanDescriptorSetImpl(const VulkanDescriptorSetLayout& layout, UInt32 unboundedArraySize) :
        m_layout(layout.shared_from_this()), m_unboundedArraySize(unboundedArraySize)
    {
        // Allocate the descriptor set binding buffer.
        VkDeviceSize descriptorSetSize{};

        if (!layout.containsUnboundedArray())
            vkGetDescriptorSetLayoutSize(m_layout->device().handle(), m_layout->handle(), &descriptorSetSize);
        else
        {
            // If the unbounded descriptor array size is set to the maximum, use the device limit instead. Note that this could still conflict with other limits (such as the per stage resource 
            // binding limit), so prefer to provide an explicit size here.
            if (unboundedArraySize == std::numeric_limits<UInt32>::max())
                m_unboundedArraySize = unboundedArraySize = layout.maxUnboundedArraySize();

            // If the layout contains an unbounded array size, the actual required address space needs to be computed. We exploit the guarantee that unbounded arrays are always put last in 
            // the descriptor set, so we compute the offset, descriptor size and from this the total amount of required memory.
            // First, we need to lookup the binding for the unbounded array. If we cannot match any binding here, we've conceptually messed up somewhere earlier, in which case no error handling
            // could save us, so we gently ignore the chance of not matching anything.
            auto descriptorLayout = std::ranges::find_if(layout.descriptors(), [](auto& layout) { return layout.unbounded(); });
            vkGetDescriptorSetLayoutBindingOffset(layout.device().handle(), layout.handle(), descriptorLayout->binding(), &descriptorSetSize);
            descriptorSetSize += unboundedArraySize * static_cast<VkDeviceSize>(layout.device().descriptorSize(descriptorLayout->descriptorType()));
        }

        m_descriptorBuffer.resize(static_cast<size_t>(descriptorSetSize), 0_b);
    }

public:
    UInt32 updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, DescriptorType bindingType, UInt32 firstDescriptor, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, Format texelFormat)
    {
        // Validate the buffer element bounds.
        UInt32 elementCount = elements > 0 ? elements : buffer.elements() - bufferElement;

        if (bufferElement + elementCount > buffer.elements()) [[unlikely]]
            throw InvalidArgumentException("bufferElement", "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), elementCount, bufferElement);

        // Validate the descriptor index.
        if (firstDescriptor + elementCount > descriptorLayout.descriptors()) [[unlikely]]
            throw InvalidArgumentException("firstDescriptor", "The descriptor array only has {0} elements, however there are {1} elements starting at descriptor {2} specified.", descriptorLayout.descriptors(), elementCount, firstDescriptor);

        // Check if the descriptor type is valid for the requested operation.
        if (bindingType != DescriptorType::Buffer &&
            bindingType != DescriptorType::ConstantBuffer &&
            bindingType != DescriptorType::RWBuffer &&
            bindingType != DescriptorType::ByteAddressBuffer &&
            bindingType != DescriptorType::RWByteAddressBuffer &&
            bindingType != DescriptorType::StructuredBuffer &&
            bindingType != DescriptorType::RWStructuredBuffer) [[unlikely]]
            throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to a buffer descriptor.", descriptorLayout.binding());

        // Offset to first array index. Arrays are tightly packed, so we simply add the descriptor size for each element.
        size_t descriptorSize = m_layout->device().descriptorSize(bindingType);

        // Update the descriptor each element.
        for (UInt32 i{ 0 }; i < elementCount; ++i)
        {
            // Acquire the binding offset.
            auto descriptorOffset = static_cast<VkDeviceSize>(m_layout->getDescriptorOffset(descriptorLayout.binding(), firstDescriptor + i));

            // Create the address info object.
            VkDescriptorAddressInfoEXT addressInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
                .address = buffer.virtualAddress() + (bufferElement + i) * buffer.alignedElementSize(),
                .range = buffer.alignedElementSize(),
                .format = VK_FORMAT_UNDEFINED
            };

            // Setup the descriptor info.
            VkDescriptorGetInfoEXT descriptorInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };

            switch (bindingType)
            {
            case DescriptorType::ConstantBuffer:
                descriptorInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorInfo.data.pUniformBuffer = &addressInfo;
                break;
            case DescriptorType::Buffer:
                descriptorInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                descriptorInfo.data.pUniformTexelBuffer = &addressInfo;
                addressInfo.format = Vk::getFormat(texelFormat);
                break;
            case DescriptorType::RWBuffer:
                descriptorInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                descriptorInfo.data.pStorageTexelBuffer = &addressInfo;
                addressInfo.format = Vk::getFormat(texelFormat);
                break;
            case DescriptorType::ByteAddressBuffer:
            case DescriptorType::RWByteAddressBuffer:
            case DescriptorType::StructuredBuffer:
            case DescriptorType::RWStructuredBuffer:
                descriptorInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorInfo.data.pStorageBuffer = &addressInfo;
                break;
            default:
                std::unreachable();
            }

            // Create the descriptor in the descriptor buffer.
            vkGetDescriptor(m_layout->device().handle(), &descriptorInfo, descriptorSize, std::next(m_descriptorBuffer.data(), static_cast<size_t>(descriptorOffset))); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
        }

        // Update the invalidated range on the global descriptor heap.
        m_layout->device().updateGlobalDescriptors(parent, descriptorLayout.binding(), firstDescriptor, elementCount);

        // Return the first descriptor index directly, as we can't index into global descriptor memory in Vulkan.
        return firstDescriptor;
    }

    UInt32 updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, DescriptorType bindingType, UInt32 descriptor, const IVulkanImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers)
    {
        // Validate the descriptor index.
        if (descriptor >= descriptorLayout.descriptors()) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor index {0} was out of bounds. The resource descriptor heap only contains {1} descriptors.", descriptor, descriptorLayout.descriptors());

        // Check if the descriptor type is valid for the requested operation.
        if (bindingType != DescriptorType::Texture &&
            bindingType != DescriptorType::RWTexture &&
            bindingType != DescriptorType::InputAttachment) [[unlikely]]
            throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to an image descriptor.", descriptorLayout.binding());

        // Remove the image view, if there is one bound to the current descriptor.
        auto binding = descriptorLayout.binding();
        auto& device = m_layout->device();

        if (m_imageViews.contains(binding))
        {
            ::vkDestroyImageView(device.handle(), m_imageViews[binding], nullptr);
            m_imageViews.erase(binding);
        }

        // Create a new image view.
        const UInt32 numLevels = levels == 0 ? image.levels() - firstLevel : levels;
        const UInt32 numLayers = layers == 0 ? image.layers() - firstLayer : layers;

        VkImageViewCreateInfo imageViewDesc = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .image = image.handle(),
            .viewType = Vk::getImageViewType(image.dimensions(), numLayers), // TODO: What if we want to bind an array with one layer only, though?!... `DescriptorLayout` should get an "isArray" property.
            .format = Vk::getFormat(image.format()),
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

        if (!::hasDepth(image.format()) && !::hasStencil(image.format()))
            imageViewDesc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        else
        {
            // TODO: This probably wont work, instead we need separate views here. Maybe we could add a "plane" parameter that addresses the depth/stencil view.
            if (::hasDepth(image.format()))
                imageViewDesc.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;

            if (::hasStencil(image.format()))
                imageViewDesc.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageView imageView{};
        raiseIfFailed(::vkCreateImageView(device.handle(), &imageViewDesc, nullptr, &imageView), "Unable to create image view.");
        m_imageViews[binding] = imageView;

        // Acquire the binding offset.
        auto descriptorOffset = static_cast<VkDeviceSize>(m_layout->getDescriptorOffset(binding, descriptor));

        // Offset to first array index. Arrays are tightly packed, so we simply add the descriptor size for each element.
        size_t descriptorSize = device.descriptorSize(bindingType);

        // Create the image info object.
        VkDescriptorImageInfo imageInfo = {
            .imageView = imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // NOTE: We do have to change this for RWimage.
        };

        // Setup the descriptor info.
        VkDescriptorGetInfoEXT descriptorInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };

        // TODO: Texel buffers need a format, which is currently not supported.
        switch (bindingType)
        {
        case DescriptorType::Texture:
            descriptorInfo.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            descriptorInfo.data.pSampledImage = &imageInfo;
            break;
        case DescriptorType::RWTexture:
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            descriptorInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            descriptorInfo.data.pStorageImage = &imageInfo;
            break;
        case DescriptorType::InputAttachment:
            descriptorInfo.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            descriptorInfo.data.pInputAttachmentImage = &imageInfo;
            break;
        default:
            std::unreachable();
        }

        // Create the descriptor in the descriptor buffer.
        vkGetDescriptor(device.handle(), &descriptorInfo, descriptorSize, std::next(m_descriptorBuffer.data(), static_cast<size_t>(descriptorOffset))); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

        // Update the invalidated range on the global descriptor heap.
        device.updateGlobalDescriptors(parent, binding, descriptor, 1u);

        // Return the descriptor index directly, as we can't index into global descriptor memory in Vulkan.
        return descriptor;
    }

    UInt32 updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, UInt32 descriptor, const IVulkanSampler& sampler)
    {
        // Validate the descriptor index.
        if (descriptor >= descriptorLayout.descriptors()) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor array at binding {1} of descriptor set {0} does only contain {2} descriptors, but the descriptor {3} has been specified for binding.", m_layout->space(), descriptorLayout.binding(), descriptorLayout.descriptors(), descriptor);

        // Validate the descriptor type.
        if (descriptorLayout.descriptorType() != DescriptorType::Sampler && descriptorLayout.descriptorType() != DescriptorType::SamplerDescriptorHeap) [[unlikely]]
            throw InvalidArgumentException("descriptorLayout", "Invalid descriptor type. The binding {0} does not bind a sampler or sampler heap, but rather a {1}.", descriptorLayout.binding(), descriptorLayout.descriptorType());

        // Validate the static sampler state.
        if (descriptorLayout.descriptorType() == DescriptorType::Sampler && descriptorLayout.staticSampler() != nullptr)
            throw InvalidArgumentException("descriptorLayout", "The provided binding does bind a sampler, but also defines a static sampler, which makes binding to it invalid.");

        // Acquire the binding offset.
        auto descriptorOffset = static_cast<VkDeviceSize>(m_layout->getDescriptorOffset(descriptorLayout.binding(), descriptor));
        auto& device = m_layout->device();

        // Offset to first array index. Arrays are tightly packed, so we simply add the descriptor size for each element.
        size_t descriptorSize = device.descriptorSize(descriptorLayout.descriptorType());

        // Setup the descriptor info.
        VkDescriptorGetInfoEXT descriptorInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .data = { .pSampler = &sampler.handle() }
        };

        // Create the descriptor in the descriptor buffer.
        vkGetDescriptor(device.handle(), &descriptorInfo, descriptorSize, std::next(m_descriptorBuffer.data(), static_cast<size_t>(descriptorOffset))); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

        // Update the invalidated range on the global descriptor heap.
        device.updateGlobalDescriptors(parent, descriptorLayout.binding(), descriptor, 1u);

        // Return the descriptor index directly, as we can't index into global descriptor memory in Vulkan.
        return descriptor;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, Array<Byte>&& buffer) :
    m_impl(layout, std::move(buffer))
{
    layout.device().allocateGlobalDescriptors(*this, DescriptorHeapType::Resource, m_impl->m_offset, m_impl->m_heapSize); // NOTE: Heap type does not matter for Vulkan backend.
}

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, UInt32 unboundedArraySize) :
    m_impl(layout, unboundedArraySize)
{
    layout.device().allocateGlobalDescriptors(*this, DescriptorHeapType::Resource, m_impl->m_offset, m_impl->m_heapSize); // NOTE: Heap type does not matter for Vulkan backend.
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

UInt32 VulkanDescriptorSet::globalHeapOffset(DescriptorHeapType heapType) const noexcept
{
    switch (heapType)
    {
    case DescriptorHeapType::Resource:
    case DescriptorHeapType::Sampler:
        return m_impl->m_offset;
    default:
        return std::numeric_limits<UInt32>::max();
    }
}

UInt32 VulkanDescriptorSet::globalHeapAddressRange(DescriptorHeapType heapType) const noexcept
{
    switch (heapType)
    {
    case DescriptorHeapType::Resource:
    case DescriptorHeapType::Sampler:
        return m_impl->m_heapSize;
    default:
        return 0u;
    }
}

UInt32 VulkanDescriptorSet::bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, Format texelFormat) const
{
    // Find the resource descriptor heap.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [](const auto& layout) { return layout.descriptorType() == DescriptorType::ResourceDescriptorHeap; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
        throw RuntimeException("The descriptor set does not contain a resource heap descriptor.");

    // Update the binding.
    return m_impl->updateBinding(*this, (*descriptorLayout), bindingType, descriptor, buffer, bufferElement, elements, texelFormat);
}

UInt32 VulkanDescriptorSet::bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IVulkanImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
{
    // Find the resource descriptor heap.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [](const auto& layout) { return layout.descriptorType() == DescriptorType::ResourceDescriptorHeap; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
        throw RuntimeException("The descriptor set does not contain a resource heap descriptor.");

    // Update the binding.
    return m_impl->updateBinding(*this, (*descriptorLayout), bindingType, descriptor, image, firstLevel, levels, firstLayer, layers);
}

UInt32 VulkanDescriptorSet::bindToHeap(UInt32 descriptor, const IVulkanSampler& sampler) const
{
    // Find the sampler descriptor heap.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [](const auto& layout) { return layout.descriptorType() == DescriptorType::SamplerDescriptorHeap; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
        throw RuntimeException("The descriptor set does not contain a sampler heap descriptor.");

    // Update the binding.
    return m_impl->updateBinding(*this, (*descriptorLayout), descriptor, sampler);
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor, Format texelFormat) const
{
    // Find the descriptor.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
        return;
    }

    // Update the binding.
    m_impl->updateBinding(*this, (*descriptorLayout), descriptorLayout->descriptorType(), firstDescriptor, buffer, bufferElement, elements, texelFormat);
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanImage& texture, UInt32 descriptor, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
{
    // Find the descriptor.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
        return;
    }

    // Update the binding.
    m_impl->updateBinding(*this, (*descriptorLayout), descriptorLayout->descriptorType(), descriptor, texture, firstLevel, levels, firstLayer, layers);
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanSampler& sampler, UInt32 descriptor) const
{
    // Find the descriptor.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
        return;
    }

    // Update the binding.
    m_impl->updateBinding(*this, (*descriptorLayout), descriptor, sampler);
}

void VulkanDescriptorSet::update(UInt32 binding, const IVulkanAccelerationStructure& accelerationStructure, UInt32 descriptor) const
{
    // Check if the acceleration structure has been initialized.
    if (accelerationStructure.buffer() == nullptr || accelerationStructure.handle() == VK_NULL_HANDLE) [[unlikely]]
        throw InvalidArgumentException("accelerationStructure", "The acceleration structure buffer has not yet been allocated.");

    // Find the descriptor.
    auto descriptors = m_impl->m_layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", m_impl->m_layout->space(), binding);
        return;
    }

    // Check if the descriptor type is valid for the requested operation.
    if (descriptorLayout->descriptorType() != DescriptorType::AccelerationStructure) [[unlikely]]
        throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to an acceleration structure descriptor.", binding);

    // Check if all elements can be bound to a bounded array.
    if (descriptorLayout->descriptors() <= descriptor) [[unlikely]]
        throw ArgumentOutOfRangeException("descriptor", "The descriptor layout can only bind up to {0} descriptors at binding {3}, however the request was to bind {1} descriptors starting at {2}.", descriptorLayout->descriptors(), 1, descriptor, binding);

    // Acquire the binding offset.
    auto descriptorOffset = static_cast<VkDeviceSize>(m_impl->m_layout->getDescriptorOffset(binding, descriptor));

    // Offset to first array index. Arrays are tightly packed, so we simply add the descriptor size for each element.
    size_t descriptorSize = m_impl->m_layout->device().descriptorSize(descriptorLayout->descriptorType());

    // Setup the descriptor info.
    VkDescriptorGetInfoEXT descriptorInfo = { 
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
        .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        .data = { .accelerationStructure = accelerationStructure.buffer()->virtualAddress() }
    };

    // Create the descriptor in the descriptor buffer.
    vkGetDescriptor(m_impl->m_layout->device().handle(), &descriptorInfo, descriptorSize, std::next(m_impl->m_descriptorBuffer.data(), static_cast<size_t>(descriptorOffset))); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

    // Update the invalidated range on the global descriptor heap.
    m_impl->m_layout->device().updateGlobalDescriptors(*this, binding, descriptor, 1u);
}