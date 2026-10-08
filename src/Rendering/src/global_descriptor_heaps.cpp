#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class GlobalDescriptorHeaps::GlobalDescriptorHeapsImpl {
public:
    friend class GlobalDescriptorHeaps;

private:
    struct Heap {
        UniquePtr<VirtualAllocator> Allocator;
        mutable std::mutex Mutex{};
    };

    Heap m_resources, m_samplers;

public:
    GlobalDescriptorHeapsImpl(UniquePtr<VirtualAllocator>&& resourceAllocator, UniquePtr<VirtualAllocator>&& samplerAllocator) :
        m_resources{ std::move(resourceAllocator) }, m_samplers{ std::move(samplerAllocator) }
    {
        if (m_resources.Allocator == nullptr) [[unlikely]]
            throw ArgumentNotInitializedException("resourceAllocator", "The resource heap allocator must be initialized.");

        if (m_samplers.Allocator == nullptr) [[unlikely]]
            throw ArgumentNotInitializedException("samplerAllocator", "The sampler heap allocator must be initialized.");
    }

public:
    const Heap& heap(DescriptorHeapType type) const
    {
        switch (type)
        {
        case DescriptorHeapType::Resource:
            return m_resources;
        case DescriptorHeapType::Sampler:
            return m_samplers;
        default: [[unlikely]]
            throw InvalidArgumentException("heap", "The descriptor heap type must be one of the following: {{ `Resource`, `Sampler` }}, but it was: `{0}`.", type);
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

GlobalDescriptorHeaps::GlobalDescriptorHeaps(UniquePtr<VirtualAllocator>&& resourceAllocator, UniquePtr<VirtualAllocator>&& samplerAllocator) :
    m_impl(std::move(resourceAllocator), std::move(samplerAllocator))
{
}

DescriptorHeapAllocation GlobalDescriptorHeaps::allocate(DescriptorHeapType heap, UInt32 descriptors)
{
    if (descriptors == 0u) [[unlikely]]
        throw ArgumentOutOfRangeException("descriptors", "Cannot allocate an empty range of descriptors.");

    auto& target = m_impl->heap(heap);
    std::lock_guard<std::mutex> lock(target.Mutex);

    auto allocation = target.Allocator->tryAllocate(descriptors, 1u, AllocationStrategy::OptimizeTime);

    if (!allocation.has_value()) [[unlikely]]
        throw RuntimeException("Unable to allocate {0} descriptors on the global {1} heap (capacity: {2} descriptors). Consider increasing the heap size.", descriptors, heap, target.Allocator->size());

    return { .Heap = heap, .Allocation = *allocation };
}

void GlobalDescriptorHeaps::release(DescriptorHeapAllocation&& allocation)
{
    if (allocation.Allocation.Offset == std::numeric_limits<UInt64>::max())
        return;

    auto& target = m_impl->heap(allocation.Heap);

    std::lock_guard<std::mutex> lock(target.Mutex);
    target.Allocator->free(std::move(allocation.Allocation));
}

UInt32 GlobalDescriptorHeaps::capacity(DescriptorHeapType heap) const
{
    return static_cast<UInt32>(m_impl->heap(heap).Allocator->size());
}