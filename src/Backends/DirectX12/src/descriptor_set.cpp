#include <litefx/backends/dx12.hpp>
#include "descriptors.hpp"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12DescriptorSet::DirectX12DescriptorSetImpl {
public:
    friend class DirectX12DescriptorSet;

private:
    struct LocalHeap {
        ComPtr<ID3D12DescriptorHeap> Heap{};
        DescriptorHeapAllocation Allocation;
    } m_resourceHeap{}, m_samplerHeap{};

    SharedPtr<const DirectX12DescriptorSetLayout> m_layout;
    UInt32 m_unboundedArrayCapacity;

public:
    DirectX12DescriptorSetImpl(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& resourceHeap, ComPtr<ID3D12DescriptorHeap>&& samplerHeap, UInt32 unboundedArrayCapacity) :
        m_resourceHeap{ .Heap = std::move(resourceHeap) }, m_samplerHeap{ .Heap = std::move(samplerHeap) }, m_layout(layout.shared_from_this()), m_unboundedArrayCapacity(unboundedArrayCapacity)
    {
        if (m_layout->bindsResources() && m_resourceHeap.Heap == nullptr) [[unlikely]]
            throw ArgumentNotInitializedException("resourceHeap", "The local resource heap must be initialized, if the descriptor set binds resources.");

        if (m_layout->bindsSamplers() && m_samplerHeap.Heap == nullptr) [[unlikely]]
            throw ArgumentNotInitializedException("samplerHeap", "The local sampler heap must be initialized, if the descriptor set binds samplers.");
    }

public:
    inline DescriptorHeapAllocation globalHeapAllocation(DescriptorHeapType heapType) const
    {
        switch (heapType)
        {
        case DescriptorHeapType::Resource:
            return m_resourceHeap.Allocation;
        case DescriptorHeapType::Sampler:
            return m_samplerHeap.Allocation;
        default:
            throw InvalidArgumentException("heapType", "The heap type must refer to a valid descriptor heap.");
        }
    }

    inline CD3DX12_CPU_DESCRIPTOR_HANDLE localHandle(DescriptorHeapType heapType, UInt32 slot) const
    {
        bool samplers = heapType == DescriptorHeapType::Sampler;
        auto& heap = samplers ? m_samplerHeap.Heap : m_resourceHeap.Heap;
        auto increment = m_layout->device()->handle()->GetDescriptorHandleIncrementSize(samplers ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        return { heap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(slot), increment };
    }

    void updateBinding(const DirectX12DescriptorSet& parent, const DirectX12DescriptorLayout& descriptorLayout, UInt32 firstDescriptor, const IDirectX12Buffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 elementsPerDescriptor, Format texelFormat, UInt32 stride)
    {
        auto binding = descriptorLayout.binding();
        auto [descriptors, rangeSize] = buffer.bufferDescriptorRange(bufferElement, elements, elementsPerDescriptor);

        if (firstDescriptor + descriptors > parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("binding", "The descriptor array at binding {0} only has {1} descriptors, however there are {2} descriptors starting at descriptor {3} specified.", binding, parent.capacity(binding), descriptors, firstDescriptor);

        // The reflected element size of a structured buffer is the shader's structure stride. A mismatch means that the shader reads the buffer with a different layout.
        stride = stride > 0 ? stride : static_cast<UInt32>(descriptorLayout.elementSize() > 0 ? descriptorLayout.elementSize() : buffer.elementSize());

        if ((descriptorLayout.descriptorType() == DescriptorType::StructuredBuffer || descriptorLayout.descriptorType() == DescriptorType::RWStructuredBuffer) &&
            descriptorLayout.elementSize() > 0 && descriptorLayout.elementSize() != stride) [[unlikely]]
            throw InvalidArgumentException("structureStride", "The shader expects a structure stride of {0} bytes at binding {1}, but the buffer is bound with a stride of {2} bytes.", descriptorLayout.elementSize(), descriptorLayout.binding(), stride);

        auto device = m_layout->device();
        auto offset = m_layout->getDescriptorOffset(binding, firstDescriptor);

        writeBufferDescriptors(*device, descriptorLayout.descriptorType(), buffer, bufferElement, descriptors, rangeSize, texelFormat, stride, this->localHandle(DescriptorHeapType::Resource, offset));
        device->updateGlobalDescriptors(parent, binding, firstDescriptor, descriptors);
    }

    void updateBinding(const DirectX12DescriptorSet& parent, const DirectX12DescriptorLayout& descriptorLayout, UInt32 descriptor, const IDirectX12Image& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers)
    {
        auto binding = descriptorLayout.binding();

        if (descriptor >= parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor index {0} was out of bounds. The descriptor array at binding {1} only contains {2} descriptors.", descriptor, binding, parent.capacity(binding));

        auto device = m_layout->device();
        auto offset = m_layout->getDescriptorOffset(binding, descriptor);

        writeImageDescriptor(*device, descriptorLayout.descriptorType(), image, firstLevel, levels, firstLayer, layers, this->localHandle(DescriptorHeapType::Resource, offset));
        device->updateGlobalDescriptors(parent, binding, descriptor, 1u);
    }

    void updateBinding(const DirectX12DescriptorSet& parent, const DirectX12DescriptorLayout& descriptorLayout, UInt32 descriptor, const IDirectX12Sampler& sampler)
    {
        sampler.validateBinding(descriptorLayout.descriptorType());
        auto binding = descriptorLayout.binding();

        if (descriptor >= parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor array at binding {1} of descriptor set {0} does only contain {2} descriptors, but the descriptor {3} has been specified for binding.", m_layout->space(), binding, parent.capacity(binding), descriptor);

        if (descriptorLayout.descriptorType() != DescriptorType::Sampler) [[unlikely]]
            throw InvalidArgumentException("descriptorLayout", "Invalid descriptor type. The binding {0} does not bind a sampler, but rather a {1}.", binding, descriptorLayout.descriptorType());

        if (descriptorLayout.staticSampler() != nullptr) [[unlikely]]
            throw InvalidArgumentException("descriptorLayout", "The provided binding does bind a sampler, but also defines a static sampler, which makes binding to it invalid.");

        auto device = m_layout->device();
        auto offset = m_layout->getDescriptorOffset(binding, descriptor);

        writeSamplerDescriptor(*device, sampler, this->localHandle(DescriptorHeapType::Sampler, offset));
        device->updateGlobalDescriptors(parent, binding, descriptor, 1u);
    }

    void updateBinding(const DirectX12DescriptorSet& parent, const DirectX12DescriptorLayout& descriptorLayout, UInt32 descriptor, const IDirectX12AccelerationStructure& accelerationStructure)
    {
        auto binding = descriptorLayout.binding();

        if (descriptor >= parent.capacity(binding)) [[unlikely]]
            throw InvalidArgumentException("descriptor", "The descriptor array at binding {1} of descriptor set {0} does only contain {2} descriptors, but the descriptor {3} has been specified for binding.", m_layout->space(), binding, parent.capacity(binding), descriptor);

        if (descriptorLayout.descriptorType() != DescriptorType::AccelerationStructure) [[unlikely]]
            throw InvalidArgumentException("descriptorLayout", "Invalid descriptor type. The binding {0} does not point to an acceleration structure descriptor, but rather a {1}.", binding, descriptorLayout.descriptorType());

        auto device = m_layout->device();
        auto offset = m_layout->getDescriptorOffset(binding, descriptor);

        writeAccelerationStructureDescriptor(*device, accelerationStructure, localHandle(DescriptorHeapType::Resource, offset));
        device->updateGlobalDescriptors(parent, binding, descriptor, 1u);
    }

    void updateBinding(const DirectX12DescriptorSet& parent, UInt32 index, const DescriptorBinding& binding) 
    {
        // Find the descriptor.
        auto& descriptors = m_layout->descriptors();
        auto bindingPoint = binding.binding.value_or(index);
        auto descriptorLayout = std::ranges::find_if(descriptors, [bindingPoint](auto& layout) { return layout.binding() == bindingPoint; });

        if (descriptorLayout == descriptors.end()) [[unlikely]]
            throw InvalidArgumentException("binding", "The descriptor set {0} does not contain a descriptor at binding {1}.", m_layout->space(), bindingPoint);

        std::visit(type_switch {
            [](const std::monostate&) {}, // Default: don't bind anything. Can be used to skip a descriptor in a set of bindings.
            [&](const IBuffer& buffer) { this->updateBinding(parent, *descriptorLayout, binding.firstDescriptor, dynamic_cast<const IDirectX12Buffer&>(buffer), binding.firstElement, binding.elements, binding.elementsPerDescriptor, binding.texelFormat, binding.stride); },
            [&](const IImage& image) { this->updateBinding(parent, *descriptorLayout, binding.firstDescriptor, dynamic_cast<const IDirectX12Image&>(image), binding.firstLevel, binding.levels, binding.firstElement, binding.elements); },
            [&](const ISampler& sampler) { this->updateBinding(parent, *descriptorLayout, binding.firstDescriptor, dynamic_cast<const IDirectX12Sampler&>(sampler)); },
            [&](const IAccelerationStructure& accelerationStructure) { this->updateBinding(parent, *descriptorLayout, binding.firstDescriptor, dynamic_cast<const IDirectX12AccelerationStructure&>(accelerationStructure)); }
        }, binding.resource);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12DescriptorSet::DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& resourceHeap, ComPtr<ID3D12DescriptorHeap>&& samplerHeap, UInt32 unboundedArrayCapacity) :
    m_impl(layout, std::move(resourceHeap), std::move(samplerHeap), unboundedArrayCapacity)
{
    if (layout.bindsResources())
        m_impl->m_resourceHeap.Allocation = layout.device()->allocateGlobalDescriptors(*this, DescriptorHeapType::Resource);

    if (layout.bindsSamplers())
        m_impl->m_samplerHeap.Allocation = layout.device()->allocateGlobalDescriptors(*this, DescriptorHeapType::Sampler);
}

DirectX12DescriptorSet::~DirectX12DescriptorSet() noexcept
{
    auto device = m_impl->m_layout->device();

    // NOLINTBEGIN(performance-move-const-arg)
    device->releaseGlobalDescriptors(std::move(m_impl->m_resourceHeap.Allocation));
    device->releaseGlobalDescriptors(std::move(m_impl->m_samplerHeap.Allocation));
    // NOLINTEND(performance-move-const-arg)
    m_impl->m_layout->free(*this);
}

const DirectX12DescriptorSetLayout& DirectX12DescriptorSet::layout() const noexcept
{
    return *m_impl->m_layout;
}

UInt32 DirectX12DescriptorSet::capacity(UInt32 binding) const noexcept
{
    auto range = m_impl->m_layout->allocationLayout().range(binding);

    if (!range.has_value())
        return 0u;

    return range->Unbounded ? m_impl->m_unboundedArrayCapacity : range->DescriptorCount;
}

DescriptorHeapAllocation DirectX12DescriptorSet::globalHeapAllocation(DescriptorHeapType heapType) const
{
    return m_impl->globalHeapAllocation(heapType);
}

void DirectX12DescriptorSet::update(UInt32 index, const DescriptorBinding& binding) const
{
    m_impl->updateBinding(*this, index, binding);
}

const ComPtr<ID3D12DescriptorHeap> DirectX12DescriptorSet::localHeap(DescriptorHeapType heapType) const noexcept
{
    switch (heapType)
    {
    case DescriptorHeapType::Resource:
        return m_impl->m_resourceHeap.Heap;
    case DescriptorHeapType::Sampler:
        return m_impl->m_samplerHeap.Heap;
    default:
        return nullptr;
    }
}