#include <litefx/backends/dx12.hpp>
#include "descriptors.hpp"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorRange::DirectX12DescriptorRangeImpl {
public:
    friend class DirectX12DescriptorRange;

private:
    SharedPtr<const DirectX12Device> m_device;
    DescriptorHeapAllocation m_allocation;
    ComPtr<ID3D12DescriptorHeap> m_staging;
    UInt32 m_size, m_increment;

public:
    DirectX12DescriptorRangeImpl(const DirectX12Device& device, DescriptorHeapAllocation&& allocation) :
        m_device(device.shared_from_this()), m_allocation(std::move(allocation)), m_size(static_cast<UInt32>(m_allocation.Allocation.Size)) // NOLINT(performance-move-const-arg)
    {
        auto heapType = m_allocation.Heap == DescriptorHeapType::Sampler ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        D3D12_DESCRIPTOR_HEAP_DESC stagingDesc {
            .Type = heapType,
            .NumDescriptors = static_cast<UInt32>(m_allocation.Allocation.Size),
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        };

        try
        {
            raiseIfFailed(device.handle()->CreateDescriptorHeap(&stagingDesc, IID_PPV_ARGS(&m_staging)), "Unable to create staging descriptor heap for descriptor range.");
        }
        catch (...)
        {
            device.releaseGlobalDescriptors(std::move(m_allocation)); // NOLINT(performance-move-const-arg)
            throw;
        }

        m_increment = device.handle()->GetDescriptorHandleIncrementSize(heapType);
    }

    DirectX12DescriptorRangeImpl(const DirectX12DescriptorRangeImpl&) = delete;
    DirectX12DescriptorRangeImpl(DirectX12DescriptorRangeImpl&&) noexcept = default;
    DirectX12DescriptorRangeImpl& operator=(const DirectX12DescriptorRangeImpl&) = delete;
    DirectX12DescriptorRangeImpl& operator=(DirectX12DescriptorRangeImpl&&) noexcept = default;

    ~DirectX12DescriptorRangeImpl() noexcept
    {
        m_device->releaseGlobalDescriptors(std::move(m_allocation)); // NOLINT(performance-move-const-arg)
    }

public:
    void validate(UInt32 element, UInt32 count, DescriptorHeapType heap) const
    {
        if (m_allocation.Heap != heap) [[unlikely]]
            throw InvalidArgumentException("element", "Cannot write a descriptor for the {0} heap into a range allocated from the {1} heap.", heap, m_allocation.Heap);

        if (element >= m_size || count > m_size - element) [[unlikely]]
            throw ArgumentOutOfRangeException("element", "The descriptor range contains {0} descriptors, but {1} descriptors starting at {2} were requested.", m_size, count, element);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE target(UInt32 element) const noexcept
    {
        return { m_staging->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(element), m_increment };
    }

    void flush(UInt32 element, UInt32 count) const
    {
        m_device->copyGlobalDescriptorHeap(m_allocation.Heap, static_cast<UInt32>(m_allocation.Allocation.Offset) + element, this->target(element), count);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorRange::DirectX12DescriptorRange(const DirectX12Device& device, DescriptorHeapAllocation&& allocation) :
    m_impl(device, std::move(allocation)) // NOLINT(performance-move-const-arg)
{
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectX12DescriptorRange::cpuHandle(UInt32 element) const
{
    if (element >= m_impl->m_size) [[unlikely]]
        throw ArgumentOutOfRangeException("element", "The descriptor range contains {0} descriptors, but element {1} was requested.", m_impl->m_size, element);

    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_impl->m_device->globalBufferHeap()->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(m_impl->m_allocation.Allocation.Offset), m_impl->m_increment);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectX12DescriptorRange::gpuHandle(UInt32 element) const
{
    if (element >= m_impl->m_size) [[unlikely]]
        throw ArgumentOutOfRangeException("element", "The descriptor range contains {0} descriptors, but element {1} was requested.", m_impl->m_size, element);

    return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_impl->m_device->globalBufferHeap()->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(m_impl->m_allocation.Allocation.Offset), m_impl->m_increment);
}

DescriptorHeapType DirectX12DescriptorRange::heap() const noexcept
{
    return m_impl->m_allocation.Heap;
}

UInt32 DirectX12DescriptorRange::size() const noexcept
{
    return m_impl->m_size;
}

UInt32 DirectX12DescriptorRange::globalIndex(UInt32 element) const
{
    if (element >= m_impl->m_size) [[unlikely]]
        throw ArgumentOutOfRangeException("element", "The descriptor range contains {0} descriptors, but element {1} was requested.", m_impl->m_size, element);

    return static_cast<UInt32>(m_impl->m_allocation.Allocation.Offset) + element;
}

void DirectX12DescriptorRange::update(UInt32 element, DescriptorType bindingType, const IDirectX12Buffer& buffer, UInt32 bufferElement, UInt32 elements, Format texelFormat, UInt32 stride) const
{
    auto count = bufferElements(buffer, bufferElement, elements);
    m_impl->validate(element, count, DescriptorHeapType::Resource);

    writeBufferDescriptors(*m_impl->m_device, bindingType, buffer, bufferElement, count, texelFormat, stride, m_impl->target(element));
    m_impl->flush(element, count);
}

void DirectX12DescriptorRange::update(UInt32 element, DescriptorType bindingType, const IDirectX12Image& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const
{
    m_impl->validate(element, 1u, DescriptorHeapType::Resource);

    writeImageDescriptor(*m_impl->m_device, bindingType, image, firstLevel, levels, firstLayer, layers, m_impl->target(element));
    m_impl->flush(element, 1u);
}

void DirectX12DescriptorRange::update(UInt32 element, const IDirectX12Sampler& sampler) const
{
    m_impl->validate(element, 1u, DescriptorHeapType::Sampler);

    writeSamplerDescriptor(*m_impl->m_device, sampler, m_impl->target(element));
    m_impl->flush(element, 1u);
}

void DirectX12DescriptorRange::update(UInt32 element, const IDirectX12AccelerationStructure& accelerationStructure) const
{
    m_impl->validate(element, 1u, DescriptorHeapType::Resource);

    writeAccelerationStructureDescriptor(*m_impl->m_device, accelerationStructure, m_impl->target(element));
    m_impl->flush(element, 1u);
}