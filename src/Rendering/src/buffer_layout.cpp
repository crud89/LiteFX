#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

class BufferLayout::BufferLayoutImpl : public Implement<BufferLayout> {
public:
    friend class BufferLayout;

private:
    Array<UniquePtr<BufferAttribute>> m_attributes;
    BufferType m_type;
    size_t m_stride;
    UInt32 m_binding, m_set;

public:
    BufferLayoutImpl(BufferLayout* parent, const BufferType& type, const size_t& stride, const UInt32& binding, const UInt32& set) :
        base(parent), m_type(type), m_stride(stride), m_binding(binding), m_set(set) { }

public:
    void add(UniquePtr<BufferAttribute>&& attribute)
    {
        m_attributes.push_back(std::move(attribute));
    }

    UniquePtr<BufferAttribute> remove(const BufferAttribute* attribute)
    {
        auto it = std::find_if(m_attributes.begin(), m_attributes.end(), [attribute](const UniquePtr<BufferAttribute>& a) { return a.get() == attribute; });

        if (it == m_attributes.end())
            return UniquePtr<BufferAttribute>();
        else
        {
            auto result = std::move(*it);
            m_attributes.erase(it);

            return std::move(result);
        }
    }

    Array<const BufferAttribute*> getAttributes() const noexcept
    {
        Array<const BufferAttribute*> results (m_attributes.size());
        std::generate(std::begin(results), std::end(results), [&, i = 0]() mutable { return m_attributes[i++].get(); });
        
        return results;
    }
};

BufferLayout::BufferLayout(const BufferType& type, const size_t& elementSize, const UInt32& binding, const UInt32& set) :
    m_impl(makePimpl<BufferLayoutImpl>(this, type, elementSize, binding, set)) 
{
}

BufferLayout::~BufferLayout() noexcept = default;

Array<const BufferAttribute*> BufferLayout::getAttributes() const noexcept
{
    return m_impl->getAttributes();
}

void BufferLayout::add(UniquePtr<BufferAttribute>&& attribute)
{
    m_impl->add(std::move(attribute));
}

void BufferLayout::remove(const BufferAttribute* attribute)
{
    m_impl->remove(attribute);
}

const size_t& BufferLayout::getElementSize() const noexcept
{
    return m_impl->m_stride;
}

const UInt32& BufferLayout::getBinding() const noexcept
{
    return m_impl->m_binding;
}

const BufferType& BufferLayout::getType() const noexcept
{
    return m_impl->m_type;
}

const UInt32& BufferLayout::getSet() const noexcept
{
    return m_impl->m_set;
}