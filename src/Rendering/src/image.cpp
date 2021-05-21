#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Image::ImageImpl : public Implement<Image> {
public:
    friend class Image;

private:
    UInt32 m_elements, m_size;
    Format m_format;
    Size2d m_extent;

public:
    ImageImpl(Image* parent, const UInt32& elements, const UInt32& size, const Size2d& extent, const Format& format) :
        base(parent), m_elements(elements), m_size(size), m_extent(extent), m_format(format) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Image::Image(const UInt32& elements, const size_t& size, const Size2d& extent, const Format& format) :
    m_impl(makePimpl<ImageImpl>(this, elements, size, extent, format))
{
}

Image::~Image() noexcept = default;

const UInt32& Image::elements() const noexcept
{
    return m_impl->m_elements;
}

size_t Image::size() const noexcept
{
    return m_impl->m_size;
}

const Size2d& Image::extent() const noexcept
{
    return m_impl->m_extent;
}

const Format& Image::format() const noexcept
{
    return m_impl->m_format;
}