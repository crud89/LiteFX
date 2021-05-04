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
        BufferAttribute(const UInt32& location, const UInt32& offset, const BufferFormat& format);
        BufferAttribute(BufferAttribute&&) noexcept;
        BufferAttribute(const BufferAttribute&);
        virtual ~BufferAttribute() noexcept;

    public:
        virtual const UInt32& getLocation() const noexcept;
        virtual const BufferFormat& getFormat() const noexcept;
        virtual const UInt32& getOffset() const noexcept;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IBufferLayout {
    public:
        virtual ~IBufferLayout() noexcept = default;

    public:
        virtual size_t getElementSize() const noexcept = 0;
        virtual UInt32 getBinding() const noexcept = 0;
        virtual BufferType getType() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IVertexBufferLayout : public IBufferLayout {
    public:
        virtual ~IVertexBufferLayout() noexcept = default;

    public:
        virtual Array<const BufferAttribute*> getAttributes() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IIndexBufferLayout : public IBufferLayout {
    public:
        virtual ~IIndexBufferLayout() noexcept = default;

    public:
        virtual const IndexType& getIndexType() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IDescriptorLayout : public IBufferLayout {
    public:
        virtual ~IDescriptorLayout() noexcept = default;

    public:
        virtual const IDescriptorSetLayout* getDescriptorSet() const noexcept = 0;
        virtual DescriptorType getDescriptorType() const noexcept = 0;
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
    class LITEFX_RENDERING_API IMappable {
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
    class LITEFX_RENDERING_API IBindable {
    public:
        virtual ~IBindable() noexcept = default;

    public:
        /// <summary>
        /// Gets the binding point, this object will be bound to.
        /// </summary>
        /// <returns>The binding point, this object will be bound to.</returns>
        virtual UInt32 getBinding() const noexcept = 0;
    };

    /// <summary>
    /// Allows the object to transfer data between its local memory from or to an arbitrary <see cref="LiteFX::Rendering::IBuffer" /> object.
    /// </summary>
    class LITEFX_RENDERING_API ITransferable {
    public:
        virtual ~ITransferable() noexcept = default;

    public:
        /// <summary>
        /// Transfers data from the <paramref name="source" /> buffer into the objects local memory.
        /// </summary>
        /// <param name="commandQueue">The command queue to issue the transfer command to.</param>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="size">The size (in bytes) to transfer from the source buffer.</param>
        /// <param name="sourceOffset">The offset (in bytes) from where to start transferring in the source buffer.</param>
        /// <param name="targetOffset">The offset (in bytes) to which the data will be transferred in the object memory.</param>
        virtual void transferFrom(const ICommandQueue* commandQueue, IBuffer* source, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) = 0;

        /// <summary>
        /// Transfers data from the objects local memory into the <paramref name="target" /> buffer.
        /// </summary>
        /// <param name="commandQueue">The command queue to issue the transfer command to.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="size">The size (in bytes) to transfer to the target buffer.</param>
        /// <param name="sourceOffset">The offset (in bytes) from where to start transferring in the object memory.</param>
        /// <param name="targetOffset">The offset (in bytes) to which the data will be transferred in the target buffer.</param>
        virtual void transferTo(const ICommandQueue* commandQueue, IBuffer* target, const size_t& size, const size_t& sourceOffset = 0, const size_t& targetOffset = 0) const = 0;
    };

    /// <summary>
    /// Describes a chunk of device memory.
    /// </summary>
    class LITEFX_RENDERING_API IDeviceMemory {
    public:
        virtual ~IDeviceMemory() noexcept = default;

    public:
        /// <summary>
        /// Gets the number of array elements inside the memory chunk.
        /// </summary>
        /// <returns>The number of array elements inside the memory chunk.</returns>
        virtual UInt32 getElements() const noexcept = 0;

        /// <summary>
        /// Gets the size (in bytes) of the memory chunk.
        /// </summary>
        /// <returns>The size (in bytes) of the memory chunk.</returns>
        virtual size_t getSize() const noexcept = 0;
    };

    /// <summary>
    /// Describes a generic buffer object.
    /// </summary>
    /// <seealso cref="Buffer" />
    class LITEFX_RENDERING_API IBuffer : public virtual IDeviceMemory, public virtual ITransferable, public virtual IMappable {
    public:
        virtual ~IBuffer() noexcept = default;

    public:
        /// <summary>
        /// Gets the type of the buffer.
        /// </summary>
        /// <returns>The type of the buffer.</returns>
        virtual BufferType getType() const noexcept = 0;
    };

    /// <summary>
    /// Describes a vertex buffer.
    /// </summary>
    class LITEFX_RENDERING_API IVertexBuffer : public virtual IBuffer, public virtual IBindable {
    public:
        virtual ~IVertexBuffer() noexcept = default;

    public:
        /// <summary>
        /// Gets the layout of the vertex buffer.
        /// </summary>
        /// <returns>The layout of the vertex buffer.</returns>
        virtual const IVertexBufferLayout* getLayout() const noexcept = 0;
    };

    /// <summary>
    /// Describes an index buffer.
    /// </summary>
    class LITEFX_RENDERING_API IIndexBuffer : public virtual IBuffer {
    public:
        virtual ~IIndexBuffer() noexcept = default;

    public:
        /// <summary>
        /// Gets the layout of the index buffer.
        /// </summary>
        /// <returns>The layout of the index buffer.</returns>
        virtual const IIndexBufferLayout* getLayout() const noexcept = 0;
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
        virtual const IDescriptorLayout* getLayout() const noexcept = 0;
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
        virtual Size2d getExtent() const noexcept = 0;

        /// <summary>
        /// Gets the internal format of the image.
        /// </summary>
        /// <returns>The internal format of the image.</returns>
        virtual Format getFormat() const noexcept = 0;
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
        virtual UInt32 getElements() const noexcept override;

        /// <inheritdoc />
        virtual size_t getSize() const noexcept override;

        /// <inheritdoc />
        virtual BufferType getType() const noexcept override;
    };

    /// <summary>
    /// A base class for a vertex buffer.
    /// </summary>
    class LITEFX_RENDERING_API VertexBuffer : public Buffer, public virtual IVertexBuffer {
        LITEFX_IMPLEMENTATION(VertexBufferImpl);

    public:
        /// <summary>
        /// Creates a new vertex buffer.
        /// </summary>
        /// <param name="layout">The layout of the vertex buffer.</param>
        /// <param name="elements">The number of elements in this buffer.</param>
        VertexBuffer(const IVertexBufferLayout* layout, const UInt32& elements);
        VertexBuffer(VertexBuffer&&) = delete;
        VertexBuffer(const VertexBuffer&) = delete;
        virtual ~VertexBuffer() noexcept;

    public:
        /// <inheritdoc />
        virtual const IVertexBufferLayout* getLayout() const noexcept override;
        
        /// <inheritdoc />
        virtual UInt32 getBinding() const noexcept override;
    };

    /// <summary>
    /// A base class for an index buffer.
    /// </summary>
    class LITEFX_RENDERING_API IndexBuffer : public Buffer, public virtual IIndexBuffer {
        LITEFX_IMPLEMENTATION(IndexBufferImpl);

    public:
        /// <summary>
        /// Creates a new index buffer.
        /// </summary>
        /// <param name="layout">The layout of the index buffer.</param>
        /// <param name="elements">The number of elements in this buffer.</param>
        IndexBuffer(const IIndexBufferLayout* layout, const UInt32& elements);
        IndexBuffer(IndexBuffer&&) = delete;
        IndexBuffer(const IndexBuffer&) = delete;
        virtual ~IndexBuffer() noexcept;

    public:
        /// <inheritdoc />
        virtual const IIndexBufferLayout* getLayout() const noexcept override;
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
        virtual UInt32 getBinding() const noexcept override;

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
        virtual UInt32 getElements() const noexcept override;

        /// <inheritdoc />
        virtual size_t getSize() const noexcept override;

        /// <inheritdoc />
        virtual Size2d getExtent() const noexcept override;

        /// <inheritdoc />
        virtual Format getFormat() const noexcept override;
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
        virtual UInt32 getBinding() const noexcept override;

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
        virtual UInt32 getBinding() const noexcept override;

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
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IDescriptorSet {
    public:
        virtual ~IDescriptorSet() noexcept = default;

    public:
        virtual const IDescriptorSetLayout* getDescriptorSetLayout() const noexcept = 0;
        virtual UniquePtr<IConstantBuffer> makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements = 1) const noexcept = 0;
        virtual UniquePtr<ITexture> makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const noexcept = 0;
        virtual UniquePtr<ISampler> makeSampler(const UInt32& binding, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const noexcept = 0;

        /// <summary>
        /// Updates a constant buffer within the current descriptor set.
        /// </summary>
        /// <param name="buffer">The constant buffer to write to the descriptor set.</param>
        /// <seealso cref="updateAll(const IConstantBuffer*)" />
        virtual void update(const IConstantBuffer* buffer) const = 0;

        /// <summary>
        /// Updates a texture within the current descriptor set.
        /// </summary>
        /// <param name="texture">The texture to write to the descriptor set.</param>
        /// <seealso cref="updateAll(const ITexture*)" />
        virtual void update(const ITexture* texture) const = 0;
        
        /// <summary>
        /// Updates a sampler within the current descriptor set.
        /// </summary>
        /// <param name="sampler">The sampler to write to the descriptor set.</param>
        /// <seealso cref="updateAll(const ISampler*)" />
        virtual void update(const ISampler* sampler) const = 0;

        /// <summary>
        /// Updates a constant buffer within all descriptor sets (i.e. for each frame in fly).
        /// </summary>
        /// <param name="buffer">The constant buffer to write to the descriptor sets.</param>
        /// <seealso cref="update(const IConstantBuffer*)" />
        virtual void updateAll(const IConstantBuffer* buffer) const = 0;

        /// <summary>
        /// Updates a texture within all descriptor sets (i.e. for each frame in fly).
        /// </summary>
        /// <param name="texture">The texture to write to the descriptor sets.</param>
        /// <seealso cref="update(const ITexture*)" />
        virtual void updateAll(const ITexture* texture) const = 0;

        /// <summary>
        /// Updates a sampler within all descriptor sets (i.e. for each frame in fly).
        /// </summary>
        /// <param name="sampler">The sampler to write to the descriptor sets.</param>
        /// <seealso cref="update(const ISampler*)" />
        virtual void updateAll(const ISampler* sampler) const = 0;

        /// <summary>
        /// Attaches the attachment identified by <paramref cref="attachmentId" /> of <paramref cref="renderPass" /> to the descriptor bound at <paramref cref="binding" />.
        /// </summary>
        /// <param name="binding">The input attachment binding point.</param>
        /// <param name="renderPass">The render pass to request the attachment from.</param>
        /// <param name="attachmentId">The id of the attachment.</param>
        virtual void attach(const UInt32& binding, const IRenderPass* renderPass, const UInt32& attachmentId) const = 0;

        /// <summary>
        /// Attaches an image as an input attachment to a descriptor bound at <paramref cref="binding" />.
        /// </summary>
        /// <param name="binding">The input attachment binding point.</param>
        /// <param name="image">The image to bind to the input attachment descriptor.</param>
        virtual void attach(const UInt32& binding, const IImage* image) const = 0;

        /// <summary>
        /// Binds the descriptor set to a render pass.
        /// </summary>
        /// <param name="renderPass">The render pass to bind the descriptor set to.</param>
        virtual void bind(const IRenderPass* renderPass) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IRenderPipeline {
    public:
        virtual ~IRenderPipeline() noexcept = default;

    public:
        virtual const IRenderPass& renderPass() const noexcept = 0;
        virtual const String& name() const noexcept = 0;

        /// <summary>
        /// Gets the ID of the pipeline.
        /// </summary>
        /// <remarks>
        /// The pipeline ID must be unique within the render pass.
        /// </remarks>
        /// <returns>The ID of the pipeline.</returns>
        virtual const UInt32& id() const noexcept = 0;
    
    public:
        /// <summary>
        /// 
        /// </summary>
        /// <param name="layout"></param>
        /// <param name="viewports"></param>
        /// <param name="scissors"></param>
        virtual void initialize(UniquePtr<IRenderPipelineLayout>&& layout, Array<SharedPtr<Viewport>>&& viewports, Array<SharedPtr<Scissor>>&& scissors) = 0;

    public:
        virtual const IRenderPipelineLayout* getLayout() const noexcept = 0;
        virtual const IInputAssembler* getInputAssembler() const noexcept = 0;
        virtual const IRasterizer* getRasterizer() const noexcept = 0;
        virtual Array<const Viewport*> getViewports() const noexcept = 0;
        virtual Array<const Scissor*> getScissors() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IRenderPipelineLayout : public IRequiresInitialization {
    public:
        virtual ~IRenderPipelineLayout() noexcept = default;

    public:
        virtual void initialize(UniquePtr<IShaderProgram>&& shaderProgram, Array<UniquePtr<IDescriptorSetLayout>>&& descriptorLayouts) = 0;

    public:
        virtual const IShaderProgram* getProgram() const noexcept = 0;
        virtual Array<const IDescriptorSetLayout*> getDescriptorSetLayouts() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IShaderModule {
    public:
        virtual ~IShaderModule() noexcept = default;

    public:
        virtual const ShaderStage& getType() const noexcept = 0;
        virtual const String& getFileName() const noexcept = 0;
        virtual const String& getEntryPoint() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IShaderProgram {
    public:
        virtual ~IShaderProgram() noexcept = default;
    
    public:
        virtual Array<const IShaderModule*> getModules() const noexcept = 0;
        virtual void use(UniquePtr<IShaderModule>&& module) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IInputAssembler {
    public:
        virtual ~IInputAssembler() noexcept = default;

    public:
        virtual Array<const IVertexBufferLayout*> getVertexBufferLayouts() const = 0;
        virtual const IVertexBufferLayout* getVertexBufferLayout(const UInt32& binding) const = 0;
        virtual const IIndexBufferLayout* getIndexBufferLayout() const = 0;
        virtual const PrimitiveTopology getTopology() const noexcept = 0;
        virtual void setTopology(const PrimitiveTopology& topology) = 0;

    public:
        virtual void use(UniquePtr<IVertexBufferLayout>&& layout) = 0;
        virtual void use(UniquePtr<IIndexBufferLayout>&& layout) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IRasterizer {
    public:
        virtual ~IRasterizer() noexcept = default;

    public:
        virtual PolygonMode getPolygonMode() const noexcept = 0;
        virtual void setPolygonMode(const PolygonMode& mode) noexcept = 0;
        virtual CullMode getCullMode() const noexcept = 0;
        virtual void setCullMode(const CullMode& mode) noexcept = 0;
        virtual CullOrder getCullOrder() const noexcept = 0;
        virtual void setCullOrder(const CullOrder& order) noexcept = 0;
        virtual Float getLineWidth() const noexcept = 0;
        virtual void setLineWidth(const Float& width) noexcept = 0;
        virtual bool getDepthBiasEnabled() const noexcept = 0;
        virtual void setDepthBiasEnabled(const bool& enable) noexcept = 0;
        virtual float getDepthBiasClamp() const noexcept = 0;
        virtual void setDepthBiasClamp(const float& clamp) noexcept = 0;
        virtual float getDepthBiasConstantFactor() const noexcept = 0;
        virtual void setDepthBiasConstantFactor(const float& factor) noexcept = 0;
        virtual float getDepthBiasSlopeFactor() const noexcept = 0;
        virtual void setDepthBiasSlopeFactor(const float& factor) noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IViewport {
    public:
        virtual ~IViewport() noexcept = default;

    public:
        virtual RectF getRectangle() const noexcept = 0;
        virtual void setRectangle(const RectF& rectangle) noexcept = 0;
        virtual float getMinDepth() const noexcept = 0;
        virtual void setMinDepth(const float& depth) const noexcept = 0;
        virtual float getMaxDepth() const noexcept = 0;
        virtual void setMaxDepth(const float& depth) const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IScissor {
    public:
        virtual ~IScissor() noexcept = default;

    public:
        virtual RectF getRectangle() const noexcept = 0;
        virtual void setRectangle(const RectF& rectangle) noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IRenderPass {
    public:
        virtual ~IRenderPass() noexcept = default;

    public:
        virtual const ICommandBuffer* getCommandBuffer() const noexcept = 0;

    public:
        virtual void addTarget(UniquePtr<IRenderTarget>&& target) = 0;
        virtual const Array<const IRenderTarget*> getTargets() const noexcept = 0;
        virtual UniquePtr<IRenderTarget> removeTarget(const IRenderTarget* target) = 0;
        virtual void setDependency(const IRenderPass* renderPass = nullptr) = 0;
        virtual const IRenderPass* getDependency() const noexcept = 0;
        virtual const IRenderPipeline* getPipeline() const noexcept = 0;
        virtual IRenderPipeline* getPipeline() noexcept = 0;
        virtual void begin() const = 0;
        virtual void end(const bool& present = false) = 0;
        virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const = 0;
        virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const = 0;
        virtual const IImage* getAttachment(const UInt32& attachmentId) const = 0;

    public:
        virtual UniquePtr<IVertexBuffer> makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding = 0) const = 0;
        virtual UniquePtr<IIndexBuffer> makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const = 0;
        virtual UniquePtr<IDescriptorSet> makeBufferPool(const UInt32& bufferSet) const = 0;
        virtual void bind(const IVertexBuffer* buffer) const = 0;
        virtual void bind(const IIndexBuffer* buffer) const = 0;
        virtual void bind(IDescriptorSet* buffer) const = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API Rasterizer : public IRasterizer {
        LITEFX_IMPLEMENTATION(RasterizerImpl);

    public:
        Rasterizer();
        Rasterizer(Rasterizer&&) noexcept = delete;
        Rasterizer(const Rasterizer&) noexcept = delete;
        virtual ~Rasterizer() noexcept;

    public:
        virtual PolygonMode getPolygonMode() const noexcept override;
        virtual void setPolygonMode(const PolygonMode& mode) noexcept override;
        virtual CullMode getCullMode() const noexcept override;
        virtual void setCullMode(const CullMode& mode) noexcept override;
        virtual CullOrder getCullOrder() const noexcept override;
        virtual void setCullOrder(const CullOrder& order) noexcept override;
        virtual Float getLineWidth() const noexcept override;
        virtual void setLineWidth(const Float& width) noexcept override;
        virtual bool getDepthBiasEnabled() const noexcept override;
        virtual void setDepthBiasEnabled(const bool& enable) noexcept override;
        virtual float getDepthBiasClamp() const noexcept override;
        virtual void setDepthBiasClamp(const float& clamp) noexcept override;
        virtual float getDepthBiasConstantFactor() const noexcept override;
        virtual void setDepthBiasConstantFactor(const float& factor) noexcept override;
        virtual float getDepthBiasSlopeFactor() const noexcept override;
        virtual void setDepthBiasSlopeFactor(const float& factor) noexcept override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API InputAssembler : public IInputAssembler {
        LITEFX_IMPLEMENTATION(InputAssemblerImpl);

    public:
        InputAssembler();
        InputAssembler(InputAssembler&&) noexcept = delete;
        InputAssembler(const InputAssembler&) noexcept = delete;
        virtual ~InputAssembler() noexcept;

    public:
        virtual Array<const IVertexBufferLayout*> getVertexBufferLayouts() const override;
        virtual const IVertexBufferLayout* getVertexBufferLayout(const UInt32& binding) const override;
        virtual const IIndexBufferLayout* getIndexBufferLayout() const override;
        virtual const PrimitiveTopology getTopology() const noexcept override;
        virtual void setTopology(const PrimitiveTopology& topology) override;

    public:
        virtual void use(UniquePtr<IVertexBufferLayout>&& layout) override;
        virtual void use(UniquePtr<IIndexBufferLayout>&& layout) override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API Viewport : public IViewport {
        LITEFX_IMPLEMENTATION(ViewportImpl);

    public:
        Viewport(const RectF& clientRect = { });
        Viewport(Viewport&&) noexcept = delete;
        Viewport(const Viewport&) noexcept = delete;
        virtual ~Viewport() noexcept;

    public:
        virtual RectF getRectangle() const noexcept override;
        virtual void setRectangle(const RectF& rectangle) noexcept override;
        virtual float getMinDepth() const noexcept override;
        virtual void setMinDepth(const float& depth) const noexcept override;
        virtual float getMaxDepth() const noexcept override;
        virtual void setMaxDepth(const float& depth) const noexcept override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API Scissor : public IScissor {
        LITEFX_IMPLEMENTATION(ScissorImpl);

    public:
        Scissor(const RectF& scissorRect = { });
        Scissor(Scissor&&) noexcept = delete;
        Scissor(const Scissor&) noexcept = delete;
        virtual ~Scissor() noexcept;

    public:
        virtual RectF getRectangle() const noexcept override;
        virtual void setRectangle(const RectF& rectangle) noexcept override;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TShaderProgram, typename TParent>
    class ShaderProgramBuilder : public Builder<TDerived, TShaderProgram, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual TDerived& addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addVertexShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addComputeShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& use(UniquePtr<IDescriptorSetLayout>&& layout) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TPipeline, typename TParent>
    class RenderPipelineBuilder : public Builder<TDerived, TPipeline, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TRenderPass>
    class RenderPassBuilder : public Builder<TDerived, TRenderPass> {
    public:
        using builder_type::Builder;

    public:
        virtual void use(UniquePtr<IRenderTarget>&& target) = 0;
        virtual void use(UniquePtr<IRenderPipeline>&& pipeline) = 0;
        virtual TDerived& attachTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;
        virtual TDerived& dependsOn(const IRenderPass* renderPass) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TPipelineLayout, typename TParent>
    class RenderPipelineLayoutBuilder : public Builder<TDerived, TPipelineLayout, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual void use(UniquePtr<IRasterizer>&& rasterizer) = 0;
        virtual void use(UniquePtr<IInputAssembler>&& inputAssembler) = 0;
        virtual void use(UniquePtr<IViewport>&& viewport) = 0;
        virtual void use(UniquePtr<IShaderProgram>&& program) = 0;
        virtual TDerived& enableDepthTest(const bool& enable = false) = 0; 
        virtual TDerived& enableStencilTest(const bool& enable = false) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TRasterizer, typename TParent>
    class RasterizerBuilder : public Builder<TDerived, TRasterizer, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual TDerived& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) = 0;
        virtual TDerived& withCullMode(const CullMode& cullMode = CullMode::BackFaces) = 0;
        virtual TDerived& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) = 0;
        virtual TDerived& withLineWidth(const Float& lineWidth = 1.f) = 0;
        virtual TDerived& withDepthBias(const bool& enable = false) = 0;
        virtual TDerived& withDepthBiasClamp(const Float& clamp = 0.f) = 0;
        virtual TDerived& withDepthBiasConstantFactor(const Float& factor = 0.f) = 0;
        virtual TDerived& withDepthBiasSlopeFactor(const Float& factor = 0.f) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TViewport, typename TParent>
    class ViewportBuilder : public Builder<TDerived, TViewport, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual TDerived& withRectangle(const RectF& rectangle) = 0;
        virtual TDerived& addScissor(const RectF& scissor) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TInputAssembler, typename TParent>
    class InputAssemblerBuilder : public Builder<TDerived, TInputAssembler, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual TDerived& withTopology(const PrimitiveTopology& topology) = 0;
        virtual void use(UniquePtr<IVertexBufferLayout>&& layout) = 0;
        virtual void use(UniquePtr<IIndexBufferLayout>&& layout) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TDescriptorSetLayout, typename TParent>
    class DescriptorSetLayoutBuilder : public Builder<TDerived, TDescriptorSetLayout, TParent> {
    public:
        using builder_type::Builder;

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
        using builder_type::Builder;

    public:
        virtual TDerived& addAttribute(UniquePtr<BufferAttribute>&& attribute) = 0;
    };

}