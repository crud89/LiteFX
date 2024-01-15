#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------
class DirectX12InputAssembler::DirectX12InputAssemblerImpl : public Implement<DirectX12InputAssembler> {
public:
    friend class DirectX12InputAssembler;

private:
    Array<UniquePtr<const DirectX12VertexBufferLayout>> m_vertexBufferLayouts;
    Dictionary<UInt32, const DirectX12VertexBufferLayout*> m_vertexBufferLayoutBindings;
    UniquePtr<const DirectX12IndexBufferLayout> m_indexBufferLayout;
    PrimitiveTopology m_primitiveTopology;

public:
    DirectX12InputAssemblerImpl(DirectX12InputAssembler* parent) :
        base(parent)
    {
    }

public:
    void initialize(Array<UniquePtr<const DirectX12VertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<const DirectX12IndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology)
    {
        m_primitiveTopology = primitiveTopology;
        m_indexBufferLayout = std::move(indexBufferLayout);
        m_vertexBufferLayouts = std::move(vertexBufferLayouts);

        for (auto& vertexBufferLayout : m_vertexBufferLayouts)
        {
            if (vertexBufferLayout == nullptr)
                throw ArgumentNotInitializedException("vertexBufferLayouts", "One of the provided vertex buffer layouts is not initialized.");

            if (m_vertexBufferLayoutBindings.contains(vertexBufferLayout->binding()))
                throw InvalidArgumentException("vertexBufferLayouts", "Multiple vertex buffer layouts use the binding point {0}, but only one layout per binding point is allowed.", vertexBufferLayout->binding());

            m_vertexBufferLayoutBindings.emplace(vertexBufferLayout->binding(), vertexBufferLayout.get());
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12InputAssembler::DirectX12InputAssembler(Enumerable<UniquePtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<DirectX12IndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology) :
    m_impl(makePimpl<DirectX12InputAssemblerImpl>(this))
{
    Array<UniquePtr<const DirectX12VertexBufferLayout>> layouts;
    layouts.assign_range(vertexBufferLayouts);

    m_impl->initialize(std::move(layouts), std::move(indexBufferLayout), primitiveTopology);
}

DirectX12InputAssembler::DirectX12InputAssembler() noexcept :
    m_impl(makePimpl<DirectX12InputAssemblerImpl>(this))
{
}

DirectX12InputAssembler::~DirectX12InputAssembler() noexcept = default;

const Array<UniquePtr<const DirectX12VertexBufferLayout>>& DirectX12InputAssembler::vertexBufferLayouts() const noexcept
{
    return m_impl->m_vertexBufferLayouts;
}

const DirectX12VertexBufferLayout& DirectX12InputAssembler::vertexBufferLayout(UInt32 binding) const
{
    [[likely]] if (m_impl->m_vertexBufferLayoutBindings.contains(binding))
        return *m_impl->m_vertexBufferLayoutBindings[binding];

    throw InvalidArgumentException("binding", "No vertex buffer layout is bound to binding point {0}.", binding);
}

const DirectX12IndexBufferLayout* DirectX12InputAssembler::indexBufferLayout() const noexcept
{
    return m_impl->m_indexBufferLayout.get();
}

PrimitiveTopology DirectX12InputAssembler::topology() const noexcept
{
    return m_impl->m_primitiveTopology;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12InputAssemblerBuilder::DirectX12InputAssemblerBuilderImpl : public Implement<DirectX12InputAssemblerBuilder> {
public:
    friend class DirectX12InputAssemblerBuilder;
    friend class DirectX12InputAssembler;

private:
    Array<UniquePtr<DirectX12VertexBufferLayout>> m_vertexBufferLayouts;
    UniquePtr<DirectX12IndexBufferLayout> m_indexBufferLayout;
    PrimitiveTopology m_primitiveTopology;

public:
    DirectX12InputAssemblerBuilderImpl(DirectX12InputAssemblerBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

constexpr DirectX12InputAssemblerBuilder::DirectX12InputAssemblerBuilder() noexcept :
    m_impl(makePimpl<DirectX12InputAssemblerBuilderImpl>(this)), InputAssemblerBuilder(SharedPtr<DirectX12InputAssembler>(new DirectX12InputAssembler()))
{
}

constexpr DirectX12InputAssemblerBuilder::~DirectX12InputAssemblerBuilder() noexcept = default;

void DirectX12InputAssemblerBuilder::build()
{
    this->instance()->m_impl->initialize(std::move(m_state.vertexBufferLayouts), std::move(m_state.indexBufferLayout), m_state.topology);
}

constexpr DirectX12VertexBufferLayoutBuilder DirectX12InputAssemblerBuilder::vertexBuffer(size_t elementSize, UInt32 binding)
{
    return DirectX12VertexBufferLayoutBuilder(*this, makeUnique<DirectX12VertexBufferLayout>(elementSize, binding));
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)