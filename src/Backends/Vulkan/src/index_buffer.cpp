#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanIndexBufferLayout::VulkanIndexBufferLayoutImpl : public Implement<VulkanIndexBufferLayout> {
public:
    friend class VulkanIndexBufferLayout;

private:
    IndexType m_type;

public:
    VulkanIndexBufferLayoutImpl(VulkanIndexBufferLayout* parent, const IndexType& type) : base(parent), m_type(type) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanIndexBufferLayout::VulkanIndexBufferLayout(const VulkanInputAssembler& inputAssembler, const IndexType& type) :
    m_impl(makePimpl<VulkanIndexBufferLayoutImpl>(this, type)), VulkanRuntimeObject(inputAssembler.getDevice())
{
}

VulkanIndexBufferLayout::~VulkanIndexBufferLayout() noexcept = default;

size_t VulkanIndexBufferLayout::getElementSize() const noexcept
{
    switch (m_impl->m_type)
    {
    case IndexType::UInt16:
        return 2;
    case IndexType::UInt32:
        return 4;
    default:
        throw std::runtime_error("Unsupported index type.");
    }
}

UInt32 VulkanIndexBufferLayout::getBinding() const noexcept
{
    return 0;
}

BufferType VulkanIndexBufferLayout::getType() const noexcept
{
    return BufferType::Index;
}

const IndexType& VulkanIndexBufferLayout::getIndexType() const noexcept
{
    return m_impl->m_type;
}