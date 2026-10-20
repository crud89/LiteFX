#include <litefx/backends/vulkan.hpp>
#include "descriptors.hpp"

using namespace LiteFX::Rendering::Backends;

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

        auto& allocationLayout = layout.allocationLayout();
        m_resourceDescriptors.resize(static_cast<size_t>(allocationLayout.descriptorCount(DescriptorHeapType::Resource, m_unboundedArraySize)) * m_layout->device().descriptorStride(DescriptorHeapType::Resource), 0_b);
        m_samplerDescriptors.resize(static_cast<size_t>(allocationLayout.descriptorCount(DescriptorHeapType::Sampler, m_unboundedArraySize)) * m_layout->device().descriptorStride(DescriptorHeapType::Sampler), 0_b);
    }

public:
    VkHostAddressRangeEXT targetAddressRange(DescriptorHeapType heap, UInt32 slot)
    {
        auto& storage = heap == DescriptorHeapType::Sampler ? m_samplerDescriptors : m_resourceDescriptors;
        size_t stride = m_layout->device().descriptorStride(heap);
        auto descriptor = Span<Byte>{ storage }.subspan(static_cast<size_t>(slot) * stride, stride);

        return { .address = descriptor.data(), .size = descriptor.size() };
    }

    UInt32 updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, DescriptorType bindingType, UInt32 firstDescriptor, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, Format texelFormat)
    {
        auto binding = descriptorLayout.binding();
        auto elementCount = bufferElements(buffer, bufferElement, elements);

        if (firstDescriptor + elementCount > parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("firstDescriptor", "The descriptor array only has {0} elements, however there are {1} elements starting at descriptor {2} specified.", parent.capacity(binding), elementCount, firstDescriptor);

        auto targets = std::views::iota(firstDescriptor, firstDescriptor + elementCount)
            | std::views::transform([&](UInt32 descriptor) { return this->targetAddressRange(DescriptorHeapType::Resource, m_layout->getDescriptorOffset(binding, descriptor)); })
            | std::ranges::to<Array<VkHostAddressRangeEXT>>();

        auto& device = m_layout->device();
        writeBufferDescriptors(device, bindingType, buffer, bufferElement, texelFormat, targets);
        device.updateGlobalDescriptors(parent, binding, firstDescriptor, elementCount);

        return static_cast<UInt32>(parent.globalHeapAllocation(DescriptorHeapType::Resource).Allocation.Offset + m_layout->getDescriptorOffset(binding, firstDescriptor));
    }

    UInt32 updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, DescriptorType bindingType, UInt32 descriptor, const IVulkanImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers)
    {
        auto binding = descriptorLayout.binding();

        if (descriptor >= parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor index {0} was out of bounds. The descriptor array at binding {1} only contains {2} descriptors.", descriptor, binding, parent.capacity(binding));

        auto& device = m_layout->device();
        auto descriptorOffset = m_layout->getDescriptorOffset(binding, descriptor);
        writeImageDescriptor(device, bindingType, image, firstLevel, levels, firstLayer, layers, this->targetAddressRange(DescriptorHeapType::Resource, descriptorOffset));
        device.updateGlobalDescriptors(parent, binding, descriptor, 1u);

        return static_cast<UInt32>(parent.globalHeapAllocation(DescriptorHeapType::Resource).Allocation.Offset + descriptorOffset);
    }

    UInt32 updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, UInt32 descriptor, const IVulkanSampler& sampler)
    {
        auto binding = descriptorLayout.binding();

        if (descriptor >= parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor array at binding {1} of descriptor set {0} does only contain {2} descriptors, but the descriptor {3} has been specified for binding.", m_layout->space(), binding, parent.capacity(binding), descriptor);

        if (descriptorLayout.descriptorType() != DescriptorType::Sampler) [[unlikely]]
            throw InvalidArgumentException("descriptorLayout", "Invalid descriptor type. The binding {0} does not bind a sampler, but rather a {1}.", binding, descriptorLayout.descriptorType());

        if (descriptorLayout.staticSampler() != nullptr) [[unlikely]]
            throw InvalidArgumentException("descriptorLayout", "The provided binding does bind a sampler, but also defines a static sampler, which makes binding to it invalid.");

        auto& device = m_layout->device();
        auto descriptorOffset = m_layout->getDescriptorOffset(binding, descriptor);

        writeSamplerDescriptor(device, sampler, this->targetAddressRange(DescriptorHeapType::Sampler, descriptorOffset));
        device.updateGlobalDescriptors(parent, binding, descriptor, 1u);

        return static_cast<UInt32>(parent.globalHeapAllocation(DescriptorHeapType::Sampler).Allocation.Offset + descriptorOffset);
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
    
    // NOLINTBEGIN(performance-move-const-arg)
    device.releaseGlobalDescriptors(std::move(m_impl->m_resourceAllocation));
    device.releaseGlobalDescriptors(std::move(m_impl->m_samplerAllocation));
    // NOLINTEND(performance-move-const-arg)
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
    auto addressRange = m_impl->targetAddressRange(DescriptorHeapType::Resource, layout->getDescriptorOffset(binding, descriptor));
    writeAccelerationStructureDescriptor(device, accelerationStructure, addressRange);

    // Update the invalidated range on the global descriptor heap.
    device.updateGlobalDescriptors(*this, binding, descriptor, 1u);
}