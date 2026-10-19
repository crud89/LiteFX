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
        m_device(device.shared_from_this()), m_allocation(std::move(allocation)),
        m_size(static_cast<UInt32>(m_allocation.Allocation.Size)), m_stride(device.descriptorStride(m_allocation.Heap))
    {
        try
        {
            m_staging.resize(static_cast<size_t>(m_allocation.Allocation.Size) * m_stride, 0_b);
        }
        catch (...)
        {
            m_device->releaseGlobalDescriptors(std::move(m_allocation));
            throw;
        }
    }

    ~VulkanDescriptorRangeImpl() noexcept
    {
        m_device->releaseGlobalDescriptors(std::move(m_allocation));
    }

    VkHostAddressRangeEXT target(UInt32 element) const noexcept
    {
        return { .address = std::next(m_staging.data(), static_cast<size_t>(element) * m_stride), .size = m_stride };
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
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorRange::VulkanDescriptorRange(const VulkanDevice& device, DescriptorHeapAllocation&& allocation) :
    m_impl(device, std::move(allocation))
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

void VulkanDescriptorRange::update(UInt32 element, DescriptorType bindingType, const IVulkanBuffer& buffer, UInt32 bufferElement, UInt32 elements, Format texelFormat) const 
{
    auto count = bufferElements(buffer, bufferElement, elements);
    m_impl->validate(element, count, DescriptorHeapType::Resource);

    auto targets = std::views::iota(element, element + count) 
        | std::views::transform([this](UInt32 e) { return m_impl->target(e); }) 
        | std::ranges::to<Array<VkHostAddressRangeEXT>>();

    writeBufferDescriptors(*m_impl->m_device, bindingType, buffer, bufferElement, texelFormat, targets);
    m_impl->flush(element, count);
}

void VulkanDescriptorRange::update(UInt32 element, DescriptorType bindingType, const IVulkanImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
{
    m_impl->validate(element, 1u, DescriptorHeapType::Resource);

    writeImageDescriptor(*m_impl->m_device, bindingType, image, firstLevel, levels, firstLayer, layers, m_impl->target(element));
    m_impl->flush(element, 1u);
}

void VulkanDescriptorRange::update(UInt32 element, const IVulkanSampler& sampler) const 
{
    m_impl->validate(element, 1u, DescriptorHeapType::Sampler);

    writeSamplerDescriptor(*m_impl->m_device, sampler, m_impl->target(element));
    m_impl->flush(element, 1u);
}

void VulkanDescriptorRange::update(UInt32 element, const IVulkanAccelerationStructure& accelerationStructure) const 
{
    m_impl->validate(element, 1u, DescriptorHeapType::Resource);

    writeAccelerationStructureDescriptor(*m_impl->m_device, accelerationStructure, m_impl->target(element));
    m_impl->flush(element, 1u);
}
