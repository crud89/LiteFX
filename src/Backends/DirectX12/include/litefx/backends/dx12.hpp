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
    class LITEFX_DIRECTX12_API DirectX12VertexBufferLayout final : public IVertexBufferLayout {
        LITEFX_IMPLEMENTATION(DirectX12VertexBufferLayoutImpl);
        LITEFX_BUILDER(DirectX12VertexBufferLayoutBuilder);

    public:
        /// <summary>
        /// Initializes a new vertex buffer layout.
        /// </summary>
        /// <param name="vertexSize">The size of a single vertex.</param>
        /// <param name="binding">The binding point of the vertex buffers using this layout.</param>
        explicit DirectX12VertexBufferLayout(size_t vertexSize, UInt32 binding = 0);
        DirectX12VertexBufferLayout(DirectX12VertexBufferLayout&&) = delete;
        DirectX12VertexBufferLayout(const DirectX12VertexBufferLayout&) = delete;
        virtual ~DirectX12VertexBufferLayout() noexcept;

        // IVertexBufferLayout interface.
    public:
        /// <inheritdoc />
        Enumerable<const BufferAttribute*> attributes() const noexcept override;

        // IBufferLayout interface.
    public:
        /// <inheritdoc />
        size_t elementSize() const noexcept override;

        /// <inheritdoc />
        UInt32 binding() const noexcept override;

        /// <inheritdoc />
        BufferType type() const noexcept override;
    };

    /// <summary>
    /// Implements a DirectX 12 index buffer layout.
    /// </summary>
    /// <seealso cref="DirectX12IndexBuffer" />
    /// <seealso cref="DirectX12VertexBufferLayout" />
    class LITEFX_DIRECTX12_API DirectX12IndexBufferLayout final : public IIndexBufferLayout {
        LITEFX_IMPLEMENTATION(DirectX12IndexBufferLayoutImpl);

    public:
        /// <summary>
        /// Initializes a new index buffer layout
        /// </summary>
        /// <param name="type">The type of the indices within the index buffer.</param>
        explicit DirectX12IndexBufferLayout(IndexType type);
        DirectX12IndexBufferLayout(DirectX12IndexBufferLayout&&) = delete;
        DirectX12IndexBufferLayout(const DirectX12IndexBufferLayout&) = delete;
        virtual ~DirectX12IndexBufferLayout() noexcept;

        // IIndexBufferLayout interface.
    public:
        /// <inheritdoc />
        IndexType indexType() const noexcept override;

        // IBufferLayout interface.
    public:
        /// <inheritdoc />
        size_t elementSize() const noexcept override;

        /// <inheritdoc />
        UInt32 binding() const noexcept override;

        /// <inheritdoc />
        BufferType type() const noexcept override;
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
        virtual ~IDirectX12Image() noexcept = default;
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
    /// Represents the base interface for a DirectX 12 acceleration structure implementation.
    /// </summary>
    /// <seealso cref="DirectX12DescriptorSet" />
    /// <seealso cref="DirectX12BottomLevelAccelerationStructure" />
    /// <seealso cref="DirectX12TopevelAccelerationStructure" />
    class LITEFX_DIRECTX12_API IDirectX12AccelerationStructure : public virtual IAccelerationStructure {
    public:
        virtual ~IDirectX12AccelerationStructure() noexcept = default;
    };

    /// <summary>
    /// Implements a DirectX 12 bottom-level acceleration structure (BLAS).
    /// </summary>
    /// <seealso cref="DirectX12TopLevelAccelerationStructure" />
    class LITEFX_DIRECTX12_API DirectX12BottomLevelAccelerationStructure final : public IBottomLevelAccelerationStructure, public virtual StateResource, public virtual IDirectX12AccelerationStructure {
        LITEFX_IMPLEMENTATION(DirectX12BottomLevelAccelerationStructureImpl);
        friend class DirectX12Device;
        friend class DirectX12CommandBuffer;

        using IAccelerationStructure::build;
        using IAccelerationStructure::update;
        using IBottomLevelAccelerationStructure::copy;

    public:
        /// <summary>
        /// Initializes a new DirectX 12 bottom-level acceleration structure (BLAS).
        /// </summary>
        /// <param name="flags">The flags that define how the acceleration structure is built.</param>
        /// <param name="name">The name of the acceleration structure resource.</param>
        /// <exception cref="InvalidArgumentException">Thrown if the provided <paramref name="flags" /> contain an unsupported combination of flags.</exception>
        /// <seealso cref="AccelerationStructureFlags" />
        explicit DirectX12BottomLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None, StringView name = "");
        DirectX12BottomLevelAccelerationStructure(const DirectX12BottomLevelAccelerationStructure&) = delete;
        DirectX12BottomLevelAccelerationStructure(DirectX12BottomLevelAccelerationStructure&&) = delete;
        virtual ~DirectX12BottomLevelAccelerationStructure() noexcept;

        // IAccelerationStructure interface.
    public:
        /// <inheritdoc />
        AccelerationStructureFlags flags() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const IDirectX12Buffer> buffer() const noexcept;

        /// <inheritdoc />
        void build(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer = nullptr, SharedPtr<const IDirectX12Buffer> buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// <inheritdoc />
        void update(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer = nullptr, SharedPtr<const IDirectX12Buffer> buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// <inheritdoc />
        void copy(const DirectX12CommandBuffer& commandBuffer, DirectX12BottomLevelAccelerationStructure& destination, bool compress = false, SharedPtr<const IDirectX12Buffer> buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const;

        /// <inheritdoc />
        UInt64 offset() const noexcept override;

        /// <inheritdoc />
        UInt64 size() const noexcept override;

        // IBottomLevelAccelerationStructure interface.
    public:
        /// <inheritdoc />
        const Array<TriangleMesh>& triangleMeshes() const noexcept override;

        /// <inheritdoc />
        void addTriangleMesh(const TriangleMesh& mesh) override;

        /// <inheritdoc />
        const Array<BoundingBoxes>& boundingBoxes() const noexcept override;

        /// <inheritdoc />
        void addBoundingBox(const BoundingBoxes& aabb) override;
        
        /// <inheritdoc />
        virtual void clear() noexcept override;

        /// <inheritdoc />
        virtual bool remove(const TriangleMesh& mesh) noexcept override;

        /// <inheritdoc />
        virtual bool remove(const BoundingBoxes& aabb) noexcept override;

    private:
        Array<D3D12_RAYTRACING_GEOMETRY_DESC> buildInfo() const;

    private:
        SharedPtr<const IBuffer> getBuffer() const noexcept override;
        void doBuild(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize) override;
        void doUpdate(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize) override;
        void doCopy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress, SharedPtr<const IBuffer> buffer, UInt64 offset, bool copyBuildInfo) const override;
    };

    /// <summary>
    /// Implements a DirectX 12 top-level acceleration structure (TLAS).
    /// </summary>
    /// <seealso cref="DirectX12BottomLevelAccelerationStructure" />
    class LITEFX_DIRECTX12_API DirectX12TopLevelAccelerationStructure final : public ITopLevelAccelerationStructure, public virtual StateResource, public virtual IDirectX12AccelerationStructure {
        LITEFX_IMPLEMENTATION(DirectX12TopLevelAccelerationStructureImpl);
        friend class DirectX12Device;
        friend class DirectX12CommandBuffer;

        using IAccelerationStructure::build;
        using IAccelerationStructure::update;
        using ITopLevelAccelerationStructure::copy;

    public:
        /// <summary>
        /// Initializes a new DirectX 12 top-level acceleration structure (BLAS).
        /// </summary>
        /// <param name="flags">The flags that define how the acceleration structure is built.</param>
        /// <param name="name">The name of the acceleration structure resource.</param>
        /// <exception cref="InvalidArgumentException">Thrown if the provided <paramref name="flags" /> contain an unsupported combination of flags.</exception>
        /// <seealso cref="AccelerationStructureFlags" />
        explicit DirectX12TopLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None, StringView name = "");
        DirectX12TopLevelAccelerationStructure(const DirectX12TopLevelAccelerationStructure&) = delete;
        DirectX12TopLevelAccelerationStructure(DirectX12TopLevelAccelerationStructure&&) = delete;
        virtual ~DirectX12TopLevelAccelerationStructure() noexcept;

        // IAccelerationStructure interface.
    public:
        /// <inheritdoc />
        AccelerationStructureFlags flags() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const IDirectX12Buffer> buffer() const noexcept;

        /// <inheritdoc />
        void build(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer = nullptr, SharedPtr<const IDirectX12Buffer> buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// <inheritdoc />
        void update(const DirectX12CommandBuffer& commandBuffer, SharedPtr<const IDirectX12Buffer> scratchBuffer = nullptr, SharedPtr<const IDirectX12Buffer> buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// <inheritdoc />
        void copy(const DirectX12CommandBuffer& commandBuffer, DirectX12TopLevelAccelerationStructure& destination, bool compress = false, SharedPtr<const IDirectX12Buffer> buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const;

        /// <inheritdoc />
        UInt64 offset() const noexcept override;

        /// <inheritdoc />
        UInt64 size() const noexcept override;

        // ITopLevelAccelerationStructure interface.
    public:
        /// <inheritdoc />
        const Array<Instance>& instances() const noexcept override;

        /// <inheritdoc />
        void addInstance(const Instance& instance) override;

        /// <inheritdoc />
        virtual void clear() noexcept override;

        /// <inheritdoc />
        virtual bool remove(const Instance& instance) noexcept override;

    private:
        Array<D3D12_RAYTRACING_INSTANCE_DESC> buildInfo() const;

    private:
        SharedPtr<const IBuffer> getBuffer() const noexcept override;
        void doBuild(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize) override;
        void doUpdate(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize) override;
        void doCopy(const ICommandBuffer& commandBuffer, ITopLevelAccelerationStructure& destination, bool compress, SharedPtr<const IBuffer> buffer, UInt64 offset, bool copyBuildInfo) const override;
    };

    /// <summary>
    /// Implements a DirectX 12 resource barrier.
    /// </summary>
    /// <seealso cref="DirectX12CommandBuffer" />
    /// <seealso cref="IDirectX12Buffer" />
    /// <seealso cref="IDirectX12Image" />
    /// <seealso cref="Barrier" />
    class LITEFX_DIRECTX12_API DirectX12Barrier final : public Barrier<IDirectX12Buffer, IDirectX12Image> {
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
        constexpr inline explicit DirectX12Barrier(PipelineStage syncBefore, PipelineStage syncAfter) noexcept;
        DirectX12Barrier(const DirectX12Barrier&) = delete;
        DirectX12Barrier(DirectX12Barrier&&) = delete;
        constexpr inline virtual ~DirectX12Barrier() noexcept;

    private:
        constexpr inline explicit DirectX12Barrier() noexcept;
        constexpr inline PipelineStage& syncBefore() noexcept;
        constexpr inline PipelineStage& syncAfter() noexcept;

        // Barrier interface.
    public:
        /// <inheritdoc />
        constexpr inline PipelineStage syncBefore() const noexcept override;

        /// <inheritdoc />
        constexpr inline PipelineStage syncAfter() const noexcept override;

        /// <inheritdoc />
        constexpr inline void wait(ResourceAccess accessBefore, ResourceAccess accessAfter) noexcept override;

        /// <inheritdoc />
        constexpr inline void transition(const IDirectX12Buffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// <inheritdoc />
        constexpr inline void transition(const IDirectX12Buffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// <inheritdoc />
        constexpr inline void transition(const IDirectX12Image& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override;

        /// <inheritdoc />
        constexpr inline void transition(const IDirectX12Image& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override;

        /// <inheritdoc />
        constexpr inline void transition(const IDirectX12Image& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override;

        /// <inheritdoc />
        constexpr inline void transition(const IDirectX12Image& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override;

    public:
        /// <summary>
        /// Adds the barrier to a command buffer and updates the resource target states.
        /// </summary>
        /// <param name="commandBuffer">The command buffer to add the barriers to.</param>
        /// <exception cref="RuntimeException">Thrown, if any of the contained barriers is a image barrier that targets a sub-resource range that does not share the same <see cref="ImageLayout" /> in all sub-resources.</exception>
        inline void execute(const DirectX12CommandBuffer& commandBuffer) const noexcept;
    };

    /// <summary>
    /// Implements a DirectX 12 <see cref="IShaderModule" />.
    /// </summary>
    /// <seealso cref="DirectX12ShaderProgram" />
    /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
    class LITEFX_DIRECTX12_API DirectX12ShaderModule final : public IShaderModule, public ComResource<IDxcBlob> {
        LITEFX_IMPLEMENTATION(DirectX12ShaderModuleImpl);

    public:
        /// <summary>
        /// Initializes a new DirectX 12 shader module.
        /// </summary>
        /// <param name="device">The parent device, this shader module has been created from.</param>
        /// <param name="type">The shader stage, this module is used in.</param>
        /// <param name="fileName">The file name of the module source.</param>
        /// <param name="entryPoint">The name of the module entry point.</param>
        /// <param name="shaderLocalDescriptor">The descriptor that binds shader-local data for ray-tracing shaders.</param>
        explicit DirectX12ShaderModule(const DirectX12Device& device, ShaderStage type, const String& fileName, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt);

        /// <summary>
        /// Initializes a new DirectX 12 shader module.
        /// </summary>
        /// <param name="device">The parent device, this shader module has been created from.</param>
        /// <param name="type">The shader stage, this module is used in.</param>
        /// <param name="stream">The file stream to read the shader module from.</param>
        /// <param name="name">The file name of the module source.</param>
        /// <param name="entryPoint">The name of the module entry point.</param>
        /// <param name="shaderLocalDescriptor">The descriptor that binds shader-local data for ray-tracing shaders.</param>
        explicit DirectX12ShaderModule(const DirectX12Device& device, ShaderStage type, std::istream& stream, const String& name, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt);
        DirectX12ShaderModule(const DirectX12ShaderModule&) noexcept = delete;
        DirectX12ShaderModule(DirectX12ShaderModule&&) noexcept = delete;
        virtual ~DirectX12ShaderModule() noexcept;

        // IShaderModule interface.
    public:
        /// <inheritdoc />
        const String& fileName() const noexcept override;

        /// <inheritdoc />
        const String& entryPoint() const noexcept override;

        /// <inheritdoc />
        ShaderStage type() const noexcept override;

        /// <inheritdoc />
        const Optional<DescriptorBindingPoint>& shaderLocalDescriptor() const noexcept override;
    };

    /// <summary>
    /// Implements a DirectX 12 <see cref="ShaderProgram" />.
    /// </summary>
    /// <seealso cref="DirectX12ShaderProgramBuilder" />
    /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
    class LITEFX_DIRECTX12_API DirectX12ShaderProgram final : public ShaderProgram<DirectX12ShaderModule> {
        LITEFX_IMPLEMENTATION(DirectX12ShaderProgramImpl);
        LITEFX_BUILDER(DirectX12ShaderProgramBuilder);

    private:
        /// <summary>
        /// Initializes a new DirectX 12 shader program.
        /// </summary>
        /// <param name="device">The parent device of the shader program.</param>
        /// <param name="modules">The shader modules used by the shader program.</param>
        explicit DirectX12ShaderProgram(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12ShaderModule>>&& modules);

        /// <summary>
        /// Initializes a new DirectX 12 shader program.
        /// </summary>
        /// <param name="device">The parent device of the shader program.</param>
        explicit DirectX12ShaderProgram(const DirectX12Device& device) noexcept;

        // Factory method.
    public:
        static SharedPtr<DirectX12ShaderProgram> create(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12ShaderModule>>&& modules);

    public:
        DirectX12ShaderProgram(DirectX12ShaderProgram&&) noexcept = delete;
        DirectX12ShaderProgram(const DirectX12ShaderProgram&) noexcept = delete;
        virtual ~DirectX12ShaderProgram() noexcept;

    public:
        /// <inheritdoc />
        Enumerable<const DirectX12ShaderModule*> modules() const noexcept override;

        /// <inheritdoc />
        virtual SharedPtr<DirectX12PipelineLayout> reflectPipelineLayout() const;

    private:
        SharedPtr<IPipelineLayout> parsePipelineLayout() const override {
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
    class LITEFX_DIRECTX12_API DirectX12DescriptorSet final : public DescriptorSet<IDirectX12Buffer, IDirectX12Image, IDirectX12Sampler, IDirectX12AccelerationStructure> {
        LITEFX_IMPLEMENTATION(DirectX12DescriptorSetImpl);

    public:
        using base_type = DescriptorSet<IDirectX12Buffer, IDirectX12Image, IDirectX12Sampler, IDirectX12AccelerationStructure>;
        using base_type::update;

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
        void update(UInt32 binding, const IDirectX12Buffer& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, UInt32 firstDescriptor = 0) const override;

        /// <inheritdoc />
        void update(UInt32 binding, const IDirectX12Image& texture, UInt32 descriptor = 0, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const override;

        /// <inheritdoc />
        void update(UInt32 binding, const IDirectX12Sampler& sampler, UInt32 descriptor = 0) const override;

        /// <inheritdoc />
        void update(UInt32 binding, const IDirectX12AccelerationStructure& accelerationStructure, UInt32 descriptor = 0) const override;

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
        virtual UInt32 bufferOffset() const noexcept;

        /// <summary>
        /// Returns the local (CPU-visible) heap that contains the sampler descriptors.
        /// </summary>
        /// <returns>The local (CPU-visible) heap that contains the sampler descriptors, or <c>nullptr</c>, if the descriptor set does not contain any samplers.</returns>
        virtual const ComPtr<ID3D12DescriptorHeap>& samplerHeap() const noexcept;

        /// <summary>
        /// Returns the offset of the sampler descriptors in the global descriptor heap.
        /// </summary>
        /// <returns>The offset of the sampler descriptors in the global descriptor heap.</returns>
        virtual UInt32 samplerOffset() const noexcept;
    };

    /// <summary>
    /// Implements a DirectX 12 <see cref="IDescriptorLayout" />
    /// </summary>
    /// <seealso cref="IDirectX12Buffer" />
    /// <seealso cref="IDirectX12Image" />
    /// <seealso cref="IDirectX12Sampler" />
    /// <seealso cref="DirectX12DescriptorSet" />
    /// <seealso cref="DirectX12DescriptorSetLayout" />
    class LITEFX_DIRECTX12_API DirectX12DescriptorLayout final : public IDescriptorLayout {
        LITEFX_IMPLEMENTATION(DirectX12DescriptorLayoutImpl);

    public:
        /// <summary>
        /// Initializes a new DirectX 12 descriptor layout.
        /// </summary>
        /// <param name="type">The type of the descriptor.</param>
        /// <param name="binding">The binding point for the descriptor.</param>
        /// <param name="elementSize">The size of the descriptor.</param>
        /// <param name="elementSize">The number of descriptors in the descriptor array.</param>
        /// <param name="local">Determines if the descriptor is part of the local or global root signature for ray-tracing shaders.</param>
        explicit DirectX12DescriptorLayout(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors = 1, bool local = false);

        /// <summary>
        /// Initializes a new DirectX 12 descriptor layout for a static sampler.
        /// </summary>
        /// <param name="staticSampler">The static sampler to initialize the state with.</param>
        /// <param name="binding">The binding point for the descriptor.</param>
        /// <param name="local">Determines if the descriptor is part of the local or global root signature for ray-tracing shaders.</param>
        explicit DirectX12DescriptorLayout(UniquePtr<IDirectX12Sampler>&& staticSampler, UInt32 binding, bool local = false);

        DirectX12DescriptorLayout(DirectX12DescriptorLayout&&) = delete;
        DirectX12DescriptorLayout(const DirectX12DescriptorLayout&) = delete;
        virtual ~DirectX12DescriptorLayout() noexcept;

        // DirectX 12 descriptor layout.
    public:
        /// <summary>
        /// Returns `true`, if the descriptor belongs to the local root signature of a ray-tracing pipeline or `false` otherwise.
        /// </summary>
        /// <remarks>
        /// Note that this value must not be set to `true` for descriptors that are bound outside of ray-tracing shaders.
        /// </remarks>
        /// <returns>`true`, if the descriptor belongs to the local root signature of a ray-tracing pipeline or `false` otherwise.</returns>
        bool local() const noexcept;

        // IDescriptorLayout interface.
    public:
        /// <inheritdoc />
        DescriptorType descriptorType() const noexcept override;

        /// <inheritdoc />
        UInt32 descriptors() const noexcept override;

        /// <inheritdoc />
        const IDirectX12Sampler* staticSampler() const noexcept override;

        // IBufferLayout interface.
    public:
        /// <inheritdoc />
        size_t elementSize() const noexcept override;

        /// <inheritdoc />
        UInt32 binding() const noexcept override;

        /// <inheritdoc />
        BufferType type() const noexcept override;
    };

    /// <summary>
    /// Implements a DirectX 12 <see cref="DescriptorSetLayout" />.
    /// </summary>
    /// <seealso cref="DirectX12DescriptorSet" />
    /// <seealso cref="DirectX12PipelineDescriptorSetLayoutBuilder" />
    class LITEFX_DIRECTX12_API DirectX12DescriptorSetLayout final : public DescriptorSetLayout<DirectX12DescriptorLayout, DirectX12DescriptorSet> {
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
        explicit DirectX12DescriptorSetLayout(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12DescriptorLayout>>&& descriptorLayouts, UInt32 space, ShaderStage stages);
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
        virtual UInt32 rootParameterIndex() const noexcept;

        /// <summary>
        /// Returns the index of the first descriptor for a certain binding. The offset is relative to the heap for the descriptor (i.e. sampler for sampler descriptors and
        /// CBV/SRV/UAV for other descriptors).
        /// </summary>
        /// <param name="binding">The binding of the descriptor.</param>
        /// <returns>The index of the first descriptor for the binding.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the descriptor set does not contain a descriptor bound to the binding point specified by <paramref name="binding"/>.</exception>
        virtual UInt32 descriptorOffsetForBinding(UInt32 binding) const;

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
        Enumerable<const DirectX12DescriptorLayout*> descriptors() const noexcept override;

        /// <inheritdoc />
        const DirectX12DescriptorLayout& descriptor(UInt32 binding) const override;

        /// <inheritdoc />
        UInt32 space() const noexcept override;

        /// <inheritdoc />
        ShaderStage shaderStages() const noexcept override;

        /// <inheritdoc />
        UInt32 uniforms() const noexcept override;

        /// <inheritdoc />
        UInt32 storages() const noexcept override;

        /// <inheritdoc />
        UInt32 images() const noexcept override;

        /// <inheritdoc />
        UInt32 buffers() const noexcept override;

        /// <inheritdoc />
        UInt32 samplers() const noexcept override;

        /// <inheritdoc />
        UInt32 staticSamplers() const noexcept override;

        /// <inheritdoc />
        UInt32 inputAttachments() const noexcept override;

    public:
        /// <inheritdoc />
        UniquePtr<DirectX12DescriptorSet> allocate(const Enumerable<DescriptorBinding>& bindings = { }) const override;

        /// <inheritdoc />
        UniquePtr<DirectX12DescriptorSet> allocate(UInt32 descriptors, const Enumerable<DescriptorBinding>& bindings = { }) const override;

        /// <inheritdoc />
        Enumerable<UniquePtr<DirectX12DescriptorSet>> allocateMultiple(UInt32 descriptorSets, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const override;

        /// <inheritdoc />
        Enumerable<UniquePtr<DirectX12DescriptorSet>> allocateMultiple(UInt32 descriptorSets, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const override;

        /// <inheritdoc />
        Enumerable<UniquePtr<DirectX12DescriptorSet>> allocateMultiple(UInt32 descriptorSets, UInt32 descriptors, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const override;

        /// <inheritdoc />
        Enumerable<UniquePtr<DirectX12DescriptorSet>> allocateMultiple(UInt32 descriptorSets, UInt32 descriptors, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const override;

        /// <inheritdoc />
        void free(const DirectX12DescriptorSet& descriptorSet) const noexcept override;
    };

    /// <summary>
    /// Implements the DirectX 12 <see cref="IPushConstantsRange" />.
    /// </summary>
    /// <seealso cref="DirectX12PushConstantsLayout" />
    class LITEFX_DIRECTX12_API DirectX12PushConstantsRange final : public IPushConstantsRange {
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
        explicit DirectX12PushConstantsRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding);
        DirectX12PushConstantsRange(const DirectX12PushConstantsRange&) = delete;
        DirectX12PushConstantsRange(DirectX12PushConstantsRange&&) = delete;
        virtual ~DirectX12PushConstantsRange() noexcept;

    public:
        /// <inheritdoc />
        UInt32 space() const noexcept override;

        /// <inheritdoc />
        UInt32 binding() const noexcept override;

        /// <inheritdoc />
        UInt32 offset() const noexcept override;

        /// <inheritdoc />
        UInt32 size() const noexcept override;

        /// <inheritdoc />
        ShaderStage stage() const noexcept override;

    public:
        /// <summary>
        /// Returns the index of the root parameter, the range is bound to.
        /// </summary>
        /// <returns>The index of the root parameter, the range is bound to.</returns>
        virtual UInt32 rootParameterIndex() const noexcept;

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
    class LITEFX_DIRECTX12_API DirectX12PushConstantsLayout final : public PushConstantsLayout<DirectX12PushConstantsRange> {
        LITEFX_IMPLEMENTATION(DirectX12PushConstantsLayoutImpl);
        LITEFX_BUILDER(DirectX12PushConstantsLayoutBuilder);
        friend class DirectX12PipelineLayout;

    public:
        /// <summary>
        /// Initializes a new push constants layout.
        /// </summary>
        /// <param name="ranges">The ranges contained by the layout.</param>
        /// <param name="size">The overall size (in bytes) of the push constants backing memory.</param>
        explicit DirectX12PushConstantsLayout(Enumerable<UniquePtr<DirectX12PushConstantsRange>>&& ranges, UInt32 size);
        DirectX12PushConstantsLayout(const DirectX12PushConstantsLayout&) = delete;
        DirectX12PushConstantsLayout(DirectX12PushConstantsLayout&&) = delete;
        virtual ~DirectX12PushConstantsLayout() noexcept;

    private:
        /// <summary>
        /// Initializes a new push constants layout.
        /// </summary>
        /// <param name="size">The overall size (in bytes) of the push constants backing memory.</param>
        explicit DirectX12PushConstantsLayout(UInt32 size);

    public:
        /// <inheritdoc />
        UInt32 size() const noexcept override;

        /// <inheritdoc />
        const DirectX12PushConstantsRange& range(ShaderStage stage) const override;

        /// <inheritdoc />
        Enumerable<const DirectX12PushConstantsRange*> ranges() const noexcept override;

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
    class LITEFX_DIRECTX12_API DirectX12PipelineLayout final : public PipelineLayout<DirectX12DescriptorSetLayout, DirectX12PushConstantsLayout>, public ComResource<ID3D12RootSignature> {
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
        const DirectX12DescriptorSetLayout& descriptorSet(UInt32 space) const override;

        /// <inheritdoc />
        Enumerable<const DirectX12DescriptorSetLayout*> descriptorSets() const noexcept override;

        /// <inheritdoc />
        const DirectX12PushConstantsLayout* pushConstants() const noexcept override;
    };

    /// <summary>
    /// Implements the DirectX 12 input assembler state.
    /// </summary>
    /// <seealso cref="DirectX12InputAssemblerBuilder" />
    class LITEFX_DIRECTX12_API DirectX12InputAssembler final : public InputAssembler<DirectX12VertexBufferLayout, DirectX12IndexBufferLayout> {
        LITEFX_IMPLEMENTATION(DirectX12InputAssemblerImpl);
        LITEFX_BUILDER(DirectX12InputAssemblerBuilder);

	public:
		/// <summary>
		/// Initializes a new DirectX 12 input assembler state.
		/// </summary>
		/// <param name="vertexBufferLayouts">The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.</param>
		/// <param name="indexBufferLayout">The index buffer layout.</param>
		/// <param name="primitiveTopology">The primitive topology.</param>
		explicit DirectX12InputAssembler(Enumerable<UniquePtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<DirectX12IndexBufferLayout>&& indexBufferLayout = nullptr, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList);
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
        Enumerable<const DirectX12VertexBufferLayout*> vertexBufferLayouts() const noexcept override;

		/// <inheritdoc />
		const DirectX12VertexBufferLayout* vertexBufferLayout(UInt32 binding) const override;

		/// <inheritdoc />
		const DirectX12IndexBufferLayout* indexBufferLayout() const noexcept override;

        /// <inheritdoc />
        PrimitiveTopology topology() const noexcept override;
    };

    /// <summary>
    /// Implements a DirectX 12 <see cref="IRasterizer" />.
    /// </summary>
    /// <seealso cref="DirectX12RasterizerBuilder" />
    class LITEFX_DIRECTX12_API DirectX12Rasterizer final : public Rasterizer {
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
        explicit DirectX12Rasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth = 1.f, const DepthStencilState& depthStencilState = {}) noexcept;
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
    /// Records commands for a <see cref="DirectX12Queue" />
    /// </summary>
    /// <seealso cref="DirectX12Queue" />
    class LITEFX_DIRECTX12_API DirectX12CommandBuffer final : public CommandBuffer<DirectX12CommandBuffer, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, DirectX12Barrier, DirectX12PipelineState, DirectX12BottomLevelAccelerationStructure, DirectX12TopLevelAccelerationStructure>, public ComResource<ID3D12GraphicsCommandList7>, public std::enable_shared_from_this<DirectX12CommandBuffer> {
        LITEFX_IMPLEMENTATION(DirectX12CommandBufferImpl);

    public:
        using base_type = CommandBuffer<DirectX12CommandBuffer, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, DirectX12Barrier, DirectX12PipelineState, DirectX12BottomLevelAccelerationStructure, DirectX12TopLevelAccelerationStructure>;
        using base_type::dispatch;
        using base_type::dispatchIndirect;
        using base_type::dispatchMesh;
        using base_type::draw;
        using base_type::drawIndirect;
        using base_type::drawIndexed;
        using base_type::drawIndexedIndirect;
        using base_type::barrier;
        using base_type::transfer;
        using base_type::generateMipMaps;
        using base_type::bind;
        using base_type::use;
        using base_type::pushConstants;
        using base_type::buildAccelerationStructure;
        using base_type::updateAccelerationStructure;
        using base_type::copyAccelerationStructure;

    private:
        /// <summary>
        /// Initializes the command buffer from a command queue.
        /// </summary>
        /// <param name="queue">The parent command queue, the buffer gets submitted to.</param>
        /// <param name="begin">If set to <c>true</c>, the command buffer automatically starts recording by calling <see cref="begin" />.</param>
        /// <param name="primary"><c>true</c>, if the command buffer is a primary command buffer.</param>
        explicit DirectX12CommandBuffer(const DirectX12Queue& queue, bool begin = false, bool primary = true);

    public:
        DirectX12CommandBuffer(const DirectX12CommandBuffer&) = delete;
        DirectX12CommandBuffer(DirectX12CommandBuffer&&) = delete;
        virtual ~DirectX12CommandBuffer() noexcept;

    public:
        /// <summary>
        /// Initializes the command buffer from a command queue.
        /// </summary>
        /// <param name="queue">The parent command queue, the buffer gets submitted to.</param>
        /// <param name="begin">If set to <c>true</c>, the command buffer automatically starts recording by calling <see cref="begin" />.</param>
        /// <param name="primary"><c>true</c>, if the command buffer is a primary command buffer.</param>
        static inline SharedPtr<DirectX12CommandBuffer> create(const DirectX12Queue& queue, bool begin = false, bool primary = true) {
            return SharedPtr<DirectX12CommandBuffer>(new DirectX12CommandBuffer(queue, begin, primary));
        }

        // CommandBuffer interface.
    public:
        /// <inheritdoc />
        const ICommandQueue& queue() const noexcept override;

        /// <inheritdoc />
        void begin() const override;

        /// <inheritdoc />
        void end() const override;

        /// <inheritdoc />
        bool isSecondary() const noexcept override;

        /// <inheritdoc />
        void setViewports(Span<const IViewport*> viewports) const noexcept override;

        /// <inheritdoc />
        void setViewports(const IViewport* viewport) const noexcept override;

        /// <inheritdoc />
        void setScissors(Span<const IScissor*> scissors) const noexcept override;

        /// <inheritdoc />
        void setScissors(const IScissor* scissor) const noexcept override;

        /// <inheritdoc />
        void setBlendFactors(const Vector4f& blendFactors) const noexcept override;

        /// <inheritdoc />
        void setStencilRef(UInt32 stencilRef) const noexcept override;

        /// <inheritdoc />
        UInt64 submit() const override;

        /// <inheritdoc />
        void generateMipMaps(IDirectX12Image& image) noexcept override;

        /// <inheritdoc />
        [[nodiscard]] UniquePtr<DirectX12Barrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept override;

        /// <inheritdoc />
        void barrier(const DirectX12Barrier& barrier) const noexcept override;

        /// <inheritdoc />
        void transfer(const IDirectX12Buffer& source, const IDirectX12Buffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// <inheritdoc />
        void transfer(const void* const data, size_t size, const IDirectX12Buffer& target, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// <inheritdoc />
        void transfer(Span<const void* const> data, size_t elementSize, const IDirectX12Buffer& target, UInt32 firstElement = 0) const override;

        /// <inheritdoc />
        void transfer(const IDirectX12Buffer& source, const IDirectX12Image& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const override;

        /// <inheritdoc />
        void transfer(const void* const data, size_t size, const IDirectX12Image& target, UInt32 subresource = 0) const override;

        /// <inheritdoc />
        void transfer(Span<const void* const> data, size_t elementSize, const IDirectX12Image& target, UInt32 firstSubresource = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void transfer(const IDirectX12Image& source, const IDirectX12Image& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void transfer(const IDirectX12Image& source, const IDirectX12Buffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void transfer(SharedPtr<const IDirectX12Buffer> source, const IDirectX12Buffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// <inheritdoc />
        void transfer(SharedPtr<const IDirectX12Buffer> source, const IDirectX12Image& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const override;

        /// <inheritdoc />
        void transfer(SharedPtr<const IDirectX12Image> source, const IDirectX12Image& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void transfer(SharedPtr<const IDirectX12Image> source, const IDirectX12Buffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void use(const DirectX12PipelineState& pipeline) const noexcept override;

		/// <inheritdoc />
		void bind(const DirectX12DescriptorSet& descriptorSet) const override;

        /// <inheritdoc />
        void bind(Span<const DirectX12DescriptorSet*> descriptorSets) const override;

        /// <inheritdoc />
        void bind(const DirectX12DescriptorSet& descriptorSet, const DirectX12PipelineState& pipeline) const noexcept override;

		/// <inheritdoc />
		void bind(Span<const DirectX12DescriptorSet*> descriptorSets, const DirectX12PipelineState& pipeline) const noexcept override;

        /// <inheritdoc />
        void bind(const IDirectX12VertexBuffer& buffer) const noexcept override;

        /// <inheritdoc />
        void bind(const IDirectX12IndexBuffer& buffer) const noexcept override;

        /// <inheritdoc />
        void dispatch(const Vector3u& threadCount) const noexcept override;

        /// <inheritdoc />
        void dispatchIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// <inheritdoc />
        void dispatchIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept;

        /// <inheritdoc />
        void dispatchMesh(const Vector3u& threadCount) const noexcept override;

        /// <inheritdoc />
        void dispatchMeshIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// <inheritdoc />
        void dispatchMeshIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// <inheritdoc />
        void draw(UInt32 vertices, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const noexcept override;

        /// <inheritdoc />
        void drawIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// <inheritdoc />
        void drawIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// <inheritdoc />
        void drawIndexed(UInt32 indices, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const noexcept override;

        /// <inheritdoc />
        void drawIndexedIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// <inheritdoc />
        void drawIndexedIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;
        
        /// <inheritdoc />
        void pushConstants(const DirectX12PushConstantsLayout& layout, const void* const memory) const noexcept override;

        /// <inheritdoc />
        void writeTimingEvent(SharedPtr<const TimingEvent> timingEvent) const override;

        /// <inheritdoc />
        void execute(SharedPtr<const DirectX12CommandBuffer> commandBuffer) const override;

        /// <inheritdoc />
        void execute(Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers) const override;

        /// <inheritdoc />
        void buildAccelerationStructure(DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset = 0) const override;

        /// <inheritdoc />
        void buildAccelerationStructure(DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset = 0) const override;

        /// <inheritdoc />
        void updateAccelerationStructure(DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset = 0) const override;

        /// <inheritdoc />
        void updateAccelerationStructure(DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer> scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset = 0) const override;

        /// <inheritdoc />
        void copyAccelerationStructure(const DirectX12BottomLevelAccelerationStructure& from, const DirectX12BottomLevelAccelerationStructure& to, bool compress = false) const noexcept override;

        /// <inheritdoc />
        void copyAccelerationStructure(const DirectX12TopLevelAccelerationStructure& from, const DirectX12TopLevelAccelerationStructure& to, bool compress = false) const noexcept override;

        /// <inheritdoc />
        void traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IDirectX12Buffer& rayGenerationShaderBindingTable, const IDirectX12Buffer* missShaderBindingTable, const IDirectX12Buffer* hitShaderBindingTable, const IDirectX12Buffer* callableShaderBindingTable) const noexcept override;

    private:
        void releaseSharedState() const override;
    };

    /// <summary>
    /// Implements a DirectX 12 command queue.
    /// </summary>
    /// <seealso cref="DirectX12CommandBuffer" />
    class LITEFX_DIRECTX12_API DirectX12Queue final : public CommandQueue<DirectX12CommandBuffer>, public ComResource<ID3D12CommandQueue> {
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
        explicit DirectX12Queue(const DirectX12Device& device, QueueType type, QueuePriority priority);
        DirectX12Queue(const DirectX12Queue&) = delete;
        DirectX12Queue(DirectX12Queue&&) = delete;
        virtual ~DirectX12Queue() noexcept;

        // DirectX12Queue interface.
    public:
        /// <summary>
        /// Returns a reference to the device that provides this queue.
        /// </summary>
        /// <returns>A reference to the queue's parent device.</returns>
        virtual const DirectX12Device& device() const noexcept;

        // CommandQueue interface.
    public:
        /// <inheritdoc />
        QueuePriority priority() const noexcept override;

        /// <inheritdoc />
        QueueType type() const noexcept override;

#if !defined(NDEBUG) && defined(_WIN64)
    public:
        /// <inheritdoc />
        void beginDebugRegion(const String& label, const Vectors::ByteVector3& color = { 128_b, 128_b, 128_b }) const noexcept override;

        /// <inheritdoc />
        void endDebugRegion() const noexcept override;

        /// <inheritdoc />
        void setDebugMarker(const String& label, const Vectors::ByteVector3& color = { 128_b, 128_b, 128_b }) const noexcept override;
#endif // !defined(NDEBUG) && defined(_WIN64)

    public:
        /// <inheritdoc />
        SharedPtr<DirectX12CommandBuffer> createCommandBuffer(bool beginRecording = false, bool secondary = false) const override;

        /// <inheritdoc />
        UInt64 submit(SharedPtr<const DirectX12CommandBuffer> commandBuffer) const override;

        /// <inheritdoc />
        UInt64 submit(const Enumerable<SharedPtr<const DirectX12CommandBuffer>>& commandBuffers) const override;

        /// <inheritdoc />
        void waitFor(UInt64 fence) const noexcept override;

        /// <inheritdoc />
        void waitFor(const DirectX12Queue& queue, UInt64 fence) const noexcept;

        /// <inheritdoc />
        UInt64 currentFence() const noexcept override;

    private:
        inline void waitForQueue(const ICommandQueue& queue, UInt64 fence) const override {
            auto d3dQueue = dynamic_cast<const DirectX12Queue*>(&queue);

            if (d3dQueue == nullptr) [[unlikely]]
                throw InvalidArgumentException("queue", "Cannot wait for queues from other backends.");

            this->waitFor(*d3dQueue, fence);
        }
    };

    /// <summary>
    /// Implements a DirectX 12 <see cref="RenderPipeline" />.
    /// </summary>
    /// <seealso cref="DirectX12ComputePipeline" />
    /// <seealso cref="DirectX12RenderPipelineBuilder" />
    class LITEFX_DIRECTX12_API DirectX12RenderPipeline final : public virtual DirectX12PipelineState, public RenderPipeline<DirectX12PipelineLayout, DirectX12ShaderProgram, DirectX12InputAssembler, DirectX12Rasterizer> {
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
        /// <param name="samples">The initial multi-sampling level of the render pipeline.</param>
        /// <param name="enableAlphaToCoverage">Whether or not to enable Alpha-to-Coverage multi-sampling.</param>
        /// <param name="name">The optional name of the render pipeline.</param>
        explicit DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, SharedPtr<DirectX12InputAssembler> inputAssembler, SharedPtr<DirectX12Rasterizer> rasterizer, MultiSamplingLevel samples = MultiSamplingLevel::x1, bool enableAlphaToCoverage = false, const String& name = "");
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
        SharedPtr<const DirectX12ShaderProgram> program() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const DirectX12PipelineLayout> layout() const noexcept override;

        // RenderPipeline interface.
    public:
        /// <inheritdoc />
        SharedPtr<DirectX12InputAssembler> inputAssembler() const noexcept override;

        /// <inheritdoc />
        SharedPtr<DirectX12Rasterizer> rasterizer() const noexcept override;

        /// <inheritdoc />
        bool alphaToCoverage() const noexcept override;

        /// <inheritdoc />
        MultiSamplingLevel samples() const noexcept override;

        /// <inheritdoc />
        void updateSamples(MultiSamplingLevel samples) override;

        // DirectX12PipelineState interface.
    public:
        /// <inheritdoc />
        void use(const DirectX12CommandBuffer& commandBuffer) const noexcept override;
    };

    /// <summary>
    /// Implements a DirectX 12 <see cref="ComputePipeline" />.
    /// </summary>
    /// <seealso cref="DirectX12RenderPipeline" />
    /// <seealso cref="DirectX12ComputePipelineBuilder" />
    class LITEFX_DIRECTX12_API DirectX12ComputePipeline final : public virtual DirectX12PipelineState, public ComputePipeline<DirectX12PipelineLayout, DirectX12ShaderProgram> {
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
        SharedPtr<const DirectX12ShaderProgram> program() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const DirectX12PipelineLayout> layout() const noexcept override;

        // DirectX12PipelineState interface.
    public:
        void use(const DirectX12CommandBuffer& commandBuffer) const noexcept override;
    };

    /// <summary>
    /// Implements a DirectX 12 <see cref="RayTracingPipeline" />.
    /// </summary>
    /// <remarks>
    /// Note that the ray tracing pipeline does not set its handle, as it is a different base type from the one used to define ray tracing pipelines. Instead to obtain the handle,
    /// call <see cref="DirectX12RayTracingPipeline::stateObject()" />.
    /// 
    /// At some point it is expected that D3D introduces a unified pipeline architecture based on state objects. At this point, the pipeline state base object will switch to using
    /// state objects as its handle type and the `stateObject` method above will be deprecated.
    /// </remarks>
    /// <seealso cref="DirectX12RenderPipeline" />
    /// <seealso cref="DirectX12RayTracingPipelineBuilder" />
    class LITEFX_DIRECTX12_API DirectX12RayTracingPipeline final : public virtual DirectX12PipelineState, public RayTracingPipeline<DirectX12PipelineLayout, DirectX12ShaderProgram> {
        LITEFX_IMPLEMENTATION(DirectX12RayTracingPipelineImpl);
        LITEFX_BUILDER(DirectX12RayTracingPipelineBuilder);

    public:
        /// <summary>
        /// Initializes a new DirectX 12 ray-tracing pipeline.
        /// </summary>
        /// <param name="device">The parent device.</param>
        /// <param name="layout">The layout of the pipeline.</param>
        /// <param name="shaderProgram">The shader program used by this pipeline.</param>
        /// <param name="shaderRecords">The shader record collection that is used to build the shader binding table for the pipeline.</param>
        /// <param name="maxRecursionDepth">The maximum number of ray bounces.</param>
        /// <param name="maxPayloadSize">The maximum size for ray payloads in the pipeline.</param>
        /// <param name="maxAttributeSize">The maximum size for ray attributes in the pipeline.</param>
        /// <param name="name">The optional debug name of the ray-tracing pipeline.</param>
        explicit DirectX12RayTracingPipeline(const DirectX12Device& device, SharedPtr<DirectX12PipelineLayout> layout, SharedPtr<DirectX12ShaderProgram> shaderProgram, ShaderRecordCollection&& shaderRecords, UInt32 maxRecursionDepth = 10, UInt32 maxPayloadSize = 0, UInt32 maxAttributeSize = 32, const String& name = "");
        DirectX12RayTracingPipeline(DirectX12RayTracingPipeline&&) noexcept = delete;
        DirectX12RayTracingPipeline(const DirectX12RayTracingPipeline&) noexcept = delete;
        virtual ~DirectX12RayTracingPipeline() noexcept;

    private:
        /// <summary>
        /// Initializes a new DirectX 12 ray-tracing pipeline.
        /// </summary>
        /// <param name="device">The parent device.</param>
        /// <param name="shaderRecords">The shader record collection that is used to build the shader binding table for the pipeline.</param>
        DirectX12RayTracingPipeline(const DirectX12Device& device, ShaderRecordCollection&& shaderRecords) noexcept;

        // Pipeline interface.
    public:
        /// <inheritdoc />
        SharedPtr<const DirectX12ShaderProgram> program() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const DirectX12PipelineLayout> layout() const noexcept override;

        // RayTracingPipeline interface.
    public:
        /// <inheritdoc />
        const ShaderRecordCollection& shaderRecords() const noexcept override;

        /// <inheritdoc />
        UInt32 maxRecursionDepth() const noexcept override;

        /// <inheritdoc />
        UInt32 maxPayloadSize() const noexcept override;

        /// <inheritdoc />
        UInt32 maxAttributeSize() const noexcept override;

        /// <inheritdoc />
        UniquePtr<IDirectX12Buffer> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups = ShaderBindingGroup::All) const noexcept override;

        // DirectX12PipelineState interface.
    public:
        /// <inheritdoc />
        void use(const DirectX12CommandBuffer& commandBuffer) const noexcept override;

        /// <summary>
        /// Returns the handle of the ray tracing pipeline state object.
        /// </summary>
        /// <returns>The handle of the ray tracing pipeline state object.</returns>
        ComPtr<ID3D12StateObject> stateObject() const noexcept;
    };

    /// <summary>
    /// Implements a DirectX 12 frame buffer.
    /// </summary>
    /// <seealso cref="DirectX12RenderPass" />
    class LITEFX_DIRECTX12_API DirectX12FrameBuffer final : public FrameBuffer<IDirectX12Image> {
        LITEFX_IMPLEMENTATION(DirectX12FrameBufferImpl);

    public:
        using FrameBuffer::addImage;
        using FrameBuffer::mapRenderTarget;
        using FrameBuffer::mapRenderTargets;

    public:
        /// <summary>
        /// Initializes a DirectX 12 frame buffer.
        /// </summary>
        /// <param name="device">The device the frame buffer is allocated on.</param>
        /// <param name="renderArea">The initial size of the render area.</param>
        /// <param name="name">The name of the frame buffer.</param>
        DirectX12FrameBuffer(const DirectX12Device& device, const Size2d& renderArea, StringView name = "");
        DirectX12FrameBuffer(const DirectX12FrameBuffer&) noexcept = delete;
        DirectX12FrameBuffer(DirectX12FrameBuffer&&) noexcept = delete;
        virtual ~DirectX12FrameBuffer() noexcept;

        // DirectX 12 FrameBuffer
    public:
        /// <summary>
        /// Returns the descriptor handle for an image at the specified index.
        /// </summary>
        /// <param name="imageIndex">The index of the image for which the descriptor handle should be returned.</param>
        /// <returns>The descriptor handle for the image.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the provided image index does not address an image within the frame buffer.</exception>
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle(UInt32 imageIndex) const;

        /// <summary>
        /// Returns the descriptor handle for an image with the specified name.
        /// </summary>
        /// <param name="imageName">The name of the image for which the descriptor handle should be returned.</param>
        /// <returns>The descriptor handle for the image.</returns>
        /// <exception cref="InvalidArgumentException">Thrown, if the provided image name does refer to an image within the frame buffer.</exception>
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle(StringView imageName) const;
        
        /// <summary>
        /// Returns the descriptor handle for an image mapped to the specified render target.
        /// </summary>
        /// <param name="renderTarget">The render target for which to return the image descriptor handle.</param>
        /// <returns>The descriptor handle for the image.</returns>
        /// <exception cref="InvalidArgumentException">Thrown, if the provided render target is not mapped to an image within the frame buffer.</exception>
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle(const RenderTarget& renderTarget) const;

        // FrameBuffer interface.
    public:
        /// <inheritdoc />
        const Size2d& size() const noexcept override;

        /// <inheritdoc />
        size_t getWidth() const noexcept override;

        /// <inheritdoc />
        size_t getHeight() const noexcept override;

        /// <inheritdoc />
        void mapRenderTarget(const RenderTarget& renderTarget, UInt32 index) override;

        /// <inheritdoc />
        void mapRenderTarget(const RenderTarget& renderTarget, StringView name) override;

        /// <inheritdoc />
        void unmapRenderTarget(const RenderTarget& renderTarget) noexcept override;

        /// <inheritdoc />
        Enumerable<const IDirectX12Image*> images() const noexcept override;

        /// <inheritdoc />
        inline const IDirectX12Image& operator[](UInt32 index) const override {
            return this->image(index);
        }

        /// <inheritdoc />
        const IDirectX12Image& image(UInt32 index) const override;

        /// <inheritdoc />
        inline const IDirectX12Image& operator[](const RenderTarget& renderTarget) const override {
            return this->image(renderTarget);
        }

        /// <inheritdoc />
        const IDirectX12Image& image(const RenderTarget& renderTarget) const override;

        /// <inheritdoc />
        inline const IDirectX12Image& operator[](StringView renderTargetName) const override {
            return this->resolveImage(hash(renderTargetName));
        }
        
        /// <inheritdoc />
        inline const IDirectX12Image& image(StringView renderTargetName) const override {
            return this->resolveImage(hash(renderTargetName));
        }
        
        /// <inheritdoc />
        const IDirectX12Image& resolveImage(UInt64 hash) const override;

        /// <inheritdoc />
        void addImage(const String& name, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) override;

        /// <inheritdoc />
        void addImage(const String& name, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) override;

        /// <inheritdoc />
        void resize(const Size2d& renderArea) override;
    };

    /// <summary>
    /// Implements a DirectX 12 render pass.
    /// </summary>
    /// <seealso cref="DirectX12RenderPassBuilder" />
    class LITEFX_DIRECTX12_API DirectX12RenderPass final : public RenderPass<DirectX12RenderPipeline, DirectX12Queue, DirectX12FrameBuffer> {
        LITEFX_IMPLEMENTATION(DirectX12RenderPassImpl);
        LITEFX_BUILDER(DirectX12RenderPassBuilder);

    public:
        using base_type = RenderPass<DirectX12RenderPipeline, DirectX12Queue, DirectX12FrameBuffer>;

    public:
        /// <summary>
        /// Creates and initializes a new DirectX 12 render pass instance that executes on the default graphics queue.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        explicit DirectX12RenderPass(const DirectX12Device& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u);

        /// <summary>
        /// Creates and initializes a new DirectX 12 render pass instance that executes on the default graphics queue.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="name">The name of the render pass state resource.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        explicit DirectX12RenderPass(const DirectX12Device& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u);
        
        /// <summary>
        /// Creates and initializes a new DirectX 12 render pass instance.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="queue">The command queue to execute the render pass on.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        explicit DirectX12RenderPass(const DirectX12Device& device, const DirectX12Queue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u);

        /// <summary>
        /// Creates and initializes a new DirectX 12 render pass instance.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="name">The name of the render pass state resource.</param>
        /// <param name="queue">The command queue to execute the render pass on.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        explicit DirectX12RenderPass(const DirectX12Device& device, const String& name, const DirectX12Queue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u);

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

        // DirectX 12 render pass.
    public:
        /// <summary>
        /// Returns a reference to the device that provides this queue.
        /// </summary>
        /// <returns>A reference to the queue's parent device.</returns>
        virtual const DirectX12Device& device() const noexcept;

        // RenderPass interface.
    public:

        /// <inheritdoc />
        const DirectX12FrameBuffer& activeFrameBuffer() const override;

        /// <inheritdoc />
        const DirectX12Queue& commandQueue() const noexcept override;

        /// <inheritdoc />
        Enumerable<const DirectX12RenderPipeline*> pipelines() const noexcept override;

        /// <inheritdoc />
        Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const DirectX12CommandBuffer> commandBuffer(UInt32 index) const override;

        /// <inheritdoc />
        UInt32 secondaryCommandBuffers() const noexcept override;

        /// <inheritdoc />
        const Array<RenderTarget>& renderTargets() const noexcept override;

        /// <inheritdoc />
        const RenderTarget& renderTarget(UInt32 location) const override;

        /// <inheritdoc />
        bool hasPresentTarget() const noexcept override;

        /// <inheritdoc />
        const Array<RenderPassDependency>& inputAttachments() const noexcept override;

        /// <inheritdoc />
        const RenderPassDependency& inputAttachment(UInt32 location) const override;

        /// <inheritdoc />
        const Optional<DescriptorBindingPoint>& inputAttachmentSamplerBinding() const noexcept override;

        /// <inheritdoc />
        void begin(const DirectX12FrameBuffer& frameBuffer) const override;

        /// <inheritdoc />
        UInt64 end() const override;
    };

    /// <summary>
    /// Implements a DirectX 12 swap chain.
    /// </summary>
    class LITEFX_DIRECTX12_API DirectX12SwapChain final : public SwapChain<IDirectX12Image>, public ComResource<IDXGISwapChain4> {
        LITEFX_IMPLEMENTATION(DirectX12SwapChainImpl);
        friend class DirectX12RenderPass;

    public:
        using base_type = SwapChain<IDirectX12Image>;

    public:
        /// <summary>
        /// Initializes a DirectX 12 swap chain.
        /// </summary>
        /// <param name="device">The device that owns the swap chain.</param>
        /// <param name="format">The initial surface format.</param>
        /// <param name="renderArea">The initial size of the render area.</param>
        /// <param name="enableVsync">`true` if vertical synchronization should be used, otherwise `false`.</param>
        /// <param name="buffers">The initial number of buffers.</param>
        explicit DirectX12SwapChain(const DirectX12Device& device, Format surfaceFormat = Format::B8G8R8A8_SRGB, const Size2d& renderArea = { 800, 600 }, UInt32 buffers = 3, bool enableVsync = false);
        DirectX12SwapChain(const DirectX12SwapChain&) = delete;
        DirectX12SwapChain(DirectX12SwapChain&&) = delete;
        virtual ~DirectX12SwapChain() noexcept;

        // DirectX 12 swap chain.
    public:
        /// <summary>
        /// Returns <c>true</c>, if the adapter supports variable refresh rates (i.e. tearing is allowed).
        /// </summary>
        /// <returns><c>true</c>, if the adapter supports variable refresh rates (i.e. tearing is allowed).</returns>
        virtual bool supportsVariableRefreshRate() const noexcept;

        /// <summary>
        /// Returns the query heap for the current frame.
        /// </summary>
        /// <returns>A pointer to the query heap for the current frame.</returns>
        virtual ID3D12QueryHeap* timestampQueryHeap() const noexcept;

        // SwapChain interface.
    public:
        /// <inheritdoc />
        Enumerable<SharedPtr<TimingEvent>> timingEvents() const noexcept override;

        /// <inheritdoc />
        SharedPtr<TimingEvent> timingEvent(UInt32 queryId) const override;

        /// <inheritdoc />
        UInt64 readTimingEvent(SharedPtr<const TimingEvent> timingEvent) const override;

        /// <inheritdoc />
        UInt32 resolveQueryId(SharedPtr<const TimingEvent> timingEvent) const override;

        /// <inheritdoc />
        Format surfaceFormat() const noexcept override;

        /// <inheritdoc />
        UInt32 buffers() const noexcept override;

        /// <inheritdoc />
        const Size2d& renderArea() const noexcept override;

        /// <inheritdoc />
        bool verticalSynchronization() const noexcept override;

        /// <inheritdoc />
        IDirectX12Image* image(UInt32 backBuffer) const override;

        /// <inheritdoc />
        const IDirectX12Image& image() const noexcept override;

        /// <inheritdoc />
        Enumerable<IDirectX12Image*> images() const noexcept override;

        /// <inheritdoc />
        void present(UInt64 fence) const override;

    public:
        /// <inheritdoc />
        Enumerable<Format> getSurfaceFormats() const noexcept override;

        /// <inheritdoc />
        void addTimingEvent(SharedPtr<TimingEvent> timingEvent) override;

        /// <inheritdoc />
        void reset(Format surfaceFormat, const Size2d& renderArea, UInt32 buffers, bool enableVsync = false) override;

        /// <inheritdoc />
        [[nodiscard]] UInt32 swapBackBuffer() const override;

    private:
        void resolveQueryHeaps(const DirectX12CommandBuffer& commandBuffer) const noexcept;
    };

    /// <summary>
    /// A graphics factory that produces objects for a <see cref="DirectX12Device" />.
    /// </summary>
    /// <remarks>
    /// The DX12 graphics factory is implemented using <a href="https://gpuopen.com/d3d12-memory-allocator/" target="_blank">D3D12 Memory Allocator</a>.
    /// </remarks>
    class LITEFX_DIRECTX12_API DirectX12GraphicsFactory final : public GraphicsFactory<DirectX12DescriptorLayout, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, IDirectX12Sampler, DirectX12BottomLevelAccelerationStructure, DirectX12TopLevelAccelerationStructure> {
        LITEFX_IMPLEMENTATION(DirectX12GraphicsFactoryImpl);

    public:
        using base_type = GraphicsFactory<DirectX12DescriptorLayout, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, IDirectX12Sampler, DirectX12BottomLevelAccelerationStructure, DirectX12TopLevelAccelerationStructure>;
        using base_type::createBuffer;
        using base_type::createVertexBuffer;
        using base_type::createIndexBuffer;
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
        UniquePtr<IDirectX12Buffer> createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        UniquePtr<IDirectX12Buffer> createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        UniquePtr<IDirectX12VertexBuffer> createVertexBuffer(const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        UniquePtr<IDirectX12VertexBuffer> createVertexBuffer(const String& name, const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        UniquePtr<IDirectX12IndexBuffer> createIndexBuffer(const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        UniquePtr<IDirectX12IndexBuffer> createIndexBuffer(const String& name, const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        UniquePtr<IDirectX12Image> createTexture(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        UniquePtr<IDirectX12Image> createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        Enumerable<UniquePtr<IDirectX12Image>> createTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        UniquePtr<IDirectX12Sampler> createSampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// <inheritdoc />
        UniquePtr<IDirectX12Sampler> createSampler(const String& name, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// <inheritdoc />
        Enumerable<UniquePtr<IDirectX12Sampler>> createSamplers(UInt32 elements, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// <inheritdoc />
        virtual UniquePtr<DirectX12BottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const override;

        /// <inheritdoc />
        virtual UniquePtr<DirectX12TopLevelAccelerationStructure> createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const override;
    };

    /// <summary>
    /// Implements a DirectX 12 graphics device.
    /// </summary>
    class LITEFX_DIRECTX12_API DirectX12Device final : public GraphicsDevice<DirectX12GraphicsFactory, DirectX12Surface, DirectX12GraphicsAdapter, DirectX12SwapChain, DirectX12Queue, DirectX12RenderPass, DirectX12ComputePipeline, DirectX12RayTracingPipeline, DirectX12Barrier>, public ComResource<ID3D12Device10> {
        LITEFX_IMPLEMENTATION(DirectX12DeviceImpl);

    public:
        /// <summary>
        /// Creates a new device instance.
        /// </summary>
        /// <param name="backend">The backend from which the device got created.</param>
        /// <param name="adapter">The adapter the device uses for drawing.</param>
        /// <param name="surface">The surface, the device should draw to.</param>
        /// <param name="features">The features that should be supported by this device.</param>
        explicit DirectX12Device(const DirectX12Backend& backend, const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, GraphicsDeviceFeatures features = {});

        /// <summary>
        /// Creates a new device instance.
        /// </summary>
        /// <param name="backend">The backend from which the device got created.</param>
        /// <param name="adapter">The adapter the device uses for drawing.</param>
        /// <param name="surface">The surface, the device should draw to.</param>
        /// <param name="format">The initial surface format, device uses for drawing.</param>
        /// <param name="renderArea">The initial size of the render area.</param>
        /// <param name="backBuffers">The initial number of back buffers.</param>
        /// <param name="enableVsync">The initial setting for vertical synchronization.</param>
        /// <param name="features">The features that should be supported by this device.</param>
        /// <param name="globalBufferHeapSize">The size of the global heap for constant buffers, shader resources and images.</param>
        /// <param name="globalSamplerHeapSize">The size of the global heap for samplers.</param>
        explicit DirectX12Device(const DirectX12Backend& backend, const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync = false, GraphicsDeviceFeatures features = {}, UInt32 globalBufferHeapSize = 524287, UInt32 globalSamplerHeapSize = 2048);

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
        virtual void updateBufferDescriptors(const DirectX12DescriptorSet& descriptorSet, UInt32 firstDescriptor, UInt32 descriptors) const noexcept;

        /// <summary>
        /// Updates a sampler descriptors in the global buffer descriptor heap with a descriptor from <paramref name="descriptorSet" />.
        /// </summary>
        /// <param name="descriptorSet">The descriptor set to copy the descriptors from.</param>
        /// <param name="firstDescriptor">The index of the first descriptor to copy.</param>
        /// <param name="descriptors">The number of descriptors to copy.</param>
        virtual void updateSamplerDescriptors(const DirectX12DescriptorSet& descriptorSet, UInt32 firstDescriptor, UInt32 descriptors) const noexcept;

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

        /// <summary>
        /// Returns the command signatures for indirect dispatch and draw calls.
        /// </summary>
        /// <param name="dispatchSignature">The command signature used to execute indirect dispatches.</param>
        /// <param name="dispatchMeshSignature">The command signature used to execute indirect mesh shader dispatches.</param>
        /// <param name="drawSignature">The command signature used to execute indirect non-indexed draw calls.</param>
        /// <param name="drawIndexedSignature">The command signature used to execute indirect indexed draw calls.</param>
        virtual void indirectDrawSignatures(ComPtr<ID3D12CommandSignature>& dispatchSignature, ComPtr<ID3D12CommandSignature>& dispatchMeshSignature, ComPtr<ID3D12CommandSignature>& drawSignature, ComPtr<ID3D12CommandSignature>& drawIndexedSignature) const noexcept;

        // GraphicsDevice interface.
    public:
        /// <inheritdoc />
        DeviceState& state() const noexcept override;

        /// <inheritdoc />
        const DirectX12SwapChain& swapChain() const noexcept override;

        /// <inheritdoc />
        DirectX12SwapChain& swapChain() noexcept override;

        /// <inheritdoc />
        const DirectX12Surface& surface() const noexcept override;

        /// <inheritdoc />
        const DirectX12GraphicsAdapter& adapter() const noexcept override;

        /// <inheritdoc />
        const DirectX12GraphicsFactory& factory() const noexcept override;

        /// <inheritdoc />
        const DirectX12Queue& defaultQueue(QueueType type) const override;

        /// <inheritdoc />
        const DirectX12Queue* createQueue(QueueType type, QueuePriority priority) noexcept override;

        /// <inheritdoc />
        [[nodiscard]] UniquePtr<DirectX12Barrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept override;

        /// <inheritdoc />
        [[nodiscard]] UniquePtr<DirectX12FrameBuffer> makeFrameBuffer(StringView name, const Size2d& renderArea) const noexcept override;

        /// <inheritdoc />
        /// <seealso href="https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels" />
        MultiSamplingLevel maximumMultiSamplingLevel(Format format) const noexcept override;

        /// <inheritdoc />
        double ticksPerMillisecond() const noexcept override;

        /// <inheritdoc />
        void wait() const override;

        /// <inheritdoc />
        void computeAccelerationStructureSizes(const DirectX12BottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const override;

        /// <inheritdoc />
        void computeAccelerationStructureSizes(const DirectX12TopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const override;

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
    public:
        /// <inheritdoc />
        [[nodiscard]] DirectX12RenderPassBuilder buildRenderPass(UInt32 commandBuffers = 1) const override;

        /// <inheritdoc />
        [[nodiscard]] DirectX12RenderPassBuilder buildRenderPass(const String& name, UInt32 commandBuffers = 1) const override;

        /// <inheritdoc />
        //[[nodiscard]] DirectX12RenderPipelineBuilder buildRenderPipeline(const String& name) const override;

        /// <inheritdoc />
        [[nodiscard]] DirectX12RenderPipelineBuilder buildRenderPipeline(const DirectX12RenderPass& renderPass, const String& name) const override;

        /// <inheritdoc />
        [[nodiscard]] DirectX12ComputePipelineBuilder buildComputePipeline(const String& name) const override;

        /// <inheritdoc />
        [[nodiscard]] DirectX12RayTracingPipelineBuilder buildRayTracingPipeline(ShaderRecordCollection&& shaderRecords) const override;

        /// <inheritdoc />
        [[nodiscard]] DirectX12RayTracingPipelineBuilder buildRayTracingPipeline(const String& name, ShaderRecordCollection&& shaderRecords) const override;
        
        /// <inheritdoc />
        [[nodiscard]] DirectX12PipelineLayoutBuilder buildPipelineLayout() const override;

        /// <inheritdoc />
        [[nodiscard]] DirectX12InputAssemblerBuilder buildInputAssembler() const override;

        /// <inheritdoc />
        [[nodiscard]] DirectX12RasterizerBuilder buildRasterizer() const override;

        /// <inheritdoc />
        [[nodiscard]] DirectX12ShaderProgramBuilder buildShaderProgram() const override;

        /// <inheritdoc />
        [[nodiscard]] DirectX12BarrierBuilder buildBarrier() const override;
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)
    };
    
    /// <summary>
    /// Implements the DirectX 12 <see cref="RenderBackend" />.
    /// </summary>
    class LITEFX_DIRECTX12_API DirectX12Backend final : public RenderBackend<DirectX12Device>, public ComResource<IDXGIFactory7> {
        LITEFX_IMPLEMENTATION(DirectX12BackendImpl);

    public:
        explicit DirectX12Backend(const App& app, bool advancedSoftwareRasterizer = false);
        DirectX12Backend(const DirectX12Backend&) noexcept = delete;
        DirectX12Backend(DirectX12Backend&&) noexcept = delete;
        virtual ~DirectX12Backend();

        // IBackend interface.
    public:
        /// <inheritdoc />
        BackendType type() const noexcept override;

        /// <inheritdoc />
        String name() const noexcept override;

    protected:
        /// <inheritdoc />
        void activate() override;

        /// <inheritdoc />
        void deactivate() override;

        // RenderBackend interface.
    public:
        /// <inheritdoc />
        Enumerable<const DirectX12GraphicsAdapter*> listAdapters() const override;

        /// <inheritdoc />
        const DirectX12GraphicsAdapter* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const override;

        /// <inheritdoc />
        void registerDevice(String name, UniquePtr<DirectX12Device>&& device) override;

        /// <inheritdoc />
        void releaseDevice(const String& name) override;

        /// <inheritdoc />
        DirectX12Device* device(const String& name) noexcept override;

        /// <inheritdoc />
        const DirectX12Device* device(const String& name) const noexcept override;

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
        virtual void enableAdvancedSoftwareRasterizer(bool enable = false);
    };

}