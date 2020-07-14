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
    Array<UniquePtr<IBufferSet>> m_bufferSets;

public: 
    InputAssemblerImpl(InputAssembler* parent) : base(parent) { }

public:
    void add(UniquePtr<IBufferSet>&& layout)
    {
        m_bufferSets.push_back(std::move(layout));
    }

    UniquePtr<IBufferSet> remove(const IBufferSet* bufferSet)
    {
        auto it = std::find_if(m_bufferSets.begin(), m_bufferSets.end(), [bufferSet](const UniquePtr<IBufferSet>& l) { return l.get() == bufferSet; });

        if (it == m_bufferSets.end())
            return UniquePtr<IBufferSet>();
        else
        {
            auto result = std::move(*it);
            m_bufferSets.erase(it);

            return std::move(result);
        }
    }

    Array<const IBufferSet*> getSets() const noexcept
    {
        Array<const IBufferSet*> results(m_bufferSets.size());
        std::generate(std::begin(results), std::end(results), [&, i = 0]() mutable { return m_bufferSets[i++].get(); });

        return results;
    }

    Array<const IBufferSet*> getSets(const BufferSetType& type) const noexcept
    {
        Array<const IBufferSet*> results;

        std::for_each(std::begin(m_bufferSets), std::end(m_bufferSets), [&](const UniquePtr<IBufferSet>& bufferSet) {
            if (bufferSet->getType() == type)
                results.push_back(bufferSet.get());
        });

        return results;
    }

    const IBufferSet* findSet(const UInt32& setId) const noexcept
    {
        auto match = std::find_if(m_bufferSets.begin(), m_bufferSets.end(), [setId](const UniquePtr<IBufferSet>& l) { return l->getSetId() == setId; });

        return match == m_bufferSets.end() ? nullptr : match->get();
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

InputAssembler::InputAssembler() : m_impl(makePimpl<InputAssemblerImpl>(this))
{
}

InputAssembler::~InputAssembler() noexcept = default;

Array<const IBufferSet*> InputAssembler::getBufferSets() const
{
    return m_impl->getSets();
}

Array<const IBufferSet*> InputAssembler::getBufferSets(const BufferSetType& type) const
{
    return m_impl->getSets(type);
}

const IBufferSet* InputAssembler::getBufferSet(const UInt32& setId) const
{
    return m_impl->findSet(setId);
}

void InputAssembler::use(UniquePtr<IBufferSet>&& layout)
{
    m_impl->add(std::move(layout));
}

UniquePtr<IBufferSet> InputAssembler::remove(const IBufferSet* layout)
{
    return m_impl->remove(layout);
}

const PrimitiveTopology InputAssembler::getTopology() const noexcept
{
    return m_impl->m_topology;
}

void InputAssembler::setTopology(const PrimitiveTopology& topology)
{
    m_impl->m_topology = topology;
}