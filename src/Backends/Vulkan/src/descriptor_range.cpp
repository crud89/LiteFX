#include <litefx/backends/vulkan.hpp>
#include "descriptors.hpp"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------
class VulkanDescriptorRange::VulkanDescriptorRangeImpl {
public:
    friend class VulkanDescriptorRange;

private:
    SharedPtr<const VulkanDevice> m_device;
    DescriptorHeapAllocation m_allocation;
    UInt32 m_size, m_stride;
    mutable Array<Byte> m_staging;

public:
    VulkanDescriptorRangeImpl(const VulkanDevice& device, DescriptorHeapAllocation&& allocation) :
        m_device(device.shared_from_this()), m_allocation(std::move(allocation)), // NOLINT(performance-move-const-arg)
        m_size(static_cast<UInt32>(m_allocation.Allocation.Size)), m_stride(device.descriptorStride(m_allocation.Heap))
    {
        try
        {
            m_staging.resize(static_cast<size_t>(m_allocation.Allocation.Size) * m_stride, 0_b);
        }
        catch (...)
        {
            m_device->releaseGlobalDescriptors(std::move(m_allocation)); // NOLINT(performance-move-const-arg)
            throw;
        }
    }

    VulkanDescriptorRangeImpl(const VulkanDescriptorRangeImpl&) = delete;
    VulkanDescriptorRangeImpl(VulkanDescriptorRangeImpl&&) noexcept = default;
    VulkanDescriptorRangeImpl& operator=(const VulkanDescriptorRangeImpl&) = delete;
    VulkanDescriptorRangeImpl& operator=(VulkanDescriptorRangeImpl&&) noexcept = default;

    ~VulkanDescriptorRangeImpl() noexcept
    {
        m_device->releaseGlobalDescriptors(std::move(m_allocation)); // NOLINT(performance-move-const-arg)
    }

    VkHostAddressRangeEXT targetAddressRange(UInt32 element) const noexcept
    {
        auto descriptor = Span<Byte>{ m_staging }.subspan(static_cast<size_t>(element) * m_stride, m_stride);
        
        return { .address = descriptor.data(), .size = descriptor.size() };
    }

    void flush(UInt32 element, UInt32 count) const
    {
        auto source = std::span{ m_staging }.subspan(static_cast<size_t>(element) * m_stride, static_cast<size_t>(count) * m_stride);
        m_device->writeGlobalDescriptorHeap(m_allocation.Heap, static_cast<UInt32>(m_allocation.Allocation.Offset) + element, source);
    }

    void validate(UInt32 element, UInt32 count, DescriptorHeapType expected) const
    {
        if (m_allocation.Heap != expected) [[unlikely]]
            throw InvalidArgumentException("element", "Cannot write a descriptor for the {0} heap into a range allocated from the {1} heap.", expected, m_allocation.Heap);

        if (count > m_size - element) [[unlikely]]
            throw ArgumentOutOfRangeException("element", "The descriptor range contains {0} descriptors, but {1} descriptors starting at {2} were requested.", m_size, count, element);
    }

    void updateBinding(UInt32 element, DescriptorType bindingType, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 elementsPerDescriptor, Format texelFormat, UInt32 stride) const
    {
        auto [descriptors, rangeSize] = buffer.bufferDescriptorRange(bufferElement, elements, elementsPerDescriptor);
        validate(element, descriptors, DescriptorHeapType::Resource);

        auto targets = std::views::iota(element, element + descriptors)
            | std::views::transform([this](UInt32 e) { return targetAddressRange(e); })
            | std::ranges::to<Array<VkHostAddressRangeEXT>>();

        writeBufferDescriptors(*m_device, bindingType, buffer, bufferElement, rangeSize, texelFormat, stride, targets);
        flush(element, descriptors);
    }

    void updateBinding(UInt32 element, DescriptorType bindingType, const IVulkanImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
    {
        validate(element, 1u, DescriptorHeapType::Resource);

        writeImageDescriptor(*m_device, bindingType, image, firstLevel, levels, firstLayer, layers, targetAddressRange(element));
        flush(element, 1u);
    }

    void updateBinding(UInt32 element, const IVulkanSampler& sampler) const
    {
        validate(element, 1u, DescriptorHeapType::Sampler);

        writeSamplerDescriptor(*m_device, sampler, targetAddressRange(element));
        flush(element, 1u);
    }

    void updateBinding(UInt32 element, const IVulkanAccelerationStructure& accelerationStructure) const
    {
        validate(element, 1u, DescriptorHeapType::Resource);

        writeAccelerationStructureDescriptor(*m_device, accelerationStructure, targetAddressRange(element));
        flush(element, 1u);
    }

    void updateBinding(DescriptorType type, const DescriptorBinding& binding) 
    {
        std::visit(type_switch {
            [](const std::monostate&) {}, // Default: don't bind anything. Can be used to skip a descriptor in a set of bindings.
            [&](const IBuffer& buffer) { this->updateBinding(binding.firstDescriptor, type, dynamic_cast<const IVulkanBuffer&>(buffer), binding.firstElement, binding.elements, binding.elementsPerDescriptor, binding.texelFormat, binding.stride); },
            [&](const IImage& image) { this->updateBinding(binding.firstDescriptor, type, dynamic_cast<const IVulkanImage&>(image), binding.firstLevel, binding.levels, binding.firstElement, binding.elements); },
            [&](const ISampler& sampler) { this->updateBinding(binding.firstDescriptor, dynamic_cast<const IVulkanSampler&>(sampler)); },
            [&](const IAccelerationStructure& accelerationStructure) { this->updateBinding(binding.firstDescriptor, dynamic_cast<const IVulkanAccelerationStructure&>(accelerationStructure)); }
        }, binding.resource);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorRange::VulkanDescriptorRange(const VulkanDevice& device, DescriptorHeapAllocation&& allocation) :
    m_impl(device, std::move(allocation)) // NOLINT(performance-move-const-arg)
{
}

DescriptorHeapType VulkanDescriptorRange::heap() const noexcept
{
    return m_impl->m_allocation.Heap;
}

UInt32 VulkanDescriptorRange::size() const noexcept
{
    return m_impl->m_size;
}

UInt32 VulkanDescriptorRange::globalIndex(UInt32 element) const
{
    m_impl->validate(element, 1u, this->heap());

    return static_cast<UInt32>(m_impl->m_allocation.Allocation.Offset) + element;
}

void VulkanDescriptorRange::update(DescriptorType type, const DescriptorBinding& binding) const
{
    m_impl->updateBinding(type, binding);
}