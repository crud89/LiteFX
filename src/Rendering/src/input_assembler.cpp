#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class InputAssembler::InputAssemblerImpl : public Implement<InputAssembler> {
public:
    friend class InputAssembler;

private:
    Array<UniquePtr<BufferLayout>> m_layouts;
    PrimitiveTopology m_topology = PrimitiveTopology::TriangleStrip;

public: 
    InputAssemblerImpl(InputAssembler* parent) : base(parent) { }

public:
    Array<const BufferLayout*> getLayouts() const
    {
        Array<const BufferLayout*> layouts(m_layouts.size());
        std::generate(std::begin(layouts), std::end(layouts), [&, i = 0]() mutable { return m_layouts[i++].get(); });
        return layouts;
    }

    void use(UniquePtr<BufferLayout>&& layout)
    {
        m_layouts.push_back(std::move(layout));
    }

    UniquePtr<BufferLayout> remove(const BufferLayout* layout)
    {
        auto it = std::find_if(m_layouts.begin(), m_layouts.end(), [layout](const UniquePtr<BufferLayout>& a) { return a.get() == layout; });

        if (it == m_layouts.end())
            return UniquePtr<BufferLayout>();
        else
        {
            auto result = std::move(*it);
            m_layouts.erase(it);

            return std::move(result);
        }
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

Array<const BufferLayout*> InputAssembler::getLayouts() const
{
    return m_impl->getLayouts();
}

void InputAssembler::use(UniquePtr<BufferLayout>&& layout)
{
    m_impl->use(std::move(layout));
}

UniquePtr<BufferLayout> InputAssembler::remove(const BufferLayout* layout)
{
    return m_impl->remove(layout);
}