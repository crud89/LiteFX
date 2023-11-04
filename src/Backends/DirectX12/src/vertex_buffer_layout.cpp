#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12VertexBufferLayout::DirectX12VertexBufferLayoutImpl : public Implement<DirectX12VertexBufferLayout> {
public:
    friend class DirectX12VertexBufferLayoutBuilder;
    friend class DirectX12VertexBufferLayout;

private:
    Array<UniquePtr<BufferAttribute>> m_attributes;
    size_t m_vertexSize;
    UInt32 m_binding;
    BufferType m_bufferType{ BufferType::Vertex };

public:
    DirectX12VertexBufferLayoutImpl(DirectX12VertexBufferLayout* parent, size_t vertexSize, UInt32 binding) :
        base(parent), m_vertexSize(vertexSize), m_binding(binding)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12VertexBufferLayout::DirectX12VertexBufferLayout(size_t vertexSize, UInt32 binding) :
    m_impl(makePimpl<DirectX12VertexBufferLayoutImpl>(this, vertexSize, binding))
{
}

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
    return m_impl->m_bufferType;
}

Enumerable<const BufferAttribute*> DirectX12VertexBufferLayout::attributes() const noexcept
{
    return m_impl->m_attributes | std::views::transform([](const UniquePtr<BufferAttribute>& attribute) { return attribute.get(); });
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

void DirectX12VertexBufferLayoutBuilder::build()
{
    this->instance()->m_impl->m_attributes = std::move(m_state.attributes);
}
#endif // defined(BUILD_DEFINE_BUILDERS)