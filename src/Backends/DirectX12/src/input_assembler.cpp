#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------
class DirectX12InputAssembler::DirectX12InputAssemblerImpl {
public:
    friend class DirectX12InputAssembler;

private:
    Dictionary<UInt32, SharedPtr<DirectX12VertexBufferLayout>> m_vertexBufferLayouts{};
    SharedPtr<DirectX12IndexBufferLayout> m_indexBufferLayout{};
    PrimitiveTopology m_primitiveTopology{};

public:
    void initialize(Enumerable<SharedPtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, SharedPtr<DirectX12IndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology)
    {
        m_primitiveTopology = primitiveTopology;
        m_indexBufferLayout = std::move(indexBufferLayout);
        auto layouts = std::move(vertexBufferLayouts);

        for (auto vertexBufferLayout : layouts)
        {
            if (vertexBufferLayout == nullptr)
                throw ArgumentNotInitializedException("vertexBufferLayouts", "One of the provided vertex buffer layouts is not initialized.");

            if (m_vertexBufferLayouts.contains(vertexBufferLayout->binding()))
                throw InvalidArgumentException("vertexBufferLayouts", "Multiple vertex buffer layouts use the binding point {0}, but only one layout per binding point is allowed.", vertexBufferLayout->binding());

            m_vertexBufferLayouts.emplace(vertexBufferLayout->binding(), std::move(vertexBufferLayout));
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12InputAssembler::DirectX12InputAssembler(Enumerable<SharedPtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, SharedPtr<DirectX12IndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology) :
    m_impl()
{
    m_impl->initialize(std::move(vertexBufferLayouts), std::move(indexBufferLayout), primitiveTopology);
}

DirectX12InputAssembler::DirectX12InputAssembler() :
    m_impl()
{
}

DirectX12InputAssembler::DirectX12InputAssembler(const DirectX12InputAssembler&) = default;
DirectX12InputAssembler::~DirectX12InputAssembler() noexcept = default;

Enumerable<const DirectX12VertexBufferLayout&> DirectX12InputAssembler::vertexBufferLayouts() const
{
    return m_impl->m_vertexBufferLayouts | std::views::transform([](const auto& pair) -> const DirectX12VertexBufferLayout& { return *pair.second; });
}

const DirectX12VertexBufferLayout& DirectX12InputAssembler::vertexBufferLayout(UInt32 binding) const
{
    [[likely]] if (m_impl->m_vertexBufferLayouts.contains(binding))
        return *m_impl->m_vertexBufferLayouts[binding];

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

class DirectX12InputAssemblerBuilder::DirectX12InputAssemblerBuilderImpl {
public:
    friend class DirectX12InputAssemblerBuilder;
    friend class DirectX12InputAssembler;

private:
    Array<SharedPtr<DirectX12VertexBufferLayout>> m_vertexBufferLayouts{};
    SharedPtr<DirectX12IndexBufferLayout> m_indexBufferLayout{};
    PrimitiveTopology m_primitiveTopology{};
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12InputAssemblerBuilder::DirectX12InputAssemblerBuilder() :
    InputAssemblerBuilder(DirectX12InputAssembler::create()), m_impl()
{
}

DirectX12InputAssemblerBuilder::~DirectX12InputAssemblerBuilder() noexcept = default;

void DirectX12InputAssemblerBuilder::build()
{
    this->instance()->m_impl->initialize(this->state().vertexBufferLayouts | std::views::as_rvalue, std::move(this->state().indexBufferLayout), this->state().topology);
}

DirectX12VertexBufferLayoutBuilder DirectX12InputAssemblerBuilder::vertexBuffer(size_t elementSize, UInt32 binding)
{
    return DirectX12VertexBufferLayoutBuilder { *this, DirectX12VertexBufferLayout::create(elementSize, binding) };
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)