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
    Array<UniquePtr<BufferAttribute>> m_attributes{};
    size_t m_vertexSize;
    UInt32 m_binding;
    BufferType m_bufferType{ BufferType::Vertex };

public:
    VulkanVertexBufferLayoutImpl(size_t vertexSize, UInt32 binding) : 
        m_vertexSize(vertexSize), m_binding(binding) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBufferLayout::VulkanVertexBufferLayout(size_t vertexSize, UInt32 binding) :
    m_impl(vertexSize, binding)
{
}

VulkanVertexBufferLayout::VulkanVertexBufferLayout(VulkanVertexBufferLayout&&) noexcept = default;
VulkanVertexBufferLayout::VulkanVertexBufferLayout(const VulkanVertexBufferLayout&) noexcept = default;
VulkanVertexBufferLayout& VulkanVertexBufferLayout::operator=(VulkanVertexBufferLayout&&) noexcept = default;
VulkanVertexBufferLayout& VulkanVertexBufferLayout::operator=(const VulkanVertexBufferLayout&) noexcept = default;
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

Enumerable<const BufferAttribute*> VulkanVertexBufferLayout::attributes() const noexcept
{
    return m_impl->m_attributes | std::views::transform([](const UniquePtr<BufferAttribute>& attribute) { return attribute.get(); });
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