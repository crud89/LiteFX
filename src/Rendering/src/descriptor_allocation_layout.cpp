#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DescriptorAllocationLayout::DescriptorAllocationLayoutImpl {
public:
    friend class DescriptorAllocationLayout;

private:
    Array<DescriptorAllocationRange> m_ranges{};
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DescriptorAllocationLayout::DescriptorAllocationLayout(Enumerable<const IDescriptorLayout&> descriptors) :
    m_impl()
{
    // Transform the descriptor layouts into ranges.
    auto getHeap = [](DescriptorType type) static noexcept { return type == DescriptorType::Sampler ? DescriptorHeapType::Sampler : DescriptorHeapType::Resource; };

    m_impl->m_ranges = descriptors
        | std::views::filter([](const IDescriptorLayout& descriptor) { return descriptor.staticSampler() == nullptr; })
        | std::views::transform([&](const IDescriptorLayout& descriptor) -> DescriptorAllocationRange { 
            return { getHeap(descriptor.descriptorType()), descriptor.binding(), 0u, descriptor.unbounded() ? 0u : descriptor.descriptors(), descriptor.unbounded() }; })
        | std::ranges::to<Array<DescriptorAllocationRange>>();

    // Sort the ranges by binding.
    std::ranges::sort(m_impl->m_ranges, {}, &DescriptorAllocationRange::Binding);

    // Reject duplicate bindings.
    if (auto match = std::ranges::adjacent_find(m_impl->m_ranges, {}, &DescriptorAllocationRange::Binding); match != m_impl->m_ranges.end()) [[unlikely]]
        throw InvalidArgumentException("descriptors", "The provided descriptors contain a duplicate binding at {}.", match->Binding);

    // Compute the offsets.
    UInt32 samplerOffset{ 0u }, resourceOffset{ 0u };

    for (auto& slot : m_impl->m_ranges) {
        if (slot.Heap == DescriptorHeapType::Resource) {
            slot.RelativeOffset = resourceOffset;
            resourceOffset += slot.DescriptorCount;
        }
        else if (slot.Heap == DescriptorHeapType::Sampler) {
            slot.RelativeOffset = samplerOffset;
            samplerOffset += slot.DescriptorCount;
        }
        else {
            std::unreachable();
        }
    }

    // Validate if the unbounded array is the last element (if it exists). This must be true even for mixed descriptor sets, i.e., no sampler goes behind an unbounded resource array. This is directly enforced
    // by the Vulkan spec: https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBindingFlagBits.html#_description.
    if (std::ranges::any_of(m_impl->m_ranges | std::views::reverse | std::views::drop(1), [](const DescriptorAllocationRange& slot) { return slot.Unbounded; }))
        throw InvalidArgumentException("descriptors", "The provided descriptors contain an unbounded array, that is not bound to the last binding in the descriptor set.");
}

const Array<DescriptorAllocationRange>& DescriptorAllocationLayout::ranges() const noexcept
{
    return m_impl->m_ranges;
}

Optional<DescriptorAllocationRange> DescriptorAllocationLayout::range(UInt32 binding) const noexcept
{
    if (auto match = std::ranges::lower_bound(m_impl->m_ranges, binding, {}, &DescriptorAllocationRange::Binding); match != m_impl->m_ranges.end() && match->Binding == binding)
        return *match;

    return std::nullopt;
}

UInt32 DescriptorAllocationLayout::descriptorCount(DescriptorHeapType heap, UInt32 unboundedArrayElements) const noexcept
{
    auto unboundedArray = this->unboundedArrayRange();
    UInt32 descriptors = unboundedArray && unboundedArray->Heap == heap ? unboundedArrayElements : 0u;

    return std::ranges::fold_left(m_impl->m_ranges 
        | std::views::filter([heap](const DescriptorAllocationRange& range) { return range.Heap == heap; })
        | std::views::transform([](const DescriptorAllocationRange& range) { return range.DescriptorCount; }),
        descriptors, std::plus{});
}

bool DescriptorAllocationLayout::binds(DescriptorHeapType heap) const noexcept
{
    return std::ranges::any_of(m_impl->m_ranges, [heap](const DescriptorAllocationRange& slot) { return slot.Heap == heap; });
}

Optional<DescriptorAllocationRange> DescriptorAllocationLayout::unboundedArrayRange() const noexcept
{
    if (m_impl->m_ranges.empty())
        return std::nullopt;

    auto& range = m_impl->m_ranges.back();
    return range.Unbounded ? Optional<DescriptorAllocationRange>{ range } : std::nullopt;
}