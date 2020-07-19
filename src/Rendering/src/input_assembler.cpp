#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class InputAssembler::InputAssemblerImpl : public Implement<InputAssembler> {
public:
    friend class InputAssembler;

private:
    PrimitiveTopology m_topology = PrimitiveTopology::TriangleStrip;
    Array<UniquePtr<IVertexBufferLayout>> m_vertexBufferLayouts;
    UniquePtr<IIndexBufferLayout> m_indexBufferLayout;

public: 
    InputAssemblerImpl(InputAssembler* parent) : base(parent) { }

public:
    void add(UniquePtr<IVertexBufferLayout>&& layout)
    {
        m_vertexBufferLayouts.push_back(std::move(layout));
    }

    Array<const IVertexBufferLayout*> getVertexBufferLayouts() const noexcept
    {
        Array<const IVertexBufferLayout*> results(m_vertexBufferLayouts.size());
        std::generate(std::begin(results), std::end(results), [&, i = 0]() mutable { return m_vertexBufferLayouts[i++].get(); });

        return results;
    }

    const IVertexBufferLayout* getVertexBufferLayout(const UInt32& binding) const noexcept
    {
        auto match = std::find_if(std::begin(m_vertexBufferLayouts), std::end(m_vertexBufferLayouts), [&](UniquePtr<IVertexBufferLayout>& layout) { return layout->getBinding() == binding; });

        return match == m_vertexBufferLayouts.end() ? nullptr : match->get();
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

InputAssembler::InputAssembler() : m_impl(makePimpl<InputAssemblerImpl>(this))
{
}

InputAssembler::~InputAssembler() noexcept = default;

Array<const IVertexBufferLayout*> InputAssembler::getVertexBufferLayouts() const
{
    return m_impl->getVertexBufferLayouts();
}

const IVertexBufferLayout* InputAssembler::getVertexBufferLayout(const UInt32& binding) const
{
    return m_impl->getVertexBufferLayout(binding);
}

const IIndexBufferLayout* InputAssembler::getIndexBufferLayout() const
{
    return m_impl->m_indexBufferLayout.get();
}

const PrimitiveTopology InputAssembler::getTopology() const noexcept
{
    return m_impl->m_topology;
}

void InputAssembler::setTopology(const PrimitiveTopology& topology)
{
    m_impl->m_topology = topology;
}

void InputAssembler::use(UniquePtr<IVertexBufferLayout>&& layout)
{
    m_impl->add(std::move(layout));
}

void InputAssembler::use(UniquePtr<IIndexBufferLayout>&& layout)
{
    m_impl->m_indexBufferLayout = std::move(layout);
}