#pragma once

#include <litefx/rendering.hpp>

#include "dx12_api.hpp"
#include "dx12_formatters.hpp"

namespace LiteFX::Rendering::Backends {
	using namespace LiteFX::Math;
	using namespace LiteFX::Rendering;

	/// <summary>
	/// Implements a DirectX 12 vertex buffer layout.
	/// </summary>
	/// <seealso cref="DirectX12VertexBuffer" />
	/// <seealso cref="DirectX12IndexBuffer" />
	/// <seealso cref="DirectX12VertexBufferLayoutBuilder" />
	class LITEFX_DIRECTX12_API DirectX12VertexBufferLayout : public IVertexBufferLayout {
		LITEFX_IMPLEMENTATION(DirectX12VertexBufferLayoutImpl);
		LITEFX_BUILDER(DirectX12VertexBufferLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a new vertex buffer layout.
		/// </summary>
		/// <param name="vertexSize">The size of a single vertex.</param>
		/// <param name="binding">The binding point of the vertex buffers using this layout.</param>
		explicit DirectX12VertexBufferLayout(const size_t& vertexSize, const UInt32& binding = 0);
		DirectX12VertexBufferLayout(DirectX12VertexBufferLayout&&) = delete;
		DirectX12VertexBufferLayout(const DirectX12VertexBufferLayout&) = delete;
		virtual ~DirectX12VertexBufferLayout() noexcept;

		// IVertexBufferLayout interface.
	public:
		/// <inheritdoc />
		virtual Enumerable<const BufferAttribute*> attributes() const noexcept override;

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
	/// Implements a DirectX 12 index buffer layout.
	/// </summary>
	/// <seealso cref="DirectX12IndexBuffer" />
	/// <seealso cref="DirectX12VertexBufferLayout" />
	class LITEFX_DIRECTX12_API DirectX12IndexBufferLayout : public IIndexBufferLayout {
		LITEFX_IMPLEMENTATION(DirectX12IndexBufferLayoutImpl);

	public:
		/// <summary>
		/// Initializes a new index buffer layout
		/// </summary>
		/// <param name="type">The type of the indices within the index buffer.</param>
		explicit DirectX12IndexBufferLayout(const IndexType& type);
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
	/// Represents the base interface for a DirectX 12 buffer implementation.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="IDirectX12Image" />
	/// <seealso cref="IDirectX12VertexBuffer" />
	/// <seealso cref="IDirectX12IndexBuffer" />
	class LITEFX_DIRECTX12_API IDirectX12Buffer : public virtual IBuffer, public virtual IResource<ComPtr<ID3D12Resource>> {
	public:
		virtual ~IDirectX12Buffer() noexcept = default;
	};

	/// <summary>
	/// Represents a DirectX 12 vertex buffer.
	/// </summary>
	/// <seealso cref="DirectX12VertexBufferLayout" />
	/// <seealso cref="IDirectX12Buffer" />
	class LITEFX_DIRECTX12_API IDirectX12VertexBuffer : public virtual VertexBuffer<DirectX12VertexBufferLayout>, public virtual IDirectX12Buffer {
	public:
		virtual ~IDirectX12VertexBuffer() noexcept = default;

	public:
		virtual const D3D12_VERTEX_BUFFER_VIEW& view() const noexcept = 0;
	};

	/// <summary>
	/// Represents a DirectX 12 index buffer.
	/// </summary>
	/// <seealso cref="DirectX12IndexBufferLayout" />
	/// <seealso cref="IDirectX12Buffer" />
	class LITEFX_DIRECTX12_API IDirectX12IndexBuffer : public virtual IndexBuffer<DirectX12IndexBufferLayout>, public virtual IDirectX12Buffer {
	public:
		virtual ~IDirectX12IndexBuffer() noexcept = default;

	public:
		virtual const D3D12_INDEX_BUFFER_VIEW& view() const noexcept = 0;
	};

	/// <summary>
	/// Represents a DirectX 12 sampled image or the base interface for a texture.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorLayout" />
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	/// <seealso cref="IDirectX12Sampler" />
	class LITEFX_DIRECTX12_API IDirectX12Image : public virtual IImage, public virtual IResource<ComPtr<ID3D12Resource>> {
	public:
		friend class DirectX12Barrier;

	public:
		virtual ~IDirectX12Image() noexcept = default;

	private:
		virtual ImageLayout& layout(const UInt32& subresource) = 0;
	};

	/// <summary>
	/// Represents a DirectX 12 sampler.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorLayout" />
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	/// <seealso cref="IDirectX12Image" />
	class LITEFX_DIRECTX12_API IDirectX12Sampler : public virtual ISampler {
	public:
		virtual ~IDirectX12Sampler() noexcept = default;
	};

	/// <summary>
	/// Implements a DirectX 12 resource barrier.
	/// </summary>
	/// <seealso cref="DirectX12CommandBuffer" />
	/// <seealso cref="IDirectX12Buffer" />
	/// <seealso cref="IDirectX12Image" />
	/// <seealso cref="Barrier" />
	class LITEFX_DIRECTX12_API DirectX12Barrier : public Barrier<IDirectX12Buffer, IDirectX12Image> {
		LITEFX_IMPLEMENTATION(DirectX12BarrierImpl);
		LITEFX_BUILDER(DirectX12BarrierBuilder);

	public:
		using base_type = Barrier<IDirectX12Buffer, IDirectX12Image>;
		using base_type::transition;

	public:
		/// <summary>
		/// Initializes a new DirectX 12 barrier.
		/// </summary>
		/// <param name="syncBefore">The pipeline stage(s) all previous commands have to finish before the barrier is executed.</param>
		/// <param name="syncAfter">The pipeline stage(s) all subsequent commands are blocked at until the barrier is executed.</param>
		explicit DirectX12Barrier(const PipelineStage& syncBefore, const PipelineStage& syncAfter) noexcept;
		DirectX12Barrier(const DirectX12Barrier&) = delete;
		DirectX12Barrier(DirectX12Barrier&&) = delete;
		virtual ~DirectX12Barrier() noexcept;

	private:
		explicit DirectX12Barrier() noexcept;
		PipelineStage& syncBefore() noexcept;
		PipelineStage& syncAfter() noexcept;

		// Barrier interface.
	public:
		/// <inheritdoc />
		virtual const PipelineStage& syncBefore() const noexcept override;

		/// <inheritdoc />
		virtual const PipelineStage& syncAfter() const noexcept override;

		/// <inheritdoc />
		virtual void wait(const ResourceAccess& accessBefore, const ResourceAccess& accessAfter) noexcept override;

		/// <inheritdoc />
		virtual void transition(IDirectX12Buffer& buffer, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter) override;

		/// <inheritdoc />
		virtual void transition(IDirectX12Buffer& buffer, const UInt32& element, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter) override;

		/// <inheritdoc />
		virtual void transition(IDirectX12Image& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout) override;

		/// <inheritdoc />
		virtual void transition(IDirectX12Image& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout) override;

		/// <inheritdoc />
		virtual void transition(IDirectX12Image& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout) override;

		/// <inheritdoc />
		virtual void transition(IDirectX12Image& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout) override;

	public:
		/// <summary>
		/// Adds the barrier to a command buffer and updates the resource target states.
		/// </summary>
		/// <param name="commandBuffer">The command buffer to add the barriers to.</param>
		/// <exception cref="RuntimeException">Thrown, if any of the contained barriers is a image barrier that targets a sub-resource range that does not share the same <see cref="ImageLayout" /> in all sub-resources.</exception>
		virtual void execute(const DirectX12CommandBuffer& commandBuffer) const;
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="IShaderModule" />.
	/// </summary>
	/// <seealso cref="DirectX12ShaderProgram" />
	/// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
	class LITEFX_DIRECTX12_API DirectX12ShaderModule : public IShaderModule, public ComResource<IDxcBlob> {
		LITEFX_IMPLEMENTATION(DirectX12ShaderModuleImpl);

	public:
		/// <summary>
		/// Initializes a new DirectX 12 shader module.
		/// </summary>
		/// <param name="device">The parent device, this shader module has been created from.</param>
		/// <param name="type">The shader stage, this module is used in.</param>
		/// <param name="fileName">The file name of the module source.</param>
		/// <param name="entryPoint">The name of the module entry point.</param>
		explicit DirectX12ShaderModule(const DirectX12Device& device, const ShaderStage& type, const String& fileName, const String& entryPoint = "main");

		/// <summary>
		/// Initializes a new DirectX 12 shader module.
		/// </summary>
		/// <param name="device">The parent device, this shader module has been created from.</param>
		/// <param name="type">The shader stage, this module is used in.</param>
		/// <param name="stream">The file stream to read the shader module from.</param>
		/// <param name="name">The file name of the module source.</param>
		/// <param name="entryPoint">The name of the module entry point.</param>
		explicit DirectX12ShaderModule(const DirectX12Device& device, const ShaderStage& type, std::istream& stream, const String& name, const String& entryPoint = "main");
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
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="ShaderProgram" />.
	/// </summary>
	/// <seealso cref="DirectX12ShaderProgramBuilder" />
	/// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
	class LITEFX_DIRECTX12_API DirectX12ShaderProgram : public ShaderProgram<DirectX12ShaderModule> {
		LITEFX_IMPLEMENTATION(DirectX12ShaderProgramImpl);
		LITEFX_BUILDER(DirectX12ShaderProgramBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX 12 shader program.
		/// </summary>
		/// <param name="device">The parent device of the shader program.</param>
		/// <param name="modules">The shader modules used by the shader program.</param>
		explicit DirectX12ShaderProgram(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12ShaderModule>>&& modules) noexcept;
		DirectX12ShaderProgram(DirectX12ShaderProgram&&) noexcept = delete;
		DirectX12ShaderProgram(const DirectX12ShaderProgram&) noexcept = delete;
		virtual ~DirectX12ShaderProgram() noexcept;

	private:
		/// <summary>
		/// Initializes a new DirectX 12 shader program.
		/// </summary>
		/// <param name="device">The parent device of the shader program.</param>
		explicit DirectX12ShaderProgram(const DirectX12Device& device) noexcept;

	public:
		/// <inheritdoc />
		virtual Enumerable<const DirectX12ShaderModule*> modules() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<DirectX12PipelineLayout> reflectPipelineLayout() const;

	private:
		virtual SharedPtr<IPipelineLayout> parsePipelineLayout() const override {
			return std::static_pointer_cast<IPipelineLayout>(this->reflectPipelineLayout());
		}

	public:
		/// <summary>
		/// Suppresses the warning that is issued, if no root signature is found on a shader module when calling <see cref="reflectPipelineLayout" />.
		/// </summary>
		/// <remarks>
		/// When a shader program is asked to build a pipeline layout, it first checks if a root signature is provided within the shader bytecode. If no root signature could 
		/// be found, it falls back to using plain reflection to extract the descriptor sets. This has the drawback, that some features are not or only partially supported.
		/// Most notably, it is not possible to reflect a pipeline layout that uses push constants this way. To ensure that you are not missing the root signature by accident,
		/// the engine warns you when it encounters this situation. However, if you are only using plain descriptor sets, this can result in noise warnings that clutter the 
		/// log. You can call this function to disable the warnings explicitly.
		/// </remarks>
		/// <param name="disableWarning"><c>true</c> to stop issuing the warning or <c>false</c> to continue.</param>
		/// <seealso cref="reflectPipelineLayout" />
		static void suppressMissingRootSignatureWarning(bool disableWarning = true) noexcept;
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="DescriptorSet" />.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	class LITEFX_DIRECTX12_API DirectX12DescriptorSet : public DescriptorSet<IDirectX12Buffer, IDirectX12Image, IDirectX12Sampler> {
		LITEFX_IMPLEMENTATION(DirectX12DescriptorSetImpl);

	public:
		using base_type = DescriptorSet<IDirectX12Buffer, IDirectX12Image, IDirectX12Sampler>;
		using base_type::update;
		using base_type::attach;

	public:
		/// <summary>
		/// Initializes a new descriptor set.
		/// </summary>
		/// <param name="layout">The parent descriptor set layout.</param>
		/// <param name="bufferHeap">A CPU-visible descriptor heap that contains all buffer descriptors of the descriptor set.</param>
		/// <param name="samplerHeap">A CPU-visible descriptor heap that contains all sampler descriptors of the descriptor set.</param>
		explicit DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& bufferHeap, ComPtr<ID3D12DescriptorHeap>&& samplerHeap);
		DirectX12DescriptorSet(DirectX12DescriptorSet&&) = delete;
		DirectX12DescriptorSet(const DirectX12DescriptorSet&) = delete;
		virtual ~DirectX12DescriptorSet() noexcept;

	public:
		/// <summary>
		/// Returns the parent descriptor set layout.
		/// </summary>
		/// <returns>The parent descriptor set layout.</returns>
		virtual const DirectX12DescriptorSetLayout& layout() const noexcept;

	public:
		/// <inheritdoc />
		virtual void update(const UInt32& binding, const IDirectX12Buffer& buffer, const UInt32& bufferElement = 0, const UInt32& elements = 0, const UInt32& firstDescriptor = 0) const override;

		/// <inheritdoc />
		virtual void update(const UInt32& binding, const IDirectX12Image& texture, const UInt32& descriptor = 0, const UInt32& firstLevel = 0, const UInt32& levels = 0, const UInt32& firstLayer = 0, const UInt32& layers = 0) const override;

		/// <inheritdoc />
		virtual void update(const UInt32& binding, const IDirectX12Sampler& sampler, const UInt32& descriptor = 0) const override;

		/// <inheritdoc />
		virtual void attach(const UInt32& binding, const IDirectX12Image& image) const override;

	public:
		/// <summary>
		/// Returns the local (CPU-visible) heap that contains the buffer descriptors.
		/// </summary>
		/// <returns>The local (CPU-visible) heap that contains the buffer descriptors, or <c>nullptr</c>, if the descriptor set does not contain any buffers.</returns>
		virtual const ComPtr<ID3D12DescriptorHeap>& bufferHeap() const noexcept;

		/// <summary>
		/// Returns the offset of the buffer descriptors in the global descriptor heap.
		/// </summary>
		/// <returns>The offset of the buffer descriptors in the global descriptor heap.</returns>
		virtual const UInt32& bufferOffset() const noexcept;

		/// <summary>
		/// Returns the local (CPU-visible) heap that contains the sampler descriptors.
		/// </summary>
		/// <returns>The local (CPU-visible) heap that contains the sampler descriptors, or <c>nullptr</c>, if the descriptor set does not contain any samplers.</returns>
		virtual const ComPtr<ID3D12DescriptorHeap>& samplerHeap() const noexcept;

		/// <summary>
		/// Returns the offset of the sampler descriptors in the global descriptor heap.
		/// </summary>
		/// <returns>The offset of the sampler descriptors in the global descriptor heap.</returns>
		virtual const UInt32& samplerOffset() const noexcept;
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="IDescriptorLayout" />
	/// </summary>
	/// <seealso cref="IDirectX12Buffer" />
	/// <seealso cref="IDirectX12Image" />
	/// <seealso cref="IDirectX12Sampler" />
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12DescriptorSetLayout" />
	class LITEFX_DIRECTX12_API DirectX12DescriptorLayout : public IDescriptorLayout {
		LITEFX_IMPLEMENTATION(DirectX12DescriptorLayoutImpl);

	public:
		/// <summary>
		/// Initializes a new DirectX 12 descriptor layout.
		/// </summary>
		/// <param name="type">The type of the descriptor.</param>
		/// <param name="binding">The binding point for the descriptor.</param>
		/// <param name="elementSize">The size of the descriptor.</param>
		/// <param name="elementSize">The number of descriptors in the descriptor array.</param>
		explicit DirectX12DescriptorLayout(const DescriptorType& type, const UInt32& binding, const size_t& elementSize, const UInt32& descriptors = 1);

		/// <summary>
		/// Initializes a new DirectX 12 descriptor layout for a static sampler.
		/// </summary>
		/// <param name="staticSampler">The static sampler to initialize the state with.</param>
		/// <param name="binding">The binding point for the descriptor.</param>
		explicit DirectX12DescriptorLayout(UniquePtr<IDirectX12Sampler>&& staticSampler, const UInt32& binding);

		DirectX12DescriptorLayout(DirectX12DescriptorLayout&&) = delete;
		DirectX12DescriptorLayout(const DirectX12DescriptorLayout&) = delete;
		virtual ~DirectX12DescriptorLayout() noexcept;

		// IDescriptorLayout interface.
	public:
		/// <inheritdoc />
		virtual const DescriptorType& descriptorType() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& descriptors() const noexcept override;

		/// <inheritdoc />
		virtual const IDirectX12Sampler* staticSampler() const noexcept override;

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
	/// Implements a DirectX 12 <see cref="DescriptorSetLayout" />.
	/// </summary>
	/// <seealso cref="DirectX12DescriptorSet" />
	/// <seealso cref="DirectX12PipelineDescriptorSetLayoutBuilder" />
	class LITEFX_DIRECTX12_API DirectX12DescriptorSetLayout : public DescriptorSetLayout<DirectX12DescriptorLayout, DirectX12DescriptorSet> {
		LITEFX_IMPLEMENTATION(DirectX12DescriptorSetLayoutImpl);
		LITEFX_BUILDER(DirectX12DescriptorSetLayoutBuilder);
		friend class DirectX12PipelineLayout;

	public:
		using base_type = DescriptorSetLayout<DirectX12DescriptorLayout, DirectX12DescriptorSet>;
		using base_type::free;

	public:
		/// <summary>
		/// Initializes a DirectX 12 descriptor set layout.
		/// </summary>
		/// <param name="device">The device, the descriptor set layout is created on.</param>
		/// <param name="descriptorLayouts">The descriptor layouts of the descriptors within the descriptor set.</param>
		/// <param name="space">The space or set id of the descriptor set.</param>
		/// <param name="stages">The shader stages, the descriptor sets are bound to.</param>
		explicit DirectX12DescriptorSetLayout(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12DescriptorLayout>>&& descriptorLayouts, const UInt32& space, const ShaderStage& stages);
		DirectX12DescriptorSetLayout(DirectX12DescriptorSetLayout&&) = delete;
		DirectX12DescriptorSetLayout(const DirectX12DescriptorSetLayout&) = delete;
		virtual ~DirectX12DescriptorSetLayout() noexcept;

	private:
		/// <summary>
		/// Initializes a DirectX 12 descriptor set layout.
		/// </summary>
		/// <param name="device">The device, the descriptor set layout is created on.</param>
		explicit DirectX12DescriptorSetLayout(const DirectX12Device& device) noexcept;

	public:
		/// <summary>
		/// Returns the index of the descriptor set root parameter.
		/// </summary>
		/// <returns>The index of the descriptor set root parameter.</returns>
		virtual const UInt32& rootParameterIndex() const noexcept;

		/// <summary>
		/// Returns the index of the first descriptor for a certain binding. The offset is relative to the heap for the descriptor (i.e. sampler for sampler descriptors and
		/// CBV/SRV/UAV for other descriptors).
		/// </summary>
		/// <param name="binding">The binding of the descriptor.</param>
		/// <returns>The index of the first descriptor for the binding.</returns>
		/// <exception cref="ArgumentOutOfRangeException">Thrown, if the descriptor set does not contain a descriptor bound to the binding point specified by <paramref name="binding"/>.</exception>
		virtual UInt32 descriptorOffsetForBinding(const UInt32& binding) const;

		/// <summary>
		/// Returns the parent device.
		/// </summary>
		/// <returns>A reference of the parent device.</returns>
		virtual const DirectX12Device& device() const noexcept;

	protected:
		/// <summary>
		/// Returns a reference of the index of the descriptor set root parameter.
		/// </summary>
		/// <returns>A reference of the index of the descriptor set root parameter.</returns>
		virtual UInt32& rootParameterIndex() noexcept;

		/// <summary>
		/// Returns <c>true</c>, if the descriptor set contains an (unbounded) runtime array.
		/// </summary>
		/// <remarks>
		/// A descriptor set is a runtime array, if it contains exactly one descriptor, which is an unbounded array, i.e. which has a descriptor count of `-1` (or `0xFFFFFFFF`).
		/// </remarks>
		/// <returns><c>true</c>, if the descriptor set contains an (unbounded) runtime array and <c>false</c> otherwise.</returns>
		virtual bool isRuntimeArray() const noexcept;

	public:
		/// <inheritdoc />
		virtual Enumerable<const DirectX12DescriptorLayout*> descriptors() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12DescriptorLayout& descriptor(const UInt32& binding) const override;

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
		virtual UInt32 buffers() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 samplers() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 staticSamplers() const noexcept override;

		/// <inheritdoc />
		virtual UInt32 inputAttachments() const noexcept override;

	public:
		/// <inheritdoc />
		virtual UniquePtr<DirectX12DescriptorSet> allocate(const Enumerable<DescriptorBinding>& bindings = { }) const override;

		/// <inheritdoc />
		virtual UniquePtr<DirectX12DescriptorSet> allocate(const UInt32& descriptors, const Enumerable<DescriptorBinding>& bindings = { }) const override;

		/// <inheritdoc />
		virtual Enumerable<UniquePtr<DirectX12DescriptorSet>> allocateMultiple(const UInt32& descriptorSets, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const override;

		/// <inheritdoc />
		virtual Enumerable<UniquePtr<DirectX12DescriptorSet>> allocateMultiple(const UInt32& descriptorSets, std::function<Enumerable<DescriptorBinding>(const UInt32&)> bindingFactory) const override;

		/// <inheritdoc />
		virtual Enumerable<UniquePtr<DirectX12DescriptorSet>> allocateMultiple(const UInt32& descriptorSets, const UInt32& descriptors, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const override;

		/// <inheritdoc />
		virtual Enumerable<UniquePtr<DirectX12DescriptorSet>> allocateMultiple(const UInt32& descriptorSets, const UInt32& descriptors, std::function<Enumerable<DescriptorBinding>(const UInt32&)> bindingFactory) const override;

		/// <inheritdoc />
		virtual void free(const DirectX12DescriptorSet& descriptorSet) const noexcept override;
	};

	/// <summary>
	/// Implements the DirectX 12 <see cref="IPushConstantsRange" />.
	/// </summary>
	/// <seealso cref="DirectX12PushConstantsLayout" />
	class LITEFX_DIRECTX12_API DirectX12PushConstantsRange : public IPushConstantsRange {
		LITEFX_IMPLEMENTATION(DirectX12PushConstantsRangeImpl);
		friend class DirectX12PipelineLayout;

	public:
		/// <summary>
		/// Initializes a new push constants range.
		/// </summary>
		/// <param name="shaderStages">The shader stages, that access the push constants from the range.</param>
		/// <param name="offset">The offset relative to the parent push constants backing memory that marks the beginning of the range.</param>
		/// <param name="size">The size of the push constants range.</param>
		/// <param name="space">The space from which the push constants of the range will be accessible in the shader.</param>
		/// <param name="binding">The register from which the push constants of the range will be accessible in the shader.</param>
		explicit DirectX12PushConstantsRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding);
		DirectX12PushConstantsRange(const DirectX12PushConstantsRange&) = delete;
		DirectX12PushConstantsRange(DirectX12PushConstantsRange&&) = delete;
		virtual ~DirectX12PushConstantsRange() noexcept;

	public:
		/// <inheritdoc />
		virtual const UInt32& space() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& binding() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& offset() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& size() const noexcept override;

		/// <inheritdoc />
		virtual const ShaderStage& stage() const noexcept override;

	public:
		/// <summary>
		/// Returns the index of the root parameter, the range is bound to.
		/// </summary>
		/// <returns>The index of the root parameter, the range is bound to.</returns>
		virtual const UInt32& rootParameterIndex() const noexcept;

	protected:
		/// <summary>
		/// Returns a reference of the index of the root parameter, the range is bound to.
		/// </summary>
		/// <returns>A reference of the index of the root parameter, the range is bound to.</returns>
		virtual UInt32& rootParameterIndex() noexcept;
	};

	/// <summary>
	/// Implements the DirectX 12 <see cref="PushConstantsLayout" />.
	/// </summary>
	/// <remarks>
	/// In DirectX 12, push constants map to root constants. Those are 32 bit values that are directly stored on the root signature. Thus, push constants can bloat your root 
	/// signature, since all the required memory is directly reserved on it. The way they are implemented is, that each range gets directly written in 4 byte chunks into the
	/// command buffer. Thus, overlapping is not directly supported (as opposed to Vulkan). If you have overlapping push constants ranges, the overlap will be duplicated in
	/// the root signature.
	/// </remarks>
	/// <seealso cref="DirectX12PushConstantsRange" />
	/// <seealso cref="DirectX12PipelinePushConstantsLayoutBuilder" />
	class LITEFX_DIRECTX12_API DirectX12PushConstantsLayout : public PushConstantsLayout<DirectX12PushConstantsRange> {
		LITEFX_IMPLEMENTATION(DirectX12PushConstantsLayoutImpl);
		LITEFX_BUILDER(DirectX12PushConstantsLayoutBuilder);
		friend class DirectX12PipelineLayout;

	public:
		/// <summary>
		/// Initializes a new push constants layout.
		/// </summary>
		/// <param name="ranges">The ranges contained by the layout.</param>
		/// <param name="size">The overall size (in bytes) of the push constants backing memory.</param>
		explicit DirectX12PushConstantsLayout(Enumerable<UniquePtr<DirectX12PushConstantsRange>>&& ranges, const UInt32& size);
		DirectX12PushConstantsLayout(const DirectX12PushConstantsLayout&) = delete;
		DirectX12PushConstantsLayout(DirectX12PushConstantsLayout&&) = delete;
		virtual ~DirectX12PushConstantsLayout() noexcept;

	private:
		/// <summary>
		/// Initializes a new push constants layout.
		/// </summary>
		/// <param name="size">The overall size (in bytes) of the push constants backing memory.</param>
		explicit DirectX12PushConstantsLayout(const UInt32& size);

	public:
		/// <inheritdoc />
		virtual const UInt32& size() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12PushConstantsRange& range(const ShaderStage& stage) const override;

		/// <inheritdoc />
		virtual Enumerable<const DirectX12PushConstantsRange*> ranges() const noexcept override;

	protected:
		/// <summary>
		/// Returns an array of pointers to the push constant ranges of the layout.
		/// </summary>
		/// <returns>An array of pointers to the push constant ranges of the layout.</returns>
		virtual Enumerable<DirectX12PushConstantsRange*> ranges() noexcept;
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="PipelineLayout" />.
	/// </summary>
	/// <seealso cref="DirectX12PipelineLayoutBuilder" />
	class LITEFX_DIRECTX12_API DirectX12PipelineLayout : public PipelineLayout<DirectX12DescriptorSetLayout, DirectX12PushConstantsLayout>, public ComResource<ID3D12RootSignature> {
		LITEFX_IMPLEMENTATION(DirectX12PipelineLayoutImpl);
		LITEFX_BUILDER(DirectX12PipelineLayoutBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX 12 render pipeline layout.
		/// </summary>
		/// <param name="device">The parent device, the layout is created from.</param>
		/// <param name="descriptorSetLayouts">The descriptor set layouts used by the pipeline.</param>
		/// <param name="pushConstantsLayout">The push constants layout used by the pipeline.</param>
		explicit DirectX12PipelineLayout(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12DescriptorSetLayout>>&& descriptorSetLayouts, UniquePtr<DirectX12PushConstantsLayout>&& pushConstantsLayout);
		DirectX12PipelineLayout(DirectX12PipelineLayout&&) noexcept = delete;
		DirectX12PipelineLayout(const DirectX12PipelineLayout&) noexcept = delete;
		virtual ~DirectX12PipelineLayout() noexcept;

	private:
		/// <summary>
		/// Initializes a new DirectX 12 render pipeline layout.
		/// </summary>
		/// <param name="device">The parent device, the layout is created from.</param>
		explicit DirectX12PipelineLayout(const DirectX12Device& device) noexcept;

	public:
		/// <summary>
		/// Returns a reference to the device that provides this layout.
		/// </summary>
		/// <returns>A reference to the layouts parent device.</returns>
		virtual const DirectX12Device& device() const noexcept;

		// PipelineLayout interface.
	public:
		/// <inheritdoc />
		virtual const DirectX12DescriptorSetLayout& descriptorSet(const UInt32& space) const override;

		/// <inheritdoc />
		virtual Enumerable<const DirectX12DescriptorSetLayout*> descriptorSets() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12PushConstantsLayout* pushConstants() const noexcept override;
	};

	/// <summary>
	/// Implements the DirectX 12 input assembler state.
	/// </summary>
	/// <seealso cref="DirectX12InputAssemblerBuilder" />
	class LITEFX_DIRECTX12_API DirectX12InputAssembler : public InputAssembler<DirectX12VertexBufferLayout, DirectX12IndexBufferLayout> {
		LITEFX_IMPLEMENTATION(DirectX12InputAssemblerImpl);
		LITEFX_BUILDER(DirectX12InputAssemblerBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX 12 input assembler state.
		/// </summary>
		/// <param name="vertexBufferLayouts">The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.</param>
		/// <param name="indexBufferLayout">The index buffer layout.</param>
		/// <param name="primitiveTopology">The primitive topology.</param>
		explicit DirectX12InputAssembler(Enumerable<UniquePtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<DirectX12IndexBufferLayout>&& indexBufferLayout, const PrimitiveTopology& primitiveTopology = PrimitiveTopology::TriangleList);
		DirectX12InputAssembler(DirectX12InputAssembler&&) noexcept = delete;
		DirectX12InputAssembler(const DirectX12InputAssembler&) noexcept = delete;
		virtual ~DirectX12InputAssembler() noexcept;

	private:
		/// <summary>
		/// Initializes a new DirectX 12 input assembler state.
		/// </summary>
		explicit DirectX12InputAssembler() noexcept;

	public:
		/// <inheritdoc />
		virtual Enumerable<const DirectX12VertexBufferLayout*> vertexBufferLayouts() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12VertexBufferLayout& vertexBufferLayout(const UInt32& binding) const override;

		/// <inheritdoc />
		virtual const DirectX12IndexBufferLayout& indexBufferLayout() const override;

		/// <inheritdoc />
		virtual const PrimitiveTopology& topology() const noexcept override;
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="IRasterizer" />.
	/// </summary>
	/// <seealso cref="DirectX12RasterizerBuilder" />
	class LITEFX_DIRECTX12_API DirectX12Rasterizer : public Rasterizer {
		LITEFX_BUILDER(DirectX12RasterizerBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX 12 rasterizer state.
		/// </summary>
		/// <param name="polygonMode">The polygon mode used by the pipeline.</param>
		/// <param name="cullMode">The cull mode used by the pipeline.</param>
		/// <param name="cullOrder">The cull order used by the pipeline.</param>
		/// <param name="lineWidth">The line width used by the pipeline.</param>
		/// <param name="depthStencilState">The rasterizer depth/stencil state.</param>
		explicit DirectX12Rasterizer(const PolygonMode& polygonMode, const CullMode& cullMode, const CullOrder& cullOrder, const Float& lineWidth = 1.f, const DepthStencilState& depthStencilState = {}) noexcept;
		DirectX12Rasterizer(DirectX12Rasterizer&&) noexcept = delete;
		DirectX12Rasterizer(const DirectX12Rasterizer&) noexcept = delete;
		virtual ~DirectX12Rasterizer() noexcept;

	private:
		/// <summary>
		/// Initializes a new DirectX 12 rasterizer state.
		/// </summary>
		explicit DirectX12Rasterizer() noexcept;
	};

	/// <summary>
	/// Defines the base class for DirectX 12 pipeline state objects.
	/// </summary>
	/// <seealso cref="DirectX12RenderPipeline" />
	/// <seealso cref="DirectX12ComputePipeline" />
	class LITEFX_DIRECTX12_API DirectX12PipelineState : public virtual Pipeline<DirectX12PipelineLayout, DirectX12ShaderProgram>, public ComResource<ID3D12PipelineState> {
	public:
		using ComResource<ID3D12PipelineState>::ComResource;
		virtual ~DirectX12PipelineState() noexcept = default;

	public:
		/// <summary>
		/// Sets the current pipeline state on the <paramref name="commandBuffer" />.
		/// </summary>
		/// <param name="commandBuffer">The command buffer to set the current pipeline state on.</param>
		virtual void use(const DirectX12CommandBuffer& commandBuffer) const noexcept = 0;
	};

	/// <summary>
	/// Records commands for a <see cref="DirectX12CommandQueue" />
	/// </summary>
	/// <seealso cref="DirectX12CommandQueue" />
	class LITEFX_DIRECTX12_API DirectX12CommandBuffer : public CommandBuffer<DirectX12CommandBuffer, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, DirectX12Barrier, DirectX12PipelineState>, public ComResource<ID3D12GraphicsCommandList7> {
		LITEFX_IMPLEMENTATION(DirectX12CommandBufferImpl);

	public:
		using base_type = CommandBuffer<DirectX12CommandBuffer, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, DirectX12Barrier, DirectX12PipelineState>;
		using base_type::dispatch;
		using base_type::draw;
		using base_type::drawIndexed;
		using base_type::barrier;
		using base_type::transfer;
		using base_type::generateMipMaps;
		using base_type::bind;
		using base_type::use;
		using base_type::pushConstants;

	public:
		/// <summary>
		/// Initializes the command buffer from a command queue.
		/// </summary>
		/// <param name="queue">The parent command queue, the buffer gets submitted to.</param>
		/// <param name="begin">If set to <c>true</c>, the command buffer automatically starts recording by calling <see cref="begin" />.</param>
		/// <param name="primary"><c>true</c>, if the command buffer is a primary command buffer.</param>
		explicit DirectX12CommandBuffer(const DirectX12Queue& queue, const bool& begin = false, const bool& primary = true);
		DirectX12CommandBuffer(const DirectX12CommandBuffer&) = delete;
		DirectX12CommandBuffer(DirectX12CommandBuffer&&) = delete;
		virtual ~DirectX12CommandBuffer() noexcept;

		// CommandBuffer interface.
	public:
		/// <inheritdoc />
		virtual void begin() const override;

		/// <inheritdoc />
		virtual void end() const override;

		/// <inheritdoc />
		virtual const bool& isSecondary() const noexcept override;

		/// <inheritdoc />
		virtual void setViewports(Span<const IViewport*> viewports) const noexcept override;

		/// <inheritdoc />
		virtual void setViewports(const IViewport* viewport) const noexcept override;

		/// <inheritdoc />
		virtual void setScissors(Span<const IScissor*> scissors) const noexcept override;

		/// <inheritdoc />
		virtual void setScissors(const IScissor* scissor) const noexcept override;

		/// <inheritdoc />
		virtual void setBlendFactors(const Vector4f& blendFactors) const noexcept override;

		/// <inheritdoc />
		virtual void setStencilRef(const UInt32& stencilRef) const noexcept override;

		/// <inheritdoc />
		virtual void generateMipMaps(IDirectX12Image& image) noexcept override;

		/// <inheritdoc />
		virtual void barrier(const DirectX12Barrier& barrier) const noexcept override;

		/// <inheritdoc />
		virtual void transfer(IDirectX12Buffer& source, IDirectX12Buffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual void transfer(IDirectX12Buffer& source, IDirectX12Image& target, const UInt32& sourceElement = 0, const UInt32& firstSubresource = 0, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual void transfer(IDirectX12Image& source, IDirectX12Image& target, const UInt32& sourceSubresource = 0, const UInt32& targetSubresource = 0, const UInt32& subresources = 1) const override;

		/// <inheritdoc />
		virtual void transfer(IDirectX12Image& source, IDirectX12Buffer& target, const UInt32& firstSubresource = 0, const UInt32& targetElement = 0, const UInt32& subresources = 1) const override;

		/// <inheritdoc />
		virtual void transfer(SharedPtr<IDirectX12Buffer> source, IDirectX12Buffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual void transfer(SharedPtr<IDirectX12Buffer> source, IDirectX12Image& target, const UInt32& sourceElement = 0, const UInt32& firstSubresource = 0, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual void transfer(SharedPtr<IDirectX12Image> source, IDirectX12Image& target, const UInt32& sourceSubresource = 0, const UInt32& targetSubresource = 0, const UInt32& subresources = 1) const override;

		/// <inheritdoc />
		virtual void transfer(SharedPtr<IDirectX12Image> source, IDirectX12Buffer& target, const UInt32& firstSubresource = 0, const UInt32& targetElement = 0, const UInt32& subresources = 1) const override;

		/// <inheritdoc />
		virtual void use(const DirectX12PipelineState& pipeline) const noexcept override;

		/// <inheritdoc />
		virtual void bind(const DirectX12DescriptorSet& descriptorSet, const DirectX12PipelineState& pipeline) const noexcept override;

		/// <inheritdoc />
		virtual void bind(const IDirectX12VertexBuffer& buffer) const noexcept override;

		/// <inheritdoc />
		virtual void bind(const IDirectX12IndexBuffer& buffer) const noexcept override;

		/// <inheritdoc />
		virtual void dispatch(const Vector3u& threadCount) const noexcept override;

		/// <inheritdoc />
		virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const noexcept override;

		/// <inheritdoc />
		virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const noexcept override;
		
		/// <inheritdoc />
		virtual void pushConstants(const DirectX12PushConstantsLayout& layout, const void* const memory) const noexcept override;

		/// <inheritdoc />
		virtual void writeTimingEvent(SharedPtr<const TimingEvent> timingEvent) const override;

		/// <inheritdoc />
		virtual void execute(SharedPtr<const DirectX12CommandBuffer> commandBuffer) const override;

		/// <inheritdoc />
		virtual void execute(Span<SharedPtr<const DirectX12CommandBuffer>> commandBuffers) const override;

	private:
		virtual void releaseSharedState() const override;
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="RenderPipeline" />.
	/// </summary>
	/// <seealso cref="DirectX12ComputePipeline" />
	/// <seealso cref="DirectX12RenderPipelineBuilder" />
	class LITEFX_DIRECTX12_API DirectX12RenderPipeline : public virtual DirectX12PipelineState, public RenderPipeline<DirectX12PipelineLayout, DirectX12ShaderProgram, DirectX12InputAssembler, DirectX12Rasterizer> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPipelineImpl);
		LITEFX_BUILDER(DirectX12RenderPipelineBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX 12 render pipeline.
		/// </summary>
		/// <param name="renderPass">The parent render pass.</param>
		/// <param name="shaderProgram">The shader program used by the pipeline.</param>
		/// <param name="layout">The layout of the pipeline.</param>
		/// <param name="inputAssembler">The input assembler state of the pipeline.</param>
		/// <param name="rasterizer">The rasterizer state of the pipeline.</param>
		/// <param name="name">The optional name of the render pipeline.</param>
		/// <param name="enableAlphaToCoverage">Whether or not to enable Alpha-to-Coverage multi-sampling.</param>
		explicit DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, SharedPtr<DirectX12InputAssembler> inputAssembler, SharedPtr<DirectX12Rasterizer> rasterizer, const bool enableAlphaToCoverage = false, const String& name = "");
		DirectX12RenderPipeline(DirectX12RenderPipeline&&) noexcept = delete;
		DirectX12RenderPipeline(const DirectX12RenderPipeline&) noexcept = delete;
		virtual ~DirectX12RenderPipeline() noexcept;

	private:
		/// <summary>
		/// Initializes a new DirectX 12 render pipeline.
		/// </summary>
		/// <param name="renderPass">The parent render pass.</param>
		/// <param name="name">The optional name of the render pipeline.</param>
		DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, const String& name = "") noexcept;

		// Pipeline interface.
	public:
		/// <inheritdoc />
		virtual SharedPtr<const DirectX12ShaderProgram> program() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<const DirectX12PipelineLayout> layout() const noexcept override;

		// RenderPipeline interface.
	public:
		/// <inheritdoc />
		virtual SharedPtr<DirectX12InputAssembler> inputAssembler() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<DirectX12Rasterizer> rasterizer() const noexcept override;

		/// <inheritdoc />
		virtual const bool& alphaToCoverage() const noexcept override;

		// DirectX12PipelineState interface.
	public:
		/// <inheritdoc />
		virtual void use(const DirectX12CommandBuffer& commandBuffer) const noexcept override;
	};

	/// <summary>
	/// Implements a DirectX 12 <see cref="ComputePipeline" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPipeline" />
	/// <seealso cref="DirectX12ComputePipelineBuilder" />
	class LITEFX_DIRECTX12_API DirectX12ComputePipeline : public virtual DirectX12PipelineState, public ComputePipeline<DirectX12PipelineLayout, DirectX12ShaderProgram> {
		LITEFX_IMPLEMENTATION(DirectX12ComputePipelineImpl);
		LITEFX_BUILDER(DirectX12ComputePipelineBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX 12 compute pipeline.
		/// </summary>
		/// <param name="device">The parent device.</param>
		/// <param name="layout">The layout of the pipeline.</param>
		/// <param name="shaderProgram">The shader program used by this pipeline.</param>
		/// <param name="name">The optional debug name of the compute pipeline.</param>
		explicit DirectX12ComputePipeline(const DirectX12Device& device, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, const String& name = "");
		DirectX12ComputePipeline(DirectX12ComputePipeline&&) noexcept = delete;
		DirectX12ComputePipeline(const DirectX12ComputePipeline&) noexcept = delete;
		virtual ~DirectX12ComputePipeline() noexcept;

	private:
		/// <summary>
		/// Initializes a new DirectX 12 compute pipeline.
		/// </summary>
		/// <param name="device">The parent device.</param>
		DirectX12ComputePipeline(const DirectX12Device& device) noexcept;

		// Pipeline interface.
	public:
		/// <inheritdoc />
		virtual SharedPtr<const DirectX12ShaderProgram> program() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<const DirectX12PipelineLayout> layout() const noexcept override;

		// DirectX12PipelineState interface.
	public:
		virtual void use(const DirectX12CommandBuffer& commandBuffer) const noexcept override;
	};

	/// <summary>
	/// Implements a DirectX 12 frame buffer.
	/// </summary>
	/// <seealso cref="DirectX12RenderPass" />
	class LITEFX_DIRECTX12_API DirectX12FrameBuffer : public FrameBuffer<DirectX12CommandBuffer> {
		LITEFX_IMPLEMENTATION(DirectX12FrameBufferImpl);

	public:
		/// <summary>
		/// Initializes a DirectX 12 frame buffer.
		/// </summary>
		/// <param name="renderPass">The parent render pass of the frame buffer.</param>
		/// <param name="bufferIndex">The index of the frame buffer within the parent render pass.</param>
		/// <param name="renderArea">The initial size of the render area.</param>
		/// <param name="commandBuffers">The number of command buffers, the frame buffer stores.</param>
		DirectX12FrameBuffer(const DirectX12RenderPass& renderPass, const UInt32& bufferIndex, const Size2d& renderArea, const UInt32& commandBuffers = 1);
		DirectX12FrameBuffer(const DirectX12FrameBuffer&) noexcept = delete;
		DirectX12FrameBuffer(DirectX12FrameBuffer&&) noexcept = delete;
		virtual ~DirectX12FrameBuffer() noexcept;

		// DirectX 12 FrameBuffer
	public:
		/// <summary>
		/// Returns a pointer to the descriptor heap that allocates the render targets for this frame buffer.
		/// </summary>
		/// <returns>A pointer to the descriptor heap that allocates the render targets for this frame buffer.</returns>
		/// <seealso cref="depthStencilTargetHeap" />
		/// <seealso cref="renderTargetDescriptorSize" />
		virtual ID3D12DescriptorHeap* renderTargetHeap() const noexcept;

		/// <summary>
		/// Returns a pointer to the descriptor heap that allocates the depth/stencil views for this frame buffer.
		/// </summary>
		/// <remarks>
		/// Note that it is typically not supported to have more than one depth/stencil output view bound to a <see cref="RenderPass" />.
		/// </remarks>
		/// <returns>A pointer to the descriptor heap that allocates the depth/stencil views for this frame buffer.</returns>
		/// <seealso cref="renderTargetHeap" />
		/// <seealso cref="depthStencilDescriptorSize" />
		virtual ID3D12DescriptorHeap* depthStencilTargetHeap() const noexcept;

		/// <summary>
		/// Returns the size of a descriptor for a render target within the frame buffer.
		/// </summary>
		/// <returns>The size of a descriptor for a render target within the frame buffer.</returns>
		/// <seealso cref="renderTargetHeap" />
		virtual const UInt32& renderTargetDescriptorSize() const noexcept;

		/// <summary>
		/// Returns the size of a descriptor for a depth/stencil view within the frame buffer.
		/// </summary>
		/// <returns>The size of a descriptor for a depth/stencil view within the frame buffer.</returns>
		/// <seealso cref="depthStencilTargetHeap" />
		virtual const UInt32& depthStencilTargetDescriptorSize() const noexcept;

		/// <summary>
		/// Returns a reference of the last fence value for the frame buffer.
		/// </summary>
		/// <remarks>
		/// The frame buffer must only be re-used, if this fence is reached in the graphics queue.
		/// </remarks>
		/// <returns>A reference of the last fence value for the frame buffer.</returns>
		virtual UInt64& lastFence() const noexcept;

		// FrameBuffer interface.
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
		virtual Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<const DirectX12CommandBuffer> commandBuffer(const UInt32& index) const override;

		/// <inheritdoc />
		virtual Enumerable<const IDirectX12Image*> images() const noexcept override;

		/// <inheritdoc />
		virtual const IDirectX12Image& image(const UInt32& location) const override;

	public:
		/// <inheritdoc />
		virtual void resize(const Size2d& renderArea) override;
	};

	/// <summary>
	/// Implements a DirectX 12 render pass.
	/// </summary>
	/// <seealso cref="DirectX12RenderPassBuilder" />
	class LITEFX_DIRECTX12_API DirectX12RenderPass : public RenderPass<DirectX12RenderPipeline, DirectX12FrameBuffer, DirectX12InputAttachmentMapping> {
		LITEFX_IMPLEMENTATION(DirectX12RenderPassImpl);
		LITEFX_BUILDER(DirectX12RenderPassBuilder);

	public:
		using base_type = RenderPass<DirectX12RenderPipeline, DirectX12FrameBuffer, DirectX12InputAttachmentMapping>;
		using base_type::updateAttachments;

	public:
		/// <summary>
		/// Creates and initializes a new DirectX 12 render pass instance.
		/// </summary>
		/// <param name="device">The parent device instance.</param>
		/// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
		/// <param name="renderTargets">The render targets that are output by the render pass.</param>
		/// <param name="samples">The number of samples for the render targets in this render pass.</param>
		/// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
		explicit DirectX12RenderPass(const DirectX12Device& device, Span<RenderTarget> renderTargets, const UInt32& commandBuffers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, Span<DirectX12InputAttachmentMapping> inputAttachments = { });

		/// <summary>
		/// Creates and initializes a new DirectX 12 render pass instance.
		/// </summary>
		/// <param name="device">The parent device instance.</param>
		/// <param name="name">The name of the render pass state resource.</param>
		/// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
		/// <param name="renderTargets">The render targets that are output by the render pass.</param>
		/// <param name="samples">The number of samples for the render targets in this render pass.</param>
		/// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
		explicit DirectX12RenderPass(const DirectX12Device& device, const String& name, Span<RenderTarget> renderTargets, const UInt32& commandBuffers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, Span<DirectX12InputAttachmentMapping> inputAttachments = { });

		DirectX12RenderPass(const DirectX12RenderPass&) = delete;
		DirectX12RenderPass(DirectX12RenderPass&&) = delete;
		virtual ~DirectX12RenderPass() noexcept;

	private:
		/// <summary>
		/// Creates an uninitialized DirectX 12 render pass instance.
		/// </summary>
		/// <remarks>
		/// This constructor is called by the <see cref="DirectX12RenderPassBuilder" /> in order to create a render pass instance without initializing it. The instance 
		/// is only initialized after calling <see cref="DirectX12RenderPassBuilder::go" />.
		/// </remarks>
		/// <param name="device">The parent device of the render pass.</param>
		/// <param name="name">The name of the render pass state resource.</param>
		explicit DirectX12RenderPass(const DirectX12Device& device, const String& name = "") noexcept;

		// IInputAttachmentMappingSource interface.
	public:
		/// <inheritdoc />
		virtual const DirectX12FrameBuffer& frameBuffer(const UInt32& buffer) const override;

		// RenderPass interface.
	public:
		/// <summary>
		/// Returns a reference to the device that provides this queue.
		/// </summary>
		/// <returns>A reference to the queue's parent device.</returns>
		virtual const DirectX12Device& device() const noexcept;

		/// <inheritdoc />
		virtual const DirectX12FrameBuffer& activeFrameBuffer() const override;

		/// <inheritdoc />
		virtual Enumerable<const DirectX12FrameBuffer*> frameBuffers() const noexcept override;

		/// <inheritdoc />
		virtual Enumerable<const DirectX12RenderPipeline*> pipelines() const noexcept override;

		/// <inheritdoc />
		virtual const RenderTarget& renderTarget(const UInt32& location) const override;

		/// <inheritdoc />
		virtual Span<const RenderTarget> renderTargets() const noexcept override;

		/// <inheritdoc />
		virtual bool hasPresentTarget() const noexcept override;

		/// <inheritdoc />
		virtual Span<const DirectX12InputAttachmentMapping> inputAttachments() const noexcept override;

		/// <inheritdoc />
		virtual const MultiSamplingLevel& multiSamplingLevel() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void begin(const UInt32& buffer) override;

		/// <inheritdoc />
		virtual void end() const override;

		/// <inheritdoc />
		virtual void resizeFrameBuffers(const Size2d& renderArea) override;

		/// <inheritdoc />
		virtual void changeMultiSamplingLevel(const MultiSamplingLevel& samples) override;

		/// <inheritdoc />
		virtual void updateAttachments(const DirectX12DescriptorSet& descriptorSet) const override;
	};

	/// <summary>
	/// Implements a <see cref="IInputAttachmentMapping" />.
	/// </summary>
	/// <seealso cref="DirectX12RenderPass" />
	/// <seealso cref="DirectX12RenderPassBuilder" />
	class LITEFX_DIRECTX12_API DirectX12InputAttachmentMapping : public IInputAttachmentMapping<DirectX12RenderPass> {
		LITEFX_IMPLEMENTATION(DirectX12InputAttachmentMappingImpl);

	public:
		/// <summary>
		/// Creates a new DirectX 12 input attachment mapping.
		/// </summary>
		DirectX12InputAttachmentMapping() noexcept;

		/// <summary>
		/// Creates a new DirectX 12 input attachment mapping.
		/// </summary>
		/// <param name="renderPass">The render pass to fetch the input attachment from.</param>
		/// <param name="renderTarget">The render target of the <paramref name="renderPass"/> that is used for the input attachment.</param>
		/// <param name="location">The location to bind the input attachment to.</param>
		DirectX12InputAttachmentMapping(const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget, const UInt32& location);

		/// <summary>
		/// Copies another input attachment mapping.
		/// </summary>
		DirectX12InputAttachmentMapping(const DirectX12InputAttachmentMapping&) noexcept;

		/// <summary>
		/// Takes over another input attachment mapping.
		/// </summary>
		DirectX12InputAttachmentMapping(DirectX12InputAttachmentMapping&&) noexcept;

		virtual ~DirectX12InputAttachmentMapping() noexcept;

	public:
		/// <summary>
		/// Copies another input attachment mapping.
		/// </summary>
		inline DirectX12InputAttachmentMapping& operator=(const DirectX12InputAttachmentMapping&) noexcept;

		/// <summary>
		/// Takes over another input attachment mapping.
		/// </summary>
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
	/// Implements a DirectX 12 swap chain.
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12SwapChain : public SwapChain<IDirectX12Image, DirectX12FrameBuffer>, public ComResource<IDXGISwapChain4> {
		LITEFX_IMPLEMENTATION(DirectX12SwapChainImpl);
		friend class DirectX12RenderPass;

	public:
		using base_type = SwapChain<IDirectX12Image, DirectX12FrameBuffer>;
		using base_type::present;

	public:
		/// <summary>
		/// Initializes a DirectX 12 swap chain.
		/// </summary>
		/// <param name="device">The device that owns the swap chain.</param>
		/// <param name="format">The initial surface format.</param>
		/// <param name="renderArea">The initial size of the render area.</param>
		/// <param name="buffers">The initial number of buffers.</param>
		explicit DirectX12SwapChain(const DirectX12Device& device, const Format& surfaceFormat = Format::B8G8R8A8_SRGB, const Size2d& renderArea = { 800, 600 }, const UInt32& buffers = 3);
		DirectX12SwapChain(const DirectX12SwapChain&) = delete;
		DirectX12SwapChain(DirectX12SwapChain&&) = delete;
		virtual ~DirectX12SwapChain() noexcept;

		// DirectX 12 swap chain.
	public:
		/// <summary>
		/// Returns <c>true</c>, if the adapter supports variable refresh rates (i.e. tearing is allowed).
		/// </summary>
		/// <returns><c>true</c>, if the adapter supports variable refresh rates (i.e. tearing is allowed).</returns>
		virtual const bool& supportsVariableRefreshRate() const noexcept;

		/// <summary>
		/// Returns the query heap for the current frame.
		/// </summary>
		/// <returns>A pointer to the query heap for the current frame.</returns>
		virtual ID3D12QueryHeap* timestampQueryHeap() const noexcept;

		// SwapChain interface.
	public:
		/// <inheritdoc />
		virtual Enumerable<SharedPtr<TimingEvent>> timingEvents() const noexcept override;

		/// <inheritdoc />
		virtual SharedPtr<TimingEvent> timingEvent(const UInt32& queryId) const override;

		/// <inheritdoc />
		virtual UInt64 readTimingEvent(SharedPtr<const TimingEvent> timingEvent) const override;

		/// <inheritdoc />
		virtual UInt32 resolveQueryId(SharedPtr<const TimingEvent> timingEvent) const override;

		/// <inheritdoc />
		virtual const Format& surfaceFormat() const noexcept override;

		/// <inheritdoc />
		virtual const UInt32& buffers() const noexcept override;

		/// <inheritdoc />
		virtual const Size2d& renderArea() const noexcept override;

		/// <inheritdoc />
		virtual const IDirectX12Image* image(const UInt32& backBuffer) const override;

		/// <inheritdoc />
		virtual Enumerable<const IDirectX12Image*> images() const noexcept override;

		/// <inheritdoc />
		virtual void present(const DirectX12FrameBuffer& frameBuffer) const override;

	public:
		/// <inheritdoc />
		virtual Enumerable<Format> getSurfaceFormats() const noexcept override;

		/// <inheritdoc />
		virtual void addTimingEvent(SharedPtr<TimingEvent> timingEvent) override;

		/// <inheritdoc />
		virtual void reset(const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers) override;

		/// <inheritdoc />
		[[nodiscard]] virtual UInt32 swapBackBuffer() const override;

	private:
		void resolveQueryHeaps(const DirectX12CommandBuffer& commandBuffer) const noexcept;
	};

	/// <summary>
	/// Implements a DirectX 12 command queue.
	/// </summary>
	/// <seealso cref="DirectX12CommandBuffer" />
	class LITEFX_DIRECTX12_API DirectX12Queue : public CommandQueue<DirectX12CommandBuffer>, public ComResource<ID3D12CommandQueue> {
		LITEFX_IMPLEMENTATION(DirectX12QueueImpl);

	public:
		using base_type = CommandQueue<DirectX12CommandBuffer>;
		using base_type::submit;

	public:
		/// <summary>
		/// Initializes the DirectX 12 command queue.
		/// </summary>
		/// <param name="device">The device, commands get send to.</param>
		/// <param name="type">The type of the command queue.</param>
		/// <param name="priority">The priority, of which commands are issued on the device.</param>
		explicit DirectX12Queue(const DirectX12Device& device, const QueueType& type, const QueuePriority& priority);
		DirectX12Queue(const DirectX12Queue&) = delete;
		DirectX12Queue(DirectX12Queue&&) = delete;
		virtual ~DirectX12Queue() noexcept;

		// DirectX12CommandQueue interface.
	public:
		/// <summary>
		/// Returns a reference to the device that provides this queue.
		/// </summary>
		/// <returns>A reference to the queue's parent device.</returns>
		virtual const DirectX12Device& device() const noexcept;

		// CommandQueue interface.
	public:
		/// <inheritdoc />
		virtual bool isBound() const noexcept override;

		/// <inheritdoc />
		virtual const QueuePriority& priority() const noexcept override;

		/// <inheritdoc />
		virtual const QueueType& type() const noexcept override;

#if !defined(NDEBUG) && defined(_WIN64)
	public:
		/// <inheritdoc />
		virtual void BeginDebugRegion(const String& label, const Vectors::ByteVector3& color = { 128_b, 128_b, 128_b }) const noexcept override;

		/// <inheritdoc />
		virtual void EndDebugRegion() const noexcept override;

		/// <inheritdoc />
		virtual void SetDebugMarker(const String& label, const Vectors::ByteVector3& color = { 128_b, 128_b, 128_b }) const noexcept override;
#endif // !defined(NDEBUG) && defined(_WIN64)

	public:
		/// <inheritdoc />
		virtual void bind() override;

		/// <inheritdoc />
		virtual void release() override;

		/// <inheritdoc />
		virtual SharedPtr<DirectX12CommandBuffer> createCommandBuffer(const bool& beginRecording = false, const bool& secondary = false) const override;

		/// <inheritdoc />
		virtual UInt64 submit(SharedPtr<const DirectX12CommandBuffer> commandBuffer) const override;

		/// <inheritdoc />
		virtual UInt64 submit(const Enumerable<SharedPtr<const DirectX12CommandBuffer>>& commandBuffers) const override;

		/// <inheritdoc />
		virtual void waitFor(const UInt64& fence) const noexcept override;

		/// <inheritdoc />
		virtual UInt64 currentFence() const noexcept override;
	};

	/// <summary>
	/// A graphics factory that produces objects for a <see cref="DirectX12Device" />.
	/// </summary>
	/// <remarks>
	/// The DX12 graphics factory is implemented using <a href="https://gpuopen.com/d3d12-memory-allocator/" target="_blank">D3D12 Memory Allocator</a>.
	/// </remarks>
	class LITEFX_DIRECTX12_API DirectX12GraphicsFactory : public GraphicsFactory<DirectX12DescriptorLayout, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, IDirectX12Sampler> {
		LITEFX_IMPLEMENTATION(DirectX12GraphicsFactoryImpl);

	public:
		using base_type = GraphicsFactory<DirectX12DescriptorLayout, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, IDirectX12Sampler>;
		using base_type::createBuffer;
		using base_type::createVertexBuffer;
		using base_type::createIndexBuffer;
		using base_type::createAttachment;
		using base_type::createTexture;
		using base_type::createTextures;
		using base_type::createSampler;
		using base_type::createSamplers;

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
		virtual UniquePtr<IDirectX12Buffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1, const bool& allowWrite = false) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Buffer> createBuffer(const String& name, const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1, const bool& allowWrite = false) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12VertexBuffer> createVertexBuffer(const DirectX12VertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12VertexBuffer> createVertexBuffer(const String& name, const DirectX12VertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12IndexBuffer> createIndexBuffer(const DirectX12IndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12IndexBuffer> createIndexBuffer(const String& name, const DirectX12IndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Image> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Image> createAttachment(const String& name, const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Image> createTexture(const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Image> createTexture(const String& name, const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const override;

		/// <inheritdoc />
		virtual Enumerable<UniquePtr<IDirectX12Image>> createTextures(const UInt32& elements, const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Sampler> createSampler(const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const override;

		/// <inheritdoc />
		virtual UniquePtr<IDirectX12Sampler> createSampler(const String& name, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const override;

		/// <inheritdoc />
		virtual Enumerable<UniquePtr<IDirectX12Sampler>> createSamplers(const UInt32& elements, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const override;
	};

	/// <summary>
	/// Implements a DirectX 12 graphics device.
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Device : public GraphicsDevice<DirectX12GraphicsFactory, DirectX12Surface, DirectX12GraphicsAdapter, DirectX12SwapChain, DirectX12Queue, DirectX12RenderPass, DirectX12ComputePipeline, DirectX12Barrier>, public ComResource<ID3D12Device10> {
		LITEFX_IMPLEMENTATION(DirectX12DeviceImpl);

	public:
		/// <summary>
		/// Creates a new device instance.
		/// </summary>
		/// <param name="backend">The backend from which the device got created.</param>
		/// <param name="adapter">The adapter the device uses for drawing.</param>
		/// <param name="surface">The surface, the device should draw to.</param>
		explicit DirectX12Device(const DirectX12Backend& backend, const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface);

		/// <summary>
		/// Creates a new device instance.
		/// </summary>
		/// <param name="backend">The backend from which the device got created.</param>
		/// <param name="adapter">The adapter the device uses for drawing.</param>
		/// <param name="surface">The surface, the device should draw to.</param>
		/// <param name="format">The initial surface format, device uses for drawing.</param>
		/// <param name="frameBufferSize">The initial size of the frame buffers.</param>
		/// <param name="frameBuffers">The initial number of frame buffers.</param>
		/// <param name="globalBufferHeapSize">The size of the global heap for constant buffers, shader resources and images.</param>
		/// <param name="globalSamplerHeapSize">The size of the global heap for samplers.</param>
		explicit DirectX12Device(const DirectX12Backend& backend, const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, const Format& format, const Size2d& frameBufferSize, const UInt32& frameBuffers, const UInt32& globalBufferHeapSize = 524287, const UInt32& globalSamplerHeapSize = 2048);

		DirectX12Device(const DirectX12Device&) = delete;
		DirectX12Device(DirectX12Device&&) = delete;
		virtual ~DirectX12Device() noexcept;

		// DirectX 12 Device interface.
	public:
		/// <summary>
		/// Returns the backend from which the device got created.
		/// </summary>
		/// <returns>The backend from which the device got created.</returns>
		virtual const DirectX12Backend& backend() const noexcept;

		/// <summary>
		/// Returns the global descriptor heap.
		/// </summary>
		/// <remarks>
		/// The DirectX 12 device uses a global heap of descriptors and samplers in a ring-buffer fashion. The heap itself is managed by the device.
		/// </remarks>
		/// <returns>A pointer to the global descriptor heap.</returns>
		virtual const ID3D12DescriptorHeap* globalBufferHeap() const noexcept;

		/// <summary>
		/// Returns the global sampler heap.
		/// </summary>
		/// <returns>A pointer to the global sampler heap.</returns>
		/// <seealso cref="globalBufferHeap" />
		virtual const ID3D12DescriptorHeap* globalSamplerHeap() const noexcept;

		/// <summary>
		/// Allocates a range of descriptors in the global descriptor heaps for the provided <paramref name="descriptorSet" />.
		/// </summary>
		/// <param name="descriptorSet">The descriptor set containing the descriptors to update.</param>
		/// <param name="bufferOffset">The offset of the descriptor range in the buffer heap.</param>
		/// <param name="samplerOffset">The offset of the descriptor range in the sampler heap.</param>
		virtual void allocateGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet, UInt32& bufferOffset, UInt32& samplerOffset) const;

		/// <summary>
		/// Releases a range of descriptors from the global descriptor heaps.
		/// </summary>
		/// <remarks>
		/// This is done, if a descriptor set layout is destroyed, of a descriptor set, which contains an unbounded array is freed. It will cause the global 
		/// descriptor heaps to fragment, which may result in inefficient future descriptor allocations and should be avoided. Consider caching descriptor
		/// sets with unbounded arrays instead. Also avoid relying on creating and releasing pipeline layouts during runtime. Instead, it may be more efficient
		/// to write shaders that support multiple pipeline variations, that can be kept alive for the lifetime of the whole application.
		/// </remarks>
		virtual void releaseGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet) const noexcept;

		/// <summary>
		/// Updates a range of descriptors in the global buffer descriptor heap with the descriptors from <paramref name="descriptorSet" />.
		/// </summary>
		/// <param name="descriptorSet">The descriptor set to copy the descriptors from.</param>
		/// <param name="firstDescriptor">The index of the first descriptor to copy.</param>
		/// <param name="descriptors">The number of descriptors to copy.</param>
		virtual void updateBufferDescriptors(const DirectX12DescriptorSet& descriptorSet, const UInt32& firstDescriptor, const UInt32& descriptors) const noexcept;

		/// <summary>
		/// Updates a sampler descriptors in the global buffer descriptor heap with a descriptor from <paramref name="descriptorSet" />.
		/// </summary>
		/// <param name="descriptorSet">The descriptor set to copy the descriptors from.</param>
		/// <param name="firstDescriptor">The index of the first descriptor to copy.</param>
		/// <param name="descriptors">The number of descriptors to copy.</param>
		virtual void updateSamplerDescriptors(const DirectX12DescriptorSet& descriptorSet, const UInt32& firstDescriptor, const UInt32& descriptors) const noexcept;

		/// <summary>
		/// Binds the descriptors of the descriptor set to the global descriptor heaps.
		/// </summary>
		/// <remarks>
		/// Note that after binding the descriptor set, the descriptors must not be updated anymore, unless they are elements on unbounded descriptor arrays, 
		/// in which case you have to ensure manually to not update them, as long as they may still be in use!
		/// </remarks>
		/// <param name="commandBuffer">The command buffer to bind the descriptor set on.</param>
		/// <param name="descriptorSet">The descriptor set to bind.</param>
		/// <param name="pipeline">The pipeline to bind the descriptor set to.</param>
		virtual void bindDescriptorSet(const DirectX12CommandBuffer& commandBuffer, const DirectX12DescriptorSet& descriptorSet, const DirectX12PipelineState& pipeline) const noexcept;

		/// <summary>
		/// Binds the global descriptor heap.
		/// </summary>
		/// <param name="commandBuffer">The command buffer to issue the bind command on.</param>
		virtual void bindGlobalDescriptorHeaps(const DirectX12CommandBuffer& commandBuffer) const noexcept;

		/// <summary>
		/// Returns the compute pipeline that can be invoked to blit an image resource.
		/// </summary>
		/// <remarks>
		/// Blitting is used by <see cref="DirectX12Texture" /> to generate mip maps.
		/// </remarks>
		/// <returns>The compute pipeline that can be invoked to blit an image resource.</returns>
		/// <seealso cref="DirectX12Texture::generateMipMaps" />
		virtual DirectX12ComputePipeline& blitPipeline() const noexcept;

		// GraphicsDevice interface.
	public:
		/// <inheritdoc />
		virtual DeviceState& state() const noexcept override;

		/// <inheritdoc />
		virtual const DirectX12SwapChain& swapChain() const noexcept override;

		/// <inheritdoc />
		virtual DirectX12SwapChain& swapChain() noexcept override;

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

		/// <inheritdoc />
		virtual const DirectX12Queue& computeQueue() const noexcept override;

		/// <inheritdoc />
		[[nodiscard]] virtual UniquePtr<DirectX12Barrier> makeBarrier(const PipelineStage& syncBefore, const PipelineStage& syncAfter) const noexcept override;

		/// <inheritdoc />
		/// <seealso href="https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels" />
		virtual MultiSamplingLevel maximumMultiSamplingLevel(const Format& format) const noexcept override;

		/// <inheritdoc />
		virtual double ticksPerMillisecond() const noexcept override;

	public:
		/// <inheritdoc />
		virtual void wait() const override;

#if defined(BUILD_DEFINE_BUILDERS)
	public:
		/// <inheritdoc />
		[[nodiscard]] virtual DirectX12RenderPassBuilder buildRenderPass(const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const UInt32& commandBuffers = 1) const override;

		/// <inheritdoc />
		[[nodiscard]] virtual DirectX12RenderPassBuilder buildRenderPass(const String& name, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const UInt32& commandBuffers = 1) const override;

		/// <inheritdoc />
		//[[nodiscard]] virtual DirectX12RenderPipelineBuilder buildRenderPipeline(const String& name) const override;

		/// <inheritdoc />
		[[nodiscard]] virtual DirectX12RenderPipelineBuilder buildRenderPipeline(const DirectX12RenderPass& renderPass, const String& name) const override;

		/// <inheritdoc />
		[[nodiscard]] virtual DirectX12ComputePipelineBuilder buildComputePipeline(const String& name) const override;
		
		/// <inheritdoc />
		[[nodiscard]] virtual DirectX12PipelineLayoutBuilder buildPipelineLayout() const override;

		/// <inheritdoc />
		[[nodiscard]] virtual DirectX12InputAssemblerBuilder buildInputAssembler() const override;

		/// <inheritdoc />
		[[nodiscard]] virtual DirectX12RasterizerBuilder buildRasterizer() const override;

		/// <inheritdoc />
		[[nodiscard]] virtual DirectX12ShaderProgramBuilder buildShaderProgram() const override;

		/// <inheritdoc />
		[[nodiscard]] virtual DirectX12BarrierBuilder buildBarrier() const override;
#endif // defined(BUILD_DEFINE_BUILDERS)
	};
	
	/// <summary>
	/// Implements the DirectX 12 <see cref="RenderBackend" />.
	/// </summary>
	class LITEFX_DIRECTX12_API DirectX12Backend : public RenderBackend<DirectX12Backend, DirectX12Device>, public ComResource<IDXGIFactory7> {
		LITEFX_IMPLEMENTATION(DirectX12BackendImpl);

	public:
		explicit DirectX12Backend(const App& app, const bool& advancedSoftwareRasterizer = false);
		DirectX12Backend(const DirectX12Backend&) noexcept = delete;
		DirectX12Backend(DirectX12Backend&&) noexcept = delete;
		virtual ~DirectX12Backend();

		// IBackend interface.
	public:
		/// <inheritdoc />
		virtual BackendType type() const noexcept override;

		/// <inheritdoc />
		virtual String name() const noexcept override;

	protected:
		/// <inheritdoc />
		virtual void activate() override;

		/// <inheritdoc />
		virtual void deactivate() override;

		// RenderBackend interface.
	public:
		/// <inheritdoc />
		virtual Enumerable<const DirectX12GraphicsAdapter*> listAdapters() const override;

		/// <inheritdoc />
		virtual const DirectX12GraphicsAdapter* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const override;

		/// <inheritdoc />
		virtual void registerDevice(String name, UniquePtr<DirectX12Device>&& device) override;

		/// <inheritdoc />
		virtual void releaseDevice(const String& name) override;

		/// <inheritdoc />
		virtual DirectX12Device* device(const String& name) noexcept override;

		/// <inheritdoc />
		virtual const DirectX12Device* device(const String& name) const noexcept override;

	public:
		/// <summary>
		/// Creates a surface on a window handle.
		/// </summary>
		/// <param name="hwnd">The window handle on which the surface should be created.</param>
		/// <returns>The instance of the created surface.</returns>
		UniquePtr<DirectX12Surface> createSurface(const HWND& hwnd) const;

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