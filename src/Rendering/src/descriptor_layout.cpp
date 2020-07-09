#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

class DescriptorSetLayout::DescriptorSetLayoutImpl : public Implement<DescriptorSetLayout> {
public:
    friend class DescriptorSetLayout;

private:
    Array<UniquePtr<BufferLayout>> m_layouts;

public:
    DescriptorSetLayoutImpl(DescriptorSetLayout* parent) : base(parent) { }

public:
    Array<const BufferLayout*> getLayouts() const
    {
        Array<const BufferLayout*> layouts(m_layouts.size());
        std::generate(std::begin(layouts), std::end(layouts), [&, i = 0]() mutable { return m_layouts[i++].get(); });
        return layouts;
    }

    Array<const BufferLayout*> getLayouts(const BufferType& type) const
    {
        Array<const BufferLayout*> layouts;
        
        for each (auto & layout in m_layouts)
            if (layout->getType() == type)
                layouts.push_back(layout.get());

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

DescriptorSetLayout::DescriptorSetLayout() : m_impl(makePimpl<DescriptorSetLayoutImpl>(this))
{
}

DescriptorSetLayout::~DescriptorSetLayout() noexcept = default;

Array<const BufferLayout*> DescriptorSetLayout::getLayouts() const
{
    return m_impl->getLayouts();
}

Array<const BufferLayout*> DescriptorSetLayout::getLayouts(const BufferType& type) const
{
    return m_impl->getLayouts(type);
}

void DescriptorSetLayout::use(UniquePtr<BufferLayout> && layout)
{
    m_impl->use(std::move(layout));
}

UniquePtr<BufferLayout> DescriptorSetLayout::remove(const BufferLayout * layout)
{
    return m_impl->remove(layout);
}