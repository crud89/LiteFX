#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

class BufferLayout::BufferLayoutImpl {
private:
    Array<UniquePtr<BufferAttribute>> m_attributes;

public:
    BufferLayoutImpl() = default;

public:
    void add(UniquePtr<BufferAttribute>&& attribute)
    {
        m_attributes.push_back(attribute);
    }

    void remove(const BufferAttribute* attribute) 
    {
        std::remove_if(std::begin(m_attributes), std::end(m_attributes), [attribute](const UniquePtr<BufferAttribute>& a) {return a.get() == attribute; });
    }

    Array<const BufferAttribute*> getAttributes() const noexcept
    {
        Array<const BufferAttribute*> results (m_attributes.size());
        std::generate(std::begin(results), std::end(results), [&, i = 0]() mutable { return m_attributes[i++].get(); });
        
        return results;
    }
};

BufferLayout::BufferLayout() : 
    m_impl(makePimpl<BufferLayoutImpl>())
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