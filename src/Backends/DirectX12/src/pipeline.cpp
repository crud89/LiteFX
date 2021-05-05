#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPipeline::DirectX12RenderPipelineImpl : public Implement<DirectX12RenderPipeline> {
public:
	friend class DirectX12RenderPipelineBuilder;
	friend class DirectX12RenderPipeline;

private:
	const DirectX12RenderPass& m_renderPass;
	UniquePtr<IRenderPipelineLayout> m_layout;
	SharedPtr<IInputAssembler> m_inputAssembler;
	SharedPtr<IRasterizer> m_rasterizer;
	Array<SharedPtr<IViewport>> m_viewports;
	Array<SharedPtr<IScissor>> m_scissors;
	const UInt32 m_id;
	const String m_name;

public:
	DirectX12RenderPipelineImpl(DirectX12RenderPipeline* parent, const DirectX12RenderPass& renderPass, const UInt32& id, const String& name) :
		base(parent), m_renderPass(renderPass), m_id(id), m_name(name)
	{
	}

public:
	ComPtr<ID3D12PipelineState> initialize(UniquePtr<IRenderPipelineLayout>&& pipelineLayout, SharedPtr<IInputAssembler>&& inputAssembler, SharedPtr<IRasterizer>&& rasterizer, Array<SharedPtr<IViewport>>&& viewports, Array<SharedPtr<IScissor>>&& scissors)
	{
		// Request the device (must be initialized, otherwise the render pass instance is invalid).
		auto device = m_renderPass.getDevice()->handle();

		// Get the root signature.
		auto layout = dynamic_cast<const DirectX12RenderPipelineLayout*>(pipelineLayout.get());

		if (layout == nullptr)
			throw InvalidArgumentException("The layout of the render pipeline is not a valid DirectX12 render pipeline layout.");

		// Create a pipeline state description.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescription = {};
		pipelineStateDescription.pRootSignature = layout->handle().Get();
		//pipelineStateDescription.
		throw;

		// Create the pipeline state instance.
		ComPtr<ID3D12PipelineState> pipelineState;
		raiseIfFailed<RuntimeException>(device->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState)), "Unable to create render pipeline state.");

		// Store the parameters.
		m_layout = std::move(pipelineLayout);
		m_inputAssembler = std::move(inputAssembler);
		m_rasterizer = std::move(rasterizer);
		m_viewports = std::move(viewports);
		m_scissors = std::move(scissors);

		return pipelineState;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipeline::DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, const UInt32& id, const String& name) :
	m_impl(makePimpl<DirectX12RenderPipelineImpl>(this, renderPass, id, name)), IComResource<ID3D12PipelineState>(nullptr)
{
}

DirectX12RenderPipeline::~DirectX12RenderPipeline() noexcept = default;

bool DirectX12RenderPipeline::isInitialized() const noexcept
{
	return this->handle() != nullptr;
}

const IRenderPass& DirectX12RenderPipeline::renderPass() const noexcept
{
	return m_impl->m_renderPass;
}

const String& DirectX12RenderPipeline::name() const noexcept
{
	return m_impl->m_name;
}

const UInt32& DirectX12RenderPipeline::id() const noexcept
{
	return m_impl->m_id;
}

void DirectX12RenderPipeline::initialize(UniquePtr<IRenderPipelineLayout> && layout, SharedPtr<IInputAssembler> inputAssembler, SharedPtr<IRasterizer> rasterizer, Array<SharedPtr<IViewport>> && viewports, Array<SharedPtr<IScissor>> && scissors)
{
	if (this->isInitialized())
		throw RuntimeException("The render pipeline already has been initialized.");

	this->handle() = m_impl->initialize(std::move(layout), std::move(inputAssembler), std::move(rasterizer), std::move(viewports), std::move(scissors));
}

const IRenderPipelineLayout* DirectX12RenderPipeline::getLayout() const noexcept
{
	return m_impl->m_layout.get();
}

SharedPtr<IInputAssembler> DirectX12RenderPipeline::getInputAssembler() const noexcept
{
	return m_impl->m_inputAssembler;
}

SharedPtr<IRasterizer> DirectX12RenderPipeline::getRasterizer() const noexcept
{
	return m_impl->m_rasterizer;
}

Array<const IViewport*> DirectX12RenderPipeline::getViewports() const noexcept
{
	Array<const IViewport*> viewports(m_impl->m_viewports.size());
	std::generate(std::begin(viewports), std::end(viewports), [&, i = 0]() mutable { return m_impl->m_viewports[i++].get(); });

	return viewports;
}

Array<const IScissor*> DirectX12RenderPipeline::getScissors() const noexcept
{
	Array<const IScissor*> scissors(m_impl->m_scissors.size());
	std::generate(std::begin(scissors), std::end(scissors), [&, i = 0]() mutable { return m_impl->m_scissors[i++].get(); });

	return scissors;
}

void DirectX12RenderPipeline::bind(const IVertexBuffer * buffer) const
{
	throw;
}

void DirectX12RenderPipeline::bind(const IIndexBuffer * buffer) const
{
	throw;
}

void DirectX12RenderPipeline::bind(const IDescriptorSet * descriptorSet) const
{
	throw;
}

void DirectX12RenderPipeline::use() const
{
	throw;
}

UniquePtr<IVertexBuffer> DirectX12RenderPipeline::makeVertexBuffer(const BufferUsage & usage, const UInt32 & elements, const UInt32 & binding) const
{
	throw;
}

UniquePtr<IIndexBuffer> DirectX12RenderPipeline::makeIndexBuffer(const BufferUsage & usage, const UInt32 & elements, const IndexType & indexType) const
{
	throw;
}

UniquePtr<IDescriptorSet> DirectX12RenderPipeline::makeBufferPool(const UInt32 & setId) const
{
	throw;
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPipelineBuilder::DirectX12RenderPipelineBuilderImpl : public Implement<DirectX12RenderPipelineBuilder> {
public:
	friend class DirectX12RenderPipelineBuilderBuilder;
	friend class DirectX12RenderPipelineBuilder;

private:
	UniquePtr<IRenderPipelineLayout> m_layout;
	SharedPtr<IInputAssembler> m_inputAssembler;
	SharedPtr<IRasterizer> m_rasterizer;
	Array<SharedPtr<IViewport>> m_viewports;
	Array<SharedPtr<IScissor>> m_scissors;

public:
	DirectX12RenderPipelineBuilderImpl(DirectX12RenderPipelineBuilder* parent) :
		base(parent)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipelineBuilder::DirectX12RenderPipelineBuilder(DirectX12RenderPassBuilder& parent, UniquePtr<DirectX12RenderPipeline>&& instance) :
	RenderPipelineBuilder(parent, std::move(instance)), m_impl(makePimpl<DirectX12RenderPipelineBuilderImpl>(this))
{
}

DirectX12RenderPipelineBuilder::~DirectX12RenderPipelineBuilder() noexcept = default;

DirectX12RenderPassBuilder& DirectX12RenderPipelineBuilder::go()
{
	this->instance()->initialize(std::move(m_impl->m_layout), std::move(m_impl->m_inputAssembler), std::move(m_impl->m_rasterizer), std::move(m_impl->m_viewports), std::move(m_impl->m_scissors));

	return RenderPipelineBuilder::go();
}

void DirectX12RenderPipelineBuilder::use(UniquePtr<IRenderPipelineLayout> && layout)
{
#ifndef NDEBUG
	if (m_impl->m_layout != nullptr)
		LITEFX_WARNING(DIRECTX12_LOG, "Another pipeline layout has already been initialized and will be replaced. A pipeline can only have one pipeline layout.");
#endif

	m_impl->m_layout = std::move(layout);
}

void DirectX12RenderPipelineBuilder::use(SharedPtr<IRasterizer> rasterizer)
{
#ifndef NDEBUG
	if (m_impl->m_rasterizer != nullptr)
		LITEFX_WARNING(DIRECTX12_LOG, "Another rasterizer has already been initialized and will be replaced. A pipeline can only have one rasterizer.");
#endif

	m_impl->m_rasterizer = rasterizer;
}

void DirectX12RenderPipelineBuilder::use(SharedPtr<IInputAssembler> inputAssembler)
{
#ifndef NDEBUG
	if (m_impl->m_inputAssembler != nullptr)
		LITEFX_WARNING(DIRECTX12_LOG, "Another input assembler has already been initialized and will be replaced. A pipeline can only have one input assembler.");
#endif

	m_impl->m_inputAssembler = inputAssembler;
}

void DirectX12RenderPipelineBuilder::use(SharedPtr<IViewport> viewport)
{
	m_impl->m_viewports.push_back(viewport);
}

void DirectX12RenderPipelineBuilder::use(SharedPtr<IScissor> scissor)
{
	m_impl->m_scissors.push_back(scissor);
}

DirectX12RenderPipelineLayoutBuilder DirectX12RenderPipelineBuilder::layout()
{
	return this->make<DirectX12RenderPipelineLayout>();
}

DirectX12RasterizerBuilder DirectX12RenderPipelineBuilder::rasterizer()
{
	return this->make<DirectX12Rasterizer>();
}

DirectX12InputAssemblerBuilder DirectX12RenderPipelineBuilder::inputAssembler()
{
	return this->make<DirectX12InputAssembler>();
}

DirectX12RenderPipelineBuilder& DirectX12RenderPipelineBuilder::withRasterizer(SharedPtr<IRasterizer> rasterizer)
{
	this->use(std::move(rasterizer));
	return *this;
}

DirectX12RenderPipelineBuilder& DirectX12RenderPipelineBuilder::withInputAssembler(SharedPtr<IInputAssembler> inputAssembler)
{
	this->use(std::move(inputAssembler));
	return *this;
}

DirectX12RenderPipelineBuilder& DirectX12RenderPipelineBuilder::withViewport(SharedPtr<IViewport> viewport)
{
	this->use(std::move(viewport));
	return *this;
}

DirectX12RenderPipelineBuilder& DirectX12RenderPipelineBuilder::withScissor(SharedPtr<IScissor> scissor)
{
	this->use(std::move(scissor));
	return *this;
}