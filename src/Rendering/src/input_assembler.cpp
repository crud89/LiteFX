#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class InputAssembler::InputAssemblerImpl : public Implement<InputAssembler> {
public:
    friend class InputAssembler;

private:
    UniquePtr<BufferLayout> m_layout;
    PrimitiveTopology m_topology = PrimitiveTopology::TriangleStrip;

public: 
    InputAssemblerImpl(InputAssembler* parent) : base(parent) { }

public:
    const BufferLayout* getLayout() const
    {
        return m_layout.get();
    }

    void use(UniquePtr<BufferLayout>&& layout)
    {
        m_layout = std::move(layout);
    }
};

InputAssembler::InputAssembler() : 
    m_impl(makePimpl<InputAssemblerImpl>(this))
{
}

InputAssembler::InputAssembler(UniquePtr<BufferLayout>&& _other) :
    m_impl(makePimpl<InputAssemblerImpl>(this))
{
    this->use(std::move(_other));
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

const BufferLayout* InputAssembler::getLayout() const
{
    return m_impl->getLayout();
}

void InputAssembler::use(UniquePtr<BufferLayout>&& layout)
{
    m_impl->use(std::move(layout));
}