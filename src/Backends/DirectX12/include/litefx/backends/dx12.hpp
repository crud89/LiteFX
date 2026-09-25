#pragma once

#include <litefx/rendering.hpp>

#include "dx12_api.hpp"
#include "dx12_formatters.hpp"

#pragma warning(push)
#pragma warning(disable:4250) // Base class members are inherited via dominance.

// NOLINTBEGIN(bugprone-derived-method-shadowing-base-method)

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Math;
    using namespace LiteFX::Rendering;

    /// @brief Implements a DirectX 12 vertex buffer layout.
    ///
    /// @see DirectX12VertexBuffer
    /// @see DirectX12IndexBuffer
    /// @see DirectX12VertexBufferLayoutBuilder
    class LITEFX_DIRECTX12_API DirectX12VertexBufferLayout final : public IVertexBufferLayout {
        LITEFX_IMPLEMENTATION(DirectX12VertexBufferLayoutImpl);
        LITEFX_BUILDER(DirectX12VertexBufferLayoutBuilder);
        friend struct SharedObject::Allocator<DirectX12VertexBufferLayout>;

    private:
        /// @brief Initializes a new vertex buffer layout.
        ///
        /// @param vertexSize The overall size of a single vertex.
        /// @param binding The binding point of the vertex buffers using this layout.
        /// @param inputRate The rate at which the vertex buffer is made available to the vertex shader.
        explicit DirectX12VertexBufferLayout(size_t vertexSize, UInt32 binding = 0, VertexBufferInputRate inputRate = VertexBufferInputRate::Vertex);

        /// @brief Initializes a new vertex buffer layout.
        ///
        /// @param vertexSize The overall size of a single vertex.
        /// @param attributes The vertex attributes.
        /// @param binding The binding point of the vertex buffers using this layout.
        /// @param inputRate The rate at which the vertex buffer is made available to the vertex shader.
        explicit DirectX12VertexBufferLayout(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding = 0, VertexBufferInputRate inputRate = VertexBufferInputRate::Vertex);

    private:
        /// @copydoc IVertexBufferLayout::IVertexBufferLayout(IVertexBufferLayout&&)
        DirectX12VertexBufferLayout(DirectX12VertexBufferLayout&&) noexcept = delete;

        /// @copydoc IVertexBufferLayout::IVertexBufferLayout(const IVertexBufferLayout&)
        DirectX12VertexBufferLayout(const DirectX12VertexBufferLayout&);

        /// @copydoc IVertexBufferLayout::operator=(IVertexBufferLayout&&)
        DirectX12VertexBufferLayout& operator=(DirectX12VertexBufferLayout&&) noexcept = delete;

        /// @copydoc IVertexBufferLayout::operator=(const IVertexBufferLayout&)
        DirectX12VertexBufferLayout& operator=(const DirectX12VertexBufferLayout&) = delete;

    public:
        /// @copydoc IVertexBufferLayout::~IVertexBufferLayout
        ~DirectX12VertexBufferLayout() noexcept override;

    public:
        /// @brief Creates a new vertex buffer layout.
        ///
        /// @param vertexSize The overall size of a single vertex.
        /// @param binding The binding point of the vertex buffers using this layout.
        /// @param inputRate The rate at which the vertex buffer is made available to the vertex shader.
        /// @return A shared pointer to the newly created vertex buffer layout.
        static inline auto create(size_t vertexSize, UInt32 binding = 0, VertexBufferInputRate inputRate = VertexBufferInputRate::Vertex) {
            return SharedObject::create<DirectX12VertexBufferLayout>(vertexSize, binding, inputRate);
        }

        /// @brief Creates a new vertex buffer layout.
        ///
        /// @param vertexSize The overall size of a single vertex.
        /// @param attributes The vertex attributes.
        /// @param binding The binding point of the vertex buffers using this layout.
        /// @param inputRate The rate at which the vertex buffer is made available to the vertex shader.
        /// @return A shared pointer to the newly created vertex buffer layout.
        static inline auto create(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding = 0, VertexBufferInputRate inputRate = VertexBufferInputRate::Vertex) {
            return SharedObject::create<DirectX12VertexBufferLayout>(vertexSize, attributes, binding, inputRate);
        }

        /// @brief Creates a copy of a vertex buffer layout.
        ///
        /// @param other The vertex buffer layout to copy.
        /// @return A shared pointer to the newly created vertex buffer layout.
        static inline auto create(const DirectX12VertexBufferLayout& other) {
            return SharedObject::create<DirectX12VertexBufferLayout>(other);
        }

        // IVertexBufferLayout interface.
    public:
        /// @copydoc IVertexBufferLayout::attributes
        const Array<BufferAttribute>& attributes() const override;

        /// @copydoc IVertexBufferLayout::inputRate
        VertexBufferInputRate inputRate() const noexcept override;

        // IBufferLayout interface.
    public:
        /// @copydoc IBufferLayout::elementSize
        size_t elementSize() const noexcept override;

        /// @copydoc IBufferLayout::binding
        UInt32 binding() const noexcept override;

        /// @copydoc IBufferLayout::type
        BufferType type() const noexcept override;
    };

    /// @brief Implements a DirectX 12 index buffer layout.
    ///
    /// @see DirectX12IndexBuffer
    /// @see DirectX12VertexBufferLayout
    class LITEFX_DIRECTX12_API DirectX12IndexBufferLayout final : public IIndexBufferLayout {
        LITEFX_IMPLEMENTATION(DirectX12IndexBufferLayoutImpl);
        friend struct SharedObject::Allocator<DirectX12IndexBufferLayout>;

    private:
        /// @brief Initializes a new index buffer layout
        ///
        /// @param type The type of the indices within the index buffer.
        explicit DirectX12IndexBufferLayout(IndexType type);

    private:
        /// @copydoc IIndexBufferLayout::IIndexBufferLayout(IIndexBufferLayout&&)
        DirectX12IndexBufferLayout(DirectX12IndexBufferLayout&&) noexcept = delete;

        /// @copydoc IIndexBufferLayout::IIndexBufferLayout(const IIndexBufferLayout&)
        DirectX12IndexBufferLayout(const DirectX12IndexBufferLayout&);

        /// @copydoc IIndexBufferLayout::operator=(IIndexBufferLayout&&)
        DirectX12IndexBufferLayout& operator=(DirectX12IndexBufferLayout&&) noexcept = delete;

        /// @copydoc IIndexBufferLayout::operator=(const IIndexBufferLayout&)
        DirectX12IndexBufferLayout& operator=(const DirectX12IndexBufferLayout&) = delete;

    public:
        /// @copydoc IIndexBufferLayout::~IIndexBufferLayout
        ~DirectX12IndexBufferLayout() noexcept override;

    public:
        /// @brief Creates a new index buffer layout
        ///
        /// @param type The type of the indices within the index buffer.
        /// @return A shared pointer to the newly created index buffer layout instance.
        static inline auto create(IndexType type) {
            return SharedObject::create<DirectX12IndexBufferLayout>(type);
        }

        /// @brief Creates a copy of an index buffer layout.
        ///
        /// @param other The index buffer layout to copy.
        /// @return A shared pointer to the newly created index buffer layout instance.
        static inline auto create(const DirectX12IndexBufferLayout& other) {
            return SharedObject::create<DirectX12IndexBufferLayout>(other);
        }

        // IIndexBufferLayout interface.
    public:
        /// @copydoc IIndexBufferLayout::indexType
        IndexType indexType() const noexcept override;

        // IBufferLayout interface.
    public:
        /// @copydoc IBufferLayout::elementSize
        size_t elementSize() const noexcept override;

        /// @copydoc IBufferLayout::binding
        UInt32 binding() const noexcept override;

        /// @copydoc IBufferLayout::type
        BufferType type() const noexcept override;
    };

    /// @brief Represents the base interface for a DirectX 12 buffer implementation.
    ///
    /// @see DirectX12DescriptorSet
    /// @see IDirectX12Image
    /// @see IDirectX12VertexBuffer
    /// @see IDirectX12IndexBuffer
    class LITEFX_DIRECTX12_API IDirectX12Buffer : public virtual IBuffer, public virtual IResource<ComPtr<ID3D12Resource>> {
    protected:
        IDirectX12Buffer() noexcept = default;
        IDirectX12Buffer(IDirectX12Buffer&&) noexcept = default;
        IDirectX12Buffer(const IDirectX12Buffer&) = delete;
        IDirectX12Buffer& operator=(IDirectX12Buffer&&) noexcept = default;
        IDirectX12Buffer& operator=(const IDirectX12Buffer&) = delete;

    public:
        ~IDirectX12Buffer() noexcept override = default;
    };

    /// @brief Represents a DirectX 12 vertex buffer.
    ///
    /// @see DirectX12VertexBufferLayout
    /// @see IDirectX12Buffer
    class LITEFX_DIRECTX12_API IDirectX12VertexBuffer : public virtual VertexBuffer<DirectX12VertexBufferLayout>, public virtual IDirectX12Buffer {
    protected:
        IDirectX12VertexBuffer() noexcept = default;
        IDirectX12VertexBuffer(IDirectX12VertexBuffer&&) noexcept = default;
        IDirectX12VertexBuffer(const IDirectX12VertexBuffer&) = delete;
        IDirectX12VertexBuffer& operator=(IDirectX12VertexBuffer&&) noexcept = default;
        IDirectX12VertexBuffer& operator=(const IDirectX12VertexBuffer&) = delete;

    public:
        ~IDirectX12VertexBuffer() noexcept override = default;

    public:
        virtual const D3D12_VERTEX_BUFFER_VIEW& view() const noexcept = 0;
    };

    /// @brief Represents a DirectX 12 index buffer.
    ///
    /// @see DirectX12IndexBufferLayout
    /// @see IDirectX12Buffer
    class LITEFX_DIRECTX12_API IDirectX12IndexBuffer : public virtual IndexBuffer<DirectX12IndexBufferLayout>, public virtual IDirectX12Buffer {
    protected:
        IDirectX12IndexBuffer() noexcept = default;
        IDirectX12IndexBuffer(IDirectX12IndexBuffer&&) noexcept = default;
        IDirectX12IndexBuffer(const IDirectX12IndexBuffer&) = delete;
        IDirectX12IndexBuffer& operator=(IDirectX12IndexBuffer&&) noexcept = default;
        IDirectX12IndexBuffer& operator=(const IDirectX12IndexBuffer&) = delete;

    public:
        ~IDirectX12IndexBuffer() noexcept override = default;

    public:
        virtual const D3D12_INDEX_BUFFER_VIEW& view() const noexcept = 0;
    };

    /// @brief Represents a DirectX 12 sampled image or the base interface for a texture.
    ///
    /// @see DirectX12DescriptorLayout
    /// @see DirectX12DescriptorSet
    /// @see DirectX12DescriptorSetLayout
    /// @see IDirectX12Sampler
    class LITEFX_DIRECTX12_API IDirectX12Image : public virtual IImage, public virtual IResource<ComPtr<ID3D12Resource>> {
    protected:
        IDirectX12Image() noexcept = default;
        IDirectX12Image(IDirectX12Image&&) noexcept = default;
        IDirectX12Image(const IDirectX12Image&) = delete;
        IDirectX12Image& operator=(IDirectX12Image&&) noexcept = default;
        IDirectX12Image& operator=(const IDirectX12Image&) = delete;

    public:
        ~IDirectX12Image() noexcept override = default;
    };

    /// @brief Represents a DirectX 12 sampler.
    ///
    /// @see DirectX12DescriptorLayout
    /// @see DirectX12DescriptorSet
    /// @see DirectX12DescriptorSetLayout
    /// @see IDirectX12Image
    class LITEFX_DIRECTX12_API IDirectX12Sampler : public virtual ISampler {
    protected:
        IDirectX12Sampler() noexcept = default;
        IDirectX12Sampler(IDirectX12Sampler&&) noexcept = default;
        IDirectX12Sampler(const IDirectX12Sampler&) = delete;
        IDirectX12Sampler& operator=(IDirectX12Sampler&&) noexcept = default;
        IDirectX12Sampler& operator=(const IDirectX12Sampler&) = delete;

    public:
        ~IDirectX12Sampler() noexcept override = default;
    };

    /// @brief Represents the base interface for a DirectX 12 acceleration structure implementation.
    ///
    /// @see DirectX12DescriptorSet
    /// @see DirectX12BottomLevelAccelerationStructure
    /// @see DirectX12TopevelAccelerationStructure
    class LITEFX_DIRECTX12_API IDirectX12AccelerationStructure : public virtual IAccelerationStructure {
    protected:
        IDirectX12AccelerationStructure() noexcept = default;
        IDirectX12AccelerationStructure(IDirectX12AccelerationStructure&&) noexcept = default;
        IDirectX12AccelerationStructure(const IDirectX12AccelerationStructure&) = delete;
        IDirectX12AccelerationStructure& operator=(IDirectX12AccelerationStructure&&) noexcept = default;
        IDirectX12AccelerationStructure& operator=(const IDirectX12AccelerationStructure&) = delete;

    public:
        ~IDirectX12AccelerationStructure() noexcept override = default;
    };

    /// @brief Implements a DirectX 12 bottom-level acceleration structure (BLAS).
    ///
    /// @see DirectX12TopLevelAccelerationStructure
    class LITEFX_DIRECTX12_API DirectX12BottomLevelAccelerationStructure final : public IBottomLevelAccelerationStructure, public virtual StateResource, public virtual IDirectX12AccelerationStructure {
        LITEFX_IMPLEMENTATION(DirectX12BottomLevelAccelerationStructureImpl);
        friend class DirectX12Device;
        friend class DirectX12CommandBuffer;

        using IAccelerationStructure::build;
        using IAccelerationStructure::update;
        using IBottomLevelAccelerationStructure::copy;

    public:
        /// @brief Initializes a new DirectX 12 bottom-level acceleration structure (BLAS).
        ///
        /// @param flags The flags that define how the acceleration structure is built.
        /// @param name The name of the acceleration structure resource.
        /// @throws InvalidArgumentException Thrown if the provided @p flags contain an unsupported combination of flags.
        /// @see AccelerationStructureFlags
        explicit DirectX12BottomLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None, StringView name = "");
        
        /// @copydoc IBottomLevelAccelerationStructure::IBottomLevelAccelerationStructure(IBottomLevelAccelerationStructure&&)
        DirectX12BottomLevelAccelerationStructure(DirectX12BottomLevelAccelerationStructure&&) noexcept;

        /// @copydoc IBottomLevelAccelerationStructure::IBottomLevelAccelerationStructure(const IBottomLevelAccelerationStructure&)
        DirectX12BottomLevelAccelerationStructure(const DirectX12BottomLevelAccelerationStructure&) = delete;

        /// @copydoc IBottomLevelAccelerationStructure::operator=(IBottomLevelAccelerationStructure&&)
        DirectX12BottomLevelAccelerationStructure& operator=(DirectX12BottomLevelAccelerationStructure&&) noexcept;

        /// @copydoc IBottomLevelAccelerationStructure::operator=(const IBottomLevelAccelerationStructure&)
        DirectX12BottomLevelAccelerationStructure& operator=(const DirectX12BottomLevelAccelerationStructure&) = delete;

        /// @copydoc IBottomLevelAccelerationStructure::~IBottomLevelAccelerationStructure
        ~DirectX12BottomLevelAccelerationStructure() noexcept override;

        // IAccelerationStructure interface.
    public:
        /// @copydoc IAccelerationStructure::flags
        AccelerationStructureFlags flags() const noexcept override;

        /// @copydoc IAccelerationStructure::buffer
        SharedPtr<const IDirectX12Buffer> buffer() const noexcept;

        /// @copydoc IAccelerationStructure::build
        void build(const DirectX12CommandBuffer& commandBuffer, const SharedPtr<const IDirectX12Buffer>& scratchBuffer = nullptr, const SharedPtr<const IDirectX12Buffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// @copydoc IAccelerationStructure::update
        void update(const DirectX12CommandBuffer& commandBuffer, const SharedPtr<const IDirectX12Buffer>& scratchBuffer = nullptr, const SharedPtr<const IDirectX12Buffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// @copydoc IBottomLevelAccelerationStructure::copy
        void copy(const DirectX12CommandBuffer& commandBuffer, DirectX12BottomLevelAccelerationStructure& destination, bool compress = false, const SharedPtr<const IDirectX12Buffer>& buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const;

        /// @copydoc IAccelerationStructure::offset
        UInt64 offset() const noexcept override;

        /// @copydoc IAccelerationStructure::size
        UInt64 size() const noexcept override;

        // IBottomLevelAccelerationStructure interface.
    public:
        /// @copydoc IBottomLevelAccelerationStructure::triangleMeshes
        const Array<TriangleMesh>& triangleMeshes() const noexcept override;

        /// @copydoc IBottomLevelAccelerationStructure::addTriangleMesh(const TriangleMesh&)
        void addTriangleMesh(const TriangleMesh& mesh) override;

        /// @copydoc IBottomLevelAccelerationStructure::boundingBoxes
        const Array<BoundingBoxes>& boundingBoxes() const noexcept override;

        /// @copydoc IBottomLevelAccelerationStructure::addBoundingBox(const BoundingBoxes&)
        void addBoundingBox(const BoundingBoxes& aabb) override;
        
        /// @copydoc IBottomLevelAccelerationStructure::clear
        void clear() noexcept override;

        /// @copydoc IBottomLevelAccelerationStructure::remove(const TriangleMesh&)
        bool remove(const TriangleMesh& mesh) noexcept override;

        /// @copydoc IBottomLevelAccelerationStructure::remove(const BoundingBoxes&)
        bool remove(const BoundingBoxes& aabb) noexcept override;

    private:
        Array<D3D12_RAYTRACING_GEOMETRY_DESC> buildInfo() const;

    private:
        SharedPtr<const IBuffer> getBuffer() const noexcept override;
        void doBuild(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doUpdate(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doCopy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress, const SharedPtr<const IBuffer>& buffer, UInt64 offset, bool copyBuildInfo) const override;
    };

    /// @brief Implements a DirectX 12 top-level acceleration structure (TLAS).
    ///
    /// @see DirectX12BottomLevelAccelerationStructure
    class LITEFX_DIRECTX12_API DirectX12TopLevelAccelerationStructure final : public ITopLevelAccelerationStructure, public virtual StateResource, public virtual IDirectX12AccelerationStructure {
        LITEFX_IMPLEMENTATION(DirectX12TopLevelAccelerationStructureImpl);
        friend class DirectX12Device;
        friend class DirectX12CommandBuffer;

        using IAccelerationStructure::build;
        using IAccelerationStructure::update;
        using ITopLevelAccelerationStructure::copy;

    public:
        /// @brief Initializes a new DirectX 12 top-level acceleration structure (BLAS).
        ///
        /// @param flags The flags that define how the acceleration structure is built.
        /// @param name The name of the acceleration structure resource.
        /// @throws InvalidArgumentException Thrown if the provided @p flags contain an unsupported combination of flags.
        /// @see AccelerationStructureFlags
        explicit DirectX12TopLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None, StringView name = "");

        /// @copydoc ITopLevelAccelerationStructure::ITopLevelAccelerationStructure(ITopLevelAccelerationStructure&&)
        DirectX12TopLevelAccelerationStructure(DirectX12TopLevelAccelerationStructure&&) noexcept;

        /// @copydoc ITopLevelAccelerationStructure::ITopLevelAccelerationStructure(const ITopLevelAccelerationStructure&)
        DirectX12TopLevelAccelerationStructure(const DirectX12TopLevelAccelerationStructure&) = delete;

        /// @copydoc ITopLevelAccelerationStructure::operator=(ITopLevelAccelerationStructure&&)
        DirectX12TopLevelAccelerationStructure& operator=(DirectX12TopLevelAccelerationStructure&&) noexcept;

        /// @copydoc ITopLevelAccelerationStructure::operator=(const ITopLevelAccelerationStructure&)
        DirectX12TopLevelAccelerationStructure& operator=(const DirectX12TopLevelAccelerationStructure&) = delete;

        /// @copydoc ITopLevelAccelerationStructure::~ITopLevelAccelerationStructure
        ~DirectX12TopLevelAccelerationStructure() noexcept override;

        // IAccelerationStructure interface.
    public:
        /// @copydoc IAccelerationStructure::flags
        AccelerationStructureFlags flags() const noexcept override;

        /// @copydoc IAccelerationStructure::buffer
        SharedPtr<const IDirectX12Buffer> buffer() const noexcept;

        /// @copydoc IAccelerationStructure::build
        void build(const DirectX12CommandBuffer& commandBuffer, const SharedPtr<const IDirectX12Buffer>& scratchBuffer = nullptr, const SharedPtr<const IDirectX12Buffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// @copydoc IAccelerationStructure::update
        void update(const DirectX12CommandBuffer& commandBuffer, const SharedPtr<const IDirectX12Buffer>& scratchBuffer = nullptr, const SharedPtr<const IDirectX12Buffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// @copydoc ITopLevelAccelerationStructure::copy
        void copy(const DirectX12CommandBuffer& commandBuffer, DirectX12TopLevelAccelerationStructure& destination, bool compress = false, const SharedPtr<const IDirectX12Buffer>& buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const;

        /// @copydoc IAccelerationStructure::offset
        UInt64 offset() const noexcept override;

        /// @copydoc IAccelerationStructure::size
        UInt64 size() const noexcept override;

        // ITopLevelAccelerationStructure interface.
    public:
        /// @copydoc ITopLevelAccelerationStructure::instances
        const Array<Instance>& instances() const noexcept override;

        /// @copydoc ITopLevelAccelerationStructure::addInstance(const Instance&)
        void addInstance(const Instance& instance) override;

        /// @copydoc ITopLevelAccelerationStructure::clear
        void clear() noexcept override;

        /// @copydoc ITopLevelAccelerationStructure::remove
        bool remove(const Instance& instance) noexcept override;

    private:
        Array<D3D12_RAYTRACING_INSTANCE_DESC> buildInfo() const;

    private:
        SharedPtr<const IBuffer> getBuffer() const noexcept override;
        void doBuild(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doUpdate(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doCopy(const ICommandBuffer& commandBuffer, ITopLevelAccelerationStructure& destination, bool compress, const SharedPtr<const IBuffer>& buffer, UInt64 offset, bool copyBuildInfo) const override;
    };

    /// @brief Implements a DirectX 12 resource barrier.
    ///
    /// @see DirectX12CommandBuffer
    /// @see IDirectX12Buffer
    /// @see IDirectX12Image
    /// @see Barrier
    class LITEFX_DIRECTX12_API DirectX12Barrier final : public Barrier<IDirectX12Buffer, IDirectX12Image> {
        LITEFX_IMPLEMENTATION(DirectX12BarrierImpl);
        LITEFX_BUILDER(DirectX12BarrierBuilder);

    public:
        using base_type = Barrier<IDirectX12Buffer, IDirectX12Image>;
        using base_type::transition;

    public:
        /// @brief Initializes a new DirectX 12 barrier.
        ///
        /// @param syncBefore The pipeline stage(s) all previous commands have to finish before the barrier is executed.
        /// @param syncAfter The pipeline stage(s) all subsequent commands are blocked at until the barrier is executed.
        explicit DirectX12Barrier(PipelineStage syncBefore, PipelineStage syncAfter) noexcept;

        /// @copydoc Barrier::Barrier(Barrier&&)
        DirectX12Barrier(DirectX12Barrier&&) noexcept;

        /// @copydoc Barrier::Barrier(const Barrier&)
        DirectX12Barrier(const DirectX12Barrier&);

        /// @copydoc Barrier::operator=(Barrier&&)
        DirectX12Barrier& operator=(DirectX12Barrier&&) noexcept;

        /// @copydoc Barrier::operator=(const Barrier&)
        DirectX12Barrier& operator=(const DirectX12Barrier&);

        /// @copydoc Barrier::~Barrier
        ~DirectX12Barrier() noexcept override;

    private:
        explicit DirectX12Barrier() noexcept;
        PipelineStage& syncBefore() noexcept;
        PipelineStage& syncAfter() noexcept;

        // Barrier interface.
    public:
        /// @copydoc IBarrier::syncBefore
        PipelineStage syncBefore() const noexcept override;

        /// @copydoc IBarrier::syncAfter
        PipelineStage syncAfter() const noexcept override;

        /// @copydoc IBarrier::wait
        void wait(ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// @copydoc Barrier::transition(const buffer_type&, ResourceAccess, ResourceAccess)
        void transition(const IDirectX12Buffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// @copydoc Barrier::transition(const buffer_type&, UInt32, ResourceAccess, ResourceAccess)
        void transition(const IDirectX12Buffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// @copydoc Barrier::transition(const image_type&, ResourceAccess, ResourceAccess, ImageLayout)
        void transition(const IDirectX12Image& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override;

        /// @copydoc Barrier::transition(const image_type&, ResourceAccess, ResourceAccess, ImageLayout, ImageLayout)
        void transition(const IDirectX12Image& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override;

        /// @copydoc Barrier::transition(const image_type&, UInt32, UInt32, UInt32, UInt32, UInt32, ResourceAccess, ResourceAccess, ImageLayout)
        void transition(const IDirectX12Image& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override;

        /// @copydoc Barrier::transition(const image_type&, UInt32, UInt32, UInt32, UInt32, UInt32, ResourceAccess, ResourceAccess, ImageLayout, ImageLayout)
        void transition(const IDirectX12Image& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override;

    public:
        /// @brief Adds the barrier to a command buffer and updates the resource target states.
        ///
        /// @param commandBuffer The command buffer to add the barriers to.
        /// @throws RuntimeException Thrown, if any of the contained barriers is a image barrier that targets a sub-resource range that does not share the same @ref ImageLayout in all sub-resources.
        void execute(const DirectX12CommandBuffer& commandBuffer) const;
    };

    /// @brief Implements a DirectX 12 @ref IShaderModule.
    ///
    /// @see DirectX12ShaderProgram
    /// @see https://github.com/crud89/LiteFX/wiki/Shader-Development
    class LITEFX_DIRECTX12_API DirectX12ShaderModule final : public IShaderModule, public ComResource<IDxcBlob> {
        LITEFX_IMPLEMENTATION(DirectX12ShaderModuleImpl);

    public:
        /// @brief Initializes a new DirectX 12 shader module.
        ///
        /// @param device The parent device, this shader module has been created from.
        /// @param type The shader stage, this module is used in.
        /// @param fileName The file name of the module source.
        /// @param entryPoint The name of the module entry point.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data for ray-tracing shaders.
        explicit DirectX12ShaderModule(const DirectX12Device& device, ShaderStage type, const String& fileName, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt);

        /// @brief Initializes a new DirectX 12 shader module.
        ///
        /// @param device The parent device, this shader module has been created from.
        /// @param type The shader stage, this module is used in.
        /// @param stream The file stream to read the shader module from.
        /// @param name The file name of the module source.
        /// @param entryPoint The name of the module entry point.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data for ray-tracing shaders.
        explicit DirectX12ShaderModule(const DirectX12Device& device, ShaderStage type, std::istream& stream, const String& name, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt);

        /// @copydoc IShaderModule::IShaderModule(IShaderModule&&)
        DirectX12ShaderModule(DirectX12ShaderModule&&) noexcept;

        /// @copydoc IShaderModule::IShaderModule(const IShaderModule&)
        DirectX12ShaderModule(const DirectX12ShaderModule&) = delete;

        /// @copydoc IShaderModule::operator=(IShaderModule&&)
        DirectX12ShaderModule& operator=(DirectX12ShaderModule&&) noexcept;

        /// @copydoc IShaderModule::operator=(const IShaderModule&)
        DirectX12ShaderModule& operator=(const DirectX12ShaderModule&) = delete;

        /// @copydoc IShaderModule::~IShaderModule
        ~DirectX12ShaderModule() noexcept override;

        // IShaderModule interface.
    public:
        /// @copydoc IShaderModule::fileName
        const String& fileName() const noexcept override;

        /// @copydoc IShaderModule::entryPoint
        const String& entryPoint() const noexcept override;

        /// @copydoc IShaderModule::type
        ShaderStage type() const noexcept override;

        /// @copydoc IShaderModule::shaderLocalDescriptor
        const Optional<DescriptorBindingPoint>& shaderLocalDescriptor() const noexcept override;
    };

    /// @brief Implements a DirectX 12 @ref ShaderProgram.
    ///
    /// @see DirectX12ShaderProgramBuilder
    /// @see https://github.com/crud89/LiteFX/wiki/Shader-Development
    class LITEFX_DIRECTX12_API DirectX12ShaderProgram final : public ShaderProgram<DirectX12ShaderModule> {
        LITEFX_IMPLEMENTATION(DirectX12ShaderProgramImpl);
        LITEFX_BUILDER(DirectX12ShaderProgramBuilder);
        friend struct SharedObject::Allocator<DirectX12ShaderProgram>;

    private:
        /// @brief Initializes a new DirectX 12 shader program.
        ///
        /// @param device The parent device of the shader program.
        /// @param modules The shader modules used by the shader program.
        explicit DirectX12ShaderProgram(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12ShaderModule>>&& modules);

        /// @brief Initializes a new DirectX 12 shader program.
        ///
        /// @param device The parent device of the shader program.
        explicit DirectX12ShaderProgram(const DirectX12Device& device) noexcept;

    private:
        /// @copydoc ShaderProgram::ShaderProgram(ShaderProgram&&)
        DirectX12ShaderProgram(DirectX12ShaderProgram&&) noexcept = delete;

        /// @copydoc ShaderProgram::ShaderProgram(const ShaderProgram&)
        DirectX12ShaderProgram(const DirectX12ShaderProgram&) = delete;

        /// @copydoc ShaderProgram::operator=(ShaderProgram&&)
        DirectX12ShaderProgram& operator=(DirectX12ShaderProgram&&) noexcept = delete;

        /// @copydoc ShaderProgram::operator=(const ShaderProgram&)
        DirectX12ShaderProgram& operator=(const DirectX12ShaderProgram&) = delete;

    public:
        /// @copydoc ShaderProgram::~ShaderProgram
        ~DirectX12ShaderProgram() noexcept override;

        // Factory method.
    public:
        /// @brief Creates a new shader program instance.
        ///
        /// @param device The device this shader program should be compiled for and executed on.
        /// @param modules The modules of the shader program.
        /// @return A pointer to the shader program.
        static inline auto create(const DirectX12Device& device, Enumerable<UniquePtr<DirectX12ShaderModule>>&& modules) -> SharedPtr<DirectX12ShaderProgram> {
            return SharedObject::create<DirectX12ShaderProgram>(device, std::move(modules));
        }

    private:
        /// @brief Creates a new shader program instance.
        ///
        /// @return A pointer to the shader program.
        static inline auto create(const DirectX12Device& device) {
            return SharedObject::create<DirectX12ShaderProgram>(device);
        }

    public:
        /// @copydoc ShaderProgram::modules
        const Array<UniquePtr<const DirectX12ShaderModule>>& modules() const noexcept override;

        /// @copydoc IShaderProgram::reflectPipelineLayout
        SharedPtr<DirectX12PipelineLayout> reflectPipelineLayout(Enumerable<PipelineBindingHint> hints = {}) const;

    private:
        SharedPtr<IPipelineLayout> parsePipelineLayout(Enumerable<PipelineBindingHint> hints) const override {
            return std::static_pointer_cast<IPipelineLayout>(this->reflectPipelineLayout(hints));
        }
    };

    /// @brief Implements a DirectX 12 @ref DescriptorSet.
    ///
    /// @see DirectX12DescriptorSetLayout
    class LITEFX_DIRECTX12_API DirectX12DescriptorSet final : public DescriptorSet<IDirectX12Buffer, IDirectX12Image, IDirectX12Sampler, IDirectX12AccelerationStructure> {
        LITEFX_IMPLEMENTATION(DirectX12DescriptorSetImpl);

    public:
        using base_type = DescriptorSet<IDirectX12Buffer, IDirectX12Image, IDirectX12Sampler, IDirectX12AccelerationStructure>;
        using base_type::update;

    public:
        /// @brief Initializes a new descriptor set.
        ///
        /// @param layout The parent descriptor set layout.
        /// @param resourceHeap A CPU-visible descriptor heap that contains the descriptors for the resources of the descriptor set.
        /// @param samplerHeap A CPU-visible descriptor heap that contains the descriptors for the samplers of the descriptor set.
        explicit DirectX12DescriptorSet(const DirectX12DescriptorSetLayout& layout, ComPtr<ID3D12DescriptorHeap>&& resourceHeap = nullptr, ComPtr<ID3D12DescriptorHeap>&& samplerHeap = nullptr);

        /// @copydoc DescriptorSet::DescriptorSet(DescriptorSet&&)
        DirectX12DescriptorSet(DirectX12DescriptorSet&&) noexcept = delete;
        
        /// @copydoc DescriptorSet::DescriptorSet(const DescriptorSet&)
        DirectX12DescriptorSet(const DirectX12DescriptorSet&) = delete;
        
        /// @copydoc DescriptorSet::operator=(DescriptorSet&&)
        DirectX12DescriptorSet& operator=(DirectX12DescriptorSet&&) noexcept = delete;
        
        /// @copydoc DescriptorSet::operator=(const DescriptorSet&)
        DirectX12DescriptorSet& operator=(const DirectX12DescriptorSet&) = delete;
        
        /// @copydoc DescriptorSet::~DescriptorSet
        ~DirectX12DescriptorSet() noexcept override;

    public:
        /// @brief Returns the parent descriptor set layout.
        ///
        /// @return The parent descriptor set layout.
        const DirectX12DescriptorSetLayout& layout() const noexcept;

    public:
        /// @copydoc IDescriptorSet::globalHeapAllocation
        VirtualAllocator::Allocation globalHeapAllocation(DescriptorHeapType heapType) const noexcept override;

        /// @copydoc DescriptorSet::bindToHeap(DescriptorType, UInt32, const buffer_type&, UInt32, UInt32, Format) const
        UInt32 bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IDirectX12Buffer& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, Format texelFormat = Format::None) const override;

        /// @copydoc DescriptorSet::bindToHeap(DescriptorType, UInt32, const image_type&, UInt32, UInt32, UInt32, UInt32) const
        UInt32 bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IDirectX12Image& image, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const override;

        /// @copydoc DescriptorSet::bindToHeap(UInt32, const sampler_type&) const
        UInt32 bindToHeap(UInt32 descriptor, const IDirectX12Sampler& sampler) const override;

        /// @copydoc DescriptorSet::update(UInt32, const buffer_type&, UInt32, UInt32, UInt32, Format) const
        void update(UInt32 binding, const IDirectX12Buffer& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, UInt32 firstDescriptor = 0, Format texelFormat = Format::None) const override;

        /// @copydoc DescriptorSet::update(UInt32, const image_type&, UInt32, UInt32, UInt32, UInt32, UInt32) const
        void update(UInt32 binding, const IDirectX12Image& texture, UInt32 descriptor = 0, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const override;

        /// @copydoc DescriptorSet::update(UInt32, const sampler_type&, UInt32) const
        void update(UInt32 binding, const IDirectX12Sampler& sampler, UInt32 descriptor = 0) const override;

        /// @copydoc DescriptorSet::update(UInt32, const acceleration_structure_type&, UInt32) const
        void update(UInt32 binding, const IDirectX12AccelerationStructure& accelerationStructure, UInt32 descriptor = 0) const override;

    public:
        /// @brief Returns the local (CPU-visible) heap that contains the set's descriptors.
        ///
        /// @param heapType The type of the descriptor heap to obtain.
        /// @return The local (CPU-visible) heap that contains the set's descriptors.
        const ComPtr<ID3D12DescriptorHeap> localHeap(DescriptorHeapType heapType) const noexcept;
    };

    /// @brief Implements a DirectX 12 @ref IDescriptorLayout
    ///
    /// @see IDirectX12Buffer
    /// @see IDirectX12Image
    /// @see IDirectX12Sampler
    /// @see DirectX12DescriptorSet
    /// @see DirectX12DescriptorSetLayout
    class LITEFX_DIRECTX12_API DirectX12DescriptorLayout final : public IDescriptorLayout {
        LITEFX_IMPLEMENTATION(DirectX12DescriptorLayoutImpl);

    public:
        /// @brief Initializes a new DirectX 12 descriptor layout.
        ///
        /// @param type The type of the descriptor.
        /// @param binding The binding point for the descriptor.
        /// @param elementSize The size of the descriptor.
        /// @param descriptors The number of descriptors in the descriptor array. If @p unbounded is set, this value sets the upper limit for the array size.
        /// @param unbounded If set to `true`, the descriptor will be defined as a runtime-allocated, unbounded array.
        /// @param local Determines if the descriptor is part of the local or global root signature for ray-tracing shaders.
        DirectX12DescriptorLayout(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors = 1, bool unbounded = false, bool local = false);

        /// @brief Initializes a new DirectX 12 descriptor layout for a static sampler.
        ///
        /// @param staticSampler The static sampler to initialize the state with.
        /// @param binding The binding point for the descriptor.
        /// @param local Determines if the descriptor is part of the local or global root signature for ray-tracing shaders.
        DirectX12DescriptorLayout(const IDirectX12Sampler& staticSampler, UInt32 binding, bool local = false);
        
        /// @copydoc IDescriptorLayout::IDescriptorLayout(IDescriptorLayout&&)
        DirectX12DescriptorLayout(DirectX12DescriptorLayout&&) noexcept;

        /// @copydoc IDescriptorLayout::IDescriptorLayout(const IDescriptorLayout&)
        DirectX12DescriptorLayout(const DirectX12DescriptorLayout&);

        /// @copydoc IDescriptorLayout::operator=(IDescriptorLayout&&)
        DirectX12DescriptorLayout& operator=(DirectX12DescriptorLayout&&) noexcept;

        /// @copydoc IDescriptorLayout::operator=(const IDescriptorLayout&)
        DirectX12DescriptorLayout& operator=(const DirectX12DescriptorLayout&);

        /// @copydoc IDescriptorLayout::~IDescriptorLayout
        ~DirectX12DescriptorLayout() noexcept override;

        // DirectX 12 descriptor layout.
    public:
        /// @brief Returns `true`, if the descriptor belongs to the local root signature of a ray-tracing pipeline or `false` otherwise.
        ///
        /// Note that this value must not be set to `true` for descriptors that are bound outside of ray-tracing shaders.
        ///
        /// @return `true`, if the descriptor belongs to the local root signature of a ray-tracing pipeline or `false` otherwise.
        bool local() const noexcept;

        // IDescriptorLayout interface.
    public:
        /// @copydoc IDescriptorLayout::descriptorType
        DescriptorType descriptorType() const noexcept override;

        /// @copydoc IDescriptorLayout::descriptors
        UInt32 descriptors() const noexcept override;

        /// @copydoc IDescriptorLayout::unbounded
        bool unbounded() const noexcept override;

        /// @copydoc IDescriptorLayout::staticSampler
        const IDirectX12Sampler* staticSampler() const noexcept override;

        // IBufferLayout interface.
    public:
        /// @copydoc IBufferLayout::elementSize
        size_t elementSize() const noexcept override;

        /// @copydoc IBufferLayout::binding
        UInt32 binding() const noexcept override;

        /// @copydoc IBufferLayout::type
        BufferType type() const noexcept override;
    };

    /// @brief Implements a DirectX 12 @ref DescriptorSetLayout.
    ///
    /// @see DirectX12DescriptorSet
    /// @see DirectX12PipelineDescriptorSetLayoutBuilder
    class LITEFX_DIRECTX12_API DirectX12DescriptorSetLayout final : public DescriptorSetLayout<DirectX12DescriptorLayout, DirectX12DescriptorSet> {
        LITEFX_IMPLEMENTATION(DirectX12DescriptorSetLayoutImpl);
        LITEFX_BUILDER(DirectX12DescriptorSetLayoutBuilder);
        friend class DirectX12PipelineLayout;
        friend struct SharedObject::Allocator<DirectX12DescriptorSetLayout>;

    public:
        using base_type = DescriptorSetLayout<DirectX12DescriptorLayout, DirectX12DescriptorSet>;
        using base_type::free;
        using base_type::allocate;

    private:
        /// @brief Initializes a DirectX 12 descriptor set layout.
        ///
        /// @param device The device, the descriptor set layout is created on.
        /// @param descriptorLayouts The descriptor layouts of the descriptors within the descriptor set.
        /// @param space The space or set id of the descriptor set.
        /// @param stages The shader stages, the descriptor sets are bound to.
        explicit DirectX12DescriptorSetLayout(const DirectX12Device& device, const Enumerable<DirectX12DescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages);

        /// @brief Initializes a DirectX 12 descriptor set layout.
        ///
        /// @param device The device, the descriptor set layout is created on.
        explicit DirectX12DescriptorSetLayout(const DirectX12Device& device);

    private:
        /// @copydoc DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&&)
        DirectX12DescriptorSetLayout(DirectX12DescriptorSetLayout&&) noexcept = delete;

        /// @copydoc DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayout&)
        DirectX12DescriptorSetLayout(const DirectX12DescriptorSetLayout& other);

        /// @copydoc DescriptorSetLayout::operator=(DescriptorSetLayout&&)
        DirectX12DescriptorSetLayout& operator=(DirectX12DescriptorSetLayout&&) noexcept = delete;

        /// @copydoc DescriptorSetLayout::operator=(const DescriptorSetLayout&)
        DirectX12DescriptorSetLayout& operator=(const DirectX12DescriptorSetLayout&) = delete;

    public:
        /// @copydoc DescriptorSetLayout::~DescriptorSetLayout
        ~DirectX12DescriptorSetLayout() noexcept override;

    public:
        /// @brief Creates a DirectX 12 descriptor set layout.
        ///
        /// @param device The device, the descriptor set layout is created on.
        /// @param descriptorLayouts The descriptor layouts of the descriptors within the descriptor set.
        /// @param space The space or set id of the descriptor set.
        /// @param stages The shader stages, the descriptor sets are bound to.
        /// @return Returns a shared pointer to the newly created descriptor set layout.
        static inline auto create(const DirectX12Device& device, const Enumerable<DirectX12DescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages) {
            return SharedObject::create<DirectX12DescriptorSetLayout>(device, descriptorLayouts, space, stages);
        }

        /// @brief Creates a copy of a DirectX 12 descriptor set layout.
        ///
        /// @param other The descriptor set layout to copy.
        /// @return Returns a shared pointer to the newly created descriptor set layout.
        static inline auto create(const DirectX12DescriptorSetLayout& other) {
            return SharedObject::create<DirectX12DescriptorSetLayout>(other);
        }

    private:
        /// @brief Creates a DirectX 12 descriptor set layout.
        ///
        /// @param device The device, the descriptor set layout is created on.
        /// @return Returns a shared pointer to the newly created descriptor set layout.
        static inline auto create(const DirectX12Device& device) {
            return SharedObject::create<DirectX12DescriptorSetLayout>(device);
        }

    public:
        /// @brief Returns the parent device or `nullptr`, if it has been released.
        ///
        /// @return A pointer to the parent device or `nullptr`, if it has been released.
        SharedPtr<const DirectX12Device> device() const noexcept;

    public:
        /// @copydoc DescriptorSetLayout::descriptors
        const Array<DirectX12DescriptorLayout>& descriptors() const noexcept override;

        /// @copydoc DescriptorSetLayout::descriptor
        const DirectX12DescriptorLayout& descriptor(UInt32 binding) const override;

        /// @copydoc IDescriptorSetLayout::space
        UInt32 space() const noexcept override;

        /// @copydoc IDescriptorSetLayout::shaderStages
        ShaderStage shaderStages() const noexcept override;

        /// @copydoc IDescriptorSetLayout::uniforms
        UInt32 uniforms() const noexcept override;

        /// @copydoc IDescriptorSetLayout::storages
        UInt32 storages() const noexcept override;

        /// @copydoc IDescriptorSetLayout::images
        UInt32 images() const noexcept override;

        /// @copydoc IDescriptorSetLayout::buffers
        UInt32 buffers() const noexcept override;

        /// @copydoc IDescriptorSetLayout::samplers
        UInt32 samplers() const noexcept override;

        /// @copydoc IDescriptorSetLayout::staticSamplers
        UInt32 staticSamplers() const noexcept override;

        /// @copydoc IDescriptorSetLayout::inputAttachments
        UInt32 inputAttachments() const noexcept override;

        /// @copydoc IDescriptorSetLayout::containsUnboundedArray
        bool containsUnboundedArray() const noexcept override;

        /// @copydoc IDescriptorSetLayout::getDescriptorOffset
        UInt32 getDescriptorOffset(UInt32 binding, UInt32 element = 0) const override;

        /// @copydoc IDescriptorSetLayout::bindsResources
        bool bindsResources() const noexcept override;

        /// @copydoc IDescriptorSetLayout::bindsSamplers
        bool bindsSamplers() const noexcept override;

    public:
        /// @copydoc DescriptorSetLayout::allocate(UInt32, std::initializer_list<DescriptorBinding>) const
        UniquePtr<DirectX12DescriptorSet> allocate(UInt32 descriptors, std::initializer_list<DescriptorBinding> bindings) const override;

        /// @copydoc DescriptorSetLayout::allocate(UInt32, Span<DescriptorBinding>) const
        UniquePtr<DirectX12DescriptorSet> allocate(UInt32 descriptors, Span<DescriptorBinding> bindings) const override;

        /// @copydoc DescriptorSetLayout::allocate(UInt32, Generator<DescriptorBinding>) const
        UniquePtr<DirectX12DescriptorSet> allocate(UInt32 descriptors, Generator<DescriptorBinding> bindings) const override;

        /// @copydoc DescriptorSetLayout::allocate(UInt32, UInt32, std::initializer_list<std::initializer_list<DescriptorBinding>>) const
        Generator<UniquePtr<DirectX12DescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::initializer_list<std::initializer_list<DescriptorBinding>> bindings = { }) const override;

#ifdef __cpp_lib_mdspan
        /// @copydoc DescriptorSetLayout::allocate(UInt32, UInt32, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>>) const
        Generator<UniquePtr<DirectX12DescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const override;
#endif

        /// @copydoc DescriptorSetLayout::allocate(UInt32, UInt32, std::function<Generator<DescriptorBinding>(UInt32)>) const
        Generator<UniquePtr<DirectX12DescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const override;

        /// @copydoc DescriptorSetLayout::free
        void free(const DirectX12DescriptorSet& descriptorSet) const override;
    };

    /// @brief Implements the DirectX 12 @ref IPushConstantsRange.
    ///
    /// @see DirectX12PushConstantsLayout
    class LITEFX_DIRECTX12_API DirectX12PushConstantsRange final : public IPushConstantsRange {
        LITEFX_IMPLEMENTATION(DirectX12PushConstantsRangeImpl);
        friend class DirectX12PipelineLayout;

    public:
        /// @brief Initializes a new push constants range.
        ///
        /// @param shaderStages The shader stages, that access the push constants from the range.
        /// @param offset The offset relative to the parent push constants backing memory that marks the beginning of the range.
        /// @param size The size of the push constants range.
        /// @param space The space from which the push constants of the range will be accessible in the shader.
        /// @param binding The register from which the push constants of the range will be accessible in the shader.
        explicit DirectX12PushConstantsRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding);

        /// @copydoc IPushConstantsRange::IPushConstantsRange(IPushConstantsRange&&)
        DirectX12PushConstantsRange(DirectX12PushConstantsRange&&) noexcept;

        /// @copydoc IPushConstantsRange::IPushConstantsRange(const IPushConstantsRange&)
        DirectX12PushConstantsRange(const DirectX12PushConstantsRange&);

        /// @copydoc IPushConstantsRange::operator=(IPushConstantsRange&&)
        DirectX12PushConstantsRange& operator=(DirectX12PushConstantsRange&&) noexcept;

        /// @copydoc IPushConstantsRange::operator=(const IPushConstantsRange&)
        DirectX12PushConstantsRange& operator=(const DirectX12PushConstantsRange&);

        /// @copydoc IPushConstantsRange::~IPushConstantsRange
        ~DirectX12PushConstantsRange() noexcept override;

    public:
        /// @copydoc IPushConstantsRange::space
        UInt32 space() const noexcept override;

        /// @copydoc IPushConstantsRange::binding
        UInt32 binding() const noexcept override;

        /// @copydoc IPushConstantsRange::offset
        UInt32 offset() const noexcept override;

        /// @copydoc IPushConstantsRange::size
        UInt32 size() const noexcept override;

        /// @copydoc IPushConstantsRange::stageMask
        ShaderStage stageMask() const noexcept override;
    };

    /// @brief Implements the DirectX 12 @ref PushConstantsLayout.
    ///
    /// In DirectX 12, push constants map to root constants. Those are 32 bit values that are directly stored on the root signature. Thus, push constants can bloat your root signature, since all the required
    /// memory is directly reserved on it. The way they are implemented is, that each range gets directly written in 4 byte chunks into the command buffer. Thus, overlapping is not directly supported (as
    /// opposed to Vulkan). If you have overlapping push constants ranges, the overlap will be duplicated in the root signature.
    ///
    /// @see DirectX12PushConstantsRange
    /// @see DirectX12PipelinePushConstantsLayoutBuilder
    class LITEFX_DIRECTX12_API DirectX12PushConstantsLayout final : public PushConstantsLayout<DirectX12PushConstantsRange> {
        LITEFX_IMPLEMENTATION(DirectX12PushConstantsLayoutImpl);
        LITEFX_BUILDER(DirectX12PushConstantsLayoutBuilder);
        friend class DirectX12PipelineLayout;

    public:
        /// @brief Initializes a new push constants layout.
        ///
        /// @param ranges The ranges contained by the layout.
        /// @param size The overall size (in bytes) of the push constants backing memory.
        explicit DirectX12PushConstantsLayout(Enumerable<UniquePtr<DirectX12PushConstantsRange>>&& ranges, UInt32 size);

        /// @copydoc PushConstantsLayout::PushConstantsLayout(PushConstantsLayout&&)
        DirectX12PushConstantsLayout(DirectX12PushConstantsLayout&&) noexcept;

        /// @copydoc PushConstantsLayout::PushConstantsLayout(const PushConstantsLayout&)
        DirectX12PushConstantsLayout(const DirectX12PushConstantsLayout&) = delete;

        /// @copydoc PushConstantsLayout::operator=(PushConstantsLayout&&)
        DirectX12PushConstantsLayout& operator=(DirectX12PushConstantsLayout&&) noexcept;

        /// @copydoc PushConstantsLayout::operator=(const PushConstantsLayout&)
        DirectX12PushConstantsLayout& operator=(const DirectX12PushConstantsLayout&) = delete;

        /// @copydoc PushConstantsLayout::~PushConstantsLayout
        ~DirectX12PushConstantsLayout() noexcept override;

    private:
        /// @brief Initializes a new push constants layout.
        ///
        /// @param size The overall size (in bytes) of the push constants backing memory.
        explicit DirectX12PushConstantsLayout(UInt32 size);

    public:
        /// @copydoc IPushConstantsLayout::size
        UInt32 size() const noexcept override;

        /// @copydoc PushConstantsLayout::ranges
        const Array<UniquePtr<DirectX12PushConstantsRange>>& ranges() const override;

    protected:
        /// @brief Returns an array of pointers to the push constant ranges of the layout.
        ///
        /// @return An array of pointers to the push constant ranges of the layout.
        Array<UniquePtr<DirectX12PushConstantsRange>>& ranges();
    };

    /// @brief Implements a DirectX 12 @ref PipelineLayout.
    ///
    /// @see DirectX12PipelineLayoutBuilder
    class LITEFX_DIRECTX12_API DirectX12PipelineLayout final : public PipelineLayout<DirectX12DescriptorSetLayout, DirectX12PushConstantsLayout>, public ComResource<ID3D12RootSignature> {
        LITEFX_IMPLEMENTATION(DirectX12PipelineLayoutImpl);
        LITEFX_BUILDER(DirectX12PipelineLayoutBuilder);
        friend struct SharedObject::Allocator<DirectX12PipelineLayout>;

    private:
        /// @brief Initializes a new DirectX 12 render pipeline layout.
        ///
        /// @param device The parent device, the layout is created from.
        /// @param descriptorSetLayouts The descriptor set layouts used by the pipeline.
        /// @param pushConstantsLayout The push constants layout used by the pipeline.
        explicit DirectX12PipelineLayout(const DirectX12Device& device, const Enumerable<SharedPtr<DirectX12DescriptorSetLayout>>& descriptorSetLayouts, UniquePtr<DirectX12PushConstantsLayout>&& pushConstantsLayout);

        /// @brief Initializes a new DirectX 12 render pipeline layout.
        ///
        /// @param device The parent device, the layout is created from.
        explicit DirectX12PipelineLayout(const DirectX12Device& device);

    private:
        /// @copydoc PipelineLayout::PipelineLayout(PipelineLayout&&)
        DirectX12PipelineLayout(DirectX12PipelineLayout&&) noexcept = delete;

        /// @copydoc PipelineLayout::PipelineLayout(const PipelineLayout&)
        DirectX12PipelineLayout(const DirectX12PipelineLayout&) = delete;

        /// @copydoc PipelineLayout::operator=(PipelineLayout&&)
        DirectX12PipelineLayout& operator=(DirectX12PipelineLayout&&) noexcept = delete;

        /// @copydoc PipelineLayout::operator=(const PipelineLayout&)
        DirectX12PipelineLayout& operator=(const DirectX12PipelineLayout&) = delete;

    public:
        /// @copydoc PipelineLayout::~PipelineLayout
        ~DirectX12PipelineLayout() noexcept override;

    public:
        /// @brief Creates a new DirectX 12 render pipeline layout.
        ///
        /// @param device The parent device, the layout is created from.
        /// @param descriptorSetLayouts The descriptor set layouts used by the pipeline.
        /// @param pushConstantsLayout The push constants layout used by the pipeline.
        /// @return A shared pointer to the newly created pipeline layout instance.
        static inline auto create(const DirectX12Device& device, const Enumerable<SharedPtr<DirectX12DescriptorSetLayout>>& descriptorSetLayouts, UniquePtr<DirectX12PushConstantsLayout>&& pushConstantsLayout) {
            return SharedObject::create<DirectX12PipelineLayout>(device, descriptorSetLayouts, std::move(pushConstantsLayout));
        }

    private:
        /// @brief Creates a new DirectX 12 render pipeline layout.
        ///
        /// @param device The parent device, the layout is created from.
        /// @return A shared pointer to the newly created pipeline layout instance.
        static inline auto create(const DirectX12Device& device) {
            return SharedObject::create<DirectX12PipelineLayout>(device);
        }

        // PipelineLayout interface.
    public:
        /// @brief Returns a reference to the parent device.
        ///
        /// @return A reference to the parent device.
        const DirectX12Device& device() const noexcept /*override*/;

        /// @copydoc PipelineLayout::descriptorSet
        const DirectX12DescriptorSetLayout& descriptorSet(UInt32 space) const override;

        /// @copydoc PipelineLayout::descriptorSets
        const Array<SharedPtr<const DirectX12DescriptorSetLayout>>& descriptorSets() const override;

        /// @copydoc PipelineLayout::pushConstants
        const DirectX12PushConstantsLayout* pushConstants() const noexcept override;

        /// @copydoc IPipelineLayout::dynamicResourceHeapAccess
        bool dynamicResourceHeapAccess() const override;

        /// @copydoc IPipelineLayout::dynamicSamplerHeapAccess
        bool dynamicSamplerHeapAccess() const override;

    public:
        /// @brief Returns the root parameter index for a descriptor set.
        ///
        /// Note that root parameter mapping is done by matching the descriptor set space. For example, providing any descriptor set layout for register space `1` will return the same root parameter index, even
        /// if a different descriptor set layout was provided for space `1` during pipeline layout construction. This allows for descriptor sets and layouts to be shared over multiple pipeline layouts, even if
        /// they are unrelated, as long as they are compatible. Compatibility must be ensured by the application.
        ///
        /// Only if no descriptor set layout was provided for register space `1` in the example above, this method will return `std::nullopt`.
        ///
        /// The @p heapType determines the target descriptor heap, for which to obtain the root parameter index. This is required, as samplers bind to another descriptor heap as all other resources. If the
        /// descriptor set does not contain descriptors that bind to the desired descriptor heap, the method will also return `std::nullopt`.
        ///
        /// @param layout The layout of the descriptor set.
        /// @param heapType The type of the descriptor heap for which to obtain the root parameter index.
        /// @return The root parameter index for the descriptor set layout, or `std::nullopt`, if the descriptor set is not part of the pipeline layout or does not contain descriptors that bind to @p heapType.
        Optional<UInt32> rootParameterIndex(const DirectX12DescriptorSetLayout& layout, DescriptorHeapType heapType) const noexcept;

        /// @brief Returns the root parameter index for a push constants range
        ///
        /// Note that root parameter mapping is done by matching the range space and register. The restrictions and implications are similar to what's described for <see cref="rootParameterIndex(const
        /// DirectX12DescriptorSetLayout&) />. Compatibility must be ensured by the application.
        ///
        /// @param range The push constants range.
        /// @return The root parameter index for the push constants range, or `std::nullopt`, if the push constants range is not part of the pipeline layout.
        Optional<UInt32> rootParameterIndex(const DirectX12PushConstantsRange& range) const noexcept;
    };

    /// @brief Implements the DirectX 12 input assembler state.
    ///
    /// @see DirectX12InputAssemblerBuilder
    class LITEFX_DIRECTX12_API DirectX12InputAssembler final : public InputAssembler<DirectX12VertexBufferLayout, DirectX12IndexBufferLayout> {
        LITEFX_IMPLEMENTATION(DirectX12InputAssemblerImpl);
        LITEFX_BUILDER(DirectX12InputAssemblerBuilder);
        friend struct SharedObject::Allocator<DirectX12InputAssembler>;

	private:
        /// @brief Initializes a new DirectX 12 input assembler state.
        ///
        /// @param vertexBufferLayouts The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.
        /// @param controlPoints The number of control points in a patch primitive. Ignored if @p primitiveTopology is not `PrimitiveTopology::PatchList`. Must be a value between 1 and 32.
        explicit DirectX12InputAssembler(PrimitiveTopology primitiveTopology, UInt32 controlPoints = 1);

        /// @brief Initializes a new DirectX 12 input assembler state.
        ///
        /// @param vertexBufferLayouts The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.
        /// @param indexBufferLayout The index buffer layout.
        /// @param primitiveTopology The primitive topology.
        /// @param controlPoints The number of control points in a patch primitive. Ignored if @p primitiveTopology is not `PrimitiveTopology::PatchList`. Must be a value between 1 and 32.
        explicit DirectX12InputAssembler(Enumerable<SharedPtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, SharedPtr<DirectX12IndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList, UInt32 controlPoints = 1);

        /// @brief Initializes a new DirectX 12 input assembler state.
        explicit DirectX12InputAssembler();

    private:
        /// @copydoc InputAssembler::InputAssembler(InputAssembler&&)
        DirectX12InputAssembler(DirectX12InputAssembler&&) noexcept;

        /// @copydoc InputAssembler::InputAssembler(const InputAssembler&)
        DirectX12InputAssembler(const DirectX12InputAssembler&);

        /// @copydoc InputAssembler::operator=(InputAssembler&&)
        DirectX12InputAssembler& operator=(DirectX12InputAssembler&&) noexcept;

        /// @copydoc InputAssembler::operator=(const InputAssembler&)
        DirectX12InputAssembler& operator=(const DirectX12InputAssembler&);

    public:
        /// @copydoc InputAssembler::~InputAssembler
        ~DirectX12InputAssembler() noexcept override;

    public:
        /// @brief Creates a new DirectX 12 input assembler state.
        ///
        /// @param primitiveTopology The primitive topology.
        /// @param controlPoints The number of control points in a patch primitive. Ignored if @p primitiveTopology is not `PrimitiveTopology::PatchList`. Must be a value between 1 and 32.
        /// @return A shared pointer to the newly created input assembler instance.
        static inline auto create(PrimitiveTopology primitiveTopology, UInt32 controlPoints = 1) {
            return SharedObject::create<DirectX12InputAssembler>(primitiveTopology, controlPoints);
        }

        /// @brief Creates a new DirectX 12 input assembler state.
        ///
        /// @param vertexBufferLayouts The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.
        /// @param indexBufferLayout The index buffer layout.
        /// @param primitiveTopology The primitive topology.
        /// @param controlPoints The number of control points in a patch primitive. Ignored if @p primitiveTopology is not `PrimitiveTopology::PatchList`. Must be a value between 1 and 32.
        /// @return A shared pointer to the newly created input assembler instance.
        static inline auto create(Enumerable<SharedPtr<DirectX12VertexBufferLayout>>&& vertexBufferLayouts, SharedPtr<DirectX12IndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList, UInt32 controlPoints = 1) {
            return SharedObject::create<DirectX12InputAssembler>(std::move(vertexBufferLayouts), std::move(indexBufferLayout), primitiveTopology, controlPoints);
        }

        /// @brief Creates a new DirectX 12 input assembler state as a copy from another one.
        ///
        /// @param other The input assembler state to copy.
        /// @return A shared pointer to the newly created input assembler instance.
        static inline auto create(const DirectX12InputAssembler& other) {
            return SharedObject::create<DirectX12InputAssembler>(other);
        }

    private:
        /// @brief Creates a new DirectX 12 input assembler state.
        ///
        /// @return A shared pointer to the newly created input assembler instance.
        static inline auto create() {
            return SharedObject::create<DirectX12InputAssembler>();
        }

    public:
        /// @copydoc InputAssembler::vertexBufferLayouts
        Enumerable<const DirectX12VertexBufferLayout&> vertexBufferLayouts() const override;

		/// @copydoc InputAssembler::vertexBufferLayout
		const DirectX12VertexBufferLayout& vertexBufferLayout(UInt32 binding) const override;

		/// @copydoc InputAssembler::indexBufferLayout
		const DirectX12IndexBufferLayout* indexBufferLayout() const noexcept override;

        /// @copydoc IInputAssembler::topology
        PrimitiveTopology topology() const noexcept override;

        /// @copydoc IInputAssembler::controlPoints
        UInt32 controlPoints() const noexcept override;
    };

    /// @brief Implements a DirectX 12 @ref IRasterizer.
    ///
    /// @see DirectX12RasterizerBuilder
    class LITEFX_DIRECTX12_API DirectX12Rasterizer final : public Rasterizer {
        LITEFX_BUILDER(DirectX12RasterizerBuilder);
        friend struct SharedObject::Allocator<DirectX12Rasterizer>;

    private:
        /// @brief Initializes a new DirectX 12 rasterizer state.
        ///
        /// @param polygonMode The polygon mode used by the pipeline.
        /// @param cullMode The cull mode used by the pipeline.
        /// @param cullOrder The cull order used by the pipeline.
        /// @param lineWidth The line width used by the pipeline.
        /// @param depthClip The depth clip toggle of the rasterizer state.
        /// @param depthStencilState The rasterizer depth/stencil state.
        /// @param conservativeRasterization Toggles the use of conservative rasterization in the rasterizer.
        explicit DirectX12Rasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth = 1.f, bool depthClip = true, const DepthStencilState& depthStencilState = {}, bool conservativeRasterization = false) noexcept;
        
        /// @brief Initializes a new DirectX 12 rasterizer state.
        explicit DirectX12Rasterizer() noexcept;

    private:        
        /// @copydoc Rasterizer::Rasterizer(Rasterizer&&)
        DirectX12Rasterizer(DirectX12Rasterizer&&) noexcept = delete;
        
        /// @copydoc Rasterizer::Rasterizer(const Rasterizer&)
        DirectX12Rasterizer(const DirectX12Rasterizer&) = default;

        /// @copydoc Rasterizer::operator=(Rasterizer&&)
        DirectX12Rasterizer& operator=(DirectX12Rasterizer&&) noexcept = delete;

        /// @copydoc Rasterizer::operator=(const Rasterizer&)
        DirectX12Rasterizer& operator=(const DirectX12Rasterizer&) = delete;

    public:
        /// @copydoc Rasterizer::~Rasterizer
        ~DirectX12Rasterizer() noexcept override = default;

    public:
        /// @brief Creates a new DirectX 12 rasterizer state.
        ///
        /// @param polygonMode The polygon mode used by the pipeline.
        /// @param cullMode The cull mode used by the pipeline.
        /// @param cullOrder The cull order used by the pipeline.
        /// @param lineWidth The line width used by the pipeline.
        /// @param depthClip The depth clip toggle of the rasterizer state.
        /// @param depthStencilState The rasterizer depth/stencil state.
        /// @param conservativeRasterization Toggles the use of conservative rasterization in the rasterizer.
        /// @return A shared pointer to the newly created rasterizer instance.
        static inline auto create(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth = 1.f, bool depthClip = true, const DepthStencilState& depthStencilState = {}, bool conservativeRasterization = false) {
            return SharedObject::create<DirectX12Rasterizer>(polygonMode, cullMode, cullOrder, lineWidth, depthClip, depthStencilState, conservativeRasterization);
        }

        /// @brief Creates a new DirectX 12 rasterizer state by copying an existing one.
        ///
        /// @param other The rasterizer state to copy.
        /// @return A shared pointer to the newly created rasterizer instance.
        static inline auto create(const DirectX12Rasterizer& other) {
            return SharedObject::create<DirectX12Rasterizer>(other);
        }

    private:
        /// @brief Creates a new DirectX 12 rasterizer state.
        ///
        /// @return A shared pointer to the newly created rasterizer instance.
        static inline auto create() {
            return SharedObject::create<DirectX12Rasterizer>();
        }
    };

    /// @brief Defines the base class for DirectX 12 pipeline state objects.
    ///
    /// @see DirectX12RenderPipeline
    /// @see DirectX12ComputePipeline
    class LITEFX_DIRECTX12_API DirectX12PipelineState : public virtual Pipeline<DirectX12PipelineLayout, DirectX12ShaderProgram>, public ComResource<ID3D12PipelineState> {
    protected:
        using ComResource<ID3D12PipelineState>::ComResource;

        DirectX12PipelineState(DirectX12PipelineState&&) noexcept = default;
        DirectX12PipelineState(const DirectX12PipelineState&) = delete;
        DirectX12PipelineState& operator=(DirectX12PipelineState&&) noexcept = default;
        DirectX12PipelineState& operator=(const DirectX12PipelineState&) = delete;
        
    public:
        ~DirectX12PipelineState() noexcept override = default;

    public:
        /// @brief Sets the current pipeline state on the @p commandBuffer.
        ///
        /// @param commandBuffer The command buffer to set the current pipeline state on.
        virtual void use(const DirectX12CommandBuffer& commandBuffer) const = 0;
    };

    /// @brief Records commands for a @ref DirectX12Queue
    ///
    /// @see DirectX12Queue
    class LITEFX_DIRECTX12_API DirectX12CommandBuffer final : public CommandBuffer<DirectX12CommandBuffer, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, DirectX12Barrier, DirectX12PipelineState, DirectX12BottomLevelAccelerationStructure, DirectX12TopLevelAccelerationStructure>, public ComResource<ID3D12GraphicsCommandList7> {
        LITEFX_IMPLEMENTATION(DirectX12CommandBufferImpl);
        friend struct SharedObject::Allocator<DirectX12CommandBuffer>;

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
        using base_type::bind;
        using base_type::use;
        using base_type::pushConstants;
        using base_type::buildAccelerationStructure;
        using base_type::updateAccelerationStructure;
        using base_type::copyAccelerationStructure;

    private:
        /// @brief Initializes the command buffer from a command queue.
        ///
        /// @param queue The parent command queue, the buffer gets submitted to.
        /// @param begin If set to `true`, the command buffer automatically starts recording by calling @ref begin.
        /// @param primary `true`, if the command buffer is a primary command buffer.
        explicit DirectX12CommandBuffer(const DirectX12Queue& queue, bool begin = false, bool primary = true);

    private:
        /// @copydoc CommandBuffer::CommandBuffer(CommandBuffer&&)
        DirectX12CommandBuffer(DirectX12CommandBuffer&&) noexcept = delete;

        /// @copydoc CommandBuffer::CommandBuffer(const CommandBuffer&)
        DirectX12CommandBuffer(const DirectX12CommandBuffer&) = delete;

        /// @copydoc CommandBuffer::operator=(CommandBuffer&&)
        DirectX12CommandBuffer& operator=(DirectX12CommandBuffer&&) noexcept = delete;

        /// @copydoc CommandBuffer::operator=(const CommandBuffer&)
        DirectX12CommandBuffer& operator=(const DirectX12CommandBuffer&) = delete;

    public:
        /// @copydoc CommandBuffer::~CommandBuffer
        ~DirectX12CommandBuffer() noexcept override;

    public:
        /// @brief Initializes the command buffer from a command queue.
        ///
        /// @param queue The parent command queue, the buffer gets submitted to.
        /// @param begin If set to `true`, the command buffer automatically starts recording by calling @ref begin.
        /// @param primary `true`, if the command buffer is a primary command buffer.
        static inline SharedPtr<DirectX12CommandBuffer> create(const DirectX12Queue& queue, bool begin = false, bool primary = true) {
            return SharedObject::create<DirectX12CommandBuffer>(queue, begin, primary);
        }

        // CommandBuffer interface.
    public:
        /// @copydoc ICommandBuffer::queue
        SharedPtr<const DirectX12Queue> queue() const noexcept;

        /// @copydoc ICommandBuffer::begin
        void begin() const override;

        /// @copydoc ICommandBuffer::end
        void end() const override;

        /// @copydoc ICommandBuffer::track(SharedPtr<const IBuffer>) const
        void track(SharedPtr<const IBuffer> buffer) const override;

        /// @copydoc ICommandBuffer::track(SharedPtr<const IImage>) const
        void track(SharedPtr<const IImage> image) const override;

        /// @copydoc ICommandBuffer::track(SharedPtr<const ISampler>) const
        void track(SharedPtr<const ISampler> sampler) const override;

        /// @copydoc ICommandBuffer::track(UniquePtr<const IDescriptorSet>&&) const
        void track(UniquePtr<const IDescriptorSet>&& descriptorSet) const override;

        /// @copydoc ICommandBuffer::isSecondary
        bool isSecondary() const noexcept override;

        /// @copydoc ICommandBuffer::setViewports(Span<const IViewport*>) const
        void setViewports(Span<const IViewport*> viewports) const override;

        /// @copydoc ICommandBuffer::setViewports(const IViewport*) const
        void setViewports(const IViewport* viewport) const override;

        /// @copydoc ICommandBuffer::setScissors(Span<const IScissor*>) const
        void setScissors(Span<const IScissor*> scissors) const override;

        /// @copydoc ICommandBuffer::setScissors(const IScissor*) const
        void setScissors(const IScissor* scissor) const override;

        /// @copydoc ICommandBuffer::setBlendFactors
        void setBlendFactors(const Vector4f& blendFactors) const noexcept override;

        /// @copydoc ICommandBuffer::setStencilRef
        void setStencilRef(UInt32 stencilRef) const noexcept override;

        /// @copydoc ICommandBuffer::setDepthBounds
        void setDepthBounds(Float minBounds, Float maxBounds) const noexcept override;

        /// @copydoc ICommandBuffer::submit
        UInt64 submit() const override;

        /// @copydoc CommandBuffer::makeBarrier
        [[nodiscard]] UniquePtr<DirectX12Barrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const override;

        /// @copydoc CommandBuffer::barrier
        void barrier(const DirectX12Barrier& barrier) const noexcept override;

        /// @copydoc CommandBuffer::transfer(const buffer_type&, const buffer_type&, UInt32, UInt32, UInt32) const
        void transfer(const IDirectX12Buffer& source, const IDirectX12Buffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// @copydoc CommandBuffer::transfer(const void* const, size_t, const buffer_type&, UInt32, UInt32) const
        void transfer(const void* const data, size_t size, const IDirectX12Buffer& target, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// @copydoc CommandBuffer::transfer(Span<const void* const>, size_t, const buffer_type&, UInt32) const
        void transfer(Span<const void* const> data, size_t elementSize, const IDirectX12Buffer& target, UInt32 firstElement = 0) const override;

        /// @copydoc CommandBuffer::transfer(const buffer_type&, const image_type&, UInt32, UInt32, UInt32) const
        void transfer(const IDirectX12Buffer& source, const IDirectX12Image& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const override;

        /// @copydoc CommandBuffer::transfer(const void* const, size_t, const image_type&, UInt32) const
        void transfer(const void* const data, size_t size, const IDirectX12Image& target, UInt32 subresource = 0) const override;

        /// @copydoc CommandBuffer::transfer(Span<const void* const>, size_t, const image_type&, UInt32, UInt32) const
        void transfer(Span<const void* const> data, size_t elementSize, const IDirectX12Image& target, UInt32 firstSubresource = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::transfer(const image_type&, const image_type&, UInt32, UInt32, UInt32) const
        void transfer(const IDirectX12Image& source, const IDirectX12Image& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::transfer(const image_type&, const buffer_type&, UInt32, UInt32, UInt32) const
        void transfer(const IDirectX12Image& source, const IDirectX12Buffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::transfer(const SharedPtr<const buffer_type>&, const buffer_type&, UInt32, UInt32, UInt32) const
        void transfer(const SharedPtr<const IDirectX12Buffer>& source, const IDirectX12Buffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// @copydoc CommandBuffer::transfer(const SharedPtr<const buffer_type>&, const image_type&, UInt32, UInt32, UInt32) const
        void transfer(const SharedPtr<const IDirectX12Buffer>& source, const IDirectX12Image& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const override;

        /// @copydoc CommandBuffer::transfer(const SharedPtr<const image_type>&, const image_type&, UInt32, UInt32, UInt32) const
        void transfer(const SharedPtr<const IDirectX12Image>& source, const IDirectX12Image& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::transfer(const SharedPtr<const image_type>&, const buffer_type&, UInt32, UInt32, UInt32) const
        void transfer(const SharedPtr<const IDirectX12Image>& source, const IDirectX12Buffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::use
        void use(const DirectX12PipelineState& pipeline) const noexcept override;

		/// @copydoc CommandBuffer::bind(const descriptor_set_type&) const
		void bind(const DirectX12DescriptorSet& descriptorSet) const override;

        /// @copydoc CommandBuffer::bind(Span<const descriptor_set_type*>) const
        void bind(Span<const DirectX12DescriptorSet*> descriptorSets) const override;

        /// @copydoc CommandBuffer::bind(const descriptor_set_type&, const pipeline_type&) const
        void bind(const DirectX12DescriptorSet& descriptorSet, const DirectX12PipelineState& pipeline) const override;

		/// @copydoc CommandBuffer::bind(Span<const descriptor_set_type*>, const pipeline_type&) const
		void bind(Span<const DirectX12DescriptorSet*> descriptorSets, const DirectX12PipelineState& pipeline) const override;

        /// @copydoc CommandBuffer::bind(const vertex_buffer_type&) const
        void bind(const IDirectX12VertexBuffer& buffer) const noexcept override;

        /// @copydoc CommandBuffer::bind(const index_buffer_type&) const
        void bind(const IDirectX12IndexBuffer& buffer) const noexcept override;

        /// @copydoc ICommandBuffer::dispatch(const Vector3u&) const
        void dispatch(const Vector3u& threadCount) const noexcept override;

        /// @copydoc CommandBuffer::dispatchIndirect
        void dispatchIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// @copydoc CommandBuffer::dispatchIndirect
        void dispatchIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept;

        /// @copydoc ICommandBuffer::dispatchMesh(const Vector3u&) const
        void dispatchMesh(const Vector3u& threadCount) const noexcept override;

        /// @copydoc CommandBuffer::dispatchMeshIndirect(const buffer_type&, UInt32, UInt64) const
        void dispatchMeshIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// @copydoc CommandBuffer::dispatchMeshIndirect(const buffer_type&, const buffer_type&, UInt64, UInt64, UInt32) const
        void dispatchMeshIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// @copydoc ICommandBuffer::draw(UInt32, UInt32, UInt32, UInt32) const
        void draw(UInt32 vertices, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const noexcept override;

        /// @copydoc CommandBuffer::drawIndirect(const buffer_type&, UInt32, UInt64) const
        void drawIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// @copydoc CommandBuffer::drawIndirect(const buffer_type&, const buffer_type&, UInt64, UInt64, UInt32) const
        void drawIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// @copydoc ICommandBuffer::drawIndexed(UInt32, UInt32, UInt32, Int32, UInt32) const
        void drawIndexed(UInt32 indices, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const noexcept override;

        /// @copydoc CommandBuffer::drawIndexedIndirect(const buffer_type&, UInt32, UInt64) const
        void drawIndexedIndirect(const IDirectX12Buffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// @copydoc CommandBuffer::drawIndexedIndirect(const buffer_type&, const buffer_type&, UInt64, UInt64, UInt32) const
        void drawIndexedIndirect(const IDirectX12Buffer& batchBuffer, const IDirectX12Buffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;
        
        /// @copydoc CommandBuffer::pushConstants
        void pushConstants(const DirectX12PushConstantsLayout& layout, const void* const memory) const override;

        /// @copydoc ICommandBuffer::writeTimingEvent
        void writeTimingEvent(const SharedPtr<const TimingEvent>& timingEvent) const override;

        /// @copydoc ICommandBuffer::execute(const SharedPtr<const ICommandBuffer>&) const
        void execute(const SharedPtr<const DirectX12CommandBuffer>& commandBuffer) const override;

        /// @copydoc ICommandBuffer::execute(Enumerable<SharedPtr<const ICommandBuffer>>) const
        void execute(Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers) const override;

        /// @copydoc CommandBuffer::buildAccelerationStructure(bottom_level_acceleration_structure_type&, const SharedPtr<const buffer_type>&, const buffer_type&, UInt64) const
        void buildAccelerationStructure(DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset = 0) const override;

        /// @copydoc CommandBuffer::buildAccelerationStructure(top_level_acceleration_structure_type&, const SharedPtr<const buffer_type>&, const buffer_type&, UInt64) const
        void buildAccelerationStructure(DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset = 0) const override;

        /// @copydoc CommandBuffer::updateAccelerationStructure(bottom_level_acceleration_structure_type&, const SharedPtr<const buffer_type>&, const buffer_type&, UInt64) const
        void updateAccelerationStructure(DirectX12BottomLevelAccelerationStructure& blas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset = 0) const override;

        /// @copydoc CommandBuffer::updateAccelerationStructure(top_level_acceleration_structure_type&, const SharedPtr<const buffer_type>&, const buffer_type&, UInt64) const
        void updateAccelerationStructure(DirectX12TopLevelAccelerationStructure& tlas, const SharedPtr<const IDirectX12Buffer>& scratchBuffer, const IDirectX12Buffer& buffer, UInt64 offset = 0) const override;

        /// @copydoc CommandBuffer::copyAccelerationStructure(const bottom_level_acceleration_structure_type&, const bottom_level_acceleration_structure_type&, bool) const
        void copyAccelerationStructure(const DirectX12BottomLevelAccelerationStructure& from, const DirectX12BottomLevelAccelerationStructure& to, bool compress = false) const noexcept override;

        /// @copydoc CommandBuffer::copyAccelerationStructure(const top_level_acceleration_structure_type&, const top_level_acceleration_structure_type&, bool) const
        void copyAccelerationStructure(const DirectX12TopLevelAccelerationStructure& from, const DirectX12TopLevelAccelerationStructure& to, bool compress = false) const noexcept override;

        /// @copydoc CommandBuffer::traceRays(UInt32, UInt32, UInt32, const ShaderBindingTableOffsets&, const buffer_type&, const buffer_type*, const buffer_type*, const buffer_type*) const
        void traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IDirectX12Buffer& rayGenerationShaderBindingTable, const IDirectX12Buffer* missShaderBindingTable, const IDirectX12Buffer* hitShaderBindingTable, const IDirectX12Buffer* callableShaderBindingTable) const noexcept override;

    private:
        inline SharedPtr<const ICommandQueue> getQueue() const noexcept override {
            return std::static_pointer_cast<const ICommandQueue>(this->queue());
        }

        void releaseSharedState() const override;
    };

    /// @brief Implements a DirectX 12 command queue.
    ///
    /// @see DirectX12CommandBuffer
    class LITEFX_DIRECTX12_API DirectX12Queue final : public CommandQueue<DirectX12CommandBuffer>, public ComResource<ID3D12CommandQueue> {
        LITEFX_IMPLEMENTATION(DirectX12QueueImpl);
        friend struct SharedObject::Allocator<DirectX12Queue>;

    public:
        using base_type = CommandQueue<DirectX12CommandBuffer>;
        using base_type::submit;

    private:
        /// @brief Initializes the DirectX 12 command queue.
        ///
        /// @param device The device, commands get send to.
        /// @param type The type of the command queue.
        /// @param priority The priority, of which commands are issued on the device.
        explicit DirectX12Queue(const DirectX12Device& device, QueueType type, QueuePriority priority);

    private:
        /// @copydoc CommandQueue::CommandQueue(CommandQueue&&)
        DirectX12Queue(DirectX12Queue&&) noexcept = delete;

        /// @copydoc CommandQueue::CommandQueue(const CommandQueue&)
        DirectX12Queue(const DirectX12Queue&) = delete;

        /// @copydoc CommandQueue::operator=(CommandQueue&&)
        DirectX12Queue& operator=(DirectX12Queue&&) noexcept = delete;

        /// @copydoc CommandQueue::operator=(const CommandQueue&)
        DirectX12Queue& operator=(const DirectX12Queue&) = delete;

    public:
        /// @copydoc CommandQueue::~CommandQueue
        ~DirectX12Queue() noexcept override;

    public:
        /// @brief Creates the DirectX 12 command queue.
        ///
        /// @param device The device, commands get send to.
        /// @param type The type of the command queue.
        /// @param priority The priority, of which commands are issued on the device.
        /// @return A pointer to the command queue instance.
        static inline SharedPtr<DirectX12Queue> create(const DirectX12Device& device, QueueType type, QueuePriority priority) {
            return SharedObject::create<DirectX12Queue>(device, type, priority);
        }

        // DirectX12Queue interface.
    public:
        /// @brief Returns a pointer to the device that provides this queue or `nullptr`, if the device has already been released.
        ///
        /// @return A reference to the queue's parent device or `nullptr`, if the device has already been released.
        SharedPtr<const DirectX12Device> device() const noexcept;

        // CommandQueue interface.
    public:
        /// @copydoc ICommandQueue::priority
        QueuePriority priority() const noexcept override;

        /// @copydoc ICommandQueue::type
        QueueType type() const noexcept override;

#if defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)
    public:
        /// @copydoc ICommandQueue::beginDebugRegion
        void beginDebugRegion(const String& label, const Vectors::ByteVector3& color = DEFAULT_DEBUG_COLOR) const noexcept override;

        /// @copydoc ICommandQueue::endDebugRegion
        void endDebugRegion() const noexcept override;

        /// @copydoc ICommandQueue::setDebugMarker
        void setDebugMarker(const String& label, const Vectors::ByteVector3& color = DEFAULT_DEBUG_COLOR) const noexcept override;
#endif // defined(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS) && defined(LITEFX_BUILD_WITH_PIX_RUNTIME)

    public:
        /// @copydoc CommandQueue::createCommandBuffer
        SharedPtr<DirectX12CommandBuffer> createCommandBuffer(bool beginRecording = false, bool secondary = false) const override;

        /// @copydoc CommandQueue::submit(const SharedPtr<const command_buffer_type>&) const
        UInt64 submit(const SharedPtr<const DirectX12CommandBuffer>& commandBuffer) const override;

        /// @copydoc CommandQueue::submit(Enumerable<SharedPtr<const command_buffer_type>>) const
        UInt64 submit(Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers) const override;

        /// @copydoc ICommandQueue::waitFor(UInt64) const
        void waitFor(UInt64 fence) const override;

        /// @copydoc ICommandQueue::waitFor(const ICommandQueue&, UInt64) const
        void waitFor(const DirectX12Queue& queue, UInt64 fence) const noexcept;

        /// @copydoc ICommandQueue::currentFence
        UInt64 currentFence() const noexcept override;
        
        /// @copydoc ICommandQueue::lastCompletedFence
        UInt64 lastCompletedFence() const noexcept override;

    private:
        inline void waitForQueue(const ICommandQueue& queue, UInt64 fence) const override {
            auto d3dQueue = dynamic_cast<const DirectX12Queue*>(&queue);

            if (d3dQueue == nullptr) [[unlikely]]
                throw InvalidArgumentException("queue", "Cannot wait for queues from other backends.");

            this->waitFor(*d3dQueue, fence);
        }
    };

    /// @brief Implements a DirectX 12 @ref RenderPipeline.
    ///
    /// @see DirectX12ComputePipeline
    /// @see DirectX12RenderPipelineBuilder
    class LITEFX_DIRECTX12_API DirectX12RenderPipeline final : public virtual DirectX12PipelineState, public RenderPipeline<DirectX12PipelineLayout, DirectX12ShaderProgram, DirectX12InputAssembler, DirectX12Rasterizer> {
        LITEFX_IMPLEMENTATION(DirectX12RenderPipelineImpl);
        LITEFX_BUILDER(DirectX12RenderPipelineBuilder);

    public:
        /// @brief Initializes a new DirectX 12 render pipeline.
        ///
        /// @param renderPass The parent render pass.
        /// @param shaderProgram The shader program used by the pipeline.
        /// @param layout The layout of the pipeline.
        /// @param inputAssembler The input assembler state of the pipeline.
        /// @param rasterizer The rasterizer state of the pipeline.
        /// @param samples The initial multi-sampling level of the render pipeline.
        /// @param enableAlphaToCoverage Whether or not to enable Alpha-to-Coverage multi-sampling.
        /// @param name The optional name of the render pipeline.
        explicit DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, const SharedPtr<DirectX12PipelineLayout>& layout, const SharedPtr<DirectX12ShaderProgram>& shaderProgram, const SharedPtr<DirectX12InputAssembler>& inputAssembler, const SharedPtr<DirectX12Rasterizer>& rasterizer, MultiSamplingLevel samples = MultiSamplingLevel::x1, bool enableAlphaToCoverage = false, const String& name = "");

        /// @copydoc DirectX12PipelineState::DirectX12PipelineState(DirectX12PipelineState&&)
        DirectX12RenderPipeline(DirectX12RenderPipeline&&) noexcept = delete;
        
        /// @copydoc DirectX12PipelineState::DirectX12PipelineState(const DirectX12PipelineState&)
        DirectX12RenderPipeline(const DirectX12RenderPipeline&) = delete;

        /// @copydoc DirectX12PipelineState::operator=(DirectX12PipelineState&&)
        DirectX12RenderPipeline& operator=(DirectX12RenderPipeline&&) noexcept = delete;

        /// @copydoc DirectX12PipelineState::operator=(const DirectX12PipelineState&)
        DirectX12RenderPipeline& operator=(const DirectX12RenderPipeline&) = delete;

        /// @copydoc DirectX12PipelineState::~DirectX12PipelineState
        ~DirectX12RenderPipeline() noexcept override;

    private:
        /// @brief Initializes a new DirectX 12 render pipeline.
        ///
        /// @param renderPass The parent render pass.
        /// @param name The optional name of the render pipeline.
        DirectX12RenderPipeline(const DirectX12RenderPass& renderPass, const String& name = "");

        // Pipeline interface.
    public:
        /// @copydoc Pipeline::program
        SharedPtr<const DirectX12ShaderProgram> program() const noexcept override;

        /// @copydoc Pipeline::layout
        SharedPtr<const DirectX12PipelineLayout> layout() const noexcept override;

        // RenderPipeline interface.
    public:
        /// @copydoc RenderPipeline::inputAssembler
        SharedPtr<DirectX12InputAssembler> inputAssembler() const noexcept override;

        /// @copydoc RenderPipeline::rasterizer
        SharedPtr<DirectX12Rasterizer> rasterizer() const noexcept override;

        /// @copydoc IRenderPipeline::alphaToCoverage
        bool alphaToCoverage() const noexcept override;

        /// @copydoc IRenderPipeline::samples
        MultiSamplingLevel samples() const noexcept override;

        /// @copydoc IRenderPipeline::updateSamples
        void updateSamples(MultiSamplingLevel samples) override;

        // DirectX12PipelineState interface.
    public:
        /// @copydoc DirectX12PipelineState::use
        void use(const DirectX12CommandBuffer& commandBuffer) const override;
    };

    /// @brief Implements a DirectX 12 @ref ComputePipeline.
    ///
    /// @see DirectX12RenderPipeline
    /// @see DirectX12ComputePipelineBuilder
    class LITEFX_DIRECTX12_API DirectX12ComputePipeline final : public virtual DirectX12PipelineState, public ComputePipeline<DirectX12PipelineLayout, DirectX12ShaderProgram> {
        LITEFX_IMPLEMENTATION(DirectX12ComputePipelineImpl);
        LITEFX_BUILDER(DirectX12ComputePipelineBuilder);

    public:
        /// @brief Initializes a new DirectX 12 compute pipeline.
        ///
        /// @param device The parent device.
        /// @param layout The layout of the pipeline.
        /// @param shaderProgram The shader program used by this pipeline.
        /// @param name The optional debug name of the compute pipeline.
        explicit DirectX12ComputePipeline(const DirectX12Device& device, const SharedPtr<DirectX12PipelineLayout>& layout, const SharedPtr<DirectX12ShaderProgram>& shaderProgram, const String& name = "");

        /// @copydoc DirectX12PipelineState::DirectX12PipelineState(DirectX12PipelineState&&)
        DirectX12ComputePipeline(DirectX12ComputePipeline&&) noexcept;

        /// @copydoc DirectX12PipelineState::DirectX12PipelineState(const DirectX12PipelineState&)
        DirectX12ComputePipeline(const DirectX12ComputePipeline&) = delete;

        /// @copydoc DirectX12PipelineState::operator=(DirectX12PipelineState&&)
        DirectX12ComputePipeline& operator=(DirectX12ComputePipeline&&) noexcept;

        /// @copydoc DirectX12PipelineState::operator=(const DirectX12PipelineState&)
        DirectX12ComputePipeline& operator=(const DirectX12ComputePipeline&) = delete;

        /// @copydoc DirectX12PipelineState::~DirectX12PipelineState
        ~DirectX12ComputePipeline() noexcept override;

    private:
        /// @brief Initializes a new DirectX 12 compute pipeline.
        ///
        /// @param device The parent device.
        DirectX12ComputePipeline(const DirectX12Device& device) noexcept;

        // Pipeline interface.
    public:
        /// @copydoc Pipeline::program
        SharedPtr<const DirectX12ShaderProgram> program() const noexcept override;

        /// @copydoc Pipeline::layout
        SharedPtr<const DirectX12PipelineLayout> layout() const noexcept override;

        // DirectX12PipelineState interface.
    public:
        void use(const DirectX12CommandBuffer& commandBuffer) const noexcept override;
    };

    /// @brief Implements a DirectX 12 @ref RayTracingPipeline.
    ///
    /// Note that the ray tracing pipeline does not set its handle, as it is a different base type from the one used to define ray tracing pipelines. Instead to obtain the handle, call @ref
    /// DirectX12RayTracingPipeline::stateObject().
    ///
    /// At some point it is expected that D3D introduces a unified pipeline architecture based on state objects. At this point, the pipeline state base object will switch to using state objects as its handle
    /// type and the `stateObject` method above will be deprecated.
    ///
    /// @see DirectX12RenderPipeline
    /// @see DirectX12RayTracingPipelineBuilder
    class LITEFX_DIRECTX12_API DirectX12RayTracingPipeline final : public virtual DirectX12PipelineState, public RayTracingPipeline<DirectX12PipelineLayout, DirectX12ShaderProgram> {
        LITEFX_IMPLEMENTATION(DirectX12RayTracingPipelineImpl);
        LITEFX_BUILDER(DirectX12RayTracingPipelineBuilder);

    public:
        /// @brief Initializes a new DirectX 12 ray-tracing pipeline.
        ///
        /// @param device The parent device.
        /// @param layout The layout of the pipeline.
        /// @param shaderProgram The shader program used by this pipeline.
        /// @param shaderRecords The shader record collection that is used to build the shader binding table for the pipeline.
        /// @param maxRecursionDepth The maximum number of ray bounces.
        /// @param maxPayloadSize The maximum size for ray payloads in the pipeline.
        /// @param maxAttributeSize The maximum size for ray attributes in the pipeline.
        /// @param name The optional debug name of the ray-tracing pipeline.
        explicit DirectX12RayTracingPipeline(const DirectX12Device& device, const SharedPtr<DirectX12PipelineLayout>& layout, const SharedPtr<DirectX12ShaderProgram>& shaderProgram, ShaderRecordCollection&& shaderRecords, UInt32 maxRecursionDepth = 10, UInt32 maxPayloadSize = 0, UInt32 maxAttributeSize = 32, const String& name = ""); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

        /// @copydoc DirectX12PipelineState::DirectX12PipelineState(DirectX12PipelineState&&)
        DirectX12RayTracingPipeline(DirectX12RayTracingPipeline&&) noexcept;

        /// @copydoc DirectX12PipelineState::DirectX12PipelineState(const DirectX12PipelineState&)
        DirectX12RayTracingPipeline(const DirectX12RayTracingPipeline&) = delete;

        /// @copydoc DirectX12PipelineState::operator=(DirectX12PipelineState&&)
        DirectX12RayTracingPipeline& operator=(DirectX12RayTracingPipeline&&) noexcept;

        /// @copydoc DirectX12PipelineState::operator=(const DirectX12PipelineState&)
        DirectX12RayTracingPipeline& operator=(const DirectX12RayTracingPipeline&) = delete;

        /// @copydoc DirectX12PipelineState::~DirectX12PipelineState
        ~DirectX12RayTracingPipeline() noexcept override;

    private:
        /// @brief Initializes a new DirectX 12 ray-tracing pipeline.
        ///
        /// @param device The parent device.
        /// @param shaderRecords The shader record collection that is used to build the shader binding table for the pipeline.
        DirectX12RayTracingPipeline(const DirectX12Device& device, ShaderRecordCollection&& shaderRecords) noexcept;

        // Pipeline interface.
    public:
        /// @copydoc Pipeline::program
        SharedPtr<const DirectX12ShaderProgram> program() const noexcept override;

        /// @copydoc Pipeline::layout
        SharedPtr<const DirectX12PipelineLayout> layout() const noexcept override;

        // RayTracingPipeline interface.
    public:
        /// @copydoc IRayTracingPipeline::shaderRecords
        const ShaderRecordCollection& shaderRecords() const noexcept override;

        /// @copydoc IRayTracingPipeline::maxRecursionDepth
        UInt32 maxRecursionDepth() const noexcept override;

        /// @copydoc IRayTracingPipeline::maxPayloadSize
        UInt32 maxPayloadSize() const noexcept override;

        /// @copydoc IRayTracingPipeline::maxAttributeSize
        UInt32 maxAttributeSize() const noexcept override;

        /// @copydoc RayTracingPipeline::allocateShaderBindingTable
        SharedPtr<IDirectX12Buffer> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups = ShaderBindingGroup::All) const override;

        // DirectX12PipelineState interface.
    public:
        /// @copydoc DirectX12PipelineState::use
        void use(const DirectX12CommandBuffer& commandBuffer) const noexcept override;

        /// @brief Returns the handle of the ray tracing pipeline state object.
        ///
        /// @return The handle of the ray tracing pipeline state object.
        ComPtr<ID3D12StateObject> stateObject() const noexcept;
    };

    /// @brief Implements a DirectX 12 frame buffer.
    ///
    /// @see DirectX12RenderPass
    class LITEFX_DIRECTX12_API DirectX12FrameBuffer final : public FrameBuffer<IDirectX12Image> {
        LITEFX_IMPLEMENTATION(DirectX12FrameBufferImpl);
        friend struct SharedObject::Allocator<DirectX12FrameBuffer>;

    public:
        using FrameBuffer::allocation_callback_type;
        using FrameBuffer::addImage;
        using FrameBuffer::mapRenderTarget;
        using FrameBuffer::mapRenderTargets;

    private:
        /// @brief Initializes a DirectX 12 frame buffer.
        ///
        /// @param device The device the frame buffer is allocated on.
        /// @param renderArea The initial size of the render area.
        /// @param name The name of the frame buffer.
        DirectX12FrameBuffer(const DirectX12Device& device, const Size2d& renderArea, StringView name = "");

        /// @brief Initializes a DirectX 12 frame buffer.
        ///
        /// @param device The device the frame buffer is allocated on.
        /// @param renderArea The initial size of the render area.
        /// @param allocationCallback A callback that gets invoked, when the frame buffer allocates a new image.
        /// @param name The name of the frame buffer.
        /// @see IFrameBuffer::allocation_callback_type
        DirectX12FrameBuffer(const DirectX12Device& device, const Size2d& renderArea, allocation_callback_type allocationCallback, StringView name = "");

    private:
        /// @copydoc FrameBuffer::FrameBuffer(FrameBuffer&&)
        DirectX12FrameBuffer(DirectX12FrameBuffer&&) noexcept = delete;

        /// @copydoc FrameBuffer::FrameBuffer(const FrameBuffer&)
        DirectX12FrameBuffer(const DirectX12FrameBuffer&) = delete;

        /// @copydoc FrameBuffer::operator=(FrameBuffer&&)
        DirectX12FrameBuffer& operator=(DirectX12FrameBuffer&&) noexcept = delete;

        /// @copydoc FrameBuffer::operator=(const FrameBuffer&)
        DirectX12FrameBuffer& operator=(const DirectX12FrameBuffer&) = delete;

    public:
        /// @copydoc FrameBuffer::~FrameBuffer
        ~DirectX12FrameBuffer() noexcept override;

    public:
        /// @brief Initializes a DirectX 12 frame buffer.
        ///
        /// @param device The device the frame buffer is allocated on.
        /// @param renderArea The initial size of the render area.
        /// @param name The name of the frame buffer.
        /// @return A pointer to the newly created frame buffer instance.
        static inline SharedPtr<DirectX12FrameBuffer> create(const DirectX12Device& device, const Size2d& renderArea, StringView name = "") {
            return SharedObject::create<DirectX12FrameBuffer>(device, renderArea, name);
        }

        /// @brief Initializes a DirectX 12 frame buffer.
        ///
        /// @param device The device the frame buffer is allocated on.
        /// @param renderArea The initial size of the render area.
        /// @param allocationCallback A callback that gets invoked, when the frame buffer allocates a new image.
        /// @param name The name of the frame buffer.
        /// @return A pointer to the newly created frame buffer instance.
        static inline SharedPtr<DirectX12FrameBuffer> create(const DirectX12Device& device, const Size2d& renderArea, allocation_callback_type allocationCallback, StringView name = "") {
            return SharedObject::create<DirectX12FrameBuffer>(device, renderArea, std::move(allocationCallback), name);
        }

        // DirectX 12 FrameBuffer
    public:
        /// @brief Returns the descriptor handle for an image at the specified index.
        ///
        /// @param imageIndex The index of the image for which the descriptor handle should be returned.
        /// @return The descriptor handle for the image.
        /// @throws ArgumentOutOfRangeException Thrown, if the provided image index does not address an image within the frame buffer.
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle(UInt32 imageIndex) const;

        /// @brief Returns the descriptor handle for an image with the specified name.
        ///
        /// @param imageName The name of the image for which the descriptor handle should be returned.
        /// @return The descriptor handle for the image.
        /// @throws InvalidArgumentException Thrown, if the provided image name does refer to an image within the frame buffer.
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle(StringView imageName) const;
        
        /// @brief Returns the descriptor handle for an image mapped to the specified render target.
        ///
        /// @param renderTarget The render target for which to return the image descriptor handle.
        /// @return The descriptor handle for the image.
        /// @throws InvalidArgumentException Thrown, if the provided render target is not mapped to an image within the frame buffer.
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle(const RenderTarget& renderTarget) const;

        // FrameBuffer interface.
    public:
        /// @copydoc IFrameBuffer::size
        const Size2d& size() const noexcept override;

        /// @copydoc IFrameBuffer::getWidth
        size_t getWidth() const noexcept override;

        /// @copydoc IFrameBuffer::getHeight
        size_t getHeight() const noexcept override;

        /// @copydoc IFrameBuffer::mapRenderTarget(const RenderTarget&, UInt32)
        void mapRenderTarget(const RenderTarget& renderTarget, UInt32 index) override;

        /// @copydoc IFrameBuffer::mapRenderTarget(const RenderTarget&, StringView)
        void mapRenderTarget(const RenderTarget& renderTarget, StringView name) override;

        /// @copydoc IFrameBuffer::unmapRenderTarget
        void unmapRenderTarget(const RenderTarget& renderTarget) noexcept override;

        /// @copydoc FrameBuffer::images
        const Array<SharedPtr<const IDirectX12Image>>& images() const override;

        /// @copydoc IFrameBuffer::operator[](UInt32) const
        inline const IDirectX12Image& operator[](UInt32 index) const override {
            return this->image(index);
        }

        /// @copydoc IFrameBuffer::image(UInt32) const
        const IDirectX12Image& image(UInt32 index) const override;

        /// @copydoc IFrameBuffer::operator[](const RenderTarget&) const
        inline const IDirectX12Image& operator[](const RenderTarget& renderTarget) const override {
            return this->image(renderTarget);
        }

        /// @copydoc IFrameBuffer::image(const RenderTarget&) const
        const IDirectX12Image& image(const RenderTarget& renderTarget) const override;

        /// @copydoc IFrameBuffer::operator[](StringView) const
        inline const IDirectX12Image& operator[](StringView renderTargetName) const override {
            return this->resolveImage(hash(renderTargetName));
        }
        
        /// @copydoc IFrameBuffer::image(StringView) const
        inline const IDirectX12Image& image(StringView renderTargetName) const override {
            return this->resolveImage(hash(renderTargetName));
        }
        
        /// @copydoc IFrameBuffer::resolveImage
        const IDirectX12Image& resolveImage(UInt64 hash) const override;

        /// @copydoc IFrameBuffer::addImage(const String&, Format, MultiSamplingLevel, ResourceUsage)
        void addImage(const String& name, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) override;

        /// @copydoc IFrameBuffer::addImage(const String&, const RenderTarget&, MultiSamplingLevel, ResourceUsage)
        void addImage(const String& name, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) override;

        /// @copydoc IFrameBuffer::resize
        void resize(const Size2d& renderArea) override;
    };

    /// @brief Implements a DirectX 12 render pass.
    ///
    /// @see DirectX12RenderPassBuilder
    class LITEFX_DIRECTX12_API DirectX12RenderPass final : public RenderPass<DirectX12Queue, DirectX12FrameBuffer> {
        LITEFX_IMPLEMENTATION(DirectX12RenderPassImpl);
        LITEFX_BUILDER(DirectX12RenderPassBuilder);
        friend struct SharedObject::Allocator<DirectX12RenderPass>;

    public:
        using base_type = RenderPass<DirectX12Queue, DirectX12FrameBuffer>;

    private:
        /// @brief Creates and initializes a new DirectX 12 render pass instance that executes on the default graphics queue.
        ///
        /// @param device The parent device instance.
        /// @param commandBuffers The number of command buffers in each frame buffer.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        explicit DirectX12RenderPass(const DirectX12Device& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000);

        /// @brief Creates and initializes a new DirectX 12 render pass instance that executes on the default graphics queue.
        ///
        /// @param device The parent device instance.
        /// @param name The name of the render pass state resource.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param commandBuffers The number of command buffers in each frame buffer.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        explicit DirectX12RenderPass(const DirectX12Device& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000);

        /// @brief Creates and initializes a new DirectX 12 render pass instance.
        ///
        /// @param device The parent device instance.
        /// @param queue The command queue to execute the render pass on.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param commandBuffers The number of command buffers in each frame buffer.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        explicit DirectX12RenderPass(const DirectX12Device& device, const DirectX12Queue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000);

        /// @brief Creates and initializes a new DirectX 12 render pass instance.
        ///
        /// @param device The parent device instance.
        /// @param name The name of the render pass state resource.
        /// @param queue The command queue to execute the render pass on.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param commandBuffers The number of command buffers in each frame buffer.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        explicit DirectX12RenderPass(const DirectX12Device& device, const String& name, const DirectX12Queue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000);

    private:
        /// @copydoc RenderPass::RenderPass(RenderPass&&)
        DirectX12RenderPass(DirectX12RenderPass&&) noexcept = delete;

        /// @copydoc RenderPass::RenderPass(const RenderPass&)
        DirectX12RenderPass(const DirectX12RenderPass&) = delete;

        /// @copydoc RenderPass::operator=(RenderPass&&)
        DirectX12RenderPass& operator=(DirectX12RenderPass&&) noexcept = delete;

        /// @copydoc RenderPass::operator=(const RenderPass&)
        DirectX12RenderPass& operator=(const DirectX12RenderPass&) = delete;

    public:
        /// @copydoc RenderPass::~RenderPass
        ~DirectX12RenderPass() noexcept override;

    public:
        /// @brief Creates and initializes a new DirectX 12 render pass instance that executes on the default graphics queue.
        ///
        /// @param device The parent device instance.
        /// @param commandBuffers The number of command buffers in each frame buffer.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<DirectX12RenderPass> create(const DirectX12Device& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000) {
            return SharedObject::create<DirectX12RenderPass>(device, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask);
        }

        /// @brief Creates and initializes a new DirectX 12 render pass instance that executes on the default graphics queue.
        ///
        /// @param device The parent device instance.
        /// @param name The name of the render pass state resource.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param commandBuffers The number of command buffers in each frame buffer.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<DirectX12RenderPass> create(const DirectX12Device& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000) {
            return SharedObject::create<DirectX12RenderPass>(device, name, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask);
        }

        /// @brief Creates and initializes a new DirectX 12 render pass instance.
        ///
        /// @param device The parent device instance.
        /// @param queue The command queue to execute the render pass on.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param commandBuffers The number of command buffers in each frame buffer.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<DirectX12RenderPass> create(const DirectX12Device& device, const DirectX12Queue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000) {
            return SharedObject::create<DirectX12RenderPass>(device, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask);
        }

        /// @brief Creates and initializes a new DirectX 12 render pass instance.
        ///
        /// @param device The parent device instance.
        /// @param name The name of the render pass state resource.
        /// @param queue The command queue to execute the render pass on.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param commandBuffers The number of command buffers in each frame buffer.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<DirectX12RenderPass> create(const DirectX12Device& device, const String& name, const DirectX12Queue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000) {
            return SharedObject::create<DirectX12RenderPass>(device, name, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask);
        }

    private:
        /// @brief Creates an uninitialized DirectX 12 render pass instance.
        ///
        /// @param device The parent device of the render pass.
        /// @param name The name of the render pass state resource.
        explicit DirectX12RenderPass(const DirectX12Device& device, const String& name = "");

        /// @brief Creates an uninitialized DirectX 12 render pass instance.
        ///
        /// This factory is called by the @ref DirectX12RenderPassBuilder in order to create a render pass instance without initializing it. The instance is only initialized after calling @ref
        /// DirectX12RenderPassBuilder::go.
        ///
        /// @param device The parent device of the render pass.
        /// @param name The name of the render pass state resource.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<DirectX12RenderPass> create(const DirectX12Device& device, const String& name = "") {
            return SharedObject::create<DirectX12RenderPass>(device, name);
        }

        // RenderPass interface.
    public:
        /// @brief Returns a reference of the device that provides this queue.
        ///
        /// @return A reference of the queue's parent device.
        const DirectX12Device& device() const noexcept /*override*/;

        /// @copydoc RenderPass::activeFrameBuffer
        SharedPtr<const DirectX12FrameBuffer> activeFrameBuffer() const noexcept override;

        /// @copydoc RenderPass::commandQueue
        const DirectX12Queue& commandQueue() const noexcept override;

        /// @copydoc RenderPass::commandBuffers
        Enumerable<SharedPtr<const DirectX12CommandBuffer>> commandBuffers() const override;

        /// @copydoc RenderPass::commandBuffer
        SharedPtr<const DirectX12CommandBuffer> commandBuffer(UInt32 index) const override;

        /// @copydoc IRenderPass::secondaryCommandBuffers
        UInt32 secondaryCommandBuffers() const noexcept override;

        /// @copydoc IRenderPass::renderTargets
        const Array<RenderTarget>& renderTargets() const noexcept override;

        /// @copydoc IRenderPass::renderTarget
        const RenderTarget& renderTarget(UInt32 location) const override;

        /// @copydoc IRenderPass::hasPresentTarget
        bool hasPresentTarget() const noexcept override;

        /// @copydoc IRenderPass::inputAttachments
        const Array<RenderPassDependency>& inputAttachments() const noexcept override;

        /// @copydoc IRenderPass::inputAttachment
        const RenderPassDependency& inputAttachment(UInt32 location) const override;

        /// @copydoc IRenderPass::inputAttachmentSamplerBinding
        const Optional<DescriptorBindingPoint>& inputAttachmentSamplerBinding() const noexcept override;

        /// @copydoc RenderPass::begin
        void begin(const DirectX12FrameBuffer& frameBuffer) const override;

        /// @copydoc IRenderPass::end
        UInt64 end() const override;

        /// @copydoc IRenderPass::viewMask
        UInt32 viewMask() const noexcept override;
    };

    /// @brief Implements a DirectX 12 swap chain.
    class LITEFX_DIRECTX12_API DirectX12SwapChain final : public SwapChain<IDirectX12Image>, public ComResource<IDXGISwapChain4> {
        LITEFX_IMPLEMENTATION(DirectX12SwapChainImpl);
        friend class DirectX12RenderPass;
        friend class DirectX12Image;
        friend class DirectX12Device;

    public:
        using base_type = SwapChain<IDirectX12Image>;

    private:
        /// @brief Initializes a DirectX 12 swap chain.
        ///
        /// @param device The device that owns the swap chain.
        /// @param backend The backend, the swap chain is initialized on.
        /// @param format The initial surface format.
        /// @param renderArea The initial size of the render area.
        /// @param enableVsync `true` if vertical synchronization should be used, otherwise `false`.
        /// @param buffers The initial number of buffers.
        explicit DirectX12SwapChain(const DirectX12Device& device, const DirectX12Backend& backend, Format surfaceFormat = Format::B8G8R8A8_SRGB, const Size2d& renderArea = { 800, 600 }, UInt32 buffers = 3, bool enableVsync = false); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    public:
        /// @copydoc SwapChain::SwapChain(SwapChain&&)
        DirectX12SwapChain(DirectX12SwapChain&&) noexcept = delete;

        /// @copydoc SwapChain::SwapChain(const SwapChain&)
        DirectX12SwapChain(const DirectX12SwapChain&) = delete;

        /// @copydoc SwapChain::operator=(SwapChain&&)
        DirectX12SwapChain& operator=(DirectX12SwapChain&&) noexcept = delete;

        /// @copydoc SwapChain::operator=(const SwapChain&)
        DirectX12SwapChain& operator=(const DirectX12SwapChain&) = delete;

        /// @copydoc SwapChain::~SwapChain
        ~DirectX12SwapChain() noexcept override;

        // DirectX 12 swap chain.
    public:
        /// @brief Returns `true`, if the adapter supports variable refresh rates (i.e. tearing is allowed).
        ///
        /// @return `true`, if the adapter supports variable refresh rates (i.e. tearing is allowed).
        bool supportsVariableRefreshRate() const noexcept;

        /// @brief Returns the query heap for the current frame.
        ///
        /// @return A pointer to the query heap for the current frame.
        ID3D12QueryHeap* timestampQueryHeap() const noexcept;

        // SwapChain interface.
    public:
        /// @copydoc ISwapChain::timingEvents
        const Array<SharedPtr<const TimingEvent>>& timingEvents() const override;

        /// @copydoc ISwapChain::timingEvent
        SharedPtr<const TimingEvent> timingEvent(UInt32 queryId) const override;

        /// @copydoc ISwapChain::readTimingEvent
        UInt64 readTimingEvent(SharedPtr<const TimingEvent> timingEvent) const override;

        /// @copydoc ISwapChain::resolveQueryId
        UInt32 resolveQueryId(SharedPtr<const TimingEvent> timingEvent) const override;

        /// @copydoc ISwapChain::device
        const IGraphicsDevice& device() const override;

        /// @copydoc ISwapChain::surfaceFormat
        Format surfaceFormat() const noexcept override;

        /// @copydoc ISwapChain::buffers
        UInt32 buffers() const noexcept override;

        /// @copydoc ISwapChain::renderArea
        const Size2d& renderArea() const noexcept override;

        /// @copydoc ISwapChain::verticalSynchronization
        bool verticalSynchronization() const noexcept override;

        /// @copydoc ISwapChain::image(UInt32) const
        IDirectX12Image* image(UInt32 backBuffer) const override;

        /// @copydoc ISwapChain::image() const
        const IDirectX12Image& image() const noexcept override;

        /// @copydoc SwapChain::images
        const Array<SharedPtr<IDirectX12Image>>& images() const noexcept override;

        /// @copydoc ISwapChain::present
        void present(UInt64 fence) const override;

    public:
        /// @copydoc ISwapChain::getSurfaceFormats
        Enumerable<Format> getSurfaceFormats() const override;

        /// @copydoc ISwapChain::addTimingEvent
        void addTimingEvent(SharedPtr<const TimingEvent> timingEvent) override;

        /// @copydoc ISwapChain::reset
        void reset(Format surfaceFormat, const Size2d& renderArea, UInt32 buffers, bool enableVsync = false) override;

        /// @copydoc ISwapChain::swapBackBuffer
        [[nodiscard]] UInt32 swapBackBuffer() const override;

    private:
        void resolveQueryHeaps(const DirectX12CommandBuffer& commandBuffer) const noexcept;
    };

    /// @brief A graphics factory that produces objects for a @ref DirectX12Device.
    ///
    /// The DX12 graphics factory is implemented using [D3D12 Memory Allocator](https://gpuopen.com/d3d12-memory-allocator/).
    class LITEFX_DIRECTX12_API DirectX12GraphicsFactory final : public GraphicsFactory<DirectX12DescriptorLayout, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, IDirectX12Sampler, DirectX12BottomLevelAccelerationStructure, DirectX12TopLevelAccelerationStructure> {
        LITEFX_IMPLEMENTATION(DirectX12GraphicsFactoryImpl);
        friend class DirectX12Device;
        friend struct SharedObject::Allocator<DirectX12GraphicsFactory>;

    public:
        using base_type = GraphicsFactory<DirectX12DescriptorLayout, IDirectX12Buffer, IDirectX12VertexBuffer, IDirectX12IndexBuffer, IDirectX12Image, IDirectX12Sampler, DirectX12BottomLevelAccelerationStructure, DirectX12TopLevelAccelerationStructure>;
        using base_type::createBuffer;
        using base_type::tryCreateBuffer;
        using base_type::createVertexBuffer;
        using base_type::tryCreateVertexBuffer;
        using base_type::createIndexBuffer;
        using base_type::tryCreateIndexBuffer;
        using base_type::createTexture;
        using base_type::tryCreateTexture;
        using base_type::createTextures;
        using base_type::createSampler;
        using base_type::createSamplers;
        using base_type::allocate;

    private:
        /// @brief Creates a new graphics factory.
        ///
        /// @param device The device the factory should produce objects for.
        explicit DirectX12GraphicsFactory(const DirectX12Device& device);

    private:
        /// @copydoc GraphicsFactory::GraphicsFactory(GraphicsFactory&&)
        DirectX12GraphicsFactory(DirectX12GraphicsFactory&&) noexcept = delete;
        
        /// @copydoc GraphicsFactory::GraphicsFactory(const GraphicsFactory&)
        DirectX12GraphicsFactory(const DirectX12GraphicsFactory&) = delete;

        /// @copydoc GraphicsFactory::operator=(GraphicsFactory&&)
        DirectX12GraphicsFactory& operator=(DirectX12GraphicsFactory&&) noexcept = delete;

        /// @copydoc GraphicsFactory::operator=(const GraphicsFactory&)
        DirectX12GraphicsFactory& operator=(const DirectX12GraphicsFactory&) = delete;

    public:
        /// @copydoc GraphicsFactory::~GraphicsFactory
        ~DirectX12GraphicsFactory() noexcept override;

    private:
        /// @brief Creates a new graphics factory.
        ///
        /// @param device The device the factory should produce objects for.
        static inline SharedPtr<DirectX12GraphicsFactory> create(const DirectX12Device& device) {
            return SharedObject::create<DirectX12GraphicsFactory>(device);
        }

    public:
        /// @copydoc IGraphicsFactory::createAllocator
        [[nodiscard]] VirtualAllocator createAllocator(UInt64 overallMemory, AllocationAlgorithm algorithm = AllocationAlgorithm::Default) const override;

        /// @copydoc IGraphicsFactory::beginDefragmentation
        void beginDefragmentation(const ICommandQueue& queue, DefragmentationStrategy strategy = DefragmentationStrategy::Balanced, UInt64 maxBytesToMove = 0u, UInt32 maxAllocationsToMove = 0u) const override;

        /// @copydoc IGraphicsFactory::beginDefragmentationPass
        UInt64 beginDefragmentationPass() const override;

        /// @copydoc IGraphicsFactory::endDefragmentationPass
        bool endDefragmentationPass() const override;

        /// @copydoc IGraphicsFactory::allocate(Enumerable<const ResourceAllocationInfo&>, AllocationBehavior, bool) const
        Generator<ResourceAllocationResult> allocate(Enumerable<const ResourceAllocationInfo&> allocationInfos, AllocationBehavior allocationBehavior = AllocationBehavior::Default, bool alias = false) const override;

        /// @copydoc IGraphicsFactory::canAlias
        bool canAlias(Enumerable<const ResourceAllocationInfo&> allocationInfos) const override;

        /// @copydoc GraphicsFactory::createBuffer(BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IDirectX12Buffer> createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createBuffer(const String&, BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IDirectX12Buffer> createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createVertexBuffer(const vertex_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IDirectX12VertexBuffer> createVertexBuffer(const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createVertexBuffer(const String&, const vertex_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IDirectX12VertexBuffer> createVertexBuffer(const String& name, const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createIndexBuffer(const index_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IDirectX12IndexBuffer> createIndexBuffer(const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createIndexBuffer(const String&, const index_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IDirectX12IndexBuffer> createIndexBuffer(const String& name, const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createTexture(Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior) const
        SharedPtr<IDirectX12Image> createTexture(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createTexture(const String&, Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior) const
        SharedPtr<IDirectX12Image> createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateBuffer(SharedPtr<TBuffer>&, BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateBuffer(SharedPtr<IDirectX12Buffer>& buffer, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateBuffer(SharedPtr<TBuffer>&, const String&, BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateBuffer(SharedPtr<IDirectX12Buffer>& buffer, const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateVertexBuffer(SharedPtr<TVertexBuffer>&, const vertex_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateVertexBuffer(SharedPtr<IDirectX12VertexBuffer>& buffer, const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateVertexBuffer(SharedPtr<TVertexBuffer>&, const String&, const vertex_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateVertexBuffer(SharedPtr<IDirectX12VertexBuffer>& buffer, const String& name, const DirectX12VertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateIndexBuffer(SharedPtr<TIndexBuffer>&, const index_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateIndexBuffer(SharedPtr<IDirectX12IndexBuffer>& buffer, const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateIndexBuffer(SharedPtr<TIndexBuffer>&, const String&, const index_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateIndexBuffer(SharedPtr<IDirectX12IndexBuffer>& buffer, const String& name, const DirectX12IndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateTexture(SharedPtr<TImage>&, Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior) const
        bool tryCreateTexture(SharedPtr<IDirectX12Image>& image, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateTexture(SharedPtr<TImage>&, const String&, Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior) const
        bool tryCreateTexture(SharedPtr<IDirectX12Image>& image, const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createTextures
        Generator<SharedPtr<IDirectX12Image>> createTextures(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createSampler(FilterMode, FilterMode, BorderMode, BorderMode, BorderMode, MipMapMode, Float, Float, Float, Float) const
        SharedPtr<IDirectX12Sampler> createSampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// @copydoc GraphicsFactory::createSampler(const String&, FilterMode, FilterMode, BorderMode, BorderMode, BorderMode, MipMapMode, Float, Float, Float, Float) const
        SharedPtr<IDirectX12Sampler> createSampler(const String& name, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// @copydoc GraphicsFactory::createSamplers
        Generator<SharedPtr<IDirectX12Sampler>> createSamplers(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// @copydoc GraphicsFactory::createBottomLevelAccelerationStructure(StringView, AccelerationStructureFlags) const
        UniquePtr<DirectX12BottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const override;

        /// @copydoc GraphicsFactory::createTopLevelAccelerationStructure(StringView, AccelerationStructureFlags) const
        UniquePtr<DirectX12TopLevelAccelerationStructure> createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const override;

        /// @copydoc IGraphicsFactory::supportsResizableBaseAddressRegister
        bool supportsResizableBaseAddressRegister() const noexcept override;

        /// @copydoc IGraphicsFactory::memoryStatistics
        Array<MemoryHeapStatistics> memoryStatistics() const override;

        /// @copydoc IGraphicsFactory::detailedMemoryStatistics
        DetailedMemoryStatistics detailedMemoryStatistics() const override;
    };

    /// @brief Implements a DirectX 12 graphics device.
    class LITEFX_DIRECTX12_API DirectX12Device final : public GraphicsDevice<DirectX12GraphicsFactory, DirectX12Surface, DirectX12GraphicsAdapter, DirectX12SwapChain, DirectX12Queue, DirectX12RenderPass, DirectX12RenderPipeline, DirectX12ComputePipeline, DirectX12RayTracingPipeline, DirectX12Barrier>, public ComResource<ID3D12Device10> {
        LITEFX_IMPLEMENTATION(DirectX12DeviceImpl);
        friend struct SharedObject::Allocator<DirectX12Device>;
        friend class DirectX12Backend;

    private:
        /// @brief Creates a new device instance.
        ///
        /// @param adapter The adapter the device uses for drawing.
        /// @param surface The surface, the device should draw to.
        /// @param globalBufferHeapSize The size of the global heap for constant buffers, shader resources and images.
        /// @param globalSamplerHeapSize The size of the global heap for samplers.
        explicit DirectX12Device(const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, UInt32 globalBufferHeapSize = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1, UInt32 globalSamplerHeapSize = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE);

    private:
        /// @copydoc GraphicsDevice::GraphicsDevice(GraphicsDevice&&)
        DirectX12Device(DirectX12Device&&) noexcept = delete;

        /// @copydoc GraphicsDevice::GraphicsDevice(const GraphicsDevice&)
        DirectX12Device(const DirectX12Device&) = delete;

        /// @copydoc GraphicsDevice::operator=(GraphicsDevice&&)
        DirectX12Device& operator=(DirectX12Device&&) noexcept = delete;

        /// @copydoc GraphicsDevice::operator=(const GraphicsDevice&)
        DirectX12Device& operator=(const DirectX12Device&) = delete;
        
    public:
        /// @copydoc GraphicsDevice::~GraphicsDevice
        ~DirectX12Device() noexcept override;

        // Factory methods.
    public:
        /// @brief Initializes the device instance.
        ///
        /// @param backend The backend from which the device got created.
        /// @param backend The backend from which the device got created.
        /// @param adapter The adapter the device uses for drawing.
        /// @param surface The surface, the device should draw to.
        /// @param features The features that should be supported by this device.
        /// @return A shared pointer to the new device instance.
        static inline SharedPtr<DirectX12Device> create(const DirectX12Backend& backend, const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, GraphicsDeviceFeatures features = {}) {
            return SharedObject::create<DirectX12Device>(adapter, std::move(surface))->initialize(backend, Format::B8G8R8A8_SRGB, { 800, 600 }, 3, false, features); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        }

        /// @brief Initializes the device instance.
        ///
        /// @param backend The backend from which the device got created.
        /// @param adapter The adapter the device uses for drawing.
        /// @param surface The surface, the device should draw to.
        /// @param format The initial surface format, device uses for drawing.
        /// @param renderArea The initial size of the render area.
        /// @param backBuffers The initial number of back buffers.
        /// @param enableVsync The initial setting for vertical synchronization.
        /// @param features The features that should be supported by this device.
        /// @param globalBufferHeapSize The size of the global heap for constant buffers, shader resources and images.
        /// @param globalSamplerHeapSize The size of the global heap for samplers.
        /// @return A shared pointer to the new device instance.
        static inline SharedPtr<DirectX12Device> create(const DirectX12Backend& backend, const DirectX12GraphicsAdapter& adapter, UniquePtr<DirectX12Surface>&& surface, Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync = false, GraphicsDeviceFeatures features = {}, UInt32 globalBufferHeapSize = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1, UInt32 globalSamplerHeapSize = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE) {
            return SharedObject::create<DirectX12Device>(adapter, std::move(surface), globalBufferHeapSize, globalSamplerHeapSize)->initialize(backend, format, renderArea, backBuffers, enableVsync, features);
        }

    private:
        /// @brief Initializes the resources owned by the device.
        ///
        /// @param backend The backend from which the device got created.
        /// @param format The initial surface format, device uses for drawing.
        /// @param renderArea The initial size of the render area.
        /// @param backBuffers The initial number of back buffers.
        /// @param enableVsync The initial setting for vertical synchronization.
        /// @param features The features that should be supported by this device.
        /// @return A shared pointer to the current device instance.
        SharedPtr<DirectX12Device> initialize(const DirectX12Backend& backend, Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync, GraphicsDeviceFeatures features);

        /// @brief Releases the device state to prepare it for destruction.
        void release() noexcept;

        // DirectX 12 Device interface.
    public:
        /// @brief Returns the global descriptor heap.
        ///
        /// The DirectX 12 device uses a global heap of descriptors and samplers in a ring-buffer fashion. The heap itself is managed by the device.
        ///
        /// @return A pointer to the global descriptor heap.
        ID3D12DescriptorHeap* globalBufferHeap() const noexcept;

        /// @brief Returns the global sampler heap.
        ///
        /// @return A pointer to the global sampler heap.
        /// @see globalBufferHeap
        ID3D12DescriptorHeap* globalSamplerHeap() const noexcept;

        /// @brief Returns the command signatures for indirect dispatch and draw calls.
        ///
        /// @param dispatchSignature The command signature used to execute indirect dispatches.
        /// @param dispatchMeshSignature The command signature used to execute indirect mesh shader dispatches.
        /// @param drawSignature The command signature used to execute indirect non-indexed draw calls.
        /// @param drawIndexedSignature The command signature used to execute indirect indexed draw calls.
        void indirectDrawSignatures(ComPtr<ID3D12CommandSignature>& dispatchSignature, ComPtr<ID3D12CommandSignature>& dispatchMeshSignature, ComPtr<ID3D12CommandSignature>& drawSignature, ComPtr<ID3D12CommandSignature>& drawIndexedSignature) const noexcept;

        /// @brief Allocates a @p descriptors on the descriptor heap indicated by <paramref name=heapType" /> for use with external clients.
        ///
        /// This call allocates a descriptor range on one of the global descriptor heaps, that can be used with external clients and libraries. Keep in mind that you have to release those descriptors manually by
        /// calling the appropriate overload to @ref releaseGlobalDescriptors. The following example demonstrates how to use this function.
        ///
        /// @par Example
        /// auto allocation = d3dDevice.allocateGlobalDescriptors(1000, DescriptorHeapType::Resource); // Use the descriptors. d3dDevice.releaseGlobalDescriptors(DescriptorHeapType::Resource,
        /// std::move(allocation));
        ///
        /// @param descriptors The number of descriptors to allocate.
        /// @param heapType The heap type, indicating the descriptor heap to allocate the descriptors from.
        /// @return The allocation for the requested descriptors in the descriptor heap indicated by @p heapType.
        [[nodiscard]] VirtualAllocator::Allocation allocateGlobalDescriptors(UInt32 descriptors, DescriptorHeapType heapType) const;

        /// @brief Releases a manually allocated descriptor range from the descriptor heap indicated by @p heapType.
        ///
        /// Calling this method with an allocation that has not been allocated from the corresponding resource heap of the same device instance is undefined behavior.
        ///
        /// @param heapType The heap type, indicating the descriptor heap to release the descriptors from.
        /// @param allocation The allocation to release.
        /// @see allocateGlobalDescriptors(descriptors, heapType)
        void releaseGlobalDescriptors(DescriptorHeapType heapType, VirtualAllocator::Allocation&& allocation) const;

        // GraphicsDevice interface.
    public:
        /// @copydoc IGraphicsDevice::state
        DeviceState& state() const noexcept override;

        /// @copydoc GraphicsDevice::swapChain() const
        const DirectX12SwapChain& swapChain() const noexcept override;

        /// @copydoc GraphicsDevice::swapChain() const
        DirectX12SwapChain& swapChain() noexcept override;

        /// @copydoc GraphicsDevice::surface
        const DirectX12Surface& surface() const noexcept override;

        /// @copydoc GraphicsDevice::adapter
        const DirectX12GraphicsAdapter& adapter() const noexcept override;

        /// @copydoc GraphicsDevice::factory
        const DirectX12GraphicsFactory& factory() const noexcept override;

        /// @copydoc GraphicsDevice::defaultQueue
        const DirectX12Queue& defaultQueue(QueueType type) const override;

        /// @copydoc GraphicsDevice::createQueue
        SharedPtr<const DirectX12Queue> createQueue(QueueType type, QueuePriority priority) override;

        /// @copydoc GraphicsDevice::makeBarrier
        [[nodiscard]] UniquePtr<DirectX12Barrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const override;

        /// @copydoc GraphicsDevice::makeFrameBuffer(StringView, const Size2d&) const
        [[nodiscard]] SharedPtr<DirectX12FrameBuffer> makeFrameBuffer(StringView name, const Size2d& renderArea) const override;

        /// @copydoc GraphicsDevice::makeFrameBuffer(StringView, const Size2d&, frame_buffer_type::allocation_callback_type) const
        [[nodiscard]] SharedPtr<DirectX12FrameBuffer> makeFrameBuffer(StringView name, const Size2d& renderArea, DirectX12FrameBuffer::allocation_callback_type allocationCallback) const override;

        /// @copydoc IGraphicsDevice::maximumMultiSamplingLevel
        ///
        /// @see https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
        MultiSamplingLevel maximumMultiSamplingLevel(Format format) const override;

        /// @copydoc IGraphicsDevice::ticksPerMillisecond
        double ticksPerMillisecond() const noexcept override;

        /// @copydoc IGraphicsDevice::wait
        void wait() const override;

        /// @copydoc GraphicsDevice::computeAccelerationStructureSizes(const bottom_level_acceleration_structure_type&, UInt64&, UInt64&, bool) const
        void computeAccelerationStructureSizes(const DirectX12BottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const override;

        /// @copydoc GraphicsDevice::computeAccelerationStructureSizes(const top_level_acceleration_structure_type&, UInt64&, UInt64&, bool) const
        void computeAccelerationStructureSizes(const DirectX12TopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const override;

        /// @copydoc GraphicsDevice::allocateGlobalDescriptors
        [[nodiscard]] VirtualAllocator::Allocation allocateGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet, DescriptorHeapType heapType) const override;

        /// @copydoc GraphicsDevice::releaseGlobalDescriptors
        void releaseGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet) const override;

        /// @copydoc GraphicsDevice::updateGlobalDescriptors
        void updateGlobalDescriptors(const DirectX12DescriptorSet& descriptorSet, UInt32 binding, UInt32 offset, UInt32 descriptors) const override;

        /// @copydoc GraphicsDevice::bindDescriptorSet
        void bindDescriptorSet(const DirectX12CommandBuffer& commandBuffer, const DirectX12DescriptorSet& descriptorSet, const DirectX12PipelineState& pipeline) const override;

        /// @copydoc GraphicsDevice::bindGlobalDescriptorHeaps
        void bindGlobalDescriptorHeaps(const DirectX12CommandBuffer& commandBuffer) const noexcept override;

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
    public:
        /// @copydoc GraphicsDevice::buildRenderPass(UInt32) const
        [[nodiscard]] DirectX12RenderPassBuilder buildRenderPass(UInt32 commandBuffers = 1) const override;

        /// @copydoc GraphicsDevice::buildRenderPass(const String&, UInt32) const
        [[nodiscard]] DirectX12RenderPassBuilder buildRenderPass(const String& name, UInt32 commandBuffers = 1) const override;

        ///// <inheritdoc cref="GraphicsDevice::buildRenderPipeline(const String&) const" />
        //[[nodiscard]] DirectX12RenderPipelineBuilder buildRenderPipeline(const String& name) const override;

        /// @copydoc GraphicsDevice::buildRenderPipeline
        [[nodiscard]] DirectX12RenderPipelineBuilder buildRenderPipeline(const DirectX12RenderPass& renderPass, const String& name) const override;

        /// @copydoc GraphicsDevice::buildComputePipeline
        [[nodiscard]] DirectX12ComputePipelineBuilder buildComputePipeline(const String& name) const override;

        /// @copydoc GraphicsDevice::buildRayTracingPipeline(ShaderRecordCollection&&) const
        [[nodiscard]] DirectX12RayTracingPipelineBuilder buildRayTracingPipeline(ShaderRecordCollection&& shaderRecords) const override;

        /// @copydoc GraphicsDevice::buildRayTracingPipeline(const String&, ShaderRecordCollection&&) const
        [[nodiscard]] DirectX12RayTracingPipelineBuilder buildRayTracingPipeline(const String& name, ShaderRecordCollection&& shaderRecords) const override;
        
        /// @copydoc GraphicsDevice::buildPipelineLayout
        [[nodiscard]] DirectX12PipelineLayoutBuilder buildPipelineLayout() const override;

        /// @copydoc GraphicsDevice::buildInputAssembler
        [[nodiscard]] DirectX12InputAssemblerBuilder buildInputAssembler() const override;

        /// @copydoc GraphicsDevice::buildRasterizer
        [[nodiscard]] DirectX12RasterizerBuilder buildRasterizer() const override;

        /// @copydoc GraphicsDevice::buildShaderProgram
        [[nodiscard]] DirectX12ShaderProgramBuilder buildShaderProgram() const override;

        /// @copydoc GraphicsDevice::buildBarrier
        [[nodiscard]] DirectX12BarrierBuilder buildBarrier() const override;
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)
    };
    
    /// @brief Implements the DirectX 12 @ref RenderBackend.
    class LITEFX_DIRECTX12_API DirectX12Backend final : public RenderBackend<DirectX12Device>, public ComResource<IDXGIFactory7> {
        LITEFX_IMPLEMENTATION(DirectX12BackendImpl);

    public:
        explicit DirectX12Backend(const App& app, bool advancedSoftwareRasterizer = false);

        /// @copydoc RenderBackend::RenderBackend(RenderBackend&&)
        DirectX12Backend(DirectX12Backend&&) noexcept;

        /// @copydoc RenderBackend::RenderBackend(const RenderBackend&)
        DirectX12Backend(const DirectX12Backend&) = delete;

        /// @copydoc RenderBackend::operator=(RenderBackend&&)
        DirectX12Backend& operator=(DirectX12Backend&&) noexcept;

        /// @copydoc RenderBackend::operator=(const RenderBackend&)
        DirectX12Backend& operator=(const DirectX12Backend&) = delete;

        /// @copydoc RenderBackend::~RenderBackend
        ~DirectX12Backend() noexcept override;

        // IBackend interface.
    public:
        /// @copydoc IBackend::type
        BackendType type() const noexcept override;

        /// @copydoc IBackend::name
        StringView name() const noexcept override;

    protected:
        /// @copydoc IBackend::activate
        void activate() override;

        /// @copydoc IBackend::deactivate
        void deactivate() override;

        // RenderBackend interface.
    public:
        /// @copydoc RenderBackend::adapters
        const Array<SharedPtr<const DirectX12GraphicsAdapter>>& adapters() const override;

        /// @copydoc RenderBackend::findAdapter(const Optional<UInt64>&) const
        const DirectX12GraphicsAdapter* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const override;

        /// @copydoc RenderBackend::findAdapter(GpuPreference) const
        const DirectX12GraphicsAdapter* findAdapter(GpuPreference preference) const override;

        /// @copydoc RenderBackend::registerDevice
        void registerDevice(const String& name, SharedPtr<DirectX12Device>&& device) override;

        /// @copydoc RenderBackend::releaseDevice
        void releaseDevice(const String& name) override;

        /// @copydoc RenderBackend::device(const String&)
        DirectX12Device* device(const String& name) override;

        /// @copydoc RenderBackend::device(const String&)
        const DirectX12Device* device(const String& name) const override;

    public:
        /// @brief Creates a surface on a window handle.
        ///
        /// @param hwnd The window handle on which the surface should be created.
        /// @return The instance of the created surface.
        UniquePtr<DirectX12Surface> createSurface(const HWND& hwnd) const;

        /// @brief Enables [Windows Advanced Software Rasterization (WARP)](https://docs.microsoft.com/en-us/windows/win32/direct3darticles/directx-warp).
        ///
        /// Enabling software rasterization disables hardware rasterization. Requesting adapters using @ref findAdapter or @ref listAdapters will only return WARP-compatible adapters.
        ///
        /// @param enable `true`, if advanced software rasterization should be used.
        void enableAdvancedSoftwareRasterizer(bool enable = false);
    };

}

// NOLINTEND(bugprone-derived-method-shadowing-base-method)

#pragma warning(pop)