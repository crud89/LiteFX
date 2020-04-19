#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class InputAssembler::InputAssemblerImpl {
private:
    UniquePtr<BufferLayout> m_layout;
    PrimitiveTopology m_topology = PrimitiveTopology::TriangleStrip;

public: 
    InputAssemblerImpl() noexcept = default;

public:
    const PrimitiveTopology getTopology() const noexcept
    {
        return m_topology;
    }

    void setTopology(const PrimitiveTopology& topology)
    {
        m_topology = topology;
    }

    const BufferLayout* getLayout() const
    {
        return m_layout.get();
    }

    void use(UniquePtr<BufferLayout>&& layout)
    {
        m_layout = std::move(layout);
    }
};

InputAssembler::InputAssembler() noexcept : 
    m_impl(makePimpl<InputAssemblerImpl>())
{
}

InputAssembler::InputAssembler(UniquePtr<BufferLayout>&& _other) noexcept :
    m_impl(makePimpl<InputAssemblerImpl>())
{
    this->use(std::move(_other));
}

InputAssembler::~InputAssembler() noexcept = default;

const PrimitiveTopology InputAssembler::getTopology() const noexcept
{
    return m_impl->getTopology();
}

void InputAssembler::setTopology(const PrimitiveTopology& topology)
{
    m_impl->setTopology(topology);
}

const BufferLayout* InputAssembler::getLayout() const
{
    return m_impl->getLayout();
}

void InputAssembler::use(UniquePtr<BufferLayout>&& layout)
{
    m_impl->use(std::move(layout));
}