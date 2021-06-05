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
	UniquePtr<DirectX12RenderPipelineLayout> m_layout;
	SharedPtr<DirectX12InputAssembler> m_inputAssembler;
	SharedPtr<DirectX12Rasterizer> m_rasterizer;
	Array<SharedPtr<IViewport>> m_viewports;
	Array<SharedPtr<IScissor>> m_scissors;
	UInt32 m_id;
	String m_name;

public:
	DirectX12RenderPipelineImpl(DirectX12RenderPipeline* parent, const UInt32& id, const String& name, UniquePtr<DirectX12RenderPipelineLayout>&& layout, SharedPtr<DirectX12InputAssembler>&& inputAssembler, SharedPtr<DirectX12Rasterizer>&& rasterizer, Array<SharedPtr<IViewport>>&& viewports, Array<SharedPtr<IScissor>>&& scissors) :
		base(parent), m_id(id), m_name(name), m_layout(std::move(layout)), m_inputAssembler(std::move(inputAssembler)), m_rasterizer(std::move(rasterizer)), m_viewports(std::move(viewports)), m_scissors(std::move(scissors))
	{
	}

	DirectX12RenderPipelineImpl(DirectX12RenderPipeline* parent) :
		base(parent)
	{
	}

public:
	ComPtr<ID3D12PipelineState> initialize()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescription = {};

		// Setup rasterizer state.
		auto& rasterizer = std::as_const(*m_rasterizer.get());
		D3D12_RASTERIZER_DESC rasterizerState = {};
		rasterizerState.DepthClipEnable = FALSE;
		rasterizerState.FillMode = ::getPolygonMode(rasterizer.polygonMode());
		rasterizerState.CullMode = ::getCullMode(rasterizer.cullMode());
		rasterizerState.FrontCounterClockwise = rasterizer.cullOrder() == CullOrder::CounterClockWise;

		LITEFX_TRACE(DIRECTX12_LOG, "Rasterizer state: {{ PolygonMode: {0}, CullMode: {1}, CullOrder: {2}, LineWidth: {3} }}", rasterizer.polygonMode(), rasterizer.cullMode(), rasterizer.cullOrder(), rasterizer.lineWidth());

		if (!rasterizer.useDepthBias())
			LITEFX_TRACE(DIRECTX12_LOG, "\tRasterizer depth bias disabled.");
		else
		{
			LITEFX_TRACE(DIRECTX12_LOG, "\tRasterizer depth bias: {{ Clamp: {0}, ConstantFactor: {1}, SlopeFactor: {2} }}", rasterizer.depthBiasClamp(), rasterizer.depthBiasConstantFactor(), rasterizer.depthBiasSlopeFactor());
			rasterizerState.DepthBiasClamp = rasterizer.depthBiasClamp();
			rasterizerState.DepthBias = static_cast<Int32>(rasterizer.depthBiasConstantFactor());
			rasterizerState.SlopeScaledDepthBias = rasterizer.depthBiasSlopeFactor();
		}

		// Setup input assembler state.
		LITEFX_TRACE(DIRECTX12_LOG, "Input assembler state: {{ PrimitiveTopology: {0} }}", m_inputAssembler->topology());
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = ::getPrimitiveTopologyType(m_inputAssembler->topology());

		auto vertexLayouts = m_inputAssembler->vertexBufferLayouts();

		Array<D3D12_INPUT_ELEMENT_DESC> inputLayoutElements;
		std::ranges::for_each(m_inputAssembler->vertexBufferLayouts(), [&, l = 0](const DirectX12VertexBufferLayout* layout) mutable {
			auto bufferAttributes = layout->attributes();
			auto bindingPoint = layout->binding();

			LITEFX_TRACE(DIRECTX12_LOG, "Defining vertex buffer layout {0}/{1} {{ Attributes: {2}, Size: {3} bytes, Binding: {4} }}...", ++l, vertexLayouts.size(), bufferAttributes.size(), layout->elementSize(), bindingPoint);

			std::ranges::for_each(bufferAttributes, [&](const BufferAttribute* attribute) {
				D3D12_INPUT_ELEMENT_DESC elementDescriptor = {};
				elementDescriptor.SemanticName = ::getSemanticName(attribute->semantic());
				elementDescriptor.SemanticIndex = attribute->semanticIndex();
				elementDescriptor.Format = ::getFormat(attribute->format());
				elementDescriptor.InputSlot = bindingPoint;
				elementDescriptor.InputSlotClass = D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				elementDescriptor.AlignedByteOffset = attribute->offset();	// TODO: May not include packing, but packing is required - need to test this!
				elementDescriptor.InstanceDataStepRate = 0;

				inputLayoutElements.push_back(elementDescriptor);
			});
		});

		D3D12_INPUT_LAYOUT_DESC inputLayout = {};
		inputLayout.pInputElementDescs = inputLayoutElements.data();
		inputLayout.NumElements = static_cast<UInt32>(inputLayoutElements.size());

		// Setup multisampling state.
		// TODO: Abstract me!
		//device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS);
		DXGI_SAMPLE_DESC multisamplingState = {};
		multisamplingState.Count = 1;
		multisamplingState.Quality = 0;

		// Setup render target states.
		// NOTE: We assume, that the targets are returned sorted by location and the location range is contiguous.
		// TODO: Add blend parameters to render target.
		D3D12_BLEND_DESC blendState = {};
		D3D12_DEPTH_STENCIL_DESC depthStencilState = {};
		auto targets = m_parent->parent().renderTargets();
		pipelineStateDescription.NumRenderTargets = std::ranges::count_if(targets, [](const RenderTarget& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; });
		UInt32 depthStencilTargets = static_cast<UInt32>(targets.size()) - pipelineStateDescription.NumRenderTargets;

		// Only 8 RTVs are allowed.
		if (pipelineStateDescription.NumRenderTargets > 8)
			throw RuntimeException("You have specified too many render targets: only 8 render targets and 1 depth/stencil target are allowed, but {0} have been specified.", pipelineStateDescription.NumRenderTargets);

		// Only one DSV is allowed.
		if (depthStencilTargets > 1)
			throw RuntimeException("You have specified too many render targets: only 1 depth/stencil target is allowed, but {0} have been specified.", depthStencilTargets);

		std::ranges::for_each(targets, [&, i = 0](const RenderTarget& renderTarget) mutable {
			if (renderTarget.type() == RenderTargetType::DepthStencil)
			{
				// Setup depth/stencil format.
				pipelineStateDescription.DSVFormat = ::getFormat(renderTarget.format());

				// Setup depth/stencil state.
				// TODO: From depth/stencil state.
				D3D12_DEPTH_STENCIL_DESC depthStencilState = {};
				depthStencilState.DepthEnable = ::hasDepth(renderTarget.format());
				depthStencilState.StencilEnable = ::hasStencil(renderTarget.format());
				depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL;
				depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
			}
			else
			{
				// Setup target formats.
				UInt32 target = i++;
				pipelineStateDescription.RTVFormats[target] = ::getFormat(renderTarget.format());

				// Setup the blend state.
				auto& targetBlendState = blendState.RenderTarget[target];
				targetBlendState.BlendEnable = FALSE;
				targetBlendState.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL;
				targetBlendState.LogicOp = D3D12_LOGIC_OP::D3D12_LOGIC_OP_COPY;
				targetBlendState.LogicOpEnable = FALSE;

			}
		});

		blendState.AlphaToCoverageEnable = FALSE;
		blendState.IndependentBlendEnable = TRUE;

		// Setup shader stages.
		auto modules = m_layout->program().modules();
		LITEFX_TRACE(DIRECTX12_LOG, "Using shader program {0} with {1} modules...", fmt::ptr(&m_layout->program()), modules.size());

		std::ranges::for_each(modules, [&, i = 0](const DirectX12ShaderModule* shaderModule) mutable {
			LITEFX_TRACE(DIRECTX12_LOG, "\tModule {0}/{1} (\"{2}\") state: {{ Type: {3}, EntryPoint: {4} }}", ++i, modules.size(), shaderModule->fileName(), shaderModule->type(), shaderModule->entryPoint());

			switch (shaderModule->type())
			{
			case ShaderStage::Vertex:
				pipelineStateDescription.VS = shaderModule->bytecode();
				break;
			case ShaderStage::TessellationControl:		// aka. Hull Shader
				pipelineStateDescription.HS = shaderModule->bytecode();
				break;
			case ShaderStage::TessellationEvaluation:	// aka. Domain Shader
				pipelineStateDescription.DS = shaderModule->bytecode();
				break;
			case ShaderStage::Geometry:
				pipelineStateDescription.GS = shaderModule->bytecode();
				break;
			case ShaderStage::Fragment:
				pipelineStateDescription.PS = shaderModule->bytecode();
				break;
			default:
				throw InvalidArgumentException("Trying to bind shader to unsupported shader stage '{0}'.", shaderModule->type());
			}
		});

		// Create a pipeline state description.
		pipelineStateDescription.RasterizerState = rasterizerState;
		pipelineStateDescription.PrimitiveTopologyType = topologyType;
		pipelineStateDescription.InputLayout = inputLayout;
		pipelineStateDescription.SampleDesc = multisamplingState;
		pipelineStateDescription.BlendState = blendState;
		pipelineStateDescription.DepthStencilState = depthStencilState;
		pipelineStateDescription.pRootSignature = std::as_const(*m_layout).handle().Get();

		// Create the pipeline state instance.
		ComPtr<ID3D12PipelineState> pipelineState;
		raiseIfFailed<RuntimeException>(m_parent->getDevice()->handle()->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState)), "Unable to create render pipeline state.");

		return pipelineState;
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipeline::DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, const UInt32& id, UniquePtr<DirectX12RenderPipelineLayout>&& layout, SharedPtr<DirectX12InputAssembler>&& inputAssembler, SharedPtr<DirectX12Rasterizer>&& rasterizer, Array<SharedPtr<IViewport>>&& viewports, Array<SharedPtr<IScissor>>&& scissors, const String& name) :
	m_impl(makePimpl<DirectX12RenderPipelineImpl>(this, id, name, std::move(layout), std::move(inputAssembler), std::move(rasterizer), std::move(viewports), std::move(scissors))), DirectX12RuntimeObject<DirectX12RenderPass>(renderPass, renderPass.getDevice()), ComResource<ID3D12PipelineState>(nullptr)
{
	this->handle() = m_impl->initialize();
}

DirectX12RenderPipeline::DirectX12RenderPipeline(const DirectX12RenderPass& renderPass) noexcept :
	m_impl(makePimpl<DirectX12RenderPipelineImpl>(this)), DirectX12RuntimeObject<DirectX12RenderPass>(renderPass, renderPass.getDevice()), ComResource<ID3D12PipelineState>(nullptr)
{
}

DirectX12RenderPipeline::~DirectX12RenderPipeline() noexcept = default;

const String& DirectX12RenderPipeline::name() const noexcept
{
	return m_impl->m_name;
}

const UInt32& DirectX12RenderPipeline::id() const noexcept
{
	return m_impl->m_id;
}

const DirectX12RenderPipelineLayout& DirectX12RenderPipeline::layout() const noexcept
{
	return *m_impl->m_layout;
}

SharedPtr<DirectX12InputAssembler> DirectX12RenderPipeline::inputAssembler() const noexcept
{
	return m_impl->m_inputAssembler;
}

SharedPtr<IRasterizer> DirectX12RenderPipeline::rasterizer() const noexcept
{
	return m_impl->m_rasterizer;
}

Array<const IViewport*> DirectX12RenderPipeline::viewports() const noexcept
{
	return m_impl->m_viewports |
		std::views::transform([](const SharedPtr<IViewport>& viewport) { return viewport.get(); }) |
		ranges::to<Array<const IViewport*>>();
}

Array<const IScissor*> DirectX12RenderPipeline::scissors() const noexcept
{
	return m_impl->m_scissors |
		std::views::transform([](const SharedPtr<IScissor>& scissor) { return scissor.get(); }) |
		ranges::to<Array<const IScissor*>>();
}

void DirectX12RenderPipeline::bind(const IDirectX12VertexBuffer& buffer) const
{
	const auto& commandBuffer = this->parent().activeFrameBuffer().commandBuffer();

	// Transition the buffer to the appropriate state, if it isn't already.
	if (buffer.state() != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
		buffer.transitionTo(commandBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	
	commandBuffer.handle()->IASetVertexBuffers(buffer.binding(), 1, &buffer.view());
	commandBuffer.handle()->IASetPrimitiveTopology(::getPrimitiveTopology(m_impl->m_inputAssembler->topology()));
}

void DirectX12RenderPipeline::bind(const IDirectX12IndexBuffer& buffer) const
{
	const auto& commandBuffer = this->parent().activeFrameBuffer().commandBuffer();

	// Transition the buffer to the appropriate state, if it isn't already.
	if (buffer.state() != D3D12_RESOURCE_STATE_INDEX_BUFFER)
		buffer.transitionTo(commandBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	commandBuffer.handle()->IASetIndexBuffer(&buffer.view());
}

void DirectX12RenderPipeline::bind(const DirectX12DescriptorSet& descriptorSet) const
{
	// TODO: Copy descriptors to global descriptor heap using CopyDescriptorsSimple (https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-copydescriptorssimple).
	throw;
}

void DirectX12RenderPipeline::use() const
{
	// TODO: Check if the arguments are right, or if they are in absolute coordinates.
	auto viewports = m_impl->m_viewports |
		std::views::transform([](const SharedPtr<IViewport>& viewport) { return CD3DX12_VIEWPORT(viewport->getRectangle().x(), viewport->getRectangle().y(), viewport->getRectangle().width(), viewport->getRectangle().height(), viewport->getMinDepth(), viewport->getMaxDepth()); }) |
		ranges::to<Array<D3D12_VIEWPORT>>();

	auto scissors= m_impl->m_scissors |
		std::views::transform([](const SharedPtr<IScissor>& scissor) { return CD3DX12_RECT(scissor->getRectangle().x(), scissor->getRectangle().y(), scissor->getRectangle().width(), scissor->getRectangle().height()); }) |
		ranges::to<Array<D3D12_RECT>>();

	const auto& commandBuffer = this->parent().activeFrameBuffer().commandBuffer();
	commandBuffer.handle()->SetPipelineState(this->handle().Get());
	commandBuffer.handle()->RSSetViewports(viewports.size(), viewports.data());
	commandBuffer.handle()->RSSetScissorRects(scissors.size(), scissors.data());
}

void DirectX12RenderPipeline::draw(const UInt32& vertices, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const
{
	throw;
}

void DirectX12RenderPipeline::drawIndexed(const UInt32& indices, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const
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
	UniquePtr<DirectX12RenderPipelineLayout> m_layout;
	SharedPtr<DirectX12InputAssembler> m_inputAssembler;
	SharedPtr<DirectX12Rasterizer> m_rasterizer;
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

DirectX12RenderPipelineBuilder::DirectX12RenderPipelineBuilder(const DirectX12RenderPass& renderPass, const UInt32& id, const String& name) :
	m_impl(makePimpl<DirectX12RenderPipelineBuilderImpl>(this)), RenderPipelineBuilder(UniquePtr<DirectX12RenderPipeline>(new DirectX12RenderPipeline(renderPass)))
{
	this->instance()->m_impl->m_id = id;
	this->instance()->m_impl->m_name = name;
}

DirectX12RenderPipelineBuilder::~DirectX12RenderPipelineBuilder() noexcept = default;

UniquePtr<DirectX12RenderPipeline> DirectX12RenderPipelineBuilder::go()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = std::move(m_impl->m_layout);
	instance->m_impl->m_inputAssembler = std::move(m_impl->m_inputAssembler);
	instance->m_impl->m_rasterizer = std::move(m_impl->m_rasterizer);
	instance->m_impl->m_viewports = std::move(m_impl->m_viewports);
	instance->m_impl->m_scissors = std::move(m_impl->m_scissors);
	instance->handle() = instance->m_impl->initialize();

	return RenderPipelineBuilder::go();
}

void DirectX12RenderPipelineBuilder::use(UniquePtr<DirectX12RenderPipelineLayout>&& layout)
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

	auto dx12Rasterizer = std::dynamic_pointer_cast<DirectX12Rasterizer>(rasterizer);

	if (dx12Rasterizer == nullptr)
		throw InvalidArgumentException("The provided rasterizer must be a DirectX12 rasterizer instance.");

	m_impl->m_rasterizer = dx12Rasterizer;
}

void DirectX12RenderPipelineBuilder::use(SharedPtr<DirectX12InputAssembler> inputAssembler)
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
	return DirectX12RenderPipelineLayoutBuilder(*this);
}

DirectX12RasterizerBuilder DirectX12RenderPipelineBuilder::rasterizer()
{
	return DirectX12RasterizerBuilder(*this);
}

DirectX12InputAssemblerBuilder DirectX12RenderPipelineBuilder::inputAssembler()
{
	return DirectX12InputAssemblerBuilder(*this);
}

DirectX12RenderPipelineBuilder& DirectX12RenderPipelineBuilder::withRasterizer(SharedPtr<IRasterizer> rasterizer)
{
	this->use(std::move(rasterizer));
	return *this;
}

DirectX12RenderPipelineBuilder& DirectX12RenderPipelineBuilder::withInputAssembler(SharedPtr<DirectX12InputAssembler> inputAssembler)
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