#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderPipelineLayout::RenderPipelineLayoutImpl : public Implement<RenderPipelineLayout> {
public:
	friend class RenderPipelineLayout;

private:
	UniquePtr<IRasterizer> m_rasterizer;
	UniquePtr<IInputAssembler> m_inputAssembler;
	UniquePtr<IShaderProgram> m_shaderProgram;
	bool m_depthTest = false, m_stencilTest = false;

public:
	RenderPipelineLayoutImpl(RenderPipelineLayout* parent) :
		base(parent) 
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderPipelineLayout::RenderPipelineLayout() :
	m_impl(makePimpl<RenderPipelineLayoutImpl>(this))
{
}

RenderPipelineLayout::~RenderPipelineLayout() noexcept = default;

IRasterizer* RenderPipelineLayout::getRasterizer() const noexcept 
{
	return m_impl->m_rasterizer.get();
}

void RenderPipelineLayout::use(UniquePtr<IRasterizer>&& rasterizer) 
{
	m_impl->m_rasterizer = std::move(rasterizer);
}

const IInputAssembler* RenderPipelineLayout::getInputAssembler() const noexcept
{
	return m_impl->m_inputAssembler.get();
}

void RenderPipelineLayout::use(UniquePtr<IInputAssembler>&& inputAssembler)
{
	m_impl->m_inputAssembler = std::move(inputAssembler);
}

const IShaderProgram* RenderPipelineLayout::getProgram() const noexcept
{
	return m_impl->m_shaderProgram.get();
}

void RenderPipelineLayout::use(UniquePtr<IShaderProgram>&& shaderProgram)
{
	m_impl->m_shaderProgram = std::move(shaderProgram);
}

bool RenderPipelineLayout::getDepthTest() const noexcept
{
	return m_impl->m_depthTest;
}

void RenderPipelineLayout::setDepthTest(const bool& enable)
{
	m_impl->m_depthTest = enable;
}

bool RenderPipelineLayout::getStencilTest() const noexcept
{
	return m_impl->m_stencilTest;
}

void RenderPipelineLayout::setStencilTest(const bool& enable)
{
	m_impl->m_stencilTest = enable;
}