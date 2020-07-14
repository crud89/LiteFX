#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class BufferSet::BufferSetImpl : public Implement<BufferSet> {
public:
    friend class BufferSet;

private:
    Array<UniquePtr<IBufferLayout>> m_layouts;
    BufferSetType m_type;
    UInt32 m_set;

public:
    BufferSetImpl(BufferSet* parent, const BufferSetType& type, const UInt32& set) : base(parent), m_type(type), m_set(set) { }

public:
    void add(UniquePtr<IBufferLayout>&& layout)
    {
        m_layouts.push_back(std::move(layout));
    }

    UniquePtr<IBufferLayout> remove(const IBufferLayout* layout)
    {
        auto it = std::find_if(m_layouts.begin(), m_layouts.end(), [layout](const UniquePtr<IBufferLayout>& l) { return l.get() == layout; });

        if (it == m_layouts.end())
            return UniquePtr<IBufferLayout>();
        else
        {
            auto result = std::move(*it);
            m_layouts.erase(it);

            return std::move(result);
        }
    }

    Array<const IBufferLayout*> getLayouts() const noexcept
    {
        Array<const IBufferLayout*> results(m_layouts.size());
        std::generate(std::begin(results), std::end(results), [&, i = 0]() mutable { return m_layouts[i++].get(); });

        return results;
    }

    Array<const IBufferLayout*> getLayouts(const BufferType& type) const noexcept
    {
        Array<const IBufferLayout*> results;
        
        std::for_each(std::begin(m_layouts), std::end(m_layouts), [&](const UniquePtr<IBufferLayout>& layout) {
            if (layout->getType() == type)
                results.push_back(layout.get());
        });

        return results;
    }

    const IBufferLayout* findLayout(const UInt32& binding) const noexcept
    {
        auto match = std::find_if(m_layouts.begin(), m_layouts.end(), [binding](const UniquePtr<IBufferLayout>& l) { return l->getBinding() == binding; });

        return match == m_layouts.end() ? nullptr : match->get();
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

BufferSet::BufferSet(const BufferSetType& type, const UInt32& id) : m_impl(makePimpl<BufferSetImpl>(this, type, id))
{
}

BufferSet::~BufferSet() noexcept = default;

void BufferSet::add(UniquePtr<IBufferLayout>&& layout)
{
    m_impl->add(std::move(layout));
}

void BufferSet::remove(const IBufferLayout* layout)
{
    m_impl->remove(layout);
}

Array<const IBufferLayout*> BufferSet::getLayouts() const noexcept
{
    return m_impl->getLayouts();
}

Array<const IBufferLayout*> BufferSet::getLayouts(const BufferType& type) const noexcept
{
    return m_impl->getLayouts(type);
}

const IBufferLayout* BufferSet::getLayout(const UInt32& binding) const noexcept
{
    return m_impl->findLayout(binding);
}

const UInt32& BufferSet::getSetId() const noexcept
{
    return m_impl->m_set;
}

const BufferSetType& BufferSet::getType() const noexcept
{
    return m_impl->m_type;
}