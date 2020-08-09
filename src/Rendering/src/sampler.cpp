#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class Sampler::SamplerImpl : public Implement<Sampler> {
public:
	friend class Sampler;

private:
	const IDescriptorLayout* m_layout;
	FilterMode m_magFilter, m_minFilter;
	BorderMode m_borderU, m_borderV, m_borderW;
	MipMapMode m_mipMapMode;
	Float m_mipBias, m_minLod, m_maxLod, m_anisotropy;

public:
	SamplerImpl(Sampler* parent, const IDescriptorLayout* layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
		base(parent), m_layout(layout), m_magFilter(magFilter), m_minFilter(minFilter), m_borderU(borderU), m_borderV(borderV), m_borderW(borderW), m_mipMapMode(mipMapMode), m_mipBias(mipMapBias), m_minLod(minLod), m_maxLod(maxLod), m_anisotropy(anisotropy) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

Sampler::Sampler(const IDescriptorLayout* layout, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& minLod, const Float& maxLod, const Float& anisotropy) :
	m_impl(makePimpl<SamplerImpl>(this, layout, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy))
{
	if (layout == nullptr)
		throw std::invalid_argument("The sampler descriptor layout must be initialized.");
}

Sampler::~Sampler() noexcept = default;

const IDescriptorLayout* Sampler::getLayout() const noexcept
{
	return m_impl->m_layout;
}

UInt32 Sampler::getBinding() const noexcept
{
	return m_impl->m_layout->getBinding();
}

const FilterMode& Sampler::getMinifyingFilter() const noexcept
{
	return m_impl->m_minFilter;
}

const FilterMode& Sampler::getMagnifyingFilter() const noexcept
{
	return m_impl->m_magFilter;
}

const BorderMode& Sampler::getBorderModeU() const noexcept
{
	return m_impl->m_borderU;
}

const BorderMode& Sampler::getBorderModeV() const noexcept
{
	return m_impl->m_borderV;
}

const BorderMode& Sampler::getBorderModeW() const noexcept
{
	return m_impl->m_borderW;
}

const Float& Sampler::getAnisotropy() const noexcept
{
	return m_impl->m_anisotropy;
}

const MipMapMode& Sampler::getMipMapMode() const noexcept
{
	return m_impl->m_mipMapMode;
}

const Float& Sampler::getMipMapBias() const noexcept
{
	return m_impl->m_mipBias;
}

const Float& Sampler::getMaxLOD() const noexcept
{
	return m_impl->m_maxLod;
}

const Float& Sampler::getMinLOD() const noexcept
{
	return m_impl->m_minLod;
}