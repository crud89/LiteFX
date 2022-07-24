#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------
class DirectX12InputAssembler::DirectX12InputAssemblerImpl : public Implement<DirectX12InputAssembler> {
public:
    friend class DirectX12InputAssembler;

private:
    Dictionary<UInt32, UniquePtr<DirectX12VertexBufferLayout>> m_vertexBufferLayouts;
    UniquePtr<DirectX12IndexBufferLayout> m_indexBufferLayout;
    PrimitiveTopology m_primitiveTopology;

public:
    DirectX12InputAssemblerImpl(DirectX12InputAssembler* parent) :
        base(parent)
    {
    }

public:
    void initialize(Array<UniquePtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<DirectX12IndexBufferLayout>&& indexBufferLayout, const PrimitiveTopology& primitiveTopology)
    {
        m_primitiveTopology = primitiveTopology;

        if (indexBufferLayout == nullptr)
            throw ArgumentNotInitializedException("The index buffer layout must be initialized.");

        m_indexBufferLayout = std::move(indexBufferLayout);

        for (auto& vertexBufferLayout : vertexBufferLayouts)
        {
            if (vertexBufferLayout == nullptr)
                throw ArgumentNotInitializedException("One of the provided vertex buffer layouts is not initialized.");

            if (m_vertexBufferLayouts.contains(vertexBufferLayout->binding()))
                throw InvalidArgumentException("Multiple vertex buffer layouts use the binding point {0}, but only one layout per binding point is allowed.", vertexBufferLayout->binding());

            m_vertexBufferLayouts.emplace(vertexBufferLayout->binding(), std::move(vertexBufferLayout));
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12InputAssembler::DirectX12InputAssembler(Array<UniquePtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<DirectX12IndexBufferLayout>&& indexBufferLayout, const PrimitiveTopology& primitiveTopology) :
    m_impl(makePimpl<DirectX12InputAssemblerImpl>(this))
{
    m_impl->initialize(std::move(vertexBufferLayouts), std::move(indexBufferLayout), primitiveTopology);
}

DirectX12InputAssembler::DirectX12InputAssembler() noexcept :
    m_impl(makePimpl<DirectX12InputAssemblerImpl>(this))
{
}

DirectX12InputAssembler::~DirectX12InputAssembler() noexcept = default;

Array<const DirectX12VertexBufferLayout*> DirectX12InputAssembler::vertexBufferLayouts() const noexcept
{
    return m_impl->m_vertexBufferLayouts |
        std::views::transform([](const auto& pair) { return pair.second.get(); }) |
        ranges::to<Array<const DirectX12VertexBufferLayout*>>();
}

const DirectX12VertexBufferLayout& DirectX12InputAssembler::vertexBufferLayout(const UInt32 & binding) const
{
    [[likely]] if (m_impl->m_vertexBufferLayouts.contains(binding))
        return *m_impl->m_vertexBufferLayouts[binding];

    throw ArgumentOutOfRangeException("No vertex buffer layout is bound to binding point {0}.", binding);
}

const DirectX12IndexBufferLayout& DirectX12InputAssembler::indexBufferLayout() const
{
    return *m_impl->m_indexBufferLayout;
}

const PrimitiveTopology& DirectX12InputAssembler::topology() const noexcept
{
    return m_impl->m_primitiveTopology;
}

#if defined(BUILD_DEFINE_BUILDERS)
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

DirectX12InputAssemblerBuilder::DirectX12InputAssemblerBuilder(DirectX12RenderPipelineBuilder& parent) noexcept :
    m_impl(makePimpl<DirectX12InputAssemblerBuilderImpl>(this)), InputAssemblerBuilder<DirectX12InputAssemblerBuilder, DirectX12InputAssembler, DirectX12RenderPipelineBuilder>(parent, SharedPtr<DirectX12InputAssembler>(new DirectX12InputAssembler(*std::as_const(parent).instance()->getDevice())))
{
}

DirectX12InputAssemblerBuilder::~DirectX12InputAssemblerBuilder() noexcept = default;

DirectX12VertexBufferLayoutBuilder DirectX12InputAssemblerBuilder::addVertexBuffer(const size_t& elementSize, const UInt32& binding)
{
    return DirectX12VertexBufferLayoutBuilder(*this, makeUnique<DirectX12VertexBufferLayout>(*this->instance(), elementSize, binding));
}

DirectX12InputAssemblerBuilder& DirectX12InputAssemblerBuilder::withIndexType(const IndexType& type)
{
    this->use(makeUnique<DirectX12IndexBufferLayout>(*this->instance(), type));
    return *this;
}

DirectX12InputAssemblerBuilder& DirectX12InputAssemblerBuilder::withTopology(const PrimitiveTopology& topology)
{
    m_impl->m_primitiveTopology = topology;
    return *this;
}

void DirectX12InputAssemblerBuilder::use(UniquePtr<DirectX12VertexBufferLayout>&& layout)
{
    m_impl->m_vertexBufferLayouts.push_back(std::move(layout));
}

void DirectX12InputAssemblerBuilder::use(UniquePtr<DirectX12IndexBufferLayout>&& layout)
{
    m_impl->m_indexBufferLayout = std::move(layout);
}

DirectX12RenderPipelineBuilder& DirectX12InputAssemblerBuilder::go()
{
    this->instance()->m_impl->initialize(std::move(m_impl->m_vertexBufferLayouts), std::move(m_impl->m_indexBufferLayout), m_impl->m_primitiveTopology);
    return InputAssemblerBuilder::go();
}
#endif // defined(BUILD_DEFINE_BUILDERS)