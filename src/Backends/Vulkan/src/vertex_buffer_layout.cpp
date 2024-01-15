#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanVertexBufferLayout::VulkanVertexBufferLayoutImpl : public Implement<VulkanVertexBufferLayout> {
public:
    friend class VulkanVertexBufferLayoutBuilder;
    friend class VulkanVertexBufferLayout;

private:
    Array<BufferAttribute> m_attributes;
    size_t m_vertexSize;
    UInt32 m_binding;
    BufferType m_bufferType{ BufferType::Vertex };

public:
    VulkanVertexBufferLayoutImpl(VulkanVertexBufferLayout* parent, size_t vertexSize, UInt32 binding, std::initializer_list<BufferAttribute> attributes) :
        base(parent), m_vertexSize(vertexSize), m_binding(binding), m_attributes(attributes)
    {
    }

    VulkanVertexBufferLayoutImpl(VulkanVertexBufferLayout* parent, size_t vertexSize, UInt32 binding, Span<BufferAttribute> attributes) :
        base(parent), m_vertexSize(vertexSize), m_binding(binding)
    {
        m_attributes.assign_range(attributes);
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBufferLayout::VulkanVertexBufferLayout(size_t vertexSize, UInt32 binding, std::initializer_list<BufferAttribute> attributes) :
    m_impl(makePimpl<VulkanVertexBufferLayoutImpl>(this, vertexSize, binding, attributes))
{
}

VulkanVertexBufferLayout::VulkanVertexBufferLayout(size_t vertexSize, UInt32 binding, Span<BufferAttribute> attributes) :
    m_impl(makePimpl<VulkanVertexBufferLayoutImpl>(this, vertexSize, binding, attributes))
{
}

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
    return m_impl->m_bufferType;
}

const Array<BufferAttribute>& VulkanVertexBufferLayout::attributes() const noexcept
{
    return m_impl->m_attributes;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

void VulkanVertexBufferLayoutBuilder::build()
{
    this->instance()->m_impl->m_attributes = std::move(m_state.attributes);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)