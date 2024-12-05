#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12VertexBufferLayout::DirectX12VertexBufferLayoutImpl {
public:
    friend class DirectX12VertexBufferLayoutBuilder;
    friend class DirectX12VertexBufferLayout;

private:
    Array<BufferAttribute> m_attributes{};
    size_t m_vertexSize;
    UInt32 m_binding;

public:
    DirectX12VertexBufferLayoutImpl(size_t vertexSize, UInt32 binding) :
        m_vertexSize(vertexSize), m_binding(binding)
    {
    }

    DirectX12VertexBufferLayoutImpl(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding) :
        m_vertexSize(vertexSize), m_binding(binding)
    {
        m_attributes = attributes | std::ranges::to<Array<BufferAttribute>>();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12VertexBufferLayout::DirectX12VertexBufferLayout(size_t vertexSize, UInt32 binding) :
    m_impl(vertexSize, binding)
{
}

DirectX12VertexBufferLayout::DirectX12VertexBufferLayout(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding) :
    m_impl(vertexSize, attributes, binding)
{
}

DirectX12VertexBufferLayout::DirectX12VertexBufferLayout(const DirectX12VertexBufferLayout&) = default;
DirectX12VertexBufferLayout::~DirectX12VertexBufferLayout() noexcept = default;

size_t DirectX12VertexBufferLayout::elementSize() const noexcept
{
    return m_impl->m_vertexSize;
}

UInt32 DirectX12VertexBufferLayout::binding() const noexcept
{
    return m_impl->m_binding;
}

BufferType DirectX12VertexBufferLayout::type() const noexcept
{
    return BufferType::Vertex;
}

Enumerable<const BufferAttribute*> DirectX12VertexBufferLayout::attributes() const
{
    return m_impl->m_attributes | std::views::transform([](const auto& attribute) { return std::addressof(attribute); });
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

void DirectX12VertexBufferLayoutBuilder::build()
{
    this->instance()->m_impl->m_attributes = std::move(this->state().attributes);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)