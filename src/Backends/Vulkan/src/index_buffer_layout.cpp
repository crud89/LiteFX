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
    UInt32 m_binding{ 0 };
    BufferType m_bufferType{ BufferType::Index };

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

VulkanIndexBufferLayout::VulkanIndexBufferLayout(VulkanIndexBufferLayout&&) noexcept = default;
VulkanIndexBufferLayout::VulkanIndexBufferLayout(const VulkanIndexBufferLayout&) = default;
VulkanIndexBufferLayout& VulkanIndexBufferLayout::operator=(VulkanIndexBufferLayout&&) noexcept = default;
VulkanIndexBufferLayout& VulkanIndexBufferLayout::operator=(const VulkanIndexBufferLayout&) = default;
VulkanIndexBufferLayout::~VulkanIndexBufferLayout() noexcept = default;

size_t VulkanIndexBufferLayout::elementSize() const noexcept
{
    return static_cast<UInt32>(m_impl->m_indexType) >> 3;
}

UInt32 VulkanIndexBufferLayout::binding() const noexcept
{
    return m_impl->m_binding;
}

BufferType VulkanIndexBufferLayout::type() const noexcept
{
    return m_impl->m_bufferType;
}

IndexType VulkanIndexBufferLayout::indexType() const noexcept
{
    return m_impl->m_indexType;
}