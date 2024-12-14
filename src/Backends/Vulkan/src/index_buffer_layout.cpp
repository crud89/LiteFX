#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanIndexBufferLayout::VulkanIndexBufferLayoutImpl {
public:
    friend class VulkanIndexBufferLayout;

private:
    IndexType m_indexType;

public:
    VulkanIndexBufferLayoutImpl(IndexType type) : 
        m_indexType(type)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanIndexBufferLayout::VulkanIndexBufferLayout(IndexType type) :
    m_impl(type)
{
}

VulkanIndexBufferLayout::VulkanIndexBufferLayout(const VulkanIndexBufferLayout&) = default;
VulkanIndexBufferLayout::~VulkanIndexBufferLayout() noexcept = default;

size_t VulkanIndexBufferLayout::elementSize() const noexcept
{
    return std::to_underlying(m_impl->m_indexType) >> 3;
}

UInt32 VulkanIndexBufferLayout::binding() const noexcept
{
    return 0u;
}

BufferType VulkanIndexBufferLayout::type() const noexcept
{
    return BufferType::Index;
}

IndexType VulkanIndexBufferLayout::indexType() const noexcept
{
    return m_impl->m_indexType;
}