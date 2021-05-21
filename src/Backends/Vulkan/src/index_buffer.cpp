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
    m_impl(makePimpl<VulkanIndexBufferLayoutImpl>(this, type)), VulkanRuntimeObject<VulkanInputAssembler>(inputAssembler, inputAssembler.getDevice())
{
}

VulkanIndexBufferLayout::~VulkanIndexBufferLayout() noexcept = default;

size_t VulkanIndexBufferLayout::elementSize() const noexcept
{
    return static_cast<UInt32>(m_impl->m_type) >> 3;
}

const UInt32& VulkanIndexBufferLayout::binding() const noexcept
{
    return 0;
}

const BufferType& VulkanIndexBufferLayout::type() const noexcept
{
    return BufferType::Index;
}

const IndexType& VulkanIndexBufferLayout::indexType() const noexcept
{
    return m_impl->m_type;
}