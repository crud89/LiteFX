#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanVertexBufferLayout::VulkanVertexBufferLayoutImpl {
public:
    friend class VulkanVertexBufferLayoutBuilder;
    friend class VulkanVertexBufferLayout;

private:
    Array<BufferAttribute> m_attributes{};
    size_t m_vertexSize;
    UInt32 m_binding;

public:
    VulkanVertexBufferLayoutImpl(size_t vertexSize, UInt32 binding) : 
        m_vertexSize(vertexSize), m_binding(binding) 
    {
    }

    VulkanVertexBufferLayoutImpl(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding) :
        m_vertexSize(vertexSize), m_binding(binding) 
    {
        m_attributes = attributes | std::ranges::to<Array<BufferAttribute>>();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBufferLayout::VulkanVertexBufferLayout(size_t vertexSize, UInt32 binding) :
    m_impl(vertexSize, binding)
{
}

VulkanVertexBufferLayout::VulkanVertexBufferLayout(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding) :
    m_impl(vertexSize, attributes, binding)
{
}

VulkanVertexBufferLayout::VulkanVertexBufferLayout(const VulkanVertexBufferLayout&) = default;
VulkanVertexBufferLayout::~VulkanVertexBufferLayout() noexcept = default;

size_t VulkanVertexBufferLayout::elementSize() const noexcept
{
    return m_impl->m_vertexSize;
}

UInt32 VulkanVertexBufferLayout::binding() const noexcept
{
    return m_impl->m_binding;
}

BufferType VulkanVertexBufferLayout::type() const noexcept
{
    return BufferType::Vertex;
}

const Array<BufferAttribute>& VulkanVertexBufferLayout::attributes() const
{
    return m_impl->m_attributes;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

void VulkanVertexBufferLayoutBuilder::build()
{
    this->instance()->m_impl->m_attributes = std::move(this->state().attributes);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)