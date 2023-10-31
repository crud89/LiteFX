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
    Array<UniquePtr<BufferAttribute>> m_attributes;
    size_t m_vertexSize;
    UInt32 m_binding;
    BufferType m_bufferType{ BufferType::Vertex };

public:
    VulkanVertexBufferLayoutImpl(VulkanVertexBufferLayout* parent, const size_t& vertexSize, const UInt32& binding) : 
        base(parent), m_vertexSize(vertexSize), m_binding(binding) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBufferLayout::VulkanVertexBufferLayout(const size_t& vertexSize, const UInt32& binding) :
    m_impl(makePimpl<VulkanVertexBufferLayoutImpl>(this, vertexSize, binding))
{
}

VulkanVertexBufferLayout::~VulkanVertexBufferLayout() noexcept = default;

size_t VulkanVertexBufferLayout::elementSize() const noexcept
{
    return m_impl->m_vertexSize;
}

const UInt32& VulkanVertexBufferLayout::binding() const noexcept
{
    return m_impl->m_binding;
}

const BufferType& VulkanVertexBufferLayout::type() const noexcept
{
    return m_impl->m_bufferType;
}

Enumerable<const BufferAttribute*> VulkanVertexBufferLayout::attributes() const noexcept
{
    return m_impl->m_attributes | std::views::transform([](const UniquePtr<BufferAttribute>& attribute) { return attribute.get(); });
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

constexpr void VulkanVertexBufferLayoutBuilder::build()
{
    this->instance()->m_impl->m_attributes = std::move(m_state.attributes);
}
#endif // defined(BUILD_DEFINE_BUILDERS)