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

    void updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, UInt32 firstDescriptor, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 elementsPerDescriptor, Format texelFormat, UInt32 stride)
    {
        auto binding = descriptorLayout.binding();
        auto [descriptors, rangeSize] = buffer.bufferDescriptorRange(bufferElement, elements, elementsPerDescriptor);

        if (firstDescriptor + descriptors > parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("binding", "The descriptor array at binding {0} only has {1} descriptors, however there are {2} descriptors starting at descriptor {3} specified.", binding, parent.capacity(binding), descriptors, firstDescriptor);

        auto targets = std::views::iota(firstDescriptor, firstDescriptor + descriptors)
            | std::views::transform([&](UInt32 descriptor) { return this->targetAddressRange(DescriptorHeapType::Resource, m_layout->getDescriptorOffset(binding, descriptor)); })
            | std::ranges::to<Array<VkHostAddressRangeEXT>>();

        auto& device = m_layout->device();
        writeBufferDescriptors(device, descriptorLayout.descriptorType(), buffer, bufferElement, rangeSize, texelFormat, stride, targets);
        device.updateGlobalDescriptors(parent, binding, firstDescriptor, descriptors);
    }

    void updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, UInt32 descriptor, const IVulkanImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers)
    {
        auto binding = descriptorLayout.binding();

        if (descriptor >= parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor index {0} was out of bounds. The descriptor array at binding {1} only contains {2} descriptors.", descriptor, binding, parent.capacity(binding));

        auto& device = m_layout->device();
        auto descriptorOffset = m_layout->getDescriptorOffset(binding, descriptor);
        writeImageDescriptor(device, descriptorLayout.descriptorType(), image, firstLevel, levels, firstLayer, layers, this->targetAddressRange(DescriptorHeapType::Resource, descriptorOffset));
        device.updateGlobalDescriptors(parent, binding, descriptor, 1u);
    }

    void updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, UInt32 descriptor, const IVulkanSampler& sampler)
    {
        sampler.validateBinding(descriptorLayout.descriptorType());
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
    }

    void updateBinding(const VulkanDescriptorSet& parent, const VulkanDescriptorLayout& descriptorLayout, UInt32 descriptor, const IVulkanAccelerationStructure& accelerationStructure)
    {
        auto binding = descriptorLayout.binding();

        if (descriptor >= parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor array at binding {1} of descriptor set {0} does only contain {2} descriptors, but the descriptor {3} has been specified for binding.", m_layout->space(), binding, parent.capacity(binding), descriptor);

        if (descriptorLayout.descriptorType() != DescriptorType::AccelerationStructure) [[unlikely]]
            throw InvalidArgumentException("descriptorLayout", "Invalid descriptor type. The binding {0} does not point to an acceleration structure descriptor, but rather a {1}.", binding, descriptorLayout.descriptorType());

        auto& device = m_layout->device();
        auto descriptorOffset = m_layout->getDescriptorOffset(binding, descriptor);

        // Setup the descriptor info.
        auto addressRange = targetAddressRange(DescriptorHeapType::Resource, descriptorOffset);
        writeAccelerationStructureDescriptor(device, accelerationStructure, addressRange);

        // Update the invalidated range on the global descriptor heap.
        device.updateGlobalDescriptors(parent, binding, descriptor, 1u);
    }

    void updateBinding(const VulkanDescriptorSet& parent, UInt32 index, const DescriptorBinding& binding)
    {
        // Find the descriptor.
        auto& descriptors = m_layout->descriptors();
        auto bindingPoint = binding.binding.value_or(index);
        auto descriptorLayout = std::ranges::find_if(descriptors, [bindingPoint](auto& layout) { return layout.binding() == bindingPoint; });

        if (descriptorLayout == descriptors.end()) [[unlikely]]
            throw InvalidArgumentException("binding", "The descriptor set {0} does not contain a descriptor at binding {1}.", m_layout->space(), bindingPoint);

        std::visit(type_switch {
            [](const std::monostate&) {}, // Default: don't bind anything. Can be used to skip a descriptor in a set of bindings.
            [&](const IBuffer& buffer) { this->updateBinding(parent, *descriptorLayout, binding.firstDescriptor, dynamic_cast<const IVulkanBuffer&>(buffer), binding.firstElement, binding.elements, binding.elementsPerDescriptor, binding.texelFormat, binding.stride); },
            [&](const IImage& image) { this->updateBinding(parent, *descriptorLayout, binding.firstDescriptor, dynamic_cast<const IVulkanImage&>(image), binding.firstLevel, binding.levels, binding.firstElement, binding.elements); },
            [&](const ISampler& sampler) { this->updateBinding(parent, *descriptorLayout, binding.firstDescriptor, dynamic_cast<const IVulkanSampler&>(sampler)); },
            [&](const IAccelerationStructure& accelerationStructure) { this->updateBinding(parent, *descriptorLayout, binding.firstDescriptor, dynamic_cast<const IVulkanAccelerationStructure&>(accelerationStructure)); }
        }, binding.resource);
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

void VulkanDescriptorSet::update(UInt32 index, const DescriptorBinding& binding) const
{
    m_impl->updateBinding(*this, index, binding);
}