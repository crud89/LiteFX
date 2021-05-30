#pragma once

#include <litefx/rendering.hpp>

#include "dx12_api.hpp"
#include "dx12_builders.hpp"
#include "dx12_formatters.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Records commands for a <see cref="DirectX12CommandQueue" />
	/// </summary>
	/// <seealso cref="DirectX12CommandQueue" />
	class LITEFX_DIRECTX12_API DirectX12CommandBuffer : public ICommandBuffer, public DirectX12RuntimeObject<DirectX12Queue>, public ComResource<ID3D12GraphicsCommandList4> {
		LITEFX_IMPLEMENTATION(DirectX12CommandBufferImpl);

	public:
		/// <summary>
		/// Initializes the command buffer from a command queue.
		/// </summary>
		/// <param name="queue">The parent command queue, the buffer gets submitted to.</param>
		/// <param name="begin">If set to <c>true</c>, the command buffer automatically starts recording by calling <see cref="begin" />.</param>
		explicit DirectX12CommandBuffer(const DirectX12Queue& queue, const bool& begin = false);
		DirectX12CommandBuffer(const DirectX12CommandBuffer&) = delete;
		DirectX12CommandBuffer(DirectX12CommandBuffer&&) = delete;
		virtual ~DirectX12CommandBuffer() noexcept;

		// ICommandBuffer interface.
	public:
		/// <inheritdoc />
		virtual void wait() const override;

		/// <inheritdoc />
		virtual void begin() const override;

		/// <inheritdoc />
		virtual void end(const bool& submit = true, const bool& wait = false) const override;

		/// <inheritdoc />
		virtual void submit(const bool& wait = false) const override;
	};

	/// <summary>
	/// Implements a DirectX12 vertex buffer layout.
	/// </summary>
	/// <seealso cref="DirectX12VertexBufferLayoutBuilder" />
	/// <seealso cref="DirectX12VertexBuffer" />
	class LITEFX_DIRECTX12_API DirectX12VertexBufferLayout : public virtual DirectX12RuntimeObject<DirectX12InputAssembler>, public IVertexBufferLayout {
		LITEFX_IMPLEMENTATION(DirectX12VertexBufferLayoutImpl);
		LITEFX_BUILDER(DirectX12VertexBufferLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a new vertex buffer layout.
		/// </summary>
		/// <param name="inputAssembler">The parent input assembler state, the vertex buffer layout is initialized for.</param>
		/// <param name="vertexSize">The size of a single vertex.</param>
		/// <param name="binding">The binding point of the vertex buffers using this layout.</param>
		explicit DirectX12VertexBufferLayout(const DirectX12InputAssembler& inputAssembler, const size_t& vertexSize, const UInt32& binding = 0);
		DirectX12VertexBufferLayout(DirectX12VertexBufferLayout&&) = delete;
		DirectX12VertexBufferLayout(const DirectX12VertexBufferLayout&) = delete;
		virtual ~DirectX12VertexBufferLayout() noexcept;

		// IVertexBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual Array<const BufferAttribute*> attributes() const noexcept override;

		// IBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;
	};

	/// <summary>
	/// Builds a see <cref="DirectX12VertexBufferLayout" />.
	/// </summary>
	/// <seealso cref="DirectX12VertexBuffer" />
	/// <seealso cref="DirectX12VertexBufferLayout" />
	class LITEFX_DIRECTX12_API DirectX12VertexBufferLayoutBuilder : public VertexBufferLayoutBuilder<DirectX12VertexBufferLayoutBuilder, DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder> {
	public:
		using VertexBufferLayoutBuilder<DirectX12VertexBufferLayoutBuilder, DirectX12VertexBufferLayout, DirectX12InputAssemblerBuilder>::VertexBufferLayoutBuilder;

	public:
		/// <inheritdoc />
		virtual DirectX12VertexBufferLayoutBuilder& addAttribute(UniquePtr<BufferAttribute>&& attribute) override;

	public:
		/// <summary>
		/// Adds an attribute to the vertex buffer layout.
		/// </summary>
		/// <reamrks>
		/// This overload implicitly determines the location based on the number of attributes already defined. It should only be used if all locations can be implicitly deducted.
		/// </reamrks>
		/// <param name="format">The format of the attribute.</param>
		/// <param name="offset">The offset of the attribute within a buffer element.</param>
		/// <param name="semantic">The semantic of the attribute.</param>
		/// <param name="semanticIndex">The semantic index of the attribute.</param>
		virtual DirectX12VertexBufferLayoutBuilder& addAttribute(const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);

		/// <summary>
		/// Adds an attribute to the vertex buffer layout.
		/// </summary>
		/// <param name="location">The location, the attribute is bound to.</param>
		/// <param name="format">The format of the attribute.</param>
		/// <param name="offset">The offset of the attribute within a buffer element.</param>
		/// <param name="semantic">The semantic of the attribute.</param>
		/// <param name="semanticIndex">The semantic index of the attribute.</param>
		virtual DirectX12VertexBufferLayoutBuilder& addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic = AttributeSemantic::Unknown, const UInt32& semanticIndex = 0);
	};

	/// <summary>
	/// Implements a DirectX12 index buffer layout.
	/// </summary>
	/// <seealso cref="DirectX12IndexBuffer" />
	/// <seealso cref="DirectX12VertexBufferLayout" />
	class LITEFX_DIRECTX12_API DirectX12IndexBufferLayout : public virtual DirectX12RuntimeObject<DirectX12InputAssembler>, public IIndexBufferLayout {
		LITEFX_IMPLEMENTATION(DirectX12IndexBufferLayoutImpl);

	public:
		/// <summary>
		/// Initializes a new index buffer layout
		/// </summary>
		/// <param name="inputAssembler">The parent input assembler state, the index buffer layout is initialized for.</param>
		/// <param name="type">The type of the indices within the index buffer.</param>
		explicit DirectX12IndexBufferLayout(const DirectX12InputAssembler& inputAssembler, const IndexType& type);
		DirectX12IndexBufferLayout(DirectX12IndexBufferLayout&&) = delete;
		DirectX12IndexBufferLayout(const DirectX12IndexBufferLayout&) = delete;
		virtual ~DirectX12IndexBufferLayout() noexcept;

		// IIndexBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual const IndexType& indexType() const noexcept override;

		// IBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;
	};

	/// <summary>
	/// Implements a DirectX12 <see cref="IDescriptorLayout" />
	/// </summary>
	/// <seealso cref="IDirectX12ConstantBuffer" />
	/// <seealso cref="IDirectX12Image" />
	/// <seealso cref="IDirectX12Texture" />
	/// <seealso cref="IDirectX12Sampler" />
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	class LITEFX_DIRECTX12_API DirectX12DescriptorLayout : public virtual DirectX12RuntimeObject<DirectX12DescriptorSetLayout>, public IDescriptorLayout {
		LITEFX_IMPLEMENTATION(DirectX12DescriptorLayoutImpl);

	public:
		/// <summary>
		/// Initializes a new DirectX12 descriptor layout.
		/// </summary>
		/// <param name="descriptorSetLayout">The parent descriptor set layout.</param>
		/// <param name="type">The type of the descriptor.</param>
		/// <param name="binding">The binding point for the descriptor.</param>
		/// <param name="elementSize">The size of the descriptor.</param>
		explicit DirectX12DescriptorLayout(const DirectX12DescriptorSetLayout& descriptorSetLayout, const DescriptorType& type, const UInt32& binding, const size_t& elementSize);
		DirectX12DescriptorLayout(DirectX12DescriptorLayout&&) = delete;
		DirectX12DescriptorLayout(const DirectX12DescriptorLayout&) = delete;
		virtual ~DirectX12DescriptorLayout() noexcept;

		// IDescriptorLayout interface.
	public:
		/// <inheritdoc />
		virtual const DescriptorType& descriptorType() const noexcept override;

		// IBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual size_t elementSize() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const BufferType& type() const noexcept override;
	};

	/// <summary>
	/// Represents the base interface for a DirectX12 buffer implementation.
	/// </summary>
	/// <seealso cref="IDirectX12DescriptorSet" />
	/// <seealso cref="IDirectX12ConstantBuffer" />
	/// <seealso cref="IDirectX12ConstantTexture" />
	/// <seealso cref="IDirectX12VertexBuffer" />
	/// <seealso cref="IDirectX12IndexBuffer" />
	class LITEFX_DIRECTX12_API IDirectX12Buffer : public virtual ITransferableBuffer<IDirectX12Buffer, DirectX12CommandBuffer>, public virtual IResource<ComPtr<ID3D12Resource>> {
	public:
		virtual ~IDirectX12Buffer() noexcept = default;
	};

	/// <summary>
	/// Represents a DirectX12 vertex buffer.
	/// </summary>
	/// <seealso cref="DirectX12VertexBufferLayout" />
	/// <seealso cref="IDirectX12Buffer" />
	class LITEFX_DIRECTX12_API IDirectX12VertexBuffer : public virtual IVertexBuffer<IDirectX12Buffer, DirectX12VertexBufferLayout, DirectX12CommandBuffer>, public IDirectX12Buffer {
	public:
		virtual ~IDirectX12VertexBuffer() noexcept = default;
	};

	/// <summary>
	/// Represents a DirectX12 index buffer.
	/// </summary>
	/// <seealso cref="DirectX12IndexBufferLayout" />
	/// <seealso cref="IDirectX12Buffer" />
	class LITEFX_DIRECTX12_API IDirectX12IndexBuffer : public virtual IIndexBuffer<IDirectX12Buffer, DirectX12IndexBufferLayout, DirectX12CommandBuffer>, public IDirectX12Buffer {
	public:
		virtual ~IDirectX12IndexBuffer() noexcept = default;
	};

	/// <summary>
	/// Represents a DirectX12 uniform or storage buffer.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorLayout" />
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	/// <seealso cref="IDirectX12Buffer" />
	class LITEFX_DIRECTX12_API IDirectX12ConstantBuffer : public virtual IConstantBuffer<IDirectX12Buffer, DirectX12CommandBuffer, DirectX12DescriptorLayout>, public IDirectX12Buffer {
	public:
		virtual ~IDirectX12ConstantBuffer() noexcept = default;
	};

	/// <summary>
	/// Represents a DirectX12 sampled image or the base interface for a texture.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorLayout" />
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	/// <seealso cref="IDirectX12Texture" />
	/// <seealso cref="IDirectX12Sampler" />
	class LITEFX_DIRECTX12_API IDirectX12Image : public virtual IImage, public virtual IResource<ComPtr<ID3D12Resource>> {
	public:
		virtual ~IDirectX12Image() noexcept = default;
	};

	/// <summary>
	/// Represents a DirectX12 texture.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorLayout" />
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	/// <seealso cref="IDirectX12Image" />
	/// <seealso cref="IDirectX12Sampler" />
	/// <seealso cref="IDirectX12Buffer" />
	class LITEFX_DIRECTX12_API IDirectX12Texture : public virtual ITexture<DirectX12DescriptorLayout, IDirectX12Buffer, DirectX12CommandBuffer>, public IDirectX12Image {
	public:
		virtual ~IDirectX12Texture() noexcept = default;
	};

	/// <summary>
	/// Represents a DirectX12 sampler.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorLayout" />
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	/// <seealso cref="IDirectX12Image" />
	/// <seealso cref="IDirectX12Texture" />
	class LITEFX_DIRECTX12_API IDirectX12Sampler : public ISampler<DirectX12DescriptorLayout>, public virtual IResource<ComPtr<ID3D12Resource>> {
	public:
		virtual ~IDirectX12Sampler() noexcept = default;
	};

	/// <summary>
	/// Implements a DirectX12 <see cref="IDescriptorSet" />.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	class LITEFX_DIRECTX12_API DirectX12DescriptorSet : public virtual DirectX12RuntimeObject<DirectX12DescriptorSetLayout>, public IDescriptorSet<IDirectX12ConstantBuffer, IDirectX12Texture, IDirectX12Sampler, IDirectX12Image, IDirectX12Buffer, DirectX12CommandBuffer>, public ComResource<ID3D12DescriptorHeap> {
	public:
		/// <summary>
		/// Initializes a new descriptor set.
		/// </summary>
		/// <param name="layout">The parent descriptor set layout.</param>
		/// <param name="descriptorHeap">A CPU-visible descriptor heap that contains all descriptors of the descriptor set.</param>
		explicit DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& descriptorHeap);
		DirectX12DescriptorSet(DirectX12DescriptorSet&&) = delete;
		DirectX12DescriptorSet(const DirectX12DescriptorSet&) = delete;
		virtual ~DirectX12DescriptorSet() noexcept;

	public:
		/// <inheritdoc />
		virtual UniquePtr<IDirectX12ConstantBuffer> makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Texture> makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Sampler> makeSampler(const UInt32& binding, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& minLod = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& anisotropy = 0.f) const override;

		/// <inheritdoc />
		virtual void update(const IDirectX12ConstantBuffer& buffer, const UInt32& bufferElement) const noexcept override;

		/// <inheritdoc />
		virtual void update(const IDirectX12Texture& texture) const noexcept override;

		/// <inheritdoc />
		virtual void update(const IDirectX12Sampler& sampler) const noexcept override;

		/// <inheritdoc />
		virtual void attach(const UInt32& binding, const IDirectX12Image& image) const noexcept override;
	};

	/// <summary>
	/// Implements a DirectX12 <see cref="IDescriptorSetLayout" />.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12DescriptorSetLayoutBuilder" />
	class LITEFX_DIRECTX12_API DirectX12DescriptorSetLayout : public virtual DirectX12RuntimeObject<DirectX12RenderPipelineLayout>, public IDescriptorSetLayout<DirectX12DescriptorLayout, DirectX12DescriptorSet> {
		LITEFX_IMPLEMENTATION(DirectX12DescriptorSetLayoutImpl);
		LITEFX_BUILDER(DirectX12DescriptorSetLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a DirectX12 descriptor set layout.
		/// </summary>
		/// <param name="pipelineLayout">The parent pipeline layout that contains the descriptor set layout.</param>
		/// <param name="descriptorLayouts">The descriptor layouts of the descriptors within the descriptor set.</param>
		/// <param name="space">The space or set id of the descriptor set.</param>
		/// <param name="stages">The shader stages, the descriptor sets are bound to.</param>
		explicit DirectX12DescriptorSetLayout(const DirectX12RenderPipelineLayout& pipelineLayout, Array<UniquePtr<DirectX12DescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages);
		DirectX12DescriptorSetLayout(DirectX12DescriptorSetLayout&&) = delete;
		DirectX12DescriptorSetLayout(const DirectX12DescriptorSetLayout&) = delete;
		virtual ~DirectX12DescriptorSetLayout() noexcept;

	private:
		explicit DirectX12DescriptorSetLayout(const DirectX12RenderPipelineLayout& pipelineLayout) noexcept;

	public:
		/// <inheritdoc />
		virtual Array<const DirectX12DescriptorLayout*> layouts() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12DescriptorLayout& layout(const UInt32& binding) const override;

		/// <inheritdoc />
		virtual const UInt32& space() const noexcept override;

		/// <inheritdoc />
		virtual const ShaderStage& shaderStages() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 uniforms() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 storages() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 images() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 samplers() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 inputAttachments() const noexcept override;

	public:
		/// <inheritdoc />
		virtual UniquePtr<DirectX12DescriptorSet> allocate() const noexcept override;

		/// <inheritdoc />
		virtual Array<UniquePtr<DirectX12DescriptorSet>> allocate(const UInt32& descriptorSets) const noexcept override;

		/// <inheritdoc />
		virtual void free(const DirectX12DescriptorSet& descriptorSet) const noexcept override;
	};

	/// <summary>
	/// Builds a <see cref="DirectX12DescriptorSetLayout" />.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	class LITEFX_DIRECTX12_API DirectX12DescriptorSetLayoutBuilder : public DescriptorSetLayoutBuilder<DirectX12DescriptorSetLayoutBuilder, DirectX12DescriptorSetLayout, DirectX12RenderPipelineLayoutBuilder> {
		LITEFX_IMPLEMENTATION(DirectX12DescriptorSetLayoutBuilderImpl);

	public:
		/// <summary>
		/// Initializes a DirectX12 descriptor set layout builder.
		/// </summary>
		/// <param name="parent">The parent pipeline layout builder.</param>
		/// <param name="space">The space the descriptor set is bound to.</param>
		/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
		explicit DirectX12DescriptorSetLayoutBuilder(DirectX12RenderPipelineLayoutBuilder& parent, const UInt32& space = 0, const ShaderStage& stages = ShaderStage::Compute | ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex);
		DirectX12DescriptorSetLayoutBuilder(const DirectX12DescriptorSetLayoutBuilder&) = delete;
		DirectX12DescriptorSetLayoutBuilder(DirectX12DescriptorSetLayoutBuilder&&) = delete;
		virtual ~DirectX12DescriptorSetLayoutBuilder() noexcept;

		// IBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12RenderPipelineLayoutBuilder& go() override;

		// DescriptorSetLayoutBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12DescriptorSetLayoutBuilder& addDescriptor(UniquePtr<DirectX12DescriptorLayout>&& layout) override;

		/// <inheritdoc />
		virtual DirectX12DescriptorSetLayoutBuilder& addDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize) override;

		// DirectX12DescriptorSetLayoutBuilder.
	public:
		/// <summary>
		/// Sets the space, the descriptor set is bound to.
		/// </summary>
		/// <param name="space">The space, the descriptor set is bound to.</param>
		virtual DirectX12DescriptorSetLayoutBuilder& space(const UInt32& space) noexcept;

		/// <summary>
		/// Sets the shader stages, the descriptor set is accessible from.
		/// </summary>
		/// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
		virtual DirectX12DescriptorSetLayoutBuilder& shaderStages(const ShaderStage& stages) noexcept;
	};

	/// <summary>
	/// Implements a DirectX12 <see cref="IShaderModule" />.
	/// </summary>
	/// <seealso cref="DirectX12ShaderProgram" />
	class LITEFX_DIRECTX12_API DirectX12ShaderModule : public virtual DirectX12RuntimeObject<DirectX12Device>, public IShaderModule {
		LITEFX_IMPLEMENTATION(DirectX12ShaderModuleImpl);

	public:
		/// <summary>
		/// Initializes a new DirectX12 shader module.
		/// </summary>
		/// <param name="device">The parent device, this shader module has been created from.</param>
		/// <param name="type">The shader stage, this module is used in.</param>
		/// <param name="fileName">The file name of the module source.</param>
		/// <param name="entryPoint">The name of the module entry point.</param>
		explicit DirectX12ShaderModule(const DirectX12Device& device, const ShaderStage& type, const String& fileName, const String& entryPoint = "main");
		DirectX12ShaderModule(const DirectX12ShaderModule&) noexcept = delete;
		DirectX12ShaderModule(DirectX12ShaderModule&&) noexcept = delete;
		virtual ~DirectX12ShaderModule() noexcept;

		// IShaderModule interface.
	public:
		/// <inheritdoc />
		virtual const String& fileName() const noexcept override;

		/// <inheritdoc />
		virtual const String& entryPoint() const noexcept override;

		/// <inheritdoc />
		virtual const ShaderStage& type() const noexcept override;

		// DirectX12 Shader module.
	public:
		virtual const D3D12_SHADER_BYTECODE& bytecode() const noexcept;
	};

	/// <summary>
	/// Implements a DirectX12 <see cref="IShaderProgram" />.
	/// </summary>
	/// <seealso cref="DirectX12ShaderProgramBuilder" />
	class LITEFX_DIRECTX12_API DirectX12ShaderProgram : public virtual DirectX12RuntimeObject<DirectX12RenderPipelineLayout>, public IShaderProgram<DirectX12ShaderModule> {
		LITEFX_IMPLEMENTATION(DirectX12ShaderProgramImpl);
		LITEFX_BUILDER(DirectX12ShaderProgramBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX12 shader program.
		/// </summary>
		/// <param name="pipelineLayout">The parent pipeline layout to initialize the shader program from.</param>
		/// <param name="modules">The shader modules used by the shader program.</param>
		explicit DirectX12ShaderProgram(const DirectX12RenderPipelineLayout& pipelineLayout, Array<UniquePtr<DirectX12ShaderModule>>&& modules);
		DirectX12ShaderProgram(DirectX12ShaderProgram&&) noexcept = delete;
		DirectX12ShaderProgram(const DirectX12ShaderProgram&) noexcept = delete;
		virtual ~DirectX12ShaderProgram() noexcept;

	private:
		explicit DirectX12ShaderProgram(const DirectX12RenderPipelineLayout& pipelineLayout) noexcept;

	public:
		/// <inheritdoc />
		virtual Array<const DirectX12ShaderModule*> modules() const noexcept override;
	};

	/// <summary>
	/// Builds a DirectX12 <see cref="IShaderProgram" />.
	/// </summary>
	/// <seealso cref="DirectX12ShaderProgram" />
	class LITEFX_DIRECTX12_API DirectX12ShaderProgramBuilder : public ShaderProgramBuilder<DirectX12ShaderProgramBuilder, DirectX12ShaderProgram, DirectX12RenderPipelineLayoutBuilder> {
		LITEFX_IMPLEMENTATION(DirectX12ShaderProgramBuilderImpl);

	public:
		/// <summary>
		/// Initializes a DirectX12 shader program builder.
		/// </summary>
		/// <param name="parent">The parent pipeline layout builder.</param>
		explicit DirectX12ShaderProgramBuilder(DirectX12RenderPipelineLayoutBuilder& parent);
		DirectX12ShaderProgramBuilder(const DirectX12ShaderProgramBuilder&) = delete;
		DirectX12ShaderProgramBuilder(DirectX12ShaderProgramBuilder&&) = delete;
		virtual ~DirectX12ShaderProgramBuilder() noexcept;

		// IBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12RenderPipelineLayoutBuilder& go() override;

		// ShaderProgramBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12ShaderProgramBuilder& addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual DirectX12ShaderProgramBuilder& addVertexShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual DirectX12ShaderProgramBuilder& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual DirectX12ShaderProgramBuilder& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual DirectX12ShaderProgramBuilder& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual DirectX12ShaderProgramBuilder& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main") override;

		/// <inheritdoc />
		virtual DirectX12ShaderProgramBuilder& addComputeShaderModule(const String& fileName, const String& entryPoint = "main") override;
	};

	/// <summary>
	/// Implements a DirectX12 <see cref="IRenderPipelineLayout" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPipelineLayoutBuilder" />
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineLayout : public virtual DirectX12RuntimeObject<DirectX12RenderPipeline>, public IRenderPipelineLayout<DirectX12DescriptorSetLayout, DirectX12ShaderProgram>, public ComResource<ID3D12RootSignature> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineLayoutImpl);
		LITEFX_BUILDER(DirectX12RenderPipelineLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX12 render pipeline layout.
		/// </summary>
		/// <param name="pipeline">The parent pipeline state the layout describes.</param>
		/// <param name="shaderProgram">The shader program used by the pipeline.</param>
		/// <param name="descriptorSetLayouts">The descriptor set layouts used by the pipeline.</param>
		explicit DirectX12RenderPipelineLayout(const DirectX12RenderPipeline& pipeline, UniquePtr<DirectX12ShaderProgram>&& shaderProgram, Array<UniquePtr<DirectX12DescriptorSetLayout>>&& descriptorSetLayouts);
		DirectX12RenderPipelineLayout(DirectX12RenderPipelineLayout&&) noexcept = delete;
		DirectX12RenderPipelineLayout(const DirectX12RenderPipelineLayout&) noexcept = delete;
		virtual ~DirectX12RenderPipelineLayout() noexcept;

	private:
		explicit DirectX12RenderPipelineLayout(const DirectX12RenderPipeline& pipeline) noexcept;

		// IRenderPipelineLayout interface.
	public:
		/// <inheritdoc />
		virtual const DirectX12ShaderProgram& program() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12DescriptorSetLayout& layout(const UInt32& space) const override;

		/// <inheritdoc />
		virtual Array<const DirectX12DescriptorSetLayout*> layouts() const noexcept override;
	};

	/// <summary>
	/// Builds a DirectX12 <see cref="IRenderPipelineLayout" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPipelineLayout" />
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineLayoutBuilder : public RenderPipelineLayoutBuilder<DirectX12RenderPipelineLayoutBuilder, DirectX12RenderPipelineLayout, DirectX12RenderPipelineBuilder> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineLayoutBuilderImpl);

	public:
		/// <summary>
		/// Initializes a new DirectX12 render pipeline layout builder.
		/// </summary>
		/// <param name="parent">The parent render pipeline, that is described by this layout.</param>
		DirectX12RenderPipelineLayoutBuilder(DirectX12RenderPipelineBuilder& parent);
		DirectX12RenderPipelineLayoutBuilder(DirectX12RenderPipelineLayoutBuilder&&) = delete;
		DirectX12RenderPipelineLayoutBuilder(const DirectX12RenderPipelineLayoutBuilder&) = delete;
		virtual ~DirectX12RenderPipelineLayoutBuilder() noexcept;

		// IBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12RenderPipelineBuilder& go() override;

		// RenderPipelineBuilder interface.
	public:
		/// <inheritdoc />
		virtual void use(UniquePtr<DirectX12ShaderProgram>&& program) override;

		/// <inheritdoc />
		virtual void use(UniquePtr<DirectX12DescriptorSetLayout>&& layout) override;

		// DirectX12RenderPipelineBuilder.
	public:
		/// <summary>
		/// Builds a shader program for the render pipeline layout.
		/// </summary>
		virtual DirectX12ShaderProgramBuilder shaderProgram();

		/// <summary>
		/// Builds a new descriptor set for the render pipeline layout.
		/// </summary>
		/// <param name="space">The space, the descriptor set is bound to.</param>
		/// <param name="stages">The stages, the descriptor set will be accessible from.</param>
		virtual DirectX12DescriptorSetLayoutBuilder addDescriptorSet(const UInt32& space = 0, const ShaderStage& stages = ShaderStage::Compute | ShaderStage::Fragment | ShaderStage::Geometry | ShaderStage::TessellationControl | ShaderStage::TessellationEvaluation | ShaderStage::Vertex);
	};

	/// <summary>
	/// Implements the DirectX12 input assembler state.
	/// </summary>
	/// <seealso cref="DirectX12InputAssemblerBuilder" />
	class LITEFX_DIRECTX12_API DirectX12InputAssembler : public virtual DirectX12RuntimeObject<DirectX12Device>, public IInputAssembler<DirectX12VertexBufferLayout, DirectX12IndexBufferLayout> {
		LITEFX_IMPLEMENTATION(DirectX12InputAssemblerImpl);
		LITEFX_BUILDER(DirectX12InputAssemblerBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX12 input assembler state.
		/// </summary>
		/// <param name="device">The parent device.</param>
		/// <param name="vertexBufferLayouts">The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.</param>
		/// <param name="indexBufferLayout">The index buffer layout.</param>
		/// <param name="primitiveTopology">The primitive topology.</param>
		explicit DirectX12InputAssembler(const DirectX12Device& device, Array<UniquePtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<DirectX12IndexBufferLayout>&& indexBufferLayout, const PrimitiveTopology& primitiveTopology = PrimitiveTopology::TriangleList);
		DirectX12InputAssembler(DirectX12InputAssembler&&) noexcept = delete;
		DirectX12InputAssembler(const DirectX12InputAssembler&) noexcept = delete;
		virtual ~DirectX12InputAssembler() noexcept;

	private:
		explicit DirectX12InputAssembler(const DirectX12Device& device) noexcept;

	public:
		/// <inheritdoc />
		virtual Array<const DirectX12VertexBufferLayout*> vertexBufferLayouts() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12VertexBufferLayout& vertexBufferLayout(const UInt32& binding) const override;

		/// <inheritdoc />
		virtual const DirectX12IndexBufferLayout& indexBufferLayout() const override;

		/// <inheritdoc />
		virtual const PrimitiveTopology& topology() const noexcept override;
	};

	/// <summary>
	/// Builds a <see cref="DirectX12InputAssembler" />.
	/// </summary>
	/// <seealso cref="DirectX12InputAssembler" />
	class LITEFX_DIRECTX12_API DirectX12InputAssemblerBuilder : public InputAssemblerBuilder<DirectX12InputAssemblerBuilder, DirectX12InputAssembler, DirectX12RenderPipelineBuilder> {
		LITEFX_IMPLEMENTATION(DirectX12InputAssemblerBuilderImpl);

	public:
		/// <summary>
		/// Initializes a DirectX12 input assembler builder.
		/// </summary>
		/// <param name="parent">The parent render pipeline builder.</param>
		explicit DirectX12InputAssemblerBuilder(DirectX12RenderPipelineBuilder& parent) noexcept;
		DirectX12InputAssemblerBuilder(const DirectX12InputAssemblerBuilder&) noexcept = delete;
		DirectX12InputAssemblerBuilder(DirectX12InputAssemblerBuilder&&) noexcept = delete;
		virtual ~DirectX12InputAssemblerBuilder() noexcept;

	public:
		/// <summary>
		/// Starts building a vertex buffer layout.
		/// </summary>
		/// <param name="elementSize">The size of a vertex within the vertex buffer.</param>
		/// <param name="binding">The binding point to bind the vertex buffer to.</param>
		virtual DirectX12VertexBufferLayoutBuilder addVertexBuffer(const size_t& elementSize, const UInt32& binding = 0);

		/// <summary>
		/// Starts building an index buffer layout.
		/// </summary>
		/// <param name="type">The type of the index buffer.</param>
		virtual DirectX12InputAssemblerBuilder& withIndexType(const IndexType& type);

		// IInputAssemblerBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12InputAssemblerBuilder& withTopology(const PrimitiveTopology& topology) override;

		/// <inheritdoc />
		virtual void use(UniquePtr<DirectX12VertexBufferLayout>&& layout) override;

		/// <inheritdoc />
		virtual void use(UniquePtr<DirectX12IndexBufferLayout>&& layout) override;

		// Builder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12RenderPipelineBuilder& go() override;
	};

	/// <summary>
	/// Implements a DirectX12 <see cref="IRasterizer" />.
	/// </summary>
	/// <seealso cref="DirectX12RasterizerBuilder" />
	class LITEFX_DIRECTX12_API DirectX12Rasterizer : public virtual DirectX12RuntimeObject<DirectX12RenderPipeline>, public Rasterizer {
		LITEFX_BUILDER(DirectX12RasterizerBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX12 rasterizer state.
		/// </summary>
		/// <param name="pipeline">The parent pipeline, the rasterizer state is created for.</param>
		/// <param name="polygonMode">The polygon mode used by the pipeline.</param>
		/// <param name="cullMode">The cull mode used by the pipeline.</param>
		/// <param name="cullOrder">The cull order used by the pipeline.</param>
		/// <param name="lineWidth">The line width used by the pipeline.</param>
		/// <param name="useDepthBias"><c>true</c>, if the depth bias should be enabled.</param>
		/// <param name="depthBiasClamp">The clamp value of the depth bias state.</param>
		/// <param name="depthBiasConstantFactor">The constant factor of the depth bias state.</param>
		/// <param name="depthBiasSlopeFactor">The slope factor of the depth bias state.</param>
		explicit DirectX12Rasterizer(const DirectX12RenderPipeline& pipeline, const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth = 1.f, const bool& useDepthBias = false, const Float& depthBiasClamp = 1.f, const Float& depthBiasConstantFactor = 0.f, const Float& depthBiasSlopeFactor = 0.f) noexcept;
		DirectX12Rasterizer(DirectX12Rasterizer&&) noexcept = delete;
		DirectX12Rasterizer(const DirectX12Rasterizer&) noexcept = delete;
		virtual ~DirectX12Rasterizer() noexcept;

	private:
		explicit DirectX12Rasterizer(const DirectX12RenderPipeline& pipeline) noexcept;
	};

	/// <summary>
	/// Builds a DirectX12 <see cref="IRasterizer" />.
	/// </summary>
	/// <seealso cref="DirectX12Rasterizer" />
	class LITEFX_DIRECTX12_API DirectX12RasterizerBuilder : public RasterizerBuilder<DirectX12RasterizerBuilder, DirectX12Rasterizer, DirectX12RenderPipelineBuilder> {
		LITEFX_IMPLEMENTATION(DirectX12RasterizerBuilderImpl);

	public:
		/// <summary>
		/// Initializes a DirectX12 input assembler builder.
		/// </summary>
		/// <param name="parent">The parent render pipeline builder.</param>
		explicit DirectX12RasterizerBuilder(DirectX12RenderPipelineBuilder& parent) noexcept;
		DirectX12RasterizerBuilder(const DirectX12RasterizerBuilder&) noexcept = delete;
		DirectX12RasterizerBuilder(DirectX12RasterizerBuilder&&) noexcept = delete;
		virtual ~DirectX12RasterizerBuilder() noexcept;

		// IBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12RenderPipelineBuilder& go() override;

		// RasterizerBuilder interface.
	public:
		/// <inheritdoc />
		virtual DirectX12RasterizerBuilder& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) noexcept override;

		/// <inheritdoc />
		virtual DirectX12RasterizerBuilder& withCullMode(const CullMode& cullMode = CullMode::BackFaces) noexcept override;

		/// <inheritdoc />
		virtual DirectX12RasterizerBuilder& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) noexcept override;

		/// <inheritdoc />
		virtual DirectX12RasterizerBuilder& withLineWidth(const Float& lineWidth = 1.f) noexcept override;

		/// <inheritdoc />
		virtual DirectX12RasterizerBuilder& enableDepthBias(const bool& enable = false) noexcept override;

		/// <inheritdoc />
		virtual DirectX12RasterizerBuilder& withDepthBiasClamp(const Float& clamp = 0.f) noexcept override;

		/// <inheritdoc />
		virtual DirectX12RasterizerBuilder& withDepthBiasConstantFactor(const Float& factor = 0.f) noexcept override;

		/// <inheritdoc />
		virtual DirectX12RasterizerBuilder& withDepthBiasSlopeFactor(const Float& factor = 0.f) noexcept override;
	};

	/// <summary>
	/// Implements a DirectX12 <see cref="IRenderPipeline" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPipelineBuilder" />
	class LITEFX_DIRECTX12_API DirectX12RenderPipeline : public virtual DirectX12RuntimeObject<DirectX12RenderPass>, public IRenderPipeline<DirectX12RenderPipelineLayout, DirectX12InputAssembler, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Buffer>, ComResource<ID3D12PipelineState> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineImpl);
		LITEFX_BUILDER(DirectX12RenderPipelineBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX12 render pipeline.
		/// </summary>
		/// <param name="renderPass"></param>
		/// <param name="id"></param>
		/// <param name="name"></param>
		explicit DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, const UInt32& id, UniquePtr<DirectX12RenderPipelineLayout>&& layout, SharedPtr<DirectX12InputAssembler>&& inputAssembler, SharedPtr<DirectX12Rasterizer>&& rasterizer, Array<SharedPtr<IViewport>>&& viewports, Array<SharedPtr<IScissor>>&& scissors, const String& name = "");
		DirectX12RenderPipeline(DirectX12RenderPipeline&&) noexcept = delete;
		DirectX12RenderPipeline(const DirectX12RenderPipeline&) noexcept = delete;
		virtual ~DirectX12RenderPipeline() noexcept;

	private:
		DirectX12RenderPipeline(const DirectX12RenderPass& renderPass) noexcept;

		// IRenderPipeline interface.
	public:
		/// <inheritdoc />
		virtual const String& name() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& id() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12RenderPipelineLayout& layout() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<DirectX12InputAssembler> inputAssembler() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<IRasterizer> rasterizer() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IViewport*> viewports() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IScissor*> scissors() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void bind(const IDirectX12VertexBuffer& buffer) const override;

		/// <inheritdoc />
		virtual void bind(const IDirectX12IndexBuffer& buffer) const override;

		/// <inheritdoc />
		virtual void bind(const DirectX12DescriptorSet& descriptorSet) const override;

		/// <inheritdoc />
		virtual void use() const override;

		/// <inheritdoc />
		virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const override;

		/// <inheritdoc />
		virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const override;
	};

	/// <summary>
	/// Builds a DirectX12 <see cref="IRenderPipeline" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPipeline" />
	class LITEFX_DIRECTX12_API DirectX12RenderPipelineBuilder : public RenderPipelineBuilder<DirectX12RenderPipelineBuilder, DirectX12RenderPipeline> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineBuilderImpl);

	public:
		/// <summary>
		/// Initializes a DirectX12 render pipeline builder.
		/// </summary>
		/// <param name="renderPass">The parent render pass</param>
		/// <param name="id">A unique identifier for the render pipeline.</param>
		/// <param name="name">A debug name for the render pipeline.</param>
		explicit DirectX12RenderPipelineBuilder(const DirectX12RenderPass& renderPass, const UInt32& id, const String& name = "");
		DirectX12RenderPipelineBuilder(DirectX12RenderPipelineBuilder&&) = delete;
		DirectX12RenderPipelineBuilder(const DirectX12RenderPipelineBuilder&) = delete;
		virtual ~DirectX12RenderPipelineBuilder() noexcept;

		// IBuilder interface.
	public:
		/// <inheritdoc />
		[[nodiscard]] virtual UniquePtr<DirectX12RenderPipeline> go() override;

		// RenderPipelineBuilder interface.
	public:
		/// <inheritdoc />
		virtual void use(UniquePtr<DirectX12RenderPipelineLayout>&& layout) override;

		/// <inheritdoc />
		virtual void use(SharedPtr<IRasterizer> rasterizer) override;

		/// <inheritdoc />
		virtual void use(SharedPtr<DirectX12InputAssembler> inputAssembler) override;

		/// <inheritdoc />
		virtual void use(SharedPtr<IViewport> viewport) override;

		/// <inheritdoc />
		virtual void use(SharedPtr<IScissor> scissor) override;

		// DirectX12RenderPipelineBuilder.
	public:
		/// <summary>
		/// Builds a <see cref="DirectX12RenderPipelineLayout" /> for the render pipeline.
		/// </summary>
		virtual DirectX12RenderPipelineLayoutBuilder layout();

		/// <summary>
		/// Builds a <see cref="DirectX12Rasterizer" /> for the render pipeline.
		/// </summary>
		virtual DirectX12RasterizerBuilder rasterizer();

		/// <summary>
		/// Builds a <see cref="DirectX12InputAssembler" /> for the render pipeline.
		/// </summary>
		virtual DirectX12InputAssemblerBuilder inputAssembler();

		/// <summary>
		/// Uses the provided rasterizer state for the render pipeline.
		/// </summary>
		/// <param name="rasterizer">The rasterizer state used by the render pipeline.</param>
		virtual DirectX12RenderPipelineBuilder& withRasterizer(SharedPtr<IRasterizer> rasterizer);

		/// <summary>
		/// Uses the provided input assembler state for the render pipeline.
		/// </summary>
		/// <param name="inputAssembler">The input assembler state used by the render pipeline.</param>
		virtual DirectX12RenderPipelineBuilder& withInputAssembler(SharedPtr<DirectX12InputAssembler> inputAssembler);

		/// <summary>
		/// Adds the provided viewport to the render pipeline.
		/// </summary>
		/// <param name="viewport">The viewport to add to the render pipeline.</param>
		virtual DirectX12RenderPipelineBuilder& withViewport(SharedPtr<IViewport> viewport);

		/// <summary>
		/// Adds the provided scissor to the render pipeline.
		/// </summary>
		/// <param name="scissor">The scissor to add to the render pipeline.</param>
		virtual DirectX12RenderPipelineBuilder& withScissor(SharedPtr<IScissor> scissor);
	};

	/// <summary>
	/// Implements a DirectX12 frame buffer.
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12FrameBuffer : public virtual DirectX12RuntimeObject<DirectX12RenderPass>, public IFrameBuffer<DirectX12CommandBuffer, IDirectX12Image> {
		LITEFX_IMPLEMENTATION(DirectX12FrameBufferImpl);

	public:
		/// <summary>
		/// Initializes a DirectX12 frame buffer.
		/// </summary>
		/// <param name="renderPass">The parent render pass of the frame buffer.</param>
		/// <param name="bufferIndex">The index of the frame buffer within the parent render pass.</param>
		/// <param name="renderArea">The initial size of the render area.</param>
		DirectX12FrameBuffer(const DirectX12RenderPass& renderPass, const UInt32& bufferIndex, const Size2d& renderArea);
		DirectX12FrameBuffer(const DirectX12FrameBuffer&) noexcept = delete;
		DirectX12FrameBuffer(DirectX12FrameBuffer&&) noexcept = delete;
		virtual ~DirectX12FrameBuffer() noexcept;

		// IFrameBuffer interface.
	public:
		/// <inheritdoc />
		virtual const UInt32& bufferIndex() const noexcept override;

		/// <inheritdoc />
		virtual const Size2d& size() const noexcept override;

		/// <inheritdoc />
		virtual size_t getWidth() const noexcept override;

		/// <inheritdoc />
		virtual size_t getHeight() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12CommandBuffer& commandBuffer() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IDirectX12Image*> images() const noexcept override;

		/// <inheritdoc />
		virtual const IDirectX12Image& image(const UInt32& location) const override;

	public:
		/// <inheritdoc />
		virtual void resize(const Size2d& renderArea) override;
	};

	/// <summary>
	/// Implements a DirectX12 render pass.
	/// </summary>
	/// <seealso cref="DirectX12RenderPassBuilder" />
	class LITEFX_DIRECTX12_API DirectX12RenderPass : public virtual DirectX12RuntimeObject<DirectX12Device>, public IRenderPass<DirectX12RenderPipeline, DirectX12FrameBuffer, DirectX12InputAttachmentMapping> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPassImpl);
		LITEFX_BUILDER(DirectX12RenderPassBuilder);

	public:
		/// <summary>
		/// Creates and initializes a new DirectX12 render pass instance.
		/// </summary>
		/// <param name="device"></param>
		/// <param name="renderTargets"></param>
		/// <param name="inputAttachments"></param>
		explicit DirectX12RenderPass(const DirectX12Device& device, Span<RenderTarget> renderTargets, Span<DirectX12InputAttachmentMapping> inputAttachments = { });
		DirectX12RenderPass(const DirectX12RenderPass&) = delete;
		DirectX12RenderPass(DirectX12RenderPass&&) = delete;
		virtual ~DirectX12RenderPass() noexcept;

	private:
		/// <summary>
		/// Creates an uninitialized DirectX12 render pass instance.
		/// </summary>
		/// <remarks>
		/// This constructor is called by the <see cref="DirectX12RenderPassBuilder" /> in order to create a render pass instance without initializing it. The instance 
		/// is only initialized after calling <see cref="DirectX12RenderPassBuilder::go" />.
		/// </remarks>
		/// <param name="device">The parent device of the render pass.</param>
		explicit DirectX12RenderPass(const DirectX12Device& device) noexcept;

		// IInputAttachmentMappingSource interface.
	public:
		/// <inheritdoc />
		virtual const DirectX12FrameBuffer& frameBuffer(const UInt32& buffer) const override;

		// IRenderPass interface.
	public:
		/// <inheritdoc />
		virtual const DirectX12FrameBuffer& activeFrameBuffer() const override;

		/// <inheritdoc />
		virtual Array<const DirectX12FrameBuffer*> frameBuffers() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12RenderPipeline& pipeline(const UInt32& id) const override;

		/// <inheritdoc />
		virtual Array<const DirectX12RenderPipeline*> pipelines() const noexcept override;

		/// <inheritdoc />
		virtual const RenderTarget& renderTarget(const UInt32& location) const override;

		/// <inheritdoc />
		virtual Span<const RenderTarget> renderTargets() const noexcept override;

		/// <inheritdoc />
		virtual bool hasPresentTarget() const noexcept override;

		/// <inheritdoc />
		virtual Span<const DirectX12InputAttachmentMapping> inputAttachments() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void begin(const UInt32& buffer) override;

		/// <inheritdoc />
		virtual void end() const override;

		/// <inheritdoc />
		virtual void resizeFrameBuffers(const Size2d& renderArea) override;

		/// <inheritdoc />
		virtual void updateAttachments(const DirectX12DescriptorSet& descriptorSet) const override;

		// DirectX12RenderPass.
	public:
		/// <summary>
		/// Starts building a render pipeline.
		/// </summary>
		/// <param name="id">A unique ID for the render pipeline.</param>
		/// <param name="name">A debug name for the render pipeline.</param>
		/// <seealso cref="DirectX12RenderPipeline" />
		/// <seealso cref="DirectX12RenderPipelineBuilder" />
		virtual DirectX12RenderPipelineBuilder makePipeline(const UInt32& id, const String& name = "") const noexcept;
	};

	/// <summary>
	/// Implements the DirectX12 <see cref="RenderPassBuilder" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPass" />
	class LITEFX_DIRECTX12_API DirectX12RenderPassBuilder : public RenderPassBuilder<DirectX12RenderPassBuilder, DirectX12RenderPass> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPassBuilderImpl)

	public:
		explicit DirectX12RenderPassBuilder(const DirectX12Device& device) noexcept;
		DirectX12RenderPassBuilder(const DirectX12RenderPassBuilder&) noexcept = delete;
		DirectX12RenderPassBuilder(DirectX12RenderPassBuilder&&) noexcept = delete;
		virtual ~DirectX12RenderPassBuilder() noexcept;
	public:
		virtual void use(RenderTarget&& target) override;
		virtual void use(DirectX12InputAttachmentMapping&& inputAttachment) override;

	public:
		virtual DirectX12RenderPassBuilder& renderTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
		virtual DirectX12RenderPassBuilder& renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
		virtual DirectX12RenderPassBuilder& renderTarget(DirectX12InputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
		virtual DirectX12RenderPassBuilder& renderTarget(DirectX12InputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) override;
		virtual DirectX12RenderPassBuilder& inputAttachment(const DirectX12InputAttachmentMapping& inputAttachment) override;
		virtual DirectX12RenderPassBuilder& inputAttachment(const UInt32& inputLocation, const DirectX12RenderPass& renderPass, const UInt32& outputLocation) override;
		virtual DirectX12RenderPassBuilder& inputAttachment(const UInt32& inputLocation, const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget) override;

	public:
		virtual UniquePtr<DirectX12RenderPass> go() override;
	};

	/// <summary>
	/// Implements a <see cref="IInputAttachmentMapping" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPass" />
	/// <seealso cref="DirectX12RenderPassBuilder" />
	class LITEFX_DIRECTX12_API DirectX12InputAttachmentMapping : public IInputAttachmentMapping<DirectX12RenderPass> {
		LITEFX_IMPLEMENTATION(DirectX12InputAttachmentMappingImpl);

	public:
		DirectX12InputAttachmentMapping() noexcept;

		/// <summary>
		/// Creates a new DirectX12 input attachment mapping.
		/// </summary>
		/// <param name="renderPass">The render pass to fetch the input attachment from.</param>
		/// <param name="renderTarget">The render target of the <paramref name="renderPass"/> that is used for the input attachment.</param>
		/// <param name="location">The location to bind the input attachment to.</param>
		DirectX12InputAttachmentMapping(const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget, const UInt32& location);
		DirectX12InputAttachmentMapping(const DirectX12InputAttachmentMapping&) noexcept;
		DirectX12InputAttachmentMapping(DirectX12InputAttachmentMapping&&) noexcept;
		virtual ~DirectX12InputAttachmentMapping() noexcept;

	public:
		inline DirectX12InputAttachmentMapping& operator=(const DirectX12InputAttachmentMapping&) noexcept;
		inline DirectX12InputAttachmentMapping& operator=(DirectX12InputAttachmentMapping&&) noexcept;

	public:
		/// <inheritdoc />
		virtual const DirectX12RenderPass* inputAttachmentSource() const noexcept override;

		/// <inheritdoc />
		virtual const RenderTarget& renderTarget() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& location() const noexcept override;
	};

	/// <summary>
	/// Implements a DirectX12 swap chain.
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12SwapChain : public virtual DirectX12RuntimeObject<DirectX12Device>, public ISwapChain<IDirectX12Image>, public ComResource<IDXGISwapChain4> {
		LITEFX_IMPLEMENTATION(DirectX12SwapChainImpl);

	public:
		/// <summary>
		/// Initializes a DirectX12 swap chain.
		/// </summary>
		/// <param name="device">The device that owns the swap chain.</param>
		/// <param name="format">The initial surface format.</param>
		/// <param name="renderArea">The initial size of the render area.</param>
		/// <param name="buffers">The initial number of buffers.</param>
		explicit DirectX12SwapChain(const DirectX12Device& device, const Format& surfaceFormat = Format::B8G8R8A8_SRGB, const Size2d& renderArea = { 800, 600 }, const UInt32& buffers = 3);
		DirectX12SwapChain(const DirectX12SwapChain&) = delete;
		DirectX12SwapChain(DirectX12SwapChain&&) = delete;
		virtual ~DirectX12SwapChain() noexcept;

		// ISwapChain interface.
	public:
		/// <inheritdoc />
		virtual const Format& surfaceFormat() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& buffers() const noexcept override;

		/// <inheritdoc />
		virtual const Size2d& renderArea() const noexcept override;

		/// <inheritdoc />
		virtual Array<const IDirectX12Image*> images() const noexcept override;

	public:
		/// <inheritdoc />
		virtual Array<Format> getSurfaceFormats() const noexcept override;

		/// <inheritdoc />
		virtual void reset(const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers) override;

		/// <inheritdoc />
		[[nodiscard]] virtual UInt32 swapBackBuffer() const override;
	};

	/// <summary>
	/// Implements a DirectX12 command queue.
	/// </summary>
	/// <seealso cref="DirectX12CommandBuffer" />
	class LITEFX_DIRECTX12_API DirectX12Queue : public virtual DirectX12RuntimeObject<DirectX12Device>, public ICommandQueue<DirectX12CommandBuffer>, public ComResource<ID3D12CommandQueue> {
		LITEFX_IMPLEMENTATION(DirectX12QueueImpl);

	public:
		/// <summary>
		/// Initializes the DirectX12 command queue.
		/// </summary>
		/// <param name="device">The device, commands get send to.</param>
		/// <param name="type">The type of the command queue.</param>
		/// <param name="priority">The priority, of which commands are issued on the device.</param>
		explicit DirectX12Queue(const DirectX12Device& device, const QueueType& type, const QueuePriority& priority);
		DirectX12Queue(const DirectX12Queue&) = delete;
		DirectX12Queue(DirectX12Queue&&) = delete;
		virtual ~DirectX12Queue() noexcept;

		// ICommandQueue interface.
	public:
		/// <inheritdoc />
		virtual bool isBound() const noexcept override;

		/// <inheritdoc />
		virtual const QueuePriority& priority() const noexcept override;

		/// <inheritdoc />
		virtual const QueueType& type() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void bind() override;

		/// <inheritdoc />
		virtual void release() override;

		/// <inheritdoc />
		virtual UniquePtr<DirectX12CommandBuffer> createCommandBuffer(const bool& beginRecording = false) const override;
	};

	/// <summary>
	/// A graphics factory that produces objects for a <see cref="DirectX12Device" />.
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12GraphicsFactory : public IGraphicsFactory<DirectX12DescriptorLayout, IDirectX12Image, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12ConstantBuffer, IDirectX12Buffer, IDirectX12Texture, IDirectX12Sampler> {
		LITEFX_IMPLEMENTATION(DirectX12GraphicsFactoryImpl);

	public:
		/// <summary>
		/// Creates a new graphics factory.
		/// </summary>
		/// <param name="device">The device the factory should produce objects for.</param>
		explicit DirectX12GraphicsFactory(const DirectX12Device& device);
		DirectX12GraphicsFactory(const DirectX12GraphicsFactory&) = delete;
		DirectX12GraphicsFactory(DirectX12GraphicsFactory&&) = delete;
		virtual ~DirectX12GraphicsFactory() noexcept;

	public:
		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Image> createImage(const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Image> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Buffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12VertexBuffer> createVertexBuffer(const DirectX12VertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12IndexBuffer> createIndexBuffer(const DirectX12IndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12ConstantBuffer> createConstantBuffer(const DirectX12DescriptorLayout& layout, const BufferUsage& usage, const UInt32& elements) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Texture> createTexture(const DirectX12DescriptorLayout& layout, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Sampler> createSampler(const DirectX12DescriptorLayout& layout, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const override;
	};

	/// <summary>
	/// Implements a DirectX12 graphics device.
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Device : public IGraphicsDevice<DirectX12GraphicsFactory, DirectX12Surface, DirectX12GraphicsAdapter, DirectX12SwapChain, DirectX12Queue, DirectX12RenderPass>, public ComResource<ID3D12Device5> {
		LITEFX_IMPLEMENTATION(DirectX12DeviceImpl);

	public:
		/// <summary>
		/// Creates a new device instance.
		/// </summary>
		/// <param name="adapter">The adapter the device uses for drawing.</param>
		/// <param name="surface">The surface, the device should draw to.</param>
		/// <param name="backend">The backend from which the device got created.</param>
		explicit DirectX12Device(const DirectX12GraphicsAdapter& adapter, const DirectX12Surface& surface, const DirectX12Backend& backend);

		/// <summary>
		/// Creates a new device instance.
		/// </summary>
		/// <param name="adapter">The adapter the device uses for drawing.</param>
		/// <param name="surface">The surface, the device should draw to.</param>
		/// <param name="backend">The backend from which the device got created.</param>
		/// <param name="format">The initial surface format, device uses for drawing.</param>
		/// <param name="frameBufferSize">The initial size of the frame buffers.</param>
		/// <param name="frameBuffers">The initial number of frame buffers.</param>
		explicit DirectX12Device(const DirectX12GraphicsAdapter& adapter, const DirectX12Surface& surface, const DirectX12Backend& backend, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers);

		DirectX12Device(const DirectX12Device&) = delete;
		DirectX12Device(DirectX12Device&&) = delete;
		virtual ~DirectX12Device() noexcept;

		// DirectX12 Device interface.
	public:
		/// <summary>
		/// Returns the backend from which the device got created.
		/// </summary>
		/// <returns>The backend from which the device got created.</returns>
		virtual const DirectX12Backend& backend() const noexcept;

	public:
		/// <summary>
		/// Returns a builder for a <see cref="DirectX12RenderPass" />.
		/// </summary>
		/// <returns>An instance of a builder that is used to create a new render pass.</returns>
		/// <seealso cref="IGraphicsDevice::build" />
		DirectX12RenderPassBuilder buildRenderPass() const;

		// IGraphicsDevice interface.
	public:
		/// <inheritdoc />
		virtual const DirectX12SwapChain& swapChain() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12Surface& surface() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12GraphicsAdapter& adapter() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12GraphicsFactory& factory() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12Queue& graphicsQueue() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12Queue& transferQueue() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12Queue& bufferQueue() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void wait() const override;
	};
	
	/// <summary>
	/// Implements the DirectX12 <see cref="IRenderBackend" />.
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Backend : public IRenderBackend<DirectX12Device>, public ComResource<IDXGIFactory7> {
		LITEFX_IMPLEMENTATION(DirectX12BackendImpl);
		LITEFX_BUILDER(DirectX12BackendBuilder);

	public:
		explicit DirectX12Backend(const App& app, const bool& advancedSoftwareRasterizer = false);
		DirectX12Backend(const DirectX12Backend&) noexcept = delete;
		DirectX12Backend(DirectX12Backend&&) noexcept = delete;
		virtual ~DirectX12Backend();

		// IBackend interface.
	public:
		/// <inheritdoc />
		virtual BackendType getType() const noexcept override;

		// IRenderBackend interface.
	public:
		/// <inheritdoc />
		virtual Array<const DirectX12GraphicsAdapter*> listAdapters() const override;

		/// <inheritdoc />
		virtual const DirectX12GraphicsAdapter* findAdapter(const Optional<UInt32>& adapterId = std::nullopt) const override;

	public:
		/// <summary>
		/// Enables <a href="https://docs.microsoft.com/en-us/windows/win32/direct3darticles/directx-warp" target="_blank">Windows Advanced Software Rasterization (WARP)</a>.
		/// </summary>
		/// <remarks>
		/// Enabling software rasterization disables hardware rasterization. Requesting adapters using <see cref="findAdapter" /> or <see cref="listAdapters" />
		/// will only return WARP-compatible adapters.
		/// </remarks>
		/// <param name="enable"><c>true</c>, if advanced software rasterization should be used.</param>
		virtual void enableAdvancedSoftwareRasterizer(const bool& enable = false);
	};
}