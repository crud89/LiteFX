#pragma once

#include <litefx/rendering_api.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;
    using namespace LiteFX::Math;
    using namespace LiteFX::Graphics;

    /// <summary>
    /// Stores meta data about a buffer attribute.
    /// </summary>
    class LITEFX_RENDERING_API BufferAttribute {
        LITEFX_IMPLEMENTATION(BufferAttributeImpl);

    public:
        BufferAttribute();
        BufferAttribute(const UInt32& location, const UInt32& offset, const BufferFormat& format, const AttributeSemantic& semantic, const UInt32& semanticIndex = 0);
        BufferAttribute(BufferAttribute&&) noexcept;
        BufferAttribute(const BufferAttribute&);
        virtual ~BufferAttribute() noexcept;

    public:
        virtual const UInt32& getLocation() const noexcept;
        virtual const BufferFormat& getFormat() const noexcept;
        virtual const UInt32& getOffset() const noexcept;
        virtual const AttributeSemantic& getSemantic() const noexcept;
        virtual const UInt32& getSemanticIndex() const noexcept;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IBufferLayout {
    public:
        virtual ~IBufferLayout() noexcept = default;

    public:
        virtual size_t elementSize() const noexcept = 0;
        virtual const UInt32& binding() const noexcept = 0;
        virtual const BufferType& type() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IVertexBufferLayout : public IBufferLayout {
    public:
        virtual ~IVertexBufferLayout() noexcept = default;

    public:
        virtual Array<const BufferAttribute*> attributes() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IIndexBufferLayout : public IBufferLayout {
    public:
        virtual ~IIndexBufferLayout() noexcept = default;

    public:
        virtual const IndexType& indexType() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IDescriptorLayout : public IBufferLayout {
    public:
        virtual ~IDescriptorLayout() noexcept = default;

    public:
        virtual const DescriptorType& descriptorType() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IDescriptorSetLayout {
    public:
        virtual ~IDescriptorSetLayout() noexcept = default;

    public:
        virtual Array<const IDescriptorLayout*> getLayouts() const noexcept = 0;
        virtual const IDescriptorLayout* getLayout(const UInt32& binding) const noexcept = 0;
        virtual const UInt32& getSetId() const noexcept = 0;
        virtual const ShaderStage& getShaderStages() const noexcept = 0;
        virtual UniquePtr<IDescriptorSet> createBufferPool() const noexcept = 0;

    public:
        virtual UInt32 uniforms() const noexcept = 0;
        virtual UInt32 storages() const noexcept = 0;
        virtual UInt32 images() const noexcept = 0;
        virtual UInt32 samplers() const noexcept = 0;
        virtual UInt32 inputAttachments() const noexcept = 0;
    };

    /// <summary>
    /// Allows for data to be mapped into the object.
    /// </summary>
    class IMappable {
    public:
        virtual ~IMappable() noexcept = default;

    public:
        /// <summary>
        /// Maps the memory at <paramref name="data" /> to the internal memory of this object.
        /// </summary>
        /// <param name="data">The address that marks the beginning of the data to map.</param>
        /// <param name="size">The number of bytes to map.</param>
        virtual void map(const void* const data, const size_t& size) = 0;
    };

    /// <summary>
    /// Exposes a binding that can be associated with the object.
    /// </summary>
    class IBindable {
    public:
        virtual ~IBindable() noexcept = default;

    public:
        /// <summary>
        /// Gets the binding point, this object will be bound to.
        /// </summary>
        /// <returns>The binding point, this object will be bound to.</returns>
        virtual const UInt32& binding() const noexcept = 0;
    };

    /// <summary>
    /// Allows the object to transfer data between its local memory from or to an arbitrary <see cref="LiteFX::Rendering::IBuffer" /> object.
    /// </summary>
    class ITransferable {
    public:
        virtual ~ITransferable() noexcept = default;

    public:
        /// <summary>
        /// Transfers data from the <paramref name="source" /> buffer into the objects local memory.
        /// </summary>
        /// <param name="commandBuffer">The command buffer to issue the transfer command to.</param>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="size">The size (in bytes) to transfer from the source buffer.</param>
        /// <param name="sourceOffset">The offset (in bytes) from where to start transferring in the source buffer.</param>
        /// <param name="targetOffset">The offset (in bytes) to which the data will be transferred in the object memory.</param>
        virtual void transferFrom(const ICommandBuffer* commandBuffer, IBuffer* source, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) = 0;

        /// <summary>
        /// Transfers data from the objects local memory into the <paramref name="target" /> buffer.
        /// </summary>
        /// <param name="commandBuffer">The command buffer to issue the transfer command to.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="size">The size (in bytes) to transfer to the target buffer.</param>
        /// <param name="sourceOffset">The offset (in bytes) from where to start transferring in the object memory.</param>
        /// <param name="targetOffset">The offset (in bytes) to which the data will be transferred in the target buffer.</param>
        virtual void transferTo(const ICommandBuffer* commandBuffer, IBuffer* target, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) const = 0;
    };

    /// <summary>
    /// Describes a chunk of device memory.
    /// </summary>
    class IDeviceMemory {
    public:
        virtual ~IDeviceMemory() noexcept = default;

    public:
        /// <summary>
        /// Gets the number of array elements inside the memory chunk.
        /// </summary>
        /// <returns>The number of array elements inside the memory chunk.</returns>
        virtual const UInt32& elements() const noexcept = 0;

        /// <summary>
        /// Gets the size (in bytes) of the memory chunk.
        /// </summary>
        /// <returns>The size (in bytes) of the memory chunk.</returns>
        virtual size_t size() const noexcept = 0;
    };

    /// <summary>
    /// Describes a generic buffer object.
    /// </summary>
    /// <seealso cref="Buffer" />
    class IBuffer : public virtual IDeviceMemory, public virtual ITransferable, public virtual IMappable {
    public:
        virtual ~IBuffer() noexcept = default;

    public:
        /// <summary>
        /// Returns the type of the buffer.
        /// </summary>
        /// <returns>The type of the buffer.</returns>
        virtual const BufferType& type() const noexcept = 0;
    };

    /// <summary>
    /// Describes a descriptor.
    /// </summary>
    class LITEFX_RENDERING_API IDescriptor : public virtual IBindable {
    public:
        virtual ~IDescriptor() noexcept = default;

    public:
        /// <summary>
        /// Gets the layout of the descriptor.
        /// </summary>
        /// <returns>The layout of the descriptor.</returns>
        virtual const IDescriptorLayout* getLayout() const noexcept = 0;    // TODO: Move to IBuffer (template) and turn this interface into a interface similar to IVertexBuffer and IIndexBuffer, specifying a concept for the layout.
    };

    /// <summary>
    /// Describes a constant buffer.
    /// </summary>
    class LITEFX_RENDERING_API IConstantBuffer : public virtual IBuffer, public virtual IDescriptor {
    public:
        virtual ~IConstantBuffer() noexcept = default;
    };

    /// <summary>
    /// Describes a generic image.
    /// </summary>
    class LITEFX_RENDERING_API IImage : public virtual IDeviceMemory {
    public:
        virtual ~IImage() noexcept = default;

    public:
        /// <summary>
        /// Gets the extent of the image.
        /// </summary>
        /// <returns>The extent of the image.</returns>
        virtual const Size2d& extent() const noexcept = 0;

        /// <summary>
        /// Gets the internal format of the image.
        /// </summary>
        /// <returns>The internal format of the image.</returns>
        virtual const Format& format() const noexcept = 0;
    };

    /// <summary>
    /// Describes a texture.
    /// </summary>
    /// <remarks>
    /// A texture is a <see cref="LiteFX::Rendering::IImage" />, that can be bound to a descriptor.
    /// </remarks>
    class LITEFX_RENDERING_API ITexture : public virtual IImage, public virtual IDescriptor, public virtual ITransferable {
    public:
        virtual ~ITexture() noexcept = default;
    
    public:
        /// <summary>
        /// Gets the number of samples of the texture.
        /// </summary>
        /// <returns>The number of samples of the texture.</returns>
        virtual MultiSamplingLevel getSamples() const noexcept = 0;

        /// <summary>
        /// Gets the number of mip-map levels of the texture.
        /// </summary>
        /// <returns>The number of mip-map levels of the texture.</returns>
        virtual UInt32 getLevels() const noexcept = 0;

        // TODO: getSampler() for combined samplers?
    };

    /// <summary>
    /// Describes a texture sampler.
    /// </summary>
    class LITEFX_RENDERING_API ISampler : public virtual IDescriptor {
    public:
        virtual ~ISampler() noexcept = default;

    public:
        /// <summary>
        /// Gets the filtering mode that is used for minifying lookups.
        /// </summary>
        /// <returns>The filtering mode that is used for minifying lookups.</returns>
        virtual const FilterMode& getMinifyingFilter() const noexcept = 0;

        /// <summary>
        /// Gets the filtering mode that is used for magnifying lookups.
        /// </summary>
        /// <returns>The filtering mode that is used for magnifying lookups.</returns>
        virtual const FilterMode& getMagnifyingFilter() const noexcept = 0;

        /// <summary>
        /// Gets the addressing mode at the horizontal border.
        /// </summary>
        /// <returns>The addressing mode at the horizontal border.</returns>
        virtual const BorderMode& getBorderModeU() const noexcept = 0;

        /// <summary>
        /// Gets the addressing mode at the vertical border.
        /// </summary>
        /// <returns>The addressing mode at the vertical border.</returns>
        virtual const BorderMode& getBorderModeV() const noexcept = 0;

        /// <summary>
        /// Gets the addressing mode at the depth border.
        /// </summary>
        /// <returns>The addressing mode at the depth border.</returns>
        virtual const BorderMode& getBorderModeW() const noexcept = 0;

        /// <summary>
        /// Gets the anisotropy value used when sampling this texture.
        /// </summary>
        /// <remarks>
        /// Anisotropy will be disabled, if this value is set to <c>0.0</c>.
        /// </remarks>
        /// <returns>The anisotropy value used when sampling this texture.</returns>
        virtual const Float& getAnisotropy() const noexcept = 0;

        /// <summary>
        /// Gets the mip-map selection mode.
        /// </summary>
        /// <returns>The mip-map selection mode.</returns>
        virtual const MipMapMode& getMipMapMode() const noexcept = 0;

        /// <summary>
        /// Gets the mip-map level of detail bias.
        /// </summary>
        /// <returns>The mip-map level of detail bias.</returns>
        virtual const Float& getMipMapBias() const noexcept = 0;

        /// <summary>
        /// Gets the maximum texture level of detail.
        /// </summary>
        /// <returns>The maximum texture level of detail.</returns>
        virtual const Float& getMaxLOD() const noexcept = 0;

        /// <summary>
        /// Gets the minimum texture level of detail.
        /// </summary>
        /// <returns>The minimum texture level of detail.</returns>
        virtual const Float& getMinLOD() const noexcept = 0;
    };

    /// <summary>
    /// A base class for a generic buffer.
    /// </summary>
    class LITEFX_RENDERING_API Buffer : public virtual IBuffer {
        LITEFX_IMPLEMENTATION(BufferImpl);
    
    public:
        /// <summary>
        /// Creates a new buffer object.
        /// </summary>
        /// <param name="type">The type of the buffer.</param>
        /// <param name="elements">The number of elements in this buffer.</param>
        /// <param name="size">The size (in bytes) of the buffer memory.</param>
        Buffer(const BufferType& type, const UInt32& elements, const UInt32& size);
        Buffer(Buffer&&) = delete;
        Buffer(const Buffer&) = delete;
        virtual ~Buffer() noexcept;

    public:
        /// <inheritdoc />
        virtual const UInt32& elements() const noexcept override;

        /// <inheritdoc />
        virtual size_t size() const noexcept override;

        /// <inheritdoc />
        virtual const BufferType& type() const noexcept override;
    };

    /// <summary>
    /// A base class for a constant buffer.
    /// </summary>
    class LITEFX_RENDERING_API ConstantBuffer : public Buffer, public virtual IConstantBuffer {
        LITEFX_IMPLEMENTATION(ConstantBufferImpl);

    public:
        /// <summary>
        /// Creates a new constant buffer.
        /// </summary>
        /// <param name="layout">The layout of the constant buffer descriptor.</param>
        /// <param name="elements">The number of elements in this buffer.</param>
        ConstantBuffer(const IDescriptorLayout* layout, const UInt32& elements);
        ConstantBuffer(ConstantBuffer&&) = delete;
        ConstantBuffer(const ConstantBuffer&) = delete;
        virtual ~ConstantBuffer() noexcept;

    public:
        /// <inheritdoc />
        virtual const UInt32& binding() const noexcept override;

        /// <inheritdoc />
        virtual const IDescriptorLayout* getLayout() const noexcept override;
    };

    /// <summary>
    /// A base class for a generic image.
    /// </summary>
    class LITEFX_RENDERING_API Image : public virtual IImage {
        LITEFX_IMPLEMENTATION(ImageImpl);

    public:
        /// <summary>
        /// Creates a new image.
        /// </summary>
        /// <param name="elements">The number of images in this buffer.</param>
        /// <param name="size">The size (in bytes) of the buffer memory.</param>
        /// <param name="extent">The extent (in pixels) of the image.</param>
        /// <param name="format">The internal format of the image.</param>
        Image(const UInt32& elements, const size_t& size, const Size2d& extent, const Format& format);
        Image(Image&&) = delete;
        Image(const Image&) = delete;
        virtual ~Image() noexcept;

    public:
        /// <inheritdoc />
        virtual const UInt32& elements() const noexcept override;

        /// <inheritdoc />
        virtual size_t size() const noexcept override;

        /// <inheritdoc />
        virtual const Size2d& extent() const noexcept override;

        /// <inheritdoc />
        virtual const Format& format() const noexcept override;
    };

    /// <summary>
    /// A base class for a generic texture.
    /// </summary>
    class LITEFX_RENDERING_API Texture : public Image, public virtual ITexture {
        LITEFX_IMPLEMENTATION(TextureImpl);

    public:
        /// <summary>
        /// Creates a new texture.
        /// </summary>
        /// <param name="layout">The layout of the image descriptor.</param>
        /// <param name="elements">The number of images in this buffer.</param>
        /// <param name="size">The size (in bytes) of the buffer memory.</param>
        /// <param name="extent">The extent (in pixels) of the image.</param>
        /// <param name="format">The internal format of the image.</param>
        /// <param name="levels">The number of mip-map levels.</param>
        /// <param name="samples">The number of samples per texel.</param>
        Texture(const IDescriptorLayout* layout, const UInt32& elements, const UInt32& size, const Size2d& extent, const Format& format, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1);
        Texture(Texture&&) = delete;
        Texture(const Texture&) = delete;
        virtual ~Texture() noexcept;

    public:
        /// <inheritdoc />
        virtual const UInt32& binding() const noexcept override;

        /// <inheritdoc />
        virtual const IDescriptorLayout* getLayout() const noexcept override;

        /// <inheritdoc />
        virtual MultiSamplingLevel getSamples() const noexcept override;

        /// <inheritdoc />
        virtual UInt32 getLevels() const noexcept override;
    };

    /// <summary>
    /// A base class for a generic sampler.
    /// </summary>
    class LITEFX_RENDERING_API Sampler : public virtual ISampler {
        LITEFX_IMPLEMENTATION(SamplerImpl);
    
    public:
        /// <summary>
        /// Creates a new sampler.
        /// </summary>
        /// <param name="layout">The layout of the sampler descriptor.</param>
        /// <param name="magFilter">The filtering mode that is used for magnifying lookups.</param>
        /// <param name="minFilter">The filtering mode that is used for minifying lookups.</param>
        /// <param name="borderU">The addressing mode at the horizontal border.</param>
        /// <param name="borderV">The addressing mode at the vertical border.</param>
        /// <param name="borderW">The addressing mode at the depth border.</param>
        /// <param name="mipMapMode">The mip-map selection mode.</param>
        /// <param name="mipMapBias">The mip-map level of detail bias.</param>
        /// <param name="maxLod">The maximum texture level of detail.</param>
        /// <param name="minLod">The minimum texture level of detail.</param>
        /// <param name="anisotropy">The anisotropy value used when sampling this texture.</param>
        Sampler(const IDescriptorLayout* layout, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f);
        Sampler(Sampler&&) = delete;
        Sampler(const Sampler&) = delete;
        virtual ~Sampler() noexcept;

    public:
        /// <inheritdoc />
        virtual const IDescriptorLayout* getLayout() const noexcept override;

        /// <inheritdoc />
        virtual const UInt32& binding() const noexcept override;

        /// <inheritdoc />
        virtual const FilterMode& getMinifyingFilter() const noexcept override;

        /// <inheritdoc />
        virtual const FilterMode& getMagnifyingFilter() const noexcept override;

        /// <inheritdoc />
        virtual const BorderMode& getBorderModeU() const noexcept override;

        /// <inheritdoc />
        virtual const BorderMode& getBorderModeV() const noexcept override;

        /// <inheritdoc />
        virtual const BorderMode& getBorderModeW() const noexcept override;

        /// <inheritdoc />
        virtual const Float& getAnisotropy() const noexcept override;

        /// <inheritdoc />
        virtual const MipMapMode& getMipMapMode() const noexcept override;

        /// <inheritdoc />
        virtual const Float& getMipMapBias() const noexcept override;

        /// <inheritdoc />
        virtual const Float& getMaxLOD() const noexcept override;

        /// <inheritdoc />
        virtual const Float& getMinLOD() const noexcept override;
    };

    /// <summary>
    /// Defines a set of descriptors.
    /// </summary>
	/// <remarks>
    /// Descriptors can be grouped into multiple descriptor sets. It is generally a good practice to group descriptors based on the frequency of the updates they receive. For
	/// example, it typically makes sense to store the camera buffer in a descriptor set, since it only needs to be updated once per frame for each camera, whilst the object
	/// or material data should be stored in separate descriptor sets, that are possibly updated before each draw call. However, other scenarios employing multiple descriptor
    /// sets are also possible. 
	/// 
	/// From a shader perspective, a descriptor set is identified by a <c>set</c> (GLSL) or <c>space</c> (HLSL), whilst a descriptor is addressed by a <c>binding</c> (GLSL) or 
	/// <c>register</c> (HLSL). Descriptor sets are read from GPU-visible memory, depending on how they are bound during the current draw call.
    /// 
    /// From a CPU-perspective, think of a descriptor set as a way to allocate buffers (i.e. descriptors) for the shader. The descriptors are not managed by the set directly,
    /// instead the descriptor set hands out a CPU-visible buffer (or multiple) for a descriptor, which then can be manipulated. Calling <see cref="IDescriptorSet::update" /> 
    /// or <see cref="IDescriptorSet::updateAll" /> copies the buffer to the GPU-visible memory. Finally, calling <see cref="IRenderPipeline::bind" /> tells the GPU to read the
    /// descriptor set from the for the subsequent draw calls (until another descriptor set gets bound).
    /// 
    /// A descriptor set only exists once within the current back-buffer of the pipeline, so it is important to carefully synchronize update, bind and draw calls, so that the 
    /// contents are not overwritten before they are used by a draw call. A descriptor set remains unchanged for the shader, as long as it has not been bound on the render 
    /// pipeline. However, updating a descriptor set that has not yet been bound overwrites it and earlier contents are lost. On the other hand, updating a descriptor set 
    /// before it got read in a draw call may cause the draw call to read wrong data. Since commands are executed asynchronously, this can happen, when a buffer that has been 
    /// mapped to a descriptor set gets updated. The easiest and recommended way to circumvent this is to use multiple buffers generated from the same descriptor set. Calling 
    /// <see cref="IDescriptorSet::update" /> only once for each buffer per frame ensures that the contents remain synchronized. This implies that each entity in the scene 
    /// should store its own buffers.
    /// 
    /// Note that input attachments are handled different, since they do not need to be mapped from host to device memory and are synchronized through render pass dependencies.
    /// Calling <see cref="IDescriptorSet::attach" /> before binding the descriptor set to the pipeline maps the attachment to a binding point.
	/// </remarks>
    class LITEFX_RENDERING_API IDescriptorSet {
    public:
        virtual ~IDescriptorSet() noexcept = default;

    public:
        virtual UniquePtr<IConstantBuffer> makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements = 1) const noexcept = 0;
        virtual UniquePtr<ITexture> makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const noexcept = 0;
        virtual UniquePtr<ISampler> makeSampler(const UInt32& binding, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const noexcept = 0;

        /// <summary>
        /// Updates a constant buffer within the current descriptor set.
        /// </summary>
        /// <param name="buffer">The constant buffer to write to the descriptor set.</param>
        /// <seealso cref="updateAll" />
        virtual void update(const IConstantBuffer* buffer) const = 0;

        /// <summary>
        /// Updates a texture within the current descriptor set.
        /// </summary>
        /// <param name="texture">The texture to write to the descriptor set.</param>
        /// <seealso cref="updateAll" />
        virtual void update(const ITexture* texture) const = 0;
        
        /// <summary>
        /// Updates a sampler within the current descriptor set.
        /// </summary>
        /// <param name="sampler">The sampler to write to the descriptor set.</param>
        /// <seealso cref="updateAll" />
        virtual void update(const ISampler* sampler) const = 0;

        /// <summary>
        /// Updates a constant buffer within all descriptor sets (i.e. for each pipeline back buffer).
        /// </summary>
        /// <param name="buffer">The constant buffer to write to the descriptor sets.</param>
        /// <seealso cref="update" />
        virtual void updateAll(const IConstantBuffer* buffer) const = 0;

        /// <summary>
        /// Updates a texture within all descriptor sets (i.e. for each pipeline back buffer).
        /// </summary>
        /// <param name="texture">The texture to write to the descriptor sets.</param>
        /// <seealso cref="update" />
        virtual void updateAll(const ITexture* texture) const = 0;

        /// <summary>
        /// Updates a sampler within all descriptor sets (i.e. for each pipeline back buffer).
        /// </summary>
        /// <param name="sampler">The sampler to write to the descriptor sets.</param>
        /// <seealso cref="update" />
        virtual void updateAll(const ISampler* sampler) const = 0;

        /// <summary>
        /// Attaches an image as an input attachment to a descriptor bound at <paramref cref="binding" />.
        /// </summary>
        /// <param name="binding">The input attachment binding point.</param>
        /// <param name="image">The image to bind to the input attachment descriptor.</param>
        virtual void attach(const UInt32& binding, const IImage* image) const = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TShaderProgram, typename TParent>
    class ShaderProgramBuilder : public Builder<TDerived, TShaderProgram, TParent> {
    public:
        using Builder<TDerived, TShaderProgram, TParent>::Builder;

    public:
        virtual TDerived& addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addVertexShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addComputeShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TPipeline, typename TParent, typename TInputAssembler = TPipeline::input_assembler_type>
    class RenderPipelineBuilder : public Builder<TDerived, TPipeline, TParent> {
    public:
        using Builder<TDerived, TPipeline, TParent>::Builder;

    public:
        virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) = 0;
        virtual void use(SharedPtr<IRasterizer> rasterizer) = 0;
        virtual void use(SharedPtr<TInputAssembler> inputAssembler) = 0;
        virtual void use(SharedPtr<IViewport> viewport) = 0;
        virtual void use(SharedPtr<IScissor> scissor) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TPipelineLayout, typename TParent>
    class RenderPipelineLayoutBuilder : public Builder<TDerived, TPipelineLayout, TParent> {
    public:
        using Builder<TDerived, TPipelineLayout, TParent>::Builder;

    public:
        virtual void use(UniquePtr<IShaderProgram>&& program) = 0;
        virtual void use(UniquePtr<IDescriptorSetLayout>&& layout) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TRasterizer, typename TParent>
    class RasterizerBuilder : public Builder<TDerived, TRasterizer, TParent, SharedPtr<TRasterizer>> {
    public:
        using Builder<TDerived, TRasterizer, TParent, SharedPtr<TRasterizer>>::Builder;

    public:
        virtual TDerived& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) = 0;
        virtual TDerived& withCullMode(const CullMode& cullMode = CullMode::BackFaces) = 0;
        virtual TDerived& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) = 0;
        virtual TDerived& withLineWidth(const Float& lineWidth = 1.f) = 0;
        virtual TDerived& enableDepthBias(const bool& enable = false) = 0;
        virtual TDerived& withDepthBiasClamp(const Float& clamp = 0.f) = 0;
        virtual TDerived& withDepthBiasConstantFactor(const Float& factor = 0.f) = 0;
        virtual TDerived& withDepthBiasSlopeFactor(const Float& factor = 0.f) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TDescriptorSetLayout, typename TParent>
    class DescriptorSetLayoutBuilder : public Builder<TDerived, TDescriptorSetLayout, TParent> {
    public:
        using Builder<TDerived, TDescriptorSetLayout, TParent>::Builder;

    public:
        virtual TDerived& addDescriptor(UniquePtr<IDescriptorLayout>&& layout) = 0;
        virtual TDerived& addDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize) = 0;
        virtual TDerived& addUniform(const UInt32& binding, const UInt32& descriptorSize) {
            return this->addDescriptor(DescriptorType::Uniform, binding, descriptorSize);
        }
        virtual TDerived& addStorage(const UInt32& binding, const UInt32& descriptorSize) {
            return this->addDescriptor(DescriptorType::Storage, binding, descriptorSize);
        }
        virtual TDerived& addImage(const UInt32& binding) {
            return this->addDescriptor(DescriptorType::Image, binding, 0);
        }
        virtual TDerived& addInputAttachment(const UInt32& binding) {
            return this->addDescriptor(DescriptorType::InputAttachment, binding, 0);
        }
        virtual TDerived& addSampler(const UInt32& binding) {
            return this->addDescriptor(DescriptorType::Sampler, binding, 0);
        }
        virtual void use(UniquePtr<IDescriptorLayout>&& layout) {
            this->addDescriptor(std::move(layout));
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TVertexBufferLayout, typename TParent>
    class VertexBufferLayoutBuilder : public Builder<TDerived, TVertexBufferLayout, TParent> {
    public:
        using Builder<TDerived, TVertexBufferLayout, TParent>::Builder;

    public:
        virtual TDerived& addAttribute(UniquePtr<BufferAttribute>&& attribute) = 0;
    };

}