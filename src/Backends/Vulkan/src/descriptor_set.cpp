#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
extern PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddress;
extern PFN_vkWriteResourceDescriptorsEXT vkWriteResourceDescriptors;
extern PFN_vkWriteSamplerDescriptorsEXT vkWriteSamplerDescriptors;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------
class VulkanDescriptorSet::VulkanDescriptorSetImpl {
public:
    friend class VulkanDescriptorSet;

private:
    SharedPtr<const VulkanDescriptorSetLayout> m_layout;
    Array<Byte> m_resourceDescriptors{}, m_samplerDescriptors{};
    DescriptorHeapAllocation m_resourceAllocation{}, m_samplerAllocation{};
    UInt32 m_unboundedArraySize;

public:
    VulkanDescriptorSetImpl(const VulkanDescriptorSetLayout& layout, Array<Byte>&& resourceHeap, Array<Byte>&& samplerHeap) :
        m_layout(layout.shared_from_this()), m_resourceDescriptors(std::move(resourceHeap)), m_samplerDescriptors(std::move(samplerHeap)), m_unboundedArraySize(0)
    {
    }

    VulkanDescriptorSetImpl(const VulkanDescriptorSetLayout& layout, UInt32 unboundedArraySize) :
        m_layout(layout.shared_from_this()), m_unboundedArraySize(unboundedArraySize)
    {
        if (unboundedArraySize == std::numeric_limits<UInt32>::max())
            m_unboundedArraySize = layout.maxUnboundedArraySize();

        // Allocate the host descriptor buffers.
        auto& allocationLayout = layout.allocationLayout();

        m_resourceDescriptors.resize(static_cast<size_t>(allocationLayout.descriptorCount(DescriptorHeapType::Resource, m_unboundedArraySize)) * VulkanDevice::RESOURCE_DESCRIPTOR_STRIDE, 0_b);
        m_samplerDescriptors.resize(static_cast<size_t>(allocationLayout.descriptorCount(DescriptorHeapType::Sampler, m_unboundedArraySize)) * VulkanDevice::SAMPLER_DESCRIPTOR_STRIDE, 0_b);
    }

public:
    VkHostAddressRangeEXT targetAddressRange(DescriptorHeapType heap, UInt32 slot) noexcept
    {
        auto& storage = heap == DescriptorHeapType::Sampler ? m_samplerDescriptors : m_resourceDescriptors;
        size_t stride = heap == DescriptorHeapType::Sampler ? VulkanDevice::SAMPLER_DESCRIPTOR_STRIDE : VulkanDevice::RESOURCE_DESCRIPTOR_STRIDE;

        return { .address = std::next(storage.data(), slot * stride), .size = stride };
    }

    UInt32 updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, DescriptorType bindingType, UInt32 firstDescriptor, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, Format texelFormat)
    {
        // Validate the buffer element bounds.
        UInt32 elementCount = elements > 0 ? elements : buffer.elements() - bufferElement;

        if (bufferElement + elementCount > buffer.elements()) [[unlikely]]
            throw InvalidArgumentException("bufferElement", "The buffer only has {0} elements, however there are {1} elements starting at element {2} specified.", buffer.elements(), elementCount, bufferElement);

        // Validate the descriptor index.
        if (firstDescriptor + elementCount > parent.capacity(descriptorLayout.binding())) [[unlikely]]
            throw InvalidArgumentException("firstDescriptor", "The descriptor array only has {0} elements, however there are {1} elements starting at descriptor {2} specified.", parent.capacity(descriptorLayout.binding()), elementCount, firstDescriptor);

        // Check if the descriptor type is valid for the requested operation.
        if (bindingType != DescriptorType::Buffer &&
            bindingType != DescriptorType::ConstantBuffer &&
            bindingType != DescriptorType::RWBuffer &&
            bindingType != DescriptorType::ByteAddressBuffer &&
            bindingType != DescriptorType::RWByteAddressBuffer &&
            bindingType != DescriptorType::StructuredBuffer &&
            bindingType != DescriptorType::RWStructuredBuffer) [[unlikely]]
            throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to a buffer descriptor.", descriptorLayout.binding());

        // Update the descriptor each element.
        Array<VkDeviceAddressRangeEXT> ranges;
        Array<VkTexelBufferDescriptorInfoEXT> texelInfos;
        Array<VkResourceDescriptorInfoEXT> infos;
        Array<VkHostAddressRangeEXT> targets;

        ranges.reserve(elementCount);
        texelInfos.reserve(elementCount);
        infos.reserve(elementCount);
        targets.reserve(elementCount);

        for (UInt32 i{ 0 }; i < elementCount; ++i)
        {
            // Compute the address range for the buffer element.
            auto& range = ranges.emplace_back(buffer.virtualAddress() + (bufferElement + i) * buffer.alignedElementSize(), buffer.alignedElementSize());

            // Setup the descriptor info.
            auto& info = infos.emplace_back(VkResourceDescriptorInfoEXT { .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT });

            switch (bindingType)
            {
            case DescriptorType::ConstantBuffer:
                info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                info.data.pAddressRange = &range;
                break;
            case DescriptorType::ByteAddressBuffer:
            case DescriptorType::RWByteAddressBuffer:
            case DescriptorType::StructuredBuffer:
            case DescriptorType::RWStructuredBuffer:
                info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                info.data.pAddressRange = &range;
                break;
            case DescriptorType::Buffer:
            case DescriptorType::RWBuffer:
                info.type = bindingType == DescriptorType::Buffer ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                info.data.pTexelBuffer = &texelInfos.emplace_back(VkTexelBufferDescriptorInfoEXT {
                    .sType = VK_STRUCTURE_TYPE_TEXEL_BUFFER_DESCRIPTOR_INFO_EXT, 
                    .format = Vk::getFormat(texelFormat), 
                    .addressRange = range 
                });
                break;
            default:
                std::unreachable();
            }

            // Define the target address range.
            targets.push_back(this->targetAddressRange(DescriptorHeapType::Resource, m_layout->getDescriptorOffset(descriptorLayout.binding(), firstDescriptor + i)));
        }

        auto& device = m_layout->device();
        raiseIfFailed(vkWriteResourceDescriptors(device.handle(), elementCount, infos.data(), targets.data()), "Unable to write buffer descriptors.");

        // Update the invalidated range on the global descriptor heap.
        device.updateGlobalDescriptors(parent, descriptorLayout.binding(), firstDescriptor, elementCount);

        // Return the first descriptor index directly, as we can't index into global descriptor memory in Vulkan.
        return firstDescriptor;
    }

    UInt32 updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, DescriptorType bindingType, UInt32 descriptor, const IVulkanImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers)
    {
        // Validate the descriptor index.
        if (descriptor >= parent.capacity(descriptorLayout.binding())) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor index {0} was out of bounds. The resource descriptor heap only contains {1} descriptors.", descriptor, parent.capacity(descriptorLayout.binding()));

        // Check if the descriptor type is valid for the requested operation.
        if (bindingType != DescriptorType::Texture &&
            bindingType != DescriptorType::RWTexture &&
            bindingType != DescriptorType::InputAttachment) [[unlikely]]
            throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to an image descriptor.", descriptorLayout.binding());

        auto binding = descriptorLayout.binding();
        auto& device = m_layout->device();

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

        VkImageDescriptorInfoEXT imageInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
            .pView = &imageViewDesc,
            .layout = bindingType == DescriptorType::RWTexture ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 
        };

        VkResourceDescriptorInfoEXT info { 
            .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT, 
            .data = { .pImage = &imageInfo } 
        };

        switch (bindingType)
        {
        case DescriptorType::Texture:
            info.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            break;
        case DescriptorType::RWTexture:
            info.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            break;
        case DescriptorType::InputAttachment:
            info.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            break;
        default:
            std::unreachable();
        }

        // Write the descriptor.
        auto addressRange = this->targetAddressRange(DescriptorHeapType::Resource, m_layout->getDescriptorOffset(binding, descriptor));
        raiseIfFailed(vkWriteResourceDescriptors(device.handle(), 1u, &info, &addressRange), "Unable to write image descriptor.");

        // Update the invalidated range on the global descriptor heap.
        device.updateGlobalDescriptors(parent, binding, descriptor, 1u);

        // Return the descriptor index directly, as we can't index into global descriptor memory in Vulkan.
        return descriptor;
    }

    UInt32 updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, UInt32 descriptor, const IVulkanSampler& sampler)
    {
        // Validate the descriptor index.
        if (descriptor >= parent.capacity(descriptorLayout.binding())) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor array at binding {1} of descriptor set {0} does only contain {2} descriptors, but the descriptor {3} has been specified for binding.", m_layout->space(), descriptorLayout.binding(), parent.capacity(descriptorLayout.binding()), descriptor);

        // Validate the descriptor type.
        if (descriptorLayout.descriptorType() != DescriptorType::Sampler && descriptorLayout.descriptorType() != DescriptorType::SamplerDescriptorHeap) [[unlikely]]
            throw InvalidArgumentException("descriptorLayout", "Invalid descriptor type. The binding {0} does not bind a sampler or sampler heap, but rather a {1}.", descriptorLayout.binding(), descriptorLayout.descriptorType());

        // Validate the static sampler state.
        if (descriptorLayout.descriptorType() == DescriptorType::Sampler && descriptorLayout.staticSampler() != nullptr)
            throw InvalidArgumentException("descriptorLayout", "The provided binding does bind a sampler, but also defines a static sampler, which makes binding to it invalid.");

        // Write the sampler descriptor.
        auto& device = m_layout->device();

        auto addressRange = this->targetAddressRange(DescriptorHeapType::Sampler, m_layout->getDescriptorOffset(descriptorLayout.binding(), descriptor));
        raiseIfFailed(vkWriteSamplerDescriptors(device.handle(), 1u, &sampler.createInfo(), &addressRange), "Unable to write sampler descriptor.");

        // Update the invalidated range on the global descriptor heap.
        device.updateGlobalDescriptors(parent, descriptorLayout.binding(), descriptor, 1u);

        // Return the descriptor index directly, as we can't index into global descriptor memory in Vulkan.
        return descriptor;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, Array<Byte>&& resourceHeap, Array<Byte>&& samplerHeap) :
    m_impl(layout, std::move(resourceHeap), std::move(samplerHeap))
{
    if (!m_impl->m_resourceDescriptors.empty())
        m_impl->m_resourceAllocation = layout.device().allocateGlobalDescriptors(*this, DescriptorHeapType::Resource);

    if (!m_impl->m_samplerDescriptors.empty())
        m_impl->m_samplerAllocation = layout.device().allocateGlobalDescriptors(*this, DescriptorHeapType::Sampler);
}

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, UInt32 unboundedArraySize) :
    m_impl(layout, unboundedArraySize)
{
    if (!m_impl->m_resourceDescriptors.empty())
        m_impl->m_resourceAllocation = layout.device().allocateGlobalDescriptors(*this, DescriptorHeapType::Resource);

    if (!m_impl->m_samplerDescriptors.empty())
        m_impl->m_samplerAllocation = layout.device().allocateGlobalDescriptors(*this, DescriptorHeapType::Sampler);
}

VulkanDescriptorSet::~VulkanDescriptorSet() noexcept
{
    const auto& device = m_impl->m_layout->device();
    device.releaseGlobalDescriptors(*this);

    m_impl->m_layout->free(*this);
}

const VulkanDescriptorSetLayout& VulkanDescriptorSet::layout() const noexcept
{
    return *m_impl->m_layout;
}

Array<Byte>&& VulkanDescriptorSet::releaseBuffer(DescriptorHeapType heapType) const
{
    switch (heapType)
    {
    case DescriptorHeapType::Resource:
        return std::move(m_impl->m_resourceDescriptors);
    case DescriptorHeapType::Sampler:
        return std::move(m_impl->m_samplerDescriptors);
    default:
        throw InvalidArgumentException("heapType", "The heap type must refer to a valid descriptor heap.");
    }
}

Span<const Byte> VulkanDescriptorSet::descriptorBuffer(DescriptorHeapType heapType) const
{
    switch (heapType)
    {
    case DescriptorHeapType::Resource:
        return m_impl->m_resourceDescriptors;
    case DescriptorHeapType::Sampler:
        return m_impl->m_samplerDescriptors;
    default:
        throw InvalidArgumentException("heapType", "The heap type must refer to a valid descriptor heap.");
    }
}

UInt32 VulkanDescriptorSet::capacity(UInt32 binding) const noexcept
{
    auto range = m_impl->m_layout->allocationLayout().range(binding);

    if (!range.has_value())
        return 0u;

    return range->Unbounded ? m_impl->m_unboundedArraySize : range->DescriptorCount;
}

DescriptorHeapAllocation VulkanDescriptorSet::globalHeapAllocation(DescriptorHeapType heapType) const
{
    switch (heapType)
    {
    case DescriptorHeapType::Resource:
        return m_impl->m_resourceAllocation;
    case DescriptorHeapType::Sampler:
        return m_impl->m_samplerAllocation;
    default:
        throw InvalidArgumentException("heapType", "The heap type must refer to a valid descriptor heap.");
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

    auto layout = m_impl->m_layout;
    auto& device = layout->device();

    // Find the descriptor.
    auto descriptors = layout->descriptors();
    auto descriptorLayout = std::ranges::find_if(descriptors, [&binding](auto& layout) { return layout.binding() == binding; });

    if (descriptorLayout == descriptors.end()) [[unlikely]]
    {
        LITEFX_WARNING(VULKAN_LOG, "The descriptor set {0} does not contain a descriptor at binding {1}.", layout->space(), binding);
        return;
    }

    // Check if the descriptor type is valid for the requested operation.
    if (descriptorLayout->descriptorType() != DescriptorType::AccelerationStructure) [[unlikely]]
        throw InvalidArgumentException("binding", "Invalid descriptor type. The binding {0} does not point to an acceleration structure descriptor.", binding);

    // Check if all elements can be bound to a bounded array.
    if (descriptor >= this->capacity(binding)) [[unlikely]]
        throw ArgumentOutOfRangeException("descriptor", "The descriptor layout can only bind up to {0} descriptors at binding {3}, however the request was to bind {1} descriptors starting at {2}.", this->capacity(binding), 1, descriptor, binding);

    // Setup the descriptor info.
    VkAccelerationStructureDeviceAddressInfoKHR addressInfo {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
        .accelerationStructure = accelerationStructure.handle()
    };

    VkDeviceAddressRangeEXT range { 
        .address = vkGetAccelerationStructureDeviceAddress(device.handle(), &addressInfo)
        /*.size = 0*/ // Unused for acceleration structures.
    };

    VkResourceDescriptorInfoEXT info {
        .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
        .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        .data = { .pAddressRange = &range } 
    };

    auto addressRange = m_impl->targetAddressRange(DescriptorHeapType::Resource, layout->getDescriptorOffset(binding, descriptor));
    raiseIfFailed(vkWriteResourceDescriptors(device.handle(), 1u, &info, &addressRange), "Unable to write acceleration structure descriptor.");

    // Update the invalidated range on the global descriptor heap.
    device.updateGlobalDescriptors(*this, binding, descriptor, 1u);
}