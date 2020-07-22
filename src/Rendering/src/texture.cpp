#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Texture::TextureImpl : public Implement<Texture> {
public:
    friend class Texture;

private:
    const IDescriptorLayout* m_layout;
    MultiSamplingLevel m_samples;
    UInt32 m_elements, m_size, m_levels;
    Format m_format;
    Size2d m_extent;

public:
    TextureImpl(Texture* parent, const IDescriptorLayout* layout, const UInt32& elements, const UInt32& size, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples) :
        base(parent), m_layout(layout), m_elements(elements), m_size(size), m_extent(extent), m_format(format), m_samples(samples), m_levels(levels) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Texture::Texture(const IDescriptorLayout* layout, const UInt32& elements, const UInt32& size, const Size2d& extent, const Format& format, const UInt32& levels, const MultiSamplingLevel& samples) :
    m_impl(makePimpl<TextureImpl>(this, layout, elements, size, extent, format, levels, samples))
{
    if (layout == nullptr)
        throw std::invalid_argument("The constant buffer descriptor layout must be initialized.");
}

Texture::~Texture() noexcept = default;

UInt32 Texture::getBinding() const noexcept
{
    return m_impl->m_layout->getBinding();
}

const IDescriptorLayout* Texture::getLayout() const noexcept
{
    return m_impl->m_layout;
}

MultiSamplingLevel Texture::getSamples() const noexcept
{
    return m_impl->m_samples;
}

UInt32 Texture::getLevels() const noexcept
{
    return m_impl->m_levels;
}