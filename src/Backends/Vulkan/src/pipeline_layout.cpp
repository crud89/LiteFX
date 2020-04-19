#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineLayout::VulkanRenderPipelineLayoutImpl {
private:
	Array<UniquePtr<IViewport>> m_viewports;
	UniquePtr<IRasterizer> m_rasterizer;
	UniquePtr<IShaderProgram> m_shaderProgram;

public:
	VulkanRenderPipelineLayoutImpl() noexcept = default;

public:
	Array<const IViewport*> getViewports() const noexcept 
	{
		Array<const IViewport*> viewports(m_viewports.size());
		std::generate(std::begin(viewports), std::end(viewports), [&, i = 0]() mutable { return m_viewports[i++].get(); });
		return viewports;
	}
	
	void use(UniquePtr<IViewport>&& viewport) 
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
	
	const IRasterizer* getRasterizer() const noexcept 
	{
		return m_rasterizer.get();
	}
	
	void use(UniquePtr<IRasterizer>&& rasterizer) 
	{
		m_rasterizer = std::move(rasterizer);
	}
	
	const IShaderProgram* getProgram() const noexcept 
	{
		return m_shaderProgram.get();
	}
	
	void use(UniquePtr<IShaderProgram>&& program) 
	{
		m_shaderProgram = std::move(program);
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout() noexcept :
	m_impl(makePimpl<VulkanRenderPipelineLayoutImpl>())
{
}

VulkanRenderPipelineLayout::~VulkanRenderPipelineLayout() noexcept = default;

Array<const IViewport*> VulkanRenderPipelineLayout::getViewports() const noexcept 
{
	return m_impl->getViewports();
}

void VulkanRenderPipelineLayout::use(UniquePtr<IViewport>&& viewport) 
{
	return m_impl->use(std::move(viewport));
}

UniquePtr<IViewport> VulkanRenderPipelineLayout::remove(const IViewport* viewport) noexcept
{
	return m_impl->remove(viewport);
}

const IRasterizer* VulkanRenderPipelineLayout::getRasterizer() const noexcept 
{
	return m_impl->getRasterizer();
}

void VulkanRenderPipelineLayout::use(UniquePtr<IRasterizer>&& rasterizer) 
{
	m_impl->use(std::move(rasterizer));
}

const IShaderProgram* VulkanRenderPipelineLayout::getProgram() const noexcept 
{
	return m_impl->getProgram();
}

void VulkanRenderPipelineLayout::use(UniquePtr<IShaderProgram>&& program) 
{
	m_impl->use(std::move(program));
}