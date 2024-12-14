#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPipeline::DirectX12RenderPipelineImpl {
public:
	friend class DirectX12RenderPipelineBuilder;
	friend class DirectX12RenderPipeline;

private:
	SharedPtr<const DirectX12RenderPass> m_renderPass;
	SharedPtr<DirectX12PipelineLayout> m_layout;
	SharedPtr<DirectX12ShaderProgram> m_program;
	SharedPtr<DirectX12InputAssembler> m_inputAssembler;
	SharedPtr<DirectX12Rasterizer> m_rasterizer;
	Vector4f m_blendFactors{ 0.f, 0.f, 0.f, 0.f };
	UInt32 m_stencilRef{ 0 };
	bool m_alphaToCoverage{ false };
	MultiSamplingLevel m_samples{ MultiSamplingLevel::x1 };
	SharedPtr<IDirectX12Sampler> m_inputAttachmentSampler;
	Dictionary<const IFrameBuffer*, Array<UniquePtr<DirectX12DescriptorSet>>> m_inputAttachmentBindings;
	Dictionary<const IFrameBuffer*, size_t> m_frameBufferResizeTokens, m_frameBufferReleaseTokens;
	mutable std::mutex m_usageMutex;

public:
	DirectX12RenderPipelineImpl(const DirectX12RenderPass& renderPass, bool alphaToCoverage, const SharedPtr<DirectX12PipelineLayout>& layout, const SharedPtr<DirectX12ShaderProgram>& shaderProgram, const SharedPtr<DirectX12InputAssembler>& inputAssembler, const SharedPtr<DirectX12Rasterizer>& rasterizer) :
		m_renderPass(renderPass.shared_from_this()), m_layout(layout), m_program(shaderProgram), m_inputAssembler(inputAssembler), m_rasterizer(rasterizer), m_alphaToCoverage(alphaToCoverage)
	{
		auto device = renderPass.device();

		if (renderPass.inputAttachmentSamplerBinding().has_value())
			m_inputAttachmentSampler = device->factory().createSampler();
	}

	DirectX12RenderPipelineImpl(const DirectX12RenderPass& renderPass) :
		m_renderPass(renderPass.shared_from_this())
	{
		auto device = renderPass.device();

		if (renderPass.inputAttachmentSamplerBinding().has_value())
			m_inputAttachmentSampler = device->factory().createSampler();
	}

	~DirectX12RenderPipelineImpl() noexcept
	{
		// Stop listening to frame buffer events.
		for (auto [frameBuffer, token] : m_frameBufferResizeTokens)
			frameBuffer->resized -= token;

		for (auto [frameBuffer, token] : m_frameBufferReleaseTokens)
			frameBuffer->released -= token;
	}

	DirectX12RenderPipelineImpl(DirectX12RenderPipelineImpl&&) noexcept = delete;
	DirectX12RenderPipelineImpl(const DirectX12RenderPipelineImpl&) = delete;
	DirectX12RenderPipelineImpl& operator=(DirectX12RenderPipelineImpl&&) noexcept = delete;
	DirectX12RenderPipelineImpl& operator=(const DirectX12RenderPipelineImpl&) = delete;

public:
	ComPtr<ID3D12PipelineState> initialize(const DirectX12RenderPipeline& pipeline, MultiSamplingLevel samples)
	{
		LITEFX_TRACE(DIRECTX12_LOG, "Creating render pipeline \"{1}\" for layout {0}...", static_cast<void*>(m_layout.get()), pipeline.name());
		m_samples = samples;

		// Check if there are mesh shaders in the program.
		auto modules = m_program->modules();
		bool hasMeshShaders = std::ranges::find_if(modules, [](const auto& module) { return LITEFX_FLAG_IS_SET(ShaderStage::Mesh | ShaderStage::Task, module->type()); }) != modules.end();

		// Setup rasterizer state.
		auto& rasterizer = std::as_const(*m_rasterizer.get());
		D3D12_RASTERIZER_DESC rasterizerState = {};
		rasterizerState.DepthClipEnable = FALSE;
		rasterizerState.FillMode = DX12::getPolygonMode(rasterizer.polygonMode());
		rasterizerState.CullMode = DX12::getCullMode(rasterizer.cullMode());
		rasterizerState.FrontCounterClockwise = rasterizer.cullOrder() == CullOrder::CounterClockWise;
		rasterizerState.MultisampleEnable = FALSE;
		rasterizerState.AntialiasedLineEnable = FALSE;
		rasterizerState.ForcedSampleCount = 0;
		rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		LITEFX_TRACE(DIRECTX12_LOG, "Rasterizer state: {{ PolygonMode: {0}, CullMode: {1}, CullOrder: {2}, LineWidth: {3} }}", rasterizer.polygonMode(), rasterizer.cullMode(), rasterizer.cullOrder(), rasterizer.lineWidth());

		if (!rasterizer.depthStencilState().depthState().Enable)
			LITEFX_TRACE(DIRECTX12_LOG, "\tRasterizer depth bias disabled.");
		else
		{
			LITEFX_TRACE(DIRECTX12_LOG, "\tRasterizer depth bias: {{ Clamp: {0}, ConstantFactor: {1}, SlopeFactor: {2} }}", rasterizer.depthStencilState().depthBias().Clamp, rasterizer.depthStencilState().depthBias().ConstantFactor, rasterizer.depthStencilState().depthBias().SlopeFactor);
			rasterizerState.DepthBiasClamp = rasterizer.depthStencilState().depthBias().Clamp;
			rasterizerState.DepthBias = static_cast<Int32>(rasterizer.depthStencilState().depthBias().ConstantFactor);
			rasterizerState.SlopeScaledDepthBias = rasterizer.depthStencilState().depthBias().SlopeFactor;
		}

		// Setup input assembler state.
		Array<D3D12_INPUT_ELEMENT_DESC> inputLayoutElements;
		
		LITEFX_TRACE(DIRECTX12_LOG, "Input assembler state: {{ PrimitiveTopology: {0} }}", m_inputAssembler->topology());
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = DX12::getPrimitiveTopologyType(m_inputAssembler->topology());

		auto vertexLayouts = m_inputAssembler->vertexBufferLayouts();

		std::ranges::for_each(m_inputAssembler->vertexBufferLayouts(), [&, l = 0](const DirectX12VertexBufferLayout* layout) mutable {
			auto bufferAttributes = layout->attributes();
			auto bindingPoint = layout->binding();

#ifdef NDEBUG
			(void)l; // Required as [[maybe_unused]] is not supported in captures.
#else
			LITEFX_TRACE(DIRECTX12_LOG, "Defining vertex buffer layout {0}/{1} {{ Attributes: {2}, Size: {3} bytes, Binding: {4} }}...", ++l, vertexLayouts.size(), bufferAttributes.size(), layout->elementSize(), bindingPoint);
#endif

			std::ranges::for_each(bufferAttributes, [&](const BufferAttribute* attribute) {
				D3D12_INPUT_ELEMENT_DESC elementDescriptor = {};
				elementDescriptor.SemanticName = DX12::getSemanticName(attribute->semantic());
				elementDescriptor.SemanticIndex = attribute->semanticIndex();
				elementDescriptor.Format = DX12::getFormat(attribute->format());
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

		// Setup multi-sampling state.
		DXGI_SAMPLE_DESC multisamplingState = samples == MultiSamplingLevel::x1 ? DXGI_SAMPLE_DESC{ 1, 0 } : DXGI_SAMPLE_DESC{ std::to_underlying(samples), DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN };

		// Setup render target states.
		// NOTE: We assume, that the targets are returned sorted by location and the location range is contiguous.
		D3D12_BLEND_DESC blendState = {};
		D3D12_DEPTH_STENCIL_DESC depthStencilState = {};
		auto targets = m_renderPass->renderTargets();
		UInt32 renderTargets = static_cast<UInt32>(std::ranges::count_if(targets, [](auto& renderTarget) { return renderTarget.type() != RenderTargetType::DepthStencil; }));
		UInt32 depthStencilTargets = static_cast<UInt32>(targets.size()) - renderTargets;
		DXGI_FORMAT dsvFormat { };
		std::array<DXGI_FORMAT, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> rtvFormats { };

		// Only 8 RTVs are allowed.
		if (renderTargets > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT) [[unlikely]]
			throw RuntimeException("You have specified too many render targets: only 8 render targets and 1 depth/stencil target are allowed, but {0} have been specified.", renderTargets);

		// Only one DSV is allowed.
		if (depthStencilTargets > 1) [[unlikely]]
			throw RuntimeException("You have specified too many render targets: only 1 depth/stencil target is allowed, but {0} have been specified.", depthStencilTargets);

		std::ranges::for_each(targets, [&, i = 0](auto& renderTarget) mutable {
			if (renderTarget.type() == RenderTargetType::DepthStencil)
			{
				// Setup depth/stencil format.
				dsvFormat = DX12::getFormat(renderTarget.format());

				// Setup depth/stencil state.
				depthStencilState.DepthEnable = rasterizer.depthStencilState().depthState().Enable;
				depthStencilState.DepthWriteMask = rasterizer.depthStencilState().depthState().Write ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
				depthStencilState.DepthFunc = DX12::getCompareOp(rasterizer.depthStencilState().depthState().Operation);

				depthStencilState.StencilEnable = rasterizer.depthStencilState().stencilState().Enable;
				depthStencilState.StencilReadMask = rasterizer.depthStencilState().stencilState().ReadMask;
				depthStencilState.StencilWriteMask = rasterizer.depthStencilState().stencilState().WriteMask;
				depthStencilState.FrontFace.StencilFunc = DX12::getCompareOp(rasterizer.depthStencilState().stencilState().FrontFace.Operation);
				depthStencilState.FrontFace.StencilDepthFailOp = DX12::getStencilOp(rasterizer.depthStencilState().stencilState().FrontFace.DepthFailOp);
				depthStencilState.FrontFace.StencilFailOp = DX12::getStencilOp(rasterizer.depthStencilState().stencilState().FrontFace.StencilFailOp);
				depthStencilState.FrontFace.StencilPassOp = DX12::getStencilOp(rasterizer.depthStencilState().stencilState().FrontFace.StencilPassOp);
				depthStencilState.BackFace.StencilFunc = DX12::getCompareOp(rasterizer.depthStencilState().stencilState().BackFace.Operation);
				depthStencilState.BackFace.StencilDepthFailOp = DX12::getStencilOp(rasterizer.depthStencilState().stencilState().BackFace.DepthFailOp);
				depthStencilState.BackFace.StencilFailOp = DX12::getStencilOp(rasterizer.depthStencilState().stencilState().BackFace.StencilFailOp);
				depthStencilState.BackFace.StencilPassOp = DX12::getStencilOp(rasterizer.depthStencilState().stencilState().BackFace.StencilPassOp);
			}
			else
			{
				// Setup target formats.
				UInt32 target = i++;
				rtvFormats[target] = DX12::getFormat(renderTarget.format()); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)

				// Setup the blend state.
				auto& targetBlendState = blendState.RenderTarget[target];
				targetBlendState.BlendEnable = renderTarget.blendState().Enable;
				targetBlendState.RenderTargetWriteMask = static_cast<UINT8>(renderTarget.blendState().ChannelWriteMask);
				targetBlendState.SrcBlend = DX12::getBlendFactor(renderTarget.blendState().SourceColor);
				targetBlendState.SrcBlendAlpha = DX12::getBlendFactor(renderTarget.blendState().SourceAlpha);
				targetBlendState.DestBlend = DX12::getBlendFactor(renderTarget.blendState().DestinationColor);
				targetBlendState.DestBlendAlpha = DX12::getBlendFactor(renderTarget.blendState().DestinationAlpha);
				targetBlendState.BlendOp = DX12::getBlendOperation(renderTarget.blendState().ColorOperation);
				targetBlendState.BlendOpAlpha = DX12::getBlendOperation(renderTarget.blendState().AlphaOperation);

				// TODO: We should also implement this, but this restricts all blend states to be equal and IndependentBlendEnable set to false.
				targetBlendState.LogicOp = D3D12_LOGIC_OP::D3D12_LOGIC_OP_COPY;
				targetBlendState.LogicOpEnable = FALSE;
			}
		});

		blendState.AlphaToCoverageEnable = m_alphaToCoverage;
		blendState.IndependentBlendEnable = TRUE;

		// Initialize the remainder depending on the pipeline type.
		if (hasMeshShaders)
			return this->initializeMeshPipeline(pipeline, blendState, rasterizerState, depthStencilState, topologyType, renderTargets, rtvFormats, dsvFormat, multisamplingState);
		else
			return this->initializeGraphicsPipeline(pipeline, blendState, rasterizerState, depthStencilState, inputLayout, topologyType, renderTargets, rtvFormats, dsvFormat, multisamplingState);

	}

	ComPtr<ID3D12PipelineState> initializeMeshPipeline([[maybe_unused]] const DirectX12RenderPipeline& pipeline, const D3D12_BLEND_DESC& blendState, const D3D12_RASTERIZER_DESC& rasterizerState, const D3D12_DEPTH_STENCIL_DESC& depthStencilState, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType, UINT renderTargets, const std::array<DXGI_FORMAT, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT>& renderTargetFormats, DXGI_FORMAT depthStencilFormat, const DXGI_SAMPLE_DESC& multisamplingState)
	{
		// Create a pipeline state description.
		D3DX12_MESH_SHADER_PIPELINE_STATE_DESC pipelineStateDescription = {
			.pRootSignature = std::as_const(*m_layout).handle().Get(),
			.BlendState = blendState,
			.SampleMask = std::numeric_limits<UInt32>::max(),
			.RasterizerState = rasterizerState,
			.DepthStencilState = depthStencilState,
			.PrimitiveTopologyType = topologyType,
			.NumRenderTargets = renderTargets,
			.DSVFormat = depthStencilFormat,
			.SampleDesc = multisamplingState
		};

		std::memcpy(&pipelineStateDescription.RTVFormats, renderTargetFormats.data(), renderTargetFormats.size() * sizeof(DXGI_FORMAT));

		// Setup shader stages.
		auto modules = m_program->modules();
		LITEFX_TRACE(DIRECTX12_LOG, "Using shader program {0} with {1} modules...", static_cast<void*>(m_program.get()), modules.size());

		std::ranges::for_each(modules, [&, i = 0](const DirectX12ShaderModule* shaderModule) mutable {
#ifdef NDEBUG
			(void)i; // Required as [[maybe_unused]] is not supported in captures.
#else
			LITEFX_TRACE(DIRECTX12_LOG, "\tModule {0}/{1} (\"{2}\") state: {{ Type: {3}, EntryPoint: {4} }}", ++i, modules.size(), shaderModule->fileName(), shaderModule->type(), shaderModule->entryPoint());
#endif

			switch (shaderModule->type())
			{
			case ShaderStage::Fragment:
				pipelineStateDescription.PS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
				pipelineStateDescription.PS.BytecodeLength = shaderModule->handle()->GetBufferSize();
				break;
			case ShaderStage::Task:
				pipelineStateDescription.AS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
				pipelineStateDescription.AS.BytecodeLength = shaderModule->handle()->GetBufferSize();
				break;
			case ShaderStage::Mesh:
				pipelineStateDescription.MS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
				pipelineStateDescription.MS.BytecodeLength = shaderModule->handle()->GetBufferSize();
				break;
			default:
				throw InvalidArgumentException("shaderProgram", "Trying to bind shader to unsupported shader stage '{0}'.", shaderModule->type());
			}
		});

		CD3DX12_PIPELINE_STATE_STREAM2 streamDesc(pipelineStateDescription);
		D3D12_PIPELINE_STATE_STREAM_DESC pipelineDesc = {
			.SizeInBytes = sizeof(streamDesc),
			.pPipelineStateSubobjectStream = &streamDesc
		};

		// Create the pipeline state instance.
		ComPtr<ID3D12PipelineState> pipelineState;
		raiseIfFailed(m_renderPass->device()->handle()->CreatePipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState)), "Unable to create render pipeline state.");
		
#ifndef NDEBUG
		pipelineState->SetName(Widen(pipeline.name()).c_str());
#endif

		return pipelineState;
	}

	ComPtr<ID3D12PipelineState> initializeGraphicsPipeline([[maybe_unused]] const DirectX12RenderPipeline& pipeline, const D3D12_BLEND_DESC& blendState, const D3D12_RASTERIZER_DESC& rasterizerState, const D3D12_DEPTH_STENCIL_DESC& depthStencilState, const D3D12_INPUT_LAYOUT_DESC& inputLayout, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType, UINT renderTargets, const std::array<DXGI_FORMAT, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT>& renderTargetFormats, DXGI_FORMAT depthStencilFormat, const DXGI_SAMPLE_DESC& multisamplingState)
	{
		// Create a pipeline state description.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescription = {
			.pRootSignature = std::as_const(*m_layout).handle().Get(),
			.BlendState = blendState,
			.SampleMask = std::numeric_limits<UInt32>::max(),
			.RasterizerState = rasterizerState,
			.DepthStencilState = depthStencilState,
			.InputLayout = inputLayout,
			.PrimitiveTopologyType = topologyType,
			.NumRenderTargets = renderTargets,
			.DSVFormat = depthStencilFormat,
			.SampleDesc = multisamplingState
		};

		std::memcpy(&pipelineStateDescription.RTVFormats, renderTargetFormats.data(), renderTargetFormats.size() * sizeof(DXGI_FORMAT));
		
		// Setup shader stages.
		auto modules = m_program->modules();
		LITEFX_TRACE(DIRECTX12_LOG, "Using shader program {0} with {1} modules...", static_cast<void*>(m_program.get()), modules.size());

		std::ranges::for_each(modules, [&, i = 0](const DirectX12ShaderModule* shaderModule) mutable {
#ifdef NDEBUG
			(void)i; // Required as [[maybe_unused]] is not supported in captures.
#else
			LITEFX_TRACE(DIRECTX12_LOG, "\tModule {0}/{1} (\"{2}\") state: {{ Type: {3}, EntryPoint: {4} }}", ++i, modules.size(), shaderModule->fileName(), shaderModule->type(), shaderModule->entryPoint());
#endif

			switch (shaderModule->type())
			{
			case ShaderStage::Vertex:
				pipelineStateDescription.VS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
				pipelineStateDescription.VS.BytecodeLength = shaderModule->handle()->GetBufferSize();
				break;
			case ShaderStage::TessellationControl:		// aka. Hull Shader
				pipelineStateDescription.HS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
				pipelineStateDescription.HS.BytecodeLength = shaderModule->handle()->GetBufferSize();
				break;
			case ShaderStage::TessellationEvaluation:	// aka. Domain Shader
				pipelineStateDescription.DS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
				pipelineStateDescription.DS.BytecodeLength = shaderModule->handle()->GetBufferSize();
				break;
			case ShaderStage::Geometry:
				pipelineStateDescription.GS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
				pipelineStateDescription.GS.BytecodeLength = shaderModule->handle()->GetBufferSize();
				break;
			case ShaderStage::Fragment:
				pipelineStateDescription.PS.pShaderBytecode = shaderModule->handle()->GetBufferPointer();
				pipelineStateDescription.PS.BytecodeLength = shaderModule->handle()->GetBufferSize();
				break;
			default:
				throw InvalidArgumentException("shaderProgram", "Trying to bind shader to unsupported shader stage '{0}'.", shaderModule->type());
			}
		});

		// Create the pipeline state instance.
		ComPtr<ID3D12PipelineState> pipelineState;
		raiseIfFailed(m_renderPass->device()->handle()->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineState)), "Unable to create render pipeline state.");
		
#ifndef NDEBUG
		pipelineState->SetName(Widen(pipeline.name()).c_str());
#endif

		return pipelineState;
	}

	void initializeInputAttachmentBindings(const DirectX12FrameBuffer& frameBuffer)
	{
		// Find out how many descriptor sets there are within the input attachments and which descriptors are bound.
		Dictionary<UInt32, Array<UInt32>> descriptorsPerSet;
		std::ranges::for_each(m_renderPass->inputAttachments(), [&descriptorsPerSet](auto& dependency) { descriptorsPerSet[dependency.binding().Space].push_back(dependency.binding().Register); });

		// Validate the descriptor sets, so that no descriptors are bound twice all descriptor sets are fully bound.
		for (auto& [set, descriptors] : descriptorsPerSet)
		{
			// Sort and check if there are duplicates.
			std::ranges::sort(descriptors);

			if (std::ranges::adjacent_find(descriptors) != descriptors.end()) [[unlikely]]
				throw RuntimeException("The descriptor set {0} has input attachment mappings that point to the same descriptor.", set);

			// Check if all descriptors in the set are mapped.
			auto& layout = m_layout->descriptorSet(set);

			for (auto& descriptor : layout.descriptors())
			{
				if (std::ranges::find(descriptors, descriptor->binding()) == descriptors.end()) [[unlikely]]
				{
					LITEFX_WARNING(DIRECTX12_LOG, "The descriptor set {0} is not fully mapped by the provided input attachments for the render pass.", set);
					break;
				}
			}
		}

		// Don't forget the sampler.
		auto& samplerBinding = m_renderPass->inputAttachmentSamplerBinding();

		if (samplerBinding.has_value())
		{
			auto space = samplerBinding.value().Space;
			auto layouts = m_layout->descriptorSets();

			if (auto samplerSet = std::ranges::find_if(layouts, [&](auto set) { return set->space() == space; }); samplerSet != layouts.end())
			{
				if (descriptorsPerSet.contains(space)) [[unlikely]]
					throw RuntimeException("The input attachment sampler is defined in a descriptor set that contains input attachment descriptors. Samplers must be defined within their own space.");

				// Store the descriptor so it gets bound.
				descriptorsPerSet[space].push_back(samplerBinding.value().Register);
			}
		}

		// Allocate the input attachment bindings.
		this->allocateInputAttachmentBindings(frameBuffer, descriptorsPerSet | std::views::keys);
	}

	void allocateInputAttachmentBindings(const DirectX12FrameBuffer& frameBuffer, const std::ranges::input_range auto& descriptorSets) requires 
		std::is_convertible_v<std::ranges::range_value_t<decltype(descriptorSets)>, UInt32>
	{
		// Allocate the bindings array.
		auto interfacePointer = static_cast<const IFrameBuffer*>(&frameBuffer);
		auto& bindings = m_inputAttachmentBindings[interfacePointer];

		// Initialize the descriptor set bindings.
		bindings.append_range(descriptorSets | std::views::transform([this](UInt32 set) { return std::move(m_layout->descriptorSet(set).allocate()); }));

		// Listen to frame buffer events and update the bindings or remove the sets (on release).
		m_frameBufferResizeTokens[interfacePointer] = frameBuffer.resized.add(std::bind(&DirectX12RenderPipelineImpl::onFrameBufferResize, this, std::placeholders::_1, std::placeholders::_2));
		m_frameBufferReleaseTokens[interfacePointer] = frameBuffer.released.add(std::bind(&DirectX12RenderPipelineImpl::onFrameBufferRelease, this, std::placeholders::_1, std::placeholders::_2));
	}

	void updateInputAttachmentBindings(const DirectX12FrameBuffer& frameBuffer)
	{
		// Get the interface pointer and obtain the descriptor sets for the input attachments.
		auto interfacePointer = static_cast<const IFrameBuffer*>(&frameBuffer);
		auto& bindings = m_inputAttachmentBindings.at(interfacePointer);

		// Iterate the dependencies and update the binding for each one.
		std::ranges::for_each(m_renderPass->inputAttachments(), [&](auto& dependency) {
			for (auto& binding : bindings)
			{
				if (binding->layout().space() == dependency.binding().Space)
				{
					// Resolve the image and update the binding.
					auto& image = frameBuffer[dependency.renderTarget()];

					if (image.samples() != m_samples) [[unlikely]]
						LITEFX_WARNING(DIRECTX12_LOG, "The image multi sampling level {0} does not match the render pipeline multi sampling state {1}.", image.samples(), m_samples);

					// Attach the image from the right frame buffer to the descriptor set.
					binding->update(dependency.binding().Register, image);
					break;
				}
			}
		});

		// If there's a sampler, bind it too.
		auto& inputAttachmentSamplerBinding = m_renderPass->inputAttachmentSamplerBinding();
		
		if (inputAttachmentSamplerBinding.has_value())
		{
			for (auto& binding : bindings)
			{
				if (binding->layout().space() == inputAttachmentSamplerBinding.value().Space)
				{
					binding->update(inputAttachmentSamplerBinding.value().Register, *m_inputAttachmentSampler);
					break;
				}
			}
		}
	}

	void bindInputAttachments(const DirectX12CommandBuffer& commandBuffer)
	{
		// If this is the first time, the current frame buffer is bound to the render pass, we need to allocate descriptors for it.
		auto frameBuffer = m_renderPass->activeFrameBuffer();

		if (frameBuffer == nullptr)
			throw RuntimeException("Cannot bind input attachments for inactive render pass.");

		auto interfacePointer = static_cast<const IFrameBuffer*>(frameBuffer.get());

		if (!m_inputAttachmentBindings.contains(interfacePointer))
		{
			// Allocate and update input attachment bindings.
			this->initializeInputAttachmentBindings(*frameBuffer);
			this->updateInputAttachmentBindings(*frameBuffer);
		}

		// Bind the input attachment sets.
		commandBuffer.bind(m_inputAttachmentBindings.at(interfacePointer) | std::views::transform([](auto& set) { return set.get(); }));
	}

	void onFrameBufferResize(const void* sender, const IFrameBuffer::ResizeEventArgs& /*args*/)
	{
		// Update the descriptors in the descriptor sets.
		// NOTE: No slicing here, as the event is always triggered by the frame buffer instance.
		auto frameBuffer = static_cast<const DirectX12FrameBuffer*>(sender);
		this->updateInputAttachmentBindings(*frameBuffer);
	}

	void onFrameBufferRelease(const void* sender, const IFrameBuffer::ReleasedEventArgs& /*args*/)
	{
		// Get the frame buffer pointer.
		auto interfacePointer = static_cast<const IFrameBuffer*>(sender);

		// Release the descriptor sets.
		m_inputAttachmentBindings.erase(interfacePointer);

		// Release the tokens.
		m_frameBufferReleaseTokens.erase(interfacePointer);
		m_frameBufferResizeTokens.erase(interfacePointer);
	}
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipeline::DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, const SharedPtr<DirectX12PipelineLayout>& layout, const SharedPtr<DirectX12ShaderProgram>& shaderProgram, const SharedPtr<DirectX12InputAssembler>& inputAssembler, const SharedPtr<DirectX12Rasterizer>& rasterizer, MultiSamplingLevel samples, bool enableAlphaToCoverage, const String& name) :
	DirectX12PipelineState(nullptr), m_impl(renderPass, enableAlphaToCoverage, layout, shaderProgram, inputAssembler, rasterizer)
{
	if (!name.empty())
		this->name() = name;

	this->handle() = m_impl->initialize(*this, samples);
}

DirectX12RenderPipeline::DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, const String& name) :
	DirectX12PipelineState(nullptr), m_impl(renderPass)
{
	if (!name.empty())
		this->name() = name;
}

//DirectX12RenderPipeline::DirectX12RenderPipeline(DirectX12RenderPipeline&&) noexcept = default;
//DirectX12RenderPipeline& DirectX12RenderPipeline::operator=(DirectX12RenderPipeline&&) noexcept = default;
DirectX12RenderPipeline::~DirectX12RenderPipeline() noexcept = default;

SharedPtr<const DirectX12ShaderProgram> DirectX12RenderPipeline::program() const noexcept
{
	return m_impl->m_program;
}

SharedPtr<const DirectX12PipelineLayout> DirectX12RenderPipeline::layout() const noexcept
{
	return m_impl->m_layout;
}

SharedPtr<DirectX12InputAssembler> DirectX12RenderPipeline::inputAssembler() const noexcept
{
	return m_impl->m_inputAssembler;
}

SharedPtr<DirectX12Rasterizer> DirectX12RenderPipeline::rasterizer() const noexcept
{
	return m_impl->m_rasterizer;
}

bool DirectX12RenderPipeline::alphaToCoverage() const noexcept
{
	return m_impl->m_alphaToCoverage;
}

MultiSamplingLevel DirectX12RenderPipeline::samples() const noexcept
{
	return m_impl->m_samples;
}

void DirectX12RenderPipeline::updateSamples(MultiSamplingLevel samples)
{
	// Release all frame buffer bindings.
	m_impl->m_inputAttachmentBindings.clear();

	// Release current pipeline state.
	this->handle().Reset();

	// Rebuild the pipeline.
	this->handle() = m_impl->initialize(*this, samples);
}

void DirectX12RenderPipeline::use(const DirectX12CommandBuffer& commandBuffer) const
{
	// Set the pipeline state.
	commandBuffer.handle()->SetPipelineState(this->handle().Get());
	commandBuffer.handle()->SetGraphicsRootSignature(std::as_const(*m_impl->m_layout).handle().Get());
	commandBuffer.handle()->IASetPrimitiveTopology(DX12::getPrimitiveTopology(m_impl->m_inputAssembler->topology()));

	// NOTE: The same pipeline can be used from multiple multi-threaded command buffers, in which case we need to prevent multiple threads 
	//       from attempting to initialize the bindings on first use.
	std::lock_guard<std::mutex> lock(m_impl->m_usageMutex);

	// Bind all the input attachments for the parent render pass.
	m_impl->bindInputAttachments(commandBuffer);
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipelineBuilder::DirectX12RenderPipelineBuilder(const DirectX12RenderPass& renderPass, const String& name) :
	RenderPipelineBuilder(UniquePtr<DirectX12RenderPipeline>(new DirectX12RenderPipeline(renderPass)))
{
	this->instance()->name() = name;
}

DirectX12RenderPipelineBuilder::~DirectX12RenderPipelineBuilder() noexcept = default;

void DirectX12RenderPipelineBuilder::build()
{
	auto instance = this->instance();
	instance->m_impl->m_layout = this->state().pipelineLayout;
	instance->m_impl->m_program = this->state().shaderProgram;
	instance->m_impl->m_inputAssembler = this->state().inputAssembler;
	instance->m_impl->m_rasterizer = this->state().rasterizer;
	instance->m_impl->m_alphaToCoverage = this->state().enableAlphaToCoverage;
	instance->handle() = instance->m_impl->initialize(*instance, this->state().samples);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)