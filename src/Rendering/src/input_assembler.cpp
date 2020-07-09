#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class InputAssembler::InputAssemblerImpl : public Implement<InputAssembler> {
public:
    friend class InputAssembler;

private:
    PrimitiveTopology m_topology = PrimitiveTopology::TriangleStrip;

public: 
    InputAssemblerImpl(InputAssembler* parent) : base(parent) { }
};

InputAssembler::InputAssembler() : m_impl(makePimpl<InputAssemblerImpl>(this))
{
}

InputAssembler::~InputAssembler() noexcept = default;

const PrimitiveTopology InputAssembler::getTopology() const noexcept
{
    return m_impl->m_topology;
}

void InputAssembler::setTopology(const PrimitiveTopology& topology)
{
    m_impl->m_topology = topology;
}