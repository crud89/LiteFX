#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderPipelineLayout::RenderPipelineLayoutImpl : public Implement<RenderPipelineLayout> {
public:
	friend class RenderPipelineLayout;

private:
	Array<UniquePtr<IViewport>> m_viewports;
	UniquePtr<IRasterizer> m_rasterizer;
	UniquePtr<IInputAssembler> m_inputAssembler;
	UniquePtr<IShaderProgram> m_shaderProgram;
	bool m_depthTest = false, m_stencilTest = false;

public:
	RenderPipelineLayoutImpl(RenderPipelineLayout* parent) : base(parent) { }

public:
	Array<IViewport*> getViewports() const noexcept 
	{
		Array<IViewport*> viewports(m_viewports.size());
		std::generate(std::begin(viewports), std::end(viewports), [&, i = 0]() mutable { return m_viewports[i++].get(); });
		return viewports;
	}
	
	void add(UniquePtr<IViewport>&& viewport) 
	{
		m_viewports.push_back(std::move(viewport));
	}
	
	UniquePtr<IViewport> remove(const IViewport* viewport) noexcept
	{
		auto it = std::find_if(m_viewports.begin(), m_viewports.end(), [viewport](const UniquePtr<IViewport>& a) { return a.get() == viewport; });

		if (it == m_viewports.end())
			return UniquePtr<IViewport>();
		else
		{
			auto result = std::move(*it);
			m_viewports.erase(it);

			return std::move(result);
		}
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

Array<IViewport*> RenderPipelineLayout::getViewports() const noexcept 
{
	return m_impl->getViewports();
}

void RenderPipelineLayout::use(UniquePtr<IViewport>&& viewport) 
{
	return m_impl->add(std::move(viewport));
}

UniquePtr<IViewport> RenderPipelineLayout::remove(const IViewport* viewport) noexcept
{
	return m_impl->remove(viewport);
}

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