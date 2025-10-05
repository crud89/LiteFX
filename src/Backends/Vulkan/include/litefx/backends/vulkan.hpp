#pragma once

#include <litefx/rendering.hpp>

#include "vulkan_api.hpp"
#include "vulkan_formatters.hpp"

#pragma warning(push)
#pragma warning(disable:4250) // Base class members are inherited via dominance.

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Math;
    using namespace LiteFX::Rendering;

    /// <summary>
    /// Implements a Vulkan vertex buffer layout.
    /// </summary>
    /// <seealso cref="VulkanVertexBuffer" />
    /// <seealso cref="VulkanIndexBufferLayout" />
    /// <seealso cref="VulkanVertexBufferLayoutBuilder" />
    class LITEFX_VULKAN_API VulkanVertexBufferLayout final : public IVertexBufferLayout {
        LITEFX_IMPLEMENTATION(VulkanVertexBufferLayoutImpl);
        LITEFX_BUILDER(VulkanVertexBufferLayoutBuilder);
        friend struct SharedObject::Allocator<VulkanVertexBufferLayout>;

    private:
        /// <summary>
        /// Initializes a new vertex buffer layout.
        /// </summary>
        /// <param name="vertexSize">The overall size of a single vertex.</param>
        /// <param name="binding">The binding point of the vertex buffers using this layout.</param>
        explicit VulkanVertexBufferLayout(size_t vertexSize, UInt32 binding = 0);

        /// <summary>
        /// Initializes a new vertex buffer layout.
        /// </summary>
        /// <param name="vertexSize">The overall size of a single vertex.</param>
        /// <param name="attributes">The vertex attributes.</param>
        /// <param name="binding">The binding point of the vertex buffers using this layout.</param>
        explicit VulkanVertexBufferLayout(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding = 0);

    private:
        /// <inheritdoc />
        VulkanVertexBufferLayout(VulkanVertexBufferLayout&&) noexcept = delete;

        /// <inheritdoc />
        VulkanVertexBufferLayout(const VulkanVertexBufferLayout&);

        /// <inheritdoc />
        VulkanVertexBufferLayout& operator=(VulkanVertexBufferLayout&&) noexcept = delete;

        /// <inheritdoc />
        VulkanVertexBufferLayout& operator=(const VulkanVertexBufferLayout&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanVertexBufferLayout() noexcept override;

    public:
        /// <summary>
        /// Creates a new vertex buffer layout.
        /// </summary>
        /// <param name="vertexSize">The overall size of a single vertex.</param>
        /// <param name="binding">The binding point of the vertex buffers using this layout.</param>
        /// <returns>A shared pointer to the newly created vertex buffer layout.</returns>
        static inline auto create(size_t vertexSize, UInt32 binding = 0) {
            return SharedObject::create<VulkanVertexBufferLayout>(vertexSize, binding);
        }

        /// <summary>
        /// Creates a new vertex buffer layout.
        /// </summary>
        /// <param name="vertexSize">The overall size of a single vertex.</param>
        /// <param name="binding">The binding point of the vertex buffers using this layout.</param>
        /// <param name="attributes">The vertex attributes.</param>
        /// <returns>A shared pointer to the newly created vertex buffer layout.</returns>
        static inline auto create(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding = 0) {
            return SharedObject::create<VulkanVertexBufferLayout>(vertexSize, attributes, binding);
        }

        /// <summary>
        /// Creates a copy of a vertex buffer layout.
        /// </summary>
        /// <param name="other">The vertex buffer layout to copy.</param>
        /// <returns>A shared pointer to the newly created vertex buffer layout.</returns>
        static inline auto create(const VulkanVertexBufferLayout& other) {
            return SharedObject::create<VulkanVertexBufferLayout>(other);
        }

        // IVertexBufferLayout interface.
    public:
        /// <inheritdoc />
        const Array<BufferAttribute>& attributes() const override;

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
    /// Implements a Vulkan index buffer layout.
    /// </summary>
    /// <seealso cref="VulkanIndexBuffer" />
    /// <seealso cref="VulkanVertexBufferLayout" />
    class LITEFX_VULKAN_API VulkanIndexBufferLayout final : public IIndexBufferLayout {
        LITEFX_IMPLEMENTATION(VulkanIndexBufferLayoutImpl);
        friend struct SharedObject::Allocator<VulkanIndexBufferLayout>;

    private:
        /// <summary>
        /// Initializes a new index buffer layout
        /// </summary>
        /// <param name="type">The type of the indices within the index buffer.</param>
        explicit VulkanIndexBufferLayout(IndexType type);

    private:
        /// <inheritdoc />
        VulkanIndexBufferLayout(VulkanIndexBufferLayout&&) noexcept = delete;

        /// <inheritdoc />
        VulkanIndexBufferLayout(const VulkanIndexBufferLayout&);

        /// <inheritdoc />
        VulkanIndexBufferLayout& operator=(VulkanIndexBufferLayout&&) noexcept = delete;

        /// <inheritdoc />
        VulkanIndexBufferLayout& operator=(const VulkanIndexBufferLayout&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanIndexBufferLayout() noexcept override;

    public:
        /// <summary>
        /// Creates a new index buffer layout
        /// </summary>
        /// <param name="type">The type of the indices within the index buffer.</param>
        /// <returns>A shared pointer to the newly created index buffer layout instance.</returns>
        static inline auto create(IndexType type) {
            return SharedObject::create<VulkanIndexBufferLayout>(type);
        }

        /// <summary>
        /// Creates a copy of an index buffer layout.
        /// </summary>
        /// <param name="other">The index buffer layout to copy.</param>
        /// <returns>A shared pointer to the newly created index buffer layout instance.</returns>
        static inline auto create(const VulkanIndexBufferLayout& other) {
            return SharedObject::create<VulkanIndexBufferLayout>(other);
        }

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
    /// Represents the base interface for a Vulkan buffer implementation.
    /// </summary>
    /// <seealso cref="VulkanDescriptorSet" />
    /// <seealso cref="IVulkanImage" />
    /// <seealso cref="IVulkanVertexBuffer" />
    /// <seealso cref="IVulkanIndexBuffer" />
    class LITEFX_VULKAN_API IVulkanBuffer : public virtual IBuffer, public virtual IResource<VkBuffer> {
    protected:
        IVulkanBuffer() noexcept = default;
        IVulkanBuffer(IVulkanBuffer&&) noexcept = default;
        IVulkanBuffer(const IVulkanBuffer&) = delete;
        IVulkanBuffer& operator=(IVulkanBuffer&&) noexcept = default;
        IVulkanBuffer& operator=(const IVulkanBuffer&) = delete;

    public:
        ~IVulkanBuffer() noexcept override = default;
    };

    /// <summary>
    /// Represents a Vulkan vertex buffer.
    /// </summary>
    /// <seealso cref="VulkanVertexBufferLayout" />
    /// <seealso cref="IVulkanBuffer" />
    class LITEFX_VULKAN_API IVulkanVertexBuffer : public virtual VertexBuffer<VulkanVertexBufferLayout>, public virtual IVulkanBuffer {
    protected:
        IVulkanVertexBuffer() noexcept = default;
        IVulkanVertexBuffer(IVulkanVertexBuffer&&) noexcept = default;
        IVulkanVertexBuffer(const IVulkanVertexBuffer&) = delete;
        IVulkanVertexBuffer& operator=(IVulkanVertexBuffer&&) noexcept = default;
        IVulkanVertexBuffer& operator=(const IVulkanVertexBuffer&) = delete;

    public:
        ~IVulkanVertexBuffer() noexcept override = default;
    };

    /// <summary>
    /// Represents a Vulkan index buffer.
    /// </summary>
    /// <seealso cref="VulkanIndexBufferLayout" />
    /// <seealso cref="IVulkanBuffer" />
    class LITEFX_VULKAN_API IVulkanIndexBuffer : public virtual IndexBuffer<VulkanIndexBufferLayout>, public virtual IVulkanBuffer {
    protected:
        IVulkanIndexBuffer() noexcept = default;
        IVulkanIndexBuffer(IVulkanIndexBuffer&&) noexcept = default;
        IVulkanIndexBuffer(const IVulkanIndexBuffer&) = delete;
        IVulkanIndexBuffer& operator=(IVulkanIndexBuffer&&) noexcept = default;
        IVulkanIndexBuffer& operator=(const IVulkanIndexBuffer&) = delete;

    public:
        ~IVulkanIndexBuffer() noexcept override = default;
    };

    /// <summary>
    /// Represents a Vulkan sampled image or the base interface for a texture.
    /// </summary>
    /// <seealso cref="VulkanDescriptorLayout" />
    /// <seealso cref="VulkanDescriptorSet" />
    /// <seealso cref="VulkanDescriptorSetLayout" />
    /// <seealso cref="IVulkanBuffer" />
    /// <seealso cref="IVulkanSampler" />
    class LITEFX_VULKAN_API IVulkanImage : public virtual IImage, public virtual IResource<VkImage> {
    protected:
        IVulkanImage() noexcept = default;
        IVulkanImage(IVulkanImage&&) noexcept = default;
        IVulkanImage(const IVulkanImage&) = delete;
        IVulkanImage& operator=(IVulkanImage&&) noexcept = default;
        IVulkanImage& operator=(const IVulkanImage&) = delete;

    public:
        ~IVulkanImage() noexcept override = default;

    public:
        /// <summary>
        /// Returns the image resource aspect mask for all sub-resources.
        /// </summary>
        /// <returns>The image resource aspect mask.</returns>
        virtual VkImageAspectFlags aspectMask() const noexcept = 0;

        /// <summary>
        /// Returns the image resource aspect mask for a single sub-resource.
        /// </summary>
        /// <param name="plane">The sub-resource identifier to query the aspect mask from.</param>
        /// <returns>The image resource aspect mask.</returns>
        virtual VkImageAspectFlags aspectMask(UInt32 plane) const = 0;
    };

    /// <summary>
    /// Represents a Vulkan sampler.
    /// </summary>
    /// <seealso cref="VulkanDescriptorLayout" />
    /// <seealso cref="VulkanDescriptorSet" />
    /// <seealso cref="VulkanDescriptorSetLayout" />
    /// <seealso cref="IVulkanImage" />
    class LITEFX_VULKAN_API IVulkanSampler : public virtual ISampler, public virtual IResource<VkSampler> {
    protected:
        IVulkanSampler() noexcept = default;
        IVulkanSampler(IVulkanSampler&&) noexcept = default;
        IVulkanSampler(const IVulkanSampler&) = delete;
        IVulkanSampler& operator=(IVulkanSampler&&) noexcept = default;
        IVulkanSampler& operator=(const IVulkanSampler&) = delete;

    public:
        ~IVulkanSampler() noexcept override = default;
    };

    /// <summary>
    /// Represents the base interface for a Vulkan acceleration structure implementation.
    /// </summary>
    /// <seealso cref="VulkanDescriptorSet" />
    /// <seealso cref="VulkanBottomLevelAccelerationStructure" />
    /// <seealso cref="VulkanTopevelAccelerationStructure" />
    class LITEFX_VULKAN_API IVulkanAccelerationStructure : public virtual IAccelerationStructure, public virtual IResource<VkAccelerationStructureKHR> {
    protected:
        IVulkanAccelerationStructure() noexcept = default;
        IVulkanAccelerationStructure(IVulkanAccelerationStructure&&) noexcept = default;
        IVulkanAccelerationStructure(const IVulkanAccelerationStructure&) = delete;
        IVulkanAccelerationStructure& operator=(IVulkanAccelerationStructure&&) noexcept = default;
        IVulkanAccelerationStructure& operator=(const IVulkanAccelerationStructure&) = delete;

    public:
        ~IVulkanAccelerationStructure() noexcept override = default;
    };

    /// <summary>
    /// Implements a Vulkan bottom-level acceleration structure (BLAS).
    /// </summary>
    /// <seealso cref="VulkanTopLevelAccelerationStructure" />
    class LITEFX_VULKAN_API VulkanBottomLevelAccelerationStructure final : public IBottomLevelAccelerationStructure, public virtual IVulkanAccelerationStructure, public virtual StateResource, public virtual Resource<VkAccelerationStructureKHR> {
        LITEFX_IMPLEMENTATION(VulkanBottomLevelAccelerationStructureImpl);
        friend class VulkanDevice;
        friend class VulkanCommandBuffer;

        using IAccelerationStructure::build;
        using IAccelerationStructure::update;
        using IBottomLevelAccelerationStructure::copy;

    public:
        /// <summary>
        /// Initializes a new Vulkan bottom-level acceleration structure (BLAS).
        /// </summary>
        /// <param name="flags">The flags that define how the acceleration structure is built.</param>
        /// <param name="name">The name of the acceleration structure resource.</param>
        /// <exception cref="InvalidArgumentException">Thrown if the provided <paramref name="flags" /> contain an unsupported combination of flags.</exception>
        /// <seealso cref="AccelerationStructureFlags" />
        explicit VulkanBottomLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None, StringView name = "");

        /// <inheritdoc />
        VulkanBottomLevelAccelerationStructure(VulkanBottomLevelAccelerationStructure&&) noexcept;

        /// <inheritdoc />
        VulkanBottomLevelAccelerationStructure(const VulkanBottomLevelAccelerationStructure&) = delete;

        /// <inheritdoc />
        VulkanBottomLevelAccelerationStructure& operator=(VulkanBottomLevelAccelerationStructure&&) noexcept;

        /// <inheritdoc />
        VulkanBottomLevelAccelerationStructure& operator=(const VulkanBottomLevelAccelerationStructure&) = delete;
        
        /// <inheritdoc />
        ~VulkanBottomLevelAccelerationStructure() noexcept override;

        // IAccelerationStructure interface.
    public:
        /// <inheritdoc />
        AccelerationStructureFlags flags() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const IVulkanBuffer> buffer() const noexcept;

        /// <inheritdoc />
        void build(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer = nullptr, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// <inheritdoc />
        void update(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer = nullptr, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// <inheritdoc />
        void copy(const VulkanCommandBuffer& commandBuffer, VulkanBottomLevelAccelerationStructure& destination, bool compress = false, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const;

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
        void clear() noexcept override;

        /// <inheritdoc />
        bool remove(const TriangleMesh& mesh) noexcept override;

        /// <inheritdoc />
        bool remove(const BoundingBoxes& aabb) noexcept override;

    private:
        Array<std::pair<UInt32, VkAccelerationStructureGeometryKHR>> buildInfo() const;
        void updateState(const VulkanDevice* device, VkAccelerationStructureKHR handle) noexcept;

    private:
        SharedPtr<const IBuffer> getBuffer() const noexcept override;
        void doBuild(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doUpdate(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doCopy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress, const SharedPtr<const IBuffer>& buffer, UInt64 offset, bool copyBuildInfo) const override;
    };

    /// <summary>
    /// Implements a Vulkan top-level acceleration structure (TLAS).
    /// </summary>
    /// <seealso cref="VulkanBottomLevelAccelerationStructure" />
    class LITEFX_VULKAN_API VulkanTopLevelAccelerationStructure final : public ITopLevelAccelerationStructure, public virtual IVulkanAccelerationStructure, public virtual StateResource, public virtual Resource<VkAccelerationStructureKHR> {
        LITEFX_IMPLEMENTATION(VulkanTopLevelAccelerationStructureImpl);
        friend class VulkanDevice;
        friend class VulkanCommandBuffer;

        using IAccelerationStructure::build;
        using IAccelerationStructure::update;
        using ITopLevelAccelerationStructure::copy;

    public:
        /// <summary>
        /// Initializes a new Vulkan top-level acceleration structure (BLAS).
        /// </summary>
        /// <param name="flags">The flags that define how the acceleration structure is built.</param>
        /// <param name="name">The name of the acceleration structure resource.</param>
        /// <exception cref="InvalidArgumentException">Thrown if the provided <paramref name="flags" /> contain an unsupported combination of flags.</exception>
        /// <seealso cref="AccelerationStructureFlags" />
        explicit VulkanTopLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None, StringView name = "");

        /// <inheritdoc />
        VulkanTopLevelAccelerationStructure(VulkanTopLevelAccelerationStructure&&) noexcept;

        /// <inheritdoc />
        VulkanTopLevelAccelerationStructure(const VulkanTopLevelAccelerationStructure&) = delete;

        /// <inheritdoc />
        VulkanTopLevelAccelerationStructure& operator=(VulkanTopLevelAccelerationStructure&&) noexcept;

        /// <inheritdoc />
        VulkanTopLevelAccelerationStructure& operator=(const VulkanTopLevelAccelerationStructure&) = delete;

        /// <inheritdoc />
        ~VulkanTopLevelAccelerationStructure() noexcept override;

        // IAccelerationStructure interface.
    public:
        /// <inheritdoc />
        AccelerationStructureFlags flags() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const IVulkanBuffer> buffer() const noexcept;

        /// <inheritdoc />
        void build(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer = nullptr, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// <inheritdoc />
        void update(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer = nullptr, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// <inheritdoc />
        void copy(const VulkanCommandBuffer& commandBuffer, VulkanTopLevelAccelerationStructure& destination, bool compress = false, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const;

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
        void clear() noexcept override;

        /// <inheritdoc />
        bool remove(const Instance& mesh) noexcept override;

    private:
        Array<VkAccelerationStructureInstanceKHR> buildInfo() const;
        void updateState(const VulkanDevice* device, VkAccelerationStructureKHR handle) noexcept;

    private:
        SharedPtr<const IBuffer> getBuffer() const noexcept override;
        void doBuild(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doUpdate(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doCopy(const ICommandBuffer& commandBuffer, ITopLevelAccelerationStructure& destination, bool compress, const SharedPtr<const IBuffer>& buffer, UInt64 offset, bool copyBuildInfo) const override;
    };

    /// <summary>
    /// Implements a Vulkan resource barrier.
    /// </summary>
    /// <seealso cref="VulkanCommandBuffer" />
    /// <seealso cref="IVulkanBuffer" />
    /// <seealso cref="IVulkanImage" />
    /// <seealso cref="Barrier" />
    class LITEFX_VULKAN_API VulkanBarrier final : public Barrier<IVulkanBuffer, IVulkanImage> {
        LITEFX_IMPLEMENTATION(VulkanBarrierImpl);
        LITEFX_BUILDER(VulkanBarrierBuilder);

    public:
        using base_type = Barrier<IVulkanBuffer, IVulkanImage>;
        using base_type::transition;

    public:
        /// <summary>
        /// Initializes a new Vulkan barrier.
        /// </summary>
        /// <param name="syncBefore">The pipeline stage(s) all previous commands have to finish before the barrier is executed.</param>
        /// <param name="syncAfter">The pipeline stage(s) all subsequent commands are blocked at until the barrier is executed.</param>
        explicit VulkanBarrier(PipelineStage syncBefore, PipelineStage syncAfter) noexcept;

        /// <inheritdoc />
        VulkanBarrier(VulkanBarrier&&) noexcept;

        /// <inheritdoc />
        VulkanBarrier(const VulkanBarrier&);

        /// <inheritdoc />
        VulkanBarrier& operator=(VulkanBarrier&&) noexcept;

        /// <inheritdoc />
        VulkanBarrier& operator=(const VulkanBarrier&);

        /// <inheritdoc />
        ~VulkanBarrier() noexcept override;

    private:
        explicit VulkanBarrier() noexcept;
        PipelineStage& syncBefore() noexcept;
        PipelineStage& syncAfter() noexcept;

        // Barrier interface.
    public:
        /// <inheritdoc />
        PipelineStage syncBefore() const noexcept override;

        /// <inheritdoc />
        PipelineStage syncAfter() const noexcept override;

        /// <inheritdoc />
        void wait(ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// <inheritdoc />
        void transition(const IVulkanBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// <inheritdoc />
        void transition(const IVulkanBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// <inheritdoc />
        void transition(const IVulkanImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override;

        /// <inheritdoc />
        void transition(const IVulkanImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override;

        /// <inheritdoc />
        void transition(const IVulkanImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override;

        /// <inheritdoc />
        void transition(const IVulkanImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override;

    public:
        /// <summary>
        /// Adds the barrier to a command buffer and updates the resource target states.
        /// </summary>
        /// <param name="commandBuffer">The command buffer to add the barriers to.</param>
        /// <exception cref="RuntimeException">Thrown, if any of the contained barriers is a image barrier that targets a sub-resource range that does not share the same <see cref="ImageLayout" /> in all sub-resources.</exception>
        void execute(const VulkanCommandBuffer& commandBuffer) const;
    };

    /// <summary>
    /// Implements a Vulkan <see cref="IShaderModule" />.
    /// </summary>
    /// <seealso cref="VulkanShaderProgram" />
    /// <seealso cref="VulkanDevice" />
    /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
    class LITEFX_VULKAN_API VulkanShaderModule final : public IShaderModule, public Resource<VkShaderModule> {
        LITEFX_IMPLEMENTATION(VulkanShaderModuleImpl);

    public:
        /// <summary>
        /// Initializes a new Vulkan shader module.
        /// </summary>
        /// <param name="device">The parent device, this shader module has been created from.</param>
        /// <param name="type">The shader stage, this module is used in.</param>
        /// <param name="fileName">The file name of the module source.</param>
        /// <param name="entryPoint">The name of the module entry point.</param>
        /// <param name="shaderLocalDescriptor">The descriptor that binds shader-local data for ray-tracing shaders.</param>
        explicit VulkanShaderModule(const VulkanDevice& device, ShaderStage type, const String& fileName, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt);

        /// <summary>
        /// Initializes a new Vulkan shader module.
        /// </summary>
        /// <param name="device">The parent device, this shader module has been created from.</param>
        /// <param name="type">The shader stage, this module is used in.</param>
        /// <param name="stream">The file stream of the module source.</param>
        /// <param name="name">The file name of the module source.</param>
        /// <param name="entryPoint">The name of the module entry point.</param>
        /// <param name="shaderLocalDescriptor">The descriptor that binds shader-local data for ray-tracing shaders.</param>
        explicit VulkanShaderModule(const VulkanDevice& device, ShaderStage type, std::istream& stream, const String& name, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt);

        /// <inheritdoc />
        VulkanShaderModule(VulkanShaderModule&&) noexcept;

        /// <inheritdoc />
        VulkanShaderModule(const VulkanShaderModule&) = delete;

        /// <inheritdoc />
        VulkanShaderModule& operator=(VulkanShaderModule&&) noexcept;

        /// <inheritdoc />
        VulkanShaderModule& operator=(const VulkanShaderModule&) = delete;

        /// <inheritdoc />
        ~VulkanShaderModule() noexcept override;

        // ShaderModule interface.
    public:
        /// <inheritdoc />
        const String& fileName() const noexcept override;

        /// <inheritdoc />
        const String& entryPoint() const noexcept override;

        /// <inheritdoc />
        ShaderStage type() const noexcept override;

        /// <inheritdoc />
        const Optional<DescriptorBindingPoint>& shaderLocalDescriptor() const noexcept override;

    public:
        /// <summary>
        /// Returns the shader byte code.
        /// </summary>
        /// <returns>The shader byte code.</returns>
        virtual const String& bytecode() const noexcept;

        /// <summary>
        /// Returns the shader stage creation info for convenience.
        /// </summary>
        /// <returns>The shader stage creation info for convenience.</returns>
        virtual VkPipelineShaderStageCreateInfo shaderStageDefinition() const;
    };

    /// <summary>
    /// Implements a Vulkan <see cref="ShaderProgram" />.
    /// </summary>
    /// <seealso cref="VulkanShaderProgramBuilder" />
    /// <seealso cref="VulkanShaderModule" />
    /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
    class LITEFX_VULKAN_API VulkanShaderProgram final : public ShaderProgram<VulkanShaderModule> {
        LITEFX_IMPLEMENTATION(VulkanShaderProgramImpl);
        LITEFX_BUILDER(VulkanShaderProgramBuilder);
        friend struct SharedObject::Allocator<VulkanShaderProgram>;

    private:
        /// <summary>
        /// Initializes a new Vulkan shader program.
        /// </summary>
        /// <param name="device">The parent device of the shader program.</param>
        /// <param name="modules">The shader modules used by the shader program.</param>
        explicit VulkanShaderProgram(const VulkanDevice& device, Enumerable<UniquePtr<VulkanShaderModule>>&& modules);

        /// <summary>
        /// Initializes a new Vulkan shader program.
        /// </summary>
        /// <param name="device">The parent device of the shader program.</param>
        explicit VulkanShaderProgram(const VulkanDevice& device);

    public:
        /// <inheritdoc />
        VulkanShaderProgram(VulkanShaderProgram&&) noexcept = delete;

        /// <inheritdoc />
        VulkanShaderProgram(const VulkanShaderProgram&) = delete;

        /// <inheritdoc />
        VulkanShaderProgram& operator=(VulkanShaderProgram&&) noexcept = delete;

        /// <inheritdoc />
        VulkanShaderProgram& operator=(const VulkanShaderProgram&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanShaderProgram() noexcept override;

        // Factory method.
    public:
        /// <summary>
        /// Creates a new Vulkan shader program.
        /// </summary>
        /// <param name="device">The parent device of the shader program.</param>
        /// <param name="modules">The shader modules used by the shader program.</param>
        /// <returns>A shared pointer to the newly created shader program instance.</returns>
        static inline auto create(const VulkanDevice& device, Enumerable<UniquePtr<VulkanShaderModule>>&& modules) {
            return SharedObject::create<VulkanShaderProgram>(device, std::move(modules));
        }

    private:
        /// <summary>
        /// Creates a new Vulkan shader program.
        /// </summary>
        /// <returns>A shared pointer to the newly created shader program instance.</returns>
        static inline auto create(const VulkanDevice& device) {
            return SharedObject::create<VulkanShaderProgram>(device);
        }

    public:
        /// <inheritdoc />
        const Array<UniquePtr<const VulkanShaderModule>>& modules() const noexcept override;

        /// <inheritdoc />
        virtual SharedPtr<VulkanPipelineLayout> reflectPipelineLayout(Enumerable<PipelineBindingHint> hints = {}) const;

    private:
        SharedPtr<IPipelineLayout> parsePipelineLayout(Enumerable<PipelineBindingHint> hints) const override {
            return std::static_pointer_cast<IPipelineLayout>(this->reflectPipelineLayout(hints));
        }
    };

    /// <summary>
    /// Implements a Vulkan <see cref="DescriptorSet" />.
    /// </summary>
    /// <seealso cref="VulkanDescriptorSetLayout" />
    class LITEFX_VULKAN_API VulkanDescriptorSet final : public DescriptorSet<IVulkanBuffer, IVulkanImage, IVulkanSampler, IVulkanAccelerationStructure> {
        LITEFX_IMPLEMENTATION(VulkanDescriptorSetImpl);
        friend class VulkanDescriptorSetLayout;

    public:
        using base_type = DescriptorSet<IVulkanBuffer, IVulkanImage, IVulkanSampler, IVulkanAccelerationStructure>;
        using base_type::update;

    private:
        /// <summary>
        /// Initializes the descriptor set from a cached buffer. This is only called from the descriptor set layout.
        /// </summary>
        /// <param name="layout">The parent layout of the descriptor set.</param>
        /// <param name="buffer">The buffer to take over.</param>
        explicit VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, Array<Byte>&& buffer);

    public:
        /// <summary>
        /// Initializes a new descriptor set.
        /// </summary>
        /// <param name="layout">The parent descriptor set layout.</param>
        /// <param name="unboundedArraySize">The size of the unbounded runtime array, if available.</param>
        explicit VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, UInt32 unboundedArraySize = std::numeric_limits<UInt32>::max());

        /// <inheritdoc />
        VulkanDescriptorSet(VulkanDescriptorSet&&) noexcept = delete;

        /// <inheritdoc />
        VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;

        /// <inheritdoc />
        VulkanDescriptorSet& operator=(VulkanDescriptorSet&&) noexcept = delete;

        /// <inheritdoc />
        VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete;

        /// <inheritdoc />
        ~VulkanDescriptorSet() noexcept override;

    public:
        /// <summary>
        /// Returns the parent descriptor set layout.
        /// </summary>
        /// <returns>The parent descriptor set layout.</returns>
        virtual const VulkanDescriptorSetLayout& layout() const noexcept;

    private:
        /// <summary>
        /// Releases the underlying buffer of the descriptor set and returns it to the caller (usually the parent descriptor set layout).
        /// </summary>
        /// <returns>The underlying descriptor buffer.</returns>
        Array<Byte>&& releaseBuffer() const noexcept;

    public:
        /// <summary>
        /// Returns a view over the underlying descriptor buffer.
        /// </summary>
        /// <returns>A view over the underlying descriptor buffer.</returns>
        Span<const Byte> descriptorBuffer() const noexcept;

    public:
        /// <inheritdoc />
        UInt32 globalHeapOffset(DescriptorHeapType heapType) const noexcept override;

        /// <inheritdoc />
        UInt32 globalHeapAddressRange(DescriptorHeapType heapType) const noexcept override;

        /// <inheritdoc />
        void update(UInt32 binding, const IVulkanBuffer& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, UInt32 firstDescriptor = 0) const override;

        /// <inheritdoc />
        void update(UInt32 binding, const IVulkanImage& texture, UInt32 descriptor = 0, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const override;

        /// <inheritdoc />
        void update(UInt32 binding, const IVulkanSampler& sampler, UInt32 descriptor = 0) const override;

        /// <inheritdoc />
        void update(UInt32 binding, const IVulkanAccelerationStructure& accelerationStructure, UInt32 descriptor = 0) const override;
    };

    /// <summary>
    /// Implements a Vulkan <see cref="IDescriptorLayout" />
    /// </summary>
    /// <seealso cref="IVulkanBuffer" />
    /// <seealso cref="IVulkanImage" />
    /// <seealso cref="IVulkanSampler" />
    /// <seealso cref="VulkanDescriptorSet" />
    /// <seealso cref="VulkanDescriptorSetLayout" />
    class LITEFX_VULKAN_API VulkanDescriptorLayout final : public IDescriptorLayout {
        LITEFX_IMPLEMENTATION(VulkanDescriptorLayoutImpl);

    public:
        /// <summary>
        /// Initializes a new Vulkan descriptor layout.
        /// </summary>
        /// <param name="type">The type of the descriptor.</param>
        /// <param name="binding">The binding point for the descriptor.</param>
        /// <param name="elementSize">The size of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the descriptor array. If <paramref name="unbounded" /> is set, this value sets the upper limit for the array size.</param>
        /// <param name="unbounded">If set to `true`, the descriptor will be defined as a runtime-allocated, unbounded array.</param>
        /// <seealso cref="descriptors" />
        VulkanDescriptorLayout(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors = 1, bool unbounded = false);

        /// <summary>
        /// Initializes a new Vulkan descriptor layout for a static sampler.
        /// </summary>
        /// <param name="staticSampler">The static sampler to initialize the state with.</param>
        /// <param name="binding">The binding point for the descriptor.</param>
        VulkanDescriptorLayout(const IVulkanSampler& staticSampler, UInt32 binding);

        /// <summary>
        /// Initializes a new Vulkan descriptor layout for an input attachment.
        /// </summary>
        /// <param name="binding">The binding point for the descriptor.</param>
        /// <param name="inputAttachmentIndex">If <paramref name="type" /> equals <see cref="DescriptorType::InputAttachment" /> this value specifies the index of the input attachment. Otherwise, the value is ignored.</param>
        VulkanDescriptorLayout(UInt32 binding, UInt32 inputAttachmentIndex);

        /// <inheritdoc />
        VulkanDescriptorLayout(VulkanDescriptorLayout&&) noexcept;

        /// <inheritdoc />
        VulkanDescriptorLayout(const VulkanDescriptorLayout&);

        /// <inheritdoc />
        VulkanDescriptorLayout& operator=(VulkanDescriptorLayout&&) noexcept;

        /// <inheritdoc />
        VulkanDescriptorLayout& operator=(const VulkanDescriptorLayout&);

        /// <inheritdoc />
        ~VulkanDescriptorLayout() noexcept override;

        // IDescriptorLayout interface.
    public:
        /// <inheritdoc />
        DescriptorType descriptorType() const noexcept override;

        /// <inheritdoc />
        UInt32 descriptors() const noexcept override;

        /// <inheritdoc />
        bool unbounded() const noexcept override;

        /// <inheritdoc />
        const IVulkanSampler* staticSampler() const noexcept override;

        // IBufferLayout interface.
    public:
        /// <inheritdoc />
        size_t elementSize() const noexcept override;

        /// <inheritdoc />
        UInt32 binding() const noexcept override;

        /// <inheritdoc />
        BufferType type() const noexcept override;

        // VulkanDescriptorLayout
    public:
        /// <summary>
        /// The index of the input attachment mapped to this descriptor.
        /// </summary>
        /// <remarks>
        /// If <see cref="descriptorType" /> returns <see cref="DescriptorType::InputAttachment" /> this value refers to the index of the input attachment within a set of input 
        /// attachments of a <see cref="VulkanRenderPass" />. Otherwise, the value is ignored.
        /// </remarks>
        /// <returns>The index of the input attachment mapped to this descriptor.</returns>
        UInt32 inputAttachmentIndex() const noexcept;
    };

    /// <summary>
    /// Implements a Vulkan <see cref="DescriptorSetLayout" />.
    /// </summary>
    /// <seealso cref="VulkanDescriptorSet" />
    /// <seealso cref="VulkanDescriptorSetLayoutBuilder" />
    class LITEFX_VULKAN_API VulkanDescriptorSetLayout final : public DescriptorSetLayout<VulkanDescriptorLayout, VulkanDescriptorSet>, public Resource<VkDescriptorSetLayout> {
        LITEFX_IMPLEMENTATION(VulkanDescriptorSetLayoutImpl);
        LITEFX_BUILDER(VulkanDescriptorSetLayoutBuilder);
        friend struct SharedObject::Allocator<VulkanDescriptorSetLayout>;

    public:
        using base_type = DescriptorSetLayout<VulkanDescriptorLayout, VulkanDescriptorSet>;
        using base_type::free;
        using base_type::allocate;

    private:
        /// <summary>
        /// Initializes a Vulkan descriptor set layout.
        /// </summary>
        /// <param name="device">The parent device, the pipeline layout has been created from.</param>
        /// <param name="descriptorLayouts">The descriptor layouts of the descriptors within the descriptor set.</param>
        /// <param name="space">The space or set id of the descriptor set.</param>
        /// <param name="stages">The shader stages, the descriptor sets are bound to.</param>
        explicit VulkanDescriptorSetLayout(const VulkanDevice& device, const Enumerable<VulkanDescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages);
        
        /// <summary>
        /// Initializes a Vulkan descriptor set layout.
        /// </summary>
        /// <param name="device">The parent device, the pipeline layout has been created from.</param>
        explicit VulkanDescriptorSetLayout(const VulkanDevice& device);

    private:
        /// <inheritdoc />
        VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&&) noexcept = delete;

        /// <inheritdoc />
        VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout& other);

        /// <inheritdoc />
        VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&&) noexcept = delete;

        /// <inheritdoc />
        VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanDescriptorSetLayout() noexcept override;

    public:
        /// <summary>
        /// Creates a Vulkan descriptor set layout.
        /// </summary>
        /// <param name="device">The device, the descriptor set layout is created on.</param>
        /// <param name="descriptorLayouts">The descriptor layouts of the descriptors within the descriptor set.</param>
        /// <param name="space">The space or set id of the descriptor set.</param>
        /// <param name="stages">The shader stages, the descriptor sets are bound to.</param>
        /// <returns>Returns a shared pointer to the newly created descriptor set layout.</returns>
        static inline auto create(const VulkanDevice& device, const Enumerable<VulkanDescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages) {
            return SharedObject::create<VulkanDescriptorSetLayout>(device, descriptorLayouts, space, stages);
        }

        /// <summary>
        /// Creates a copy of a Vulkan descriptor set layout.
        /// </summary>
        /// <param name="other">The descriptor set layout to copy.</param>
        /// <returns>Returns a shared pointer to the newly created descriptor set layout.</returns>
        static inline auto create(const VulkanDescriptorSetLayout& other) {
            return SharedObject::create<VulkanDescriptorSetLayout>(other);
        }

    private:
        /// <summary>
        /// Creates a Vulkan descriptor set layout.
        /// </summary>
        /// <param name="device">The device, the descriptor set layout is created on.</param>
        /// <returns>Returns a shared pointer to the newly created descriptor set layout.</returns>
        static inline auto create(const VulkanDevice& device) {
            return SharedObject::create<VulkanDescriptorSetLayout>(device);
        }

    public:
        /// <summary>
        /// Returns a reference of the device, the pipeline layout has been created from.
        /// </summary>
        /// <returns>A reference of the device, the pipeline layout has been created from.</returns>
        const VulkanDevice& device() const noexcept;

        /// <summary>
        /// Returns the maximum allowed size for an unbounded array in a descriptor set created with this layout, or `0` if the layout does not contain an unbounded array.
        /// </summary>
        /// <returns>The maximum allowed size for an unbounded array in a descriptor set created with this layout</returns>
        UInt32 maxUnboundedArraySize() const noexcept;

    public:
        /// <inheritdoc />
        const Array<VulkanDescriptorLayout>& descriptors() const noexcept override;

        /// <inheritdoc />
        const VulkanDescriptorLayout& descriptor(UInt32 binding) const override;

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

        /// <inheritdoc />
        bool containsUnboundedArray() const noexcept override;

        /// <inheritdoc />
        UInt32 getDescriptorOffset(UInt32 binding, UInt32 element = 0) const override;

        /// <inheritdoc />
        bool bindsResources() const noexcept override;

        /// <inheritdoc />
        bool bindsSamplers() const noexcept override;

    public:
        /// <inheritdoc />
        UniquePtr<VulkanDescriptorSet> allocate(UInt32 descriptors, std::initializer_list<DescriptorBinding> bindings) const override;

        /// <inheritdoc />
        UniquePtr<VulkanDescriptorSet> allocate(UInt32 descriptors, Span<DescriptorBinding> bindings) const override;

        /// <inheritdoc />
        UniquePtr<VulkanDescriptorSet> allocate(UInt32 descriptors, Generator<DescriptorBinding> bindings) const override;

        /// <inheritdoc />
        Generator<UniquePtr<VulkanDescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::initializer_list<std::initializer_list<DescriptorBinding>> bindings = { }) const override;

#ifdef __cpp_lib_mdspan
        /// <inheritdoc />
        Generator<UniquePtr<VulkanDescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const override;
#endif

        /// <inheritdoc />
        Generator<UniquePtr<VulkanDescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const override;

        /// <inheritdoc />
        void free(const VulkanDescriptorSet& descriptorSet) const override;
    };

    /// <summary>
    /// Implements the Vulkan <see cref="IPushConstantsRange" />.
    /// </summary>
    /// <seealso cref="VulkanPushConstantsLayout" />
    class LITEFX_VULKAN_API VulkanPushConstantsRange final : public IPushConstantsRange {
        LITEFX_IMPLEMENTATION(VulkanPushConstantsRangeImpl);

    public:
        /// <summary>
        /// Initializes a new push constants range.
        /// </summary>
        /// <param name="shaderStage">The shader stage, that access the push constants from the range.</param>
        /// <param name="offset">The offset relative to the parent push constants backing memory that marks the beginning of the range.</param>
        /// <param name="size">The size of the push constants range.</param>
        /// <param name="space">The space from which the push constants of the range will be accessible in the shader.</param>
        /// <param name="binding">The register from which the push constants of the range will be accessible in the shader.</param>
        explicit VulkanPushConstantsRange(ShaderStage shaderStage, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding);

        /// <inheritdoc />
        VulkanPushConstantsRange(VulkanPushConstantsRange&&) noexcept;

        /// <inheritdoc />
        VulkanPushConstantsRange(const VulkanPushConstantsRange&);

        /// <inheritdoc />
        VulkanPushConstantsRange& operator=(VulkanPushConstantsRange&&) noexcept;

        /// <inheritdoc />
        VulkanPushConstantsRange& operator=(const VulkanPushConstantsRange&);

        /// <inheritdoc />
        ~VulkanPushConstantsRange() noexcept override;

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
    };

    /// <summary>
    /// Implements the Vulkan <see cref="PushConstantsLayout" />.
    /// </summary>
    /// <seealso cref="VulkanPushConstantsRange" />
    /// <seealso cref="VulkanPushConstantsLayoutBuilder" />
    /// <seealso cref="VulkanPushConstantsLayoutBuilder" />
    class LITEFX_VULKAN_API VulkanPushConstantsLayout final : public PushConstantsLayout<VulkanPushConstantsRange> {
        LITEFX_IMPLEMENTATION(VulkanPushConstantsLayoutImpl);
        LITEFX_BUILDER(VulkanPushConstantsLayoutBuilder);
        friend class VulkanPipelineLayout;

    public:
        /// <summary>
        /// Initializes a new push constants layout.
        /// </summary>
        /// <param name="ranges">The ranges contained by the layout.</param>
        /// <param name="size">The overall size (in bytes) of the push constants backing memory.</param>
        explicit VulkanPushConstantsLayout(Enumerable<UniquePtr<VulkanPushConstantsRange>>&& ranges, UInt32 size);
        
        /// <inheritdoc />
        VulkanPushConstantsLayout(VulkanPushConstantsLayout&&) noexcept;

        /// <inheritdoc />
        VulkanPushConstantsLayout(const VulkanPushConstantsLayout&) = delete;

        /// <inheritdoc />
        VulkanPushConstantsLayout& operator=(VulkanPushConstantsLayout&&) noexcept;

        /// <inheritdoc />
        VulkanPushConstantsLayout& operator=(const VulkanPushConstantsLayout&) = delete;

        /// <inheritdoc />
        ~VulkanPushConstantsLayout() noexcept override;

    private:
        /// <summary>
        /// Initializes a new push constants layout.
        /// </summary>
        /// <param name="size">The overall size (in bytes) of the push constants backing memory.</param>
        explicit VulkanPushConstantsLayout(UInt32 size);
    
    public:
        /// <inheritdoc />
        UInt32 size() const noexcept override;

        /// <inheritdoc />
        const VulkanPushConstantsRange& range(ShaderStage stage) const override;

        /// <inheritdoc />
        const Array<UniquePtr<VulkanPushConstantsRange>>& ranges() const override;
    };

    /// <summary>
    /// Implements a Vulkan <see cref="PipelineLayout" />.
    /// </summary>
    /// <seealso cref="VulkanPipelineLayoutBuilder" />
    class LITEFX_VULKAN_API VulkanPipelineLayout final : public PipelineLayout<VulkanDescriptorSetLayout, VulkanPushConstantsLayout>, public Resource<VkPipelineLayout> {
        LITEFX_IMPLEMENTATION(VulkanPipelineLayoutImpl);
        LITEFX_BUILDER(VulkanPipelineLayoutBuilder);
        friend struct SharedObject::Allocator<VulkanPipelineLayout>;

    private:
        /// <summary>
        /// Initializes a new Vulkan render pipeline layout.
        /// </summary>
        /// <param name="device">The parent device, the layout is created from.</param>
        /// <param name="descriptorSetLayouts">The descriptor set layouts used by the pipeline.</param>
        /// <param name="pushConstantsLayout">The push constants layout used by the pipeline.</param>
        explicit VulkanPipelineLayout(const VulkanDevice& device, const Enumerable<SharedPtr<VulkanDescriptorSetLayout>>& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout);

        /// <summary>
        /// Initializes a new Vulkan render pipeline layout.
        /// </summary>
        /// <param name="device">The parent device, the layout is created from.</param>
        explicit VulkanPipelineLayout(const VulkanDevice& device) noexcept;

    private:
        /// <inheritdoc />
        VulkanPipelineLayout(VulkanPipelineLayout&&) noexcept = delete;

        /// <inheritdoc />
        VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;

        /// <inheritdoc />
        VulkanPipelineLayout& operator=(VulkanPipelineLayout&&) noexcept = delete;

        /// <inheritdoc />
        VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanPipelineLayout() noexcept override;

    public:
        /// <summary>
        /// Creates a new Vulkan render pipeline layout.
        /// </summary>
        /// <param name="device">The parent device, the layout is created from.</param>
        /// <param name="descriptorSetLayouts">The descriptor set layouts used by the pipeline.</param>
        /// <param name="pushConstantsLayout">The push constants layout used by the pipeline.</param>
        /// <returns>A shared pointer to the newly created pipeline layout instance.</returns>
        static inline auto create(const VulkanDevice& device, const Enumerable<SharedPtr<VulkanDescriptorSetLayout>>& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) {
            return SharedObject::create<VulkanPipelineLayout>(device, descriptorSetLayouts, std::move(pushConstantsLayout));
        }

    private:
        /// <summary>
        /// Creates a new Vulkan render pipeline layout.
        /// </summary>
        /// <param name="device">The parent device, the layout is created from.</param>
        /// <returns>A shared pointer to the newly created pipeline layout instance.</returns>
        static inline auto create(const VulkanDevice& device) {
            return SharedObject::create<VulkanPipelineLayout>(device);
        }

        // PipelineLayout interface.
    public:
        /// <inheritdoc />
        const VulkanDevice& device() const noexcept /*override*/;

        /// <inheritdoc />
        const VulkanDescriptorSetLayout& descriptorSet(UInt32 space) const override;

        /// <inheritdoc />
        const Array<SharedPtr<const VulkanDescriptorSetLayout>>& descriptorSets() const override;

        /// <inheritdoc />
        const VulkanPushConstantsLayout* pushConstants() const noexcept override;
    };

    /// <summary>
    /// Implements the Vulkan input assembler state.
    /// </summary>
    /// <seealso cref="VulkanInputAssemblerBuilder" />
    class LITEFX_VULKAN_API VulkanInputAssembler final : public InputAssembler<VulkanVertexBufferLayout, VulkanIndexBufferLayout> {
        LITEFX_IMPLEMENTATION(VulkanInputAssemblerImpl);
        LITEFX_BUILDER(VulkanInputAssemblerBuilder);
        friend struct SharedObject::Allocator<VulkanInputAssembler>;

	private:
		/// <summary>
		/// Initializes a new Vulkan input assembler state.
		/// </summary>
		/// <param name="vertexBufferLayouts">The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.</param>
		/// <param name="indexBufferLayout">The index buffer layout.</param>
		/// <param name="primitiveTopology">The primitive topology.</param>
		explicit VulkanInputAssembler(Enumerable<SharedPtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, SharedPtr<VulkanIndexBufferLayout>&& indexBufferLayout = nullptr, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList);

        /// <summary>
        /// Initializes a new Vulkan input assembler state.
        /// </summary>
        explicit VulkanInputAssembler();

    private:
        /// <inheritdoc />
        VulkanInputAssembler(VulkanInputAssembler&&) noexcept = delete;

        /// <inheritdoc />
		VulkanInputAssembler(const VulkanInputAssembler&);

        /// <inheritdoc />
        VulkanInputAssembler& operator=(VulkanInputAssembler&&) noexcept = delete;

        /// <inheritdoc />
        VulkanInputAssembler& operator=(const VulkanInputAssembler&) = delete;

    public:
        /// <inheritdoc />
		~VulkanInputAssembler() noexcept override;

    public:
        /// <summary>
        /// Creates a new Vulkan input assembler state.
        /// </summary>
        /// <param name="vertexBufferLayouts">The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.</param>
        /// <param name="indexBufferLayout">The index buffer layout.</param>
        /// <param name="primitiveTopology">The primitive topology.</param>
        /// <returns>A shared pointer to the newly created input assembler instance.</returns>
        static inline auto create(Enumerable<SharedPtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, SharedPtr<VulkanIndexBufferLayout>&& indexBufferLayout = nullptr, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList) {
            return SharedObject::create<VulkanInputAssembler>(std::move(vertexBufferLayouts), std::move(indexBufferLayout), primitiveTopology);
        }

        /// <summary>
        /// Creates a new Vulkan input assembler state as a copy from another one.
        /// </summary>
        /// <param name="other">The input assembler state to copy.</param>
        /// <returns>A shared pointer to the newly created input assembler instance.</returns>
        static inline auto create(const VulkanInputAssembler& other) {
            return SharedObject::create<VulkanInputAssembler>(other);
        }

    private:
        /// <summary>
        /// Creates a new Vulkan input assembler state.
        /// </summary>
        /// <returns>A shared pointer to the newly created input assembler instance.</returns>
        static inline auto create() {
            return SharedObject::create<VulkanInputAssembler>();
        }

    public:
        /// <inheritdoc />
        Enumerable<const VulkanVertexBufferLayout&> vertexBufferLayouts() const override;

		/// <inheritdoc />
		const VulkanVertexBufferLayout& vertexBufferLayout(UInt32 binding) const override;

		/// <inheritdoc />
		const VulkanIndexBufferLayout* indexBufferLayout() const noexcept override;

        /// <inheritdoc />
        PrimitiveTopology topology() const noexcept override;
    };

    /// <summary>
    /// Implements a Vulkan <see cref="IRasterizer" />.
    /// </summary>
    /// <seealso cref="VulkanRasterizerBuilder" />
    class LITEFX_VULKAN_API VulkanRasterizer final : public Rasterizer {
        LITEFX_BUILDER(VulkanRasterizerBuilder);
        friend struct SharedObject::Allocator<VulkanRasterizer>;

    private:
        /// <summary>
        /// Initializes a new Vulkan rasterizer state.
        /// </summary>
        /// <param name="polygonMode">The polygon mode used by the pipeline.</param>
        /// <param name="cullMode">The cull mode used by the pipeline.</param>
        /// <param name="cullOrder">The cull order used by the pipeline.</param>
        /// <param name="lineWidth">The line width used by the pipeline.</param>
        /// <param name="depthStencilState">The rasterizer depth/stencil state.</param>
        explicit VulkanRasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth = 1.f, const DepthStencilState& depthStencilState = {}) noexcept;

        /// <summary>
        /// Initializes a new Vulkan rasterizer state.
        /// </summary>
        explicit VulkanRasterizer() noexcept;

    private:
        /// <inheritdoc />
        VulkanRasterizer(VulkanRasterizer&&) noexcept = delete;

        /// <inheritdoc />
        VulkanRasterizer(const VulkanRasterizer&) = default;

        /// <inheritdoc />
        VulkanRasterizer& operator=(VulkanRasterizer&&) noexcept = delete;

        /// <inheritdoc />
        VulkanRasterizer& operator=(const VulkanRasterizer&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanRasterizer() noexcept override;

    public:
        /// <summary>
        /// Creates a new Vulkan rasterizer state.
        /// </summary>
        /// <param name="polygonMode">The polygon mode used by the pipeline.</param>
        /// <param name="cullMode">The cull mode used by the pipeline.</param>
        /// <param name="cullOrder">The cull order used by the pipeline.</param>
        /// <param name="lineWidth">The line width used by the pipeline.</param>
        /// <param name="depthStencilState">The rasterizer depth/stencil state.</param>
        /// <returns>A shared pointer to the newly created rasterizer instance.</returns>
        static inline auto create(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth = 1.f, const DepthStencilState& depthStencilState = {}) {
            return SharedObject::create<VulkanRasterizer>(polygonMode, cullMode, cullOrder, lineWidth, depthStencilState);
        }

        /// <summary>
        /// Creates a new Vulkan rasterizer state by copying an existing one.
        /// </summary>
        /// <param name="other">The rasterizer state to copy.</param>
        /// <returns>A shared pointer to the newly created rasterizer instance.</returns>
        static inline auto create(const VulkanRasterizer& other) {
            return SharedObject::create<VulkanRasterizer>(other);
        }

    private:
        /// <summary>
        /// Creates a new Vulkan rasterizer state.
        /// </summary>
        /// <returns>A shared pointer to the newly created rasterizer instance.</returns>
        static inline auto create() {
            return SharedObject::create<VulkanRasterizer>();
        }

    public:
        /// <summary>
        /// Sets the line width on the rasterizer.
        /// </summary>
        /// <remarks>
        /// Note that updating the line width requires the "wide lines" feature to be available. If it is not, the line width **must** be `1.0`. This
        /// constraint is not enforced by the engine and you are responsible of making sure that it is fulfilled.
        /// 
        /// Furthermore, note that the DirectX 12 back-end does have any representation for the line width concept. Thus you should only use the line 
        /// width, if you plan to only support Vulkan.
        /// </remarks>
        /// <returns>A reference to the line width.</returns>
        /// <seealso href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#features-wideLines" />
        virtual void updateLineWidth(Float lineWidth) noexcept;
    };

    /// <summary>
    /// Defines the base class for Vulkan pipeline state objects.
    /// </summary>
    /// <seealso cref="VulkanRenderPipeline" />
    /// <seealso cref="VulkanComputePipeline" />
    class LITEFX_VULKAN_API VulkanPipelineState : public virtual Pipeline<VulkanPipelineLayout, VulkanShaderProgram>, public Resource<VkPipeline> {
    protected:
        using Resource<VkPipeline>::Resource;

        VulkanPipelineState(VulkanPipelineState&&) noexcept = default;
        VulkanPipelineState(const VulkanPipelineState&) = delete;
        VulkanPipelineState& operator=(VulkanPipelineState&&) noexcept = default;
        VulkanPipelineState& operator=(const VulkanPipelineState&) = delete;

    public:
        ~VulkanPipelineState() noexcept override = default;

    public:
        /// <summary>
        /// Returns the type of the pipeline.
        /// </summary>
        /// <returns>The type of the pipeline.</returns>
        virtual VkPipelineBindPoint pipelineType() const noexcept = 0;

        /// <summary>
        /// Sets the current pipeline state on the <paramref name="commandBuffer" />.
        /// </summary>
        /// <param name="commandBuffer">The command buffer to set the current pipeline state on.</param>
        virtual void use(const VulkanCommandBuffer& commandBuffer) const = 0;
    };

    /// <summary>
    /// Records commands for a <see cref="VulkanQueue" />
    /// </summary>
    /// <seealso cref="VulkanQueue" />
    class LITEFX_VULKAN_API VulkanCommandBuffer final : public CommandBuffer<VulkanCommandBuffer, IVulkanBuffer, IVulkanVertexBuffer, IVulkanIndexBuffer, IVulkanImage, VulkanBarrier, VulkanPipelineState, VulkanBottomLevelAccelerationStructure, VulkanTopLevelAccelerationStructure>, public Resource<VkCommandBuffer> {
        LITEFX_IMPLEMENTATION(VulkanCommandBufferImpl);
        friend struct SharedObject::Allocator<VulkanCommandBuffer>;

    public:
        using base_type = CommandBuffer<VulkanCommandBuffer, IVulkanBuffer, IVulkanVertexBuffer, IVulkanIndexBuffer, IVulkanImage, VulkanBarrier, VulkanPipelineState, VulkanBottomLevelAccelerationStructure, VulkanTopLevelAccelerationStructure>;
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
        /// <summary>
        /// Initializes a command buffer from a command queue.
        /// </summary>
        /// <param name="queue">The parent command queue, the buffer gets submitted to.</param>
        /// <param name="begin">If set to <c>true</c>, the command buffer automatically starts recording by calling <see cref="begin" />.</param>
        /// <param name="primary"><c>true</c>, if the command buffer is a primary command buffer.</param>
        explicit VulkanCommandBuffer(const VulkanQueue& queue, bool begin = false, bool primary = true);

    private:
        /// <inheritdoc />
        VulkanCommandBuffer(VulkanCommandBuffer&&) noexcept = delete;

        /// <inheritdoc />
        VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;

        /// <inheritdoc />
        VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) noexcept = delete;

        /// <inheritdoc />
        VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
        
    public:
        /// <inheritdoc />
        ~VulkanCommandBuffer() noexcept override;

        // Factory method.
    public:
        /// <summary>
        /// Initializes a command buffer from a command queue.
        /// </summary>
        /// <param name="queue">The parent command queue, the buffer gets submitted to.</param>
        /// <param name="begin">If set to <c>true</c>, the command buffer automatically starts recording by calling <see cref="begin" />.</param>
        /// <param name="primary"><c>true</c>, if the command buffer is a primary command buffer.</param>
        static inline SharedPtr<VulkanCommandBuffer> create(const VulkanQueue& queue, bool begin = false, bool primary = true) {
            return SharedObject::create<VulkanCommandBuffer>(queue, begin, primary);
        }

        // Vulkan Command Buffer interface.
    public:
        /// <summary>
        /// Begins the command buffer as a secondary command buffer that inherits the state of <paramref name="renderPass" />.
        /// </summary>
        /// <param name="renderPass">The render pass state to inherit.</param>
        virtual void begin(const VulkanRenderPass& renderPass) const;

        // CommandBuffer interface.
    public:
        /// <inheritdoc />
        SharedPtr<const VulkanQueue> queue() const noexcept;

        /// <inheritdoc />
        void begin() const override;

        /// <inheritdoc />
        void end() const override;

        /// <inheritdoc />
        void track(SharedPtr<const IBuffer> buffer) const override;

        /// <inheritdoc />
        void track(SharedPtr<const IImage> image) const override;

        /// <inheritdoc />
        void track(SharedPtr<const ISampler> sampler) const override;

        /// <inheritdoc />
        bool isSecondary() const noexcept override;

        /// <inheritdoc />
        void setViewports(Span<const IViewport*> viewports) const override;

        /// <inheritdoc />
        void setViewports(const IViewport* viewport) const override;

        /// <inheritdoc />
        void setScissors(Span<const IScissor*> scissors) const override;

        /// <inheritdoc />
        void setScissors(const IScissor* scissor) const override;

        /// <inheritdoc />
        void setBlendFactors(const Vector4f& blendFactors) const noexcept override;

        /// <inheritdoc />
        void setStencilRef(UInt32 stencilRef) const noexcept override;

        /// <inheritdoc />
        UInt64 submit() const override;

        /// <inheritdoc />
        [[nodiscard]] UniquePtr<VulkanBarrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const override;

        /// <inheritdoc />
        void barrier(const VulkanBarrier& barrier) const noexcept override;

        /// <inheritdoc />
        void transfer(const IVulkanBuffer& source, const IVulkanBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// <inheritdoc />
        void transfer(const void* const data, size_t size, const IVulkanBuffer& target, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// <inheritdoc />
        void transfer(Span<const void* const> data, size_t elementSize, const IVulkanBuffer& target, UInt32 firstElement = 0) const override;

        /// <inheritdoc />
        void transfer(const IVulkanBuffer& source, const IVulkanImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void transfer(const void* const data, size_t size, const IVulkanImage& target, UInt32 subresource = 0) const override;

        /// <inheritdoc />
        void transfer(Span<const void* const> data, size_t elementSize, const IVulkanImage& target, UInt32 firstSubresource = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void transfer(const IVulkanImage& source, const IVulkanImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void transfer(const IVulkanImage& source, const IVulkanBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void transfer(const SharedPtr<const IVulkanBuffer>& source, const IVulkanBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// <inheritdoc />
        void transfer(const SharedPtr<const IVulkanBuffer>& source, const IVulkanImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const override;

        /// <inheritdoc />
        void transfer(const SharedPtr<const IVulkanImage>& source, const IVulkanImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void transfer(const SharedPtr<const IVulkanImage>& source, const IVulkanBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const override;

        /// <inheritdoc />
        void use(const VulkanPipelineState& pipeline) const noexcept override;

		/// <inheritdoc />
		void bind(const VulkanDescriptorSet& descriptorSet) const override;

        /// <inheritdoc />
        void bind(Span<const VulkanDescriptorSet*> descriptorSets) const override;

		/// <inheritdoc />
		void bind(const VulkanDescriptorSet& descriptorSet, const VulkanPipelineState& pipeline) const override;

        /// <inheritdoc />
        void bind(Span<const VulkanDescriptorSet*> descriptorSets, const VulkanPipelineState& pipeline) const override;

        /// <inheritdoc />
        void bind(const IVulkanVertexBuffer& buffer) const noexcept override;

        /// <inheritdoc />
        void bind(const IVulkanIndexBuffer& buffer) const noexcept override;

        /// <inheritdoc />
        void dispatch(const Vector3u& threadCount) const noexcept override;

        /// <inheritdoc />
        void dispatchIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// <inheritdoc />
        void dispatchMesh(const Vector3u& threadCount) const noexcept override;

        /// <inheritdoc />
        void dispatchMeshIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// <inheritdoc />
        void dispatchMeshIndirect(const IVulkanBuffer& batchBuffer, const IVulkanBuffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// <inheritdoc />
        void draw(UInt32 vertices, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const noexcept override;

        /// <inheritdoc />
        void drawIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// <inheritdoc />
        void drawIndirect(const IVulkanBuffer& batchBuffer, const IVulkanBuffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// <inheritdoc />
        void drawIndexed(UInt32 indices, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const noexcept override;

        /// <inheritdoc />
        void drawIndexedIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// <inheritdoc />
        void drawIndexedIndirect(const IVulkanBuffer& batchBuffer, const IVulkanBuffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// <inheritdoc />
        void pushConstants(const VulkanPushConstantsLayout& layout, const void* const memory) const override;

        /// <inheritdoc />
        void writeTimingEvent(const SharedPtr<const TimingEvent>& timingEvent) const override;

        /// <inheritdoc />
        void execute(const SharedPtr<const VulkanCommandBuffer>& commandBuffer) const override;

        /// <inheritdoc />
        void execute(Enumerable<SharedPtr<const VulkanCommandBuffer>> commandBuffers) const override;

        /// <inheritdoc />
        void buildAccelerationStructure(VulkanBottomLevelAccelerationStructure& blas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const override;

        /// <inheritdoc />
        void buildAccelerationStructure(VulkanTopLevelAccelerationStructure& tlas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const override;

        /// <inheritdoc />
        void updateAccelerationStructure(VulkanBottomLevelAccelerationStructure& blas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const override;

        /// <inheritdoc />
        void updateAccelerationStructure(VulkanTopLevelAccelerationStructure& tlas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const override;

        /// <inheritdoc />
        void copyAccelerationStructure(const VulkanBottomLevelAccelerationStructure& from, const VulkanBottomLevelAccelerationStructure& to, bool compress = false) const noexcept override;

        /// <inheritdoc />
        void copyAccelerationStructure(const VulkanTopLevelAccelerationStructure& from, const VulkanTopLevelAccelerationStructure& to, bool compress = false) const noexcept override;

        /// <inheritdoc />
        void traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IVulkanBuffer& rayGenerationShaderBindingTable, const IVulkanBuffer* missShaderBindingTable, const IVulkanBuffer* hitShaderBindingTable, const IVulkanBuffer* callableShaderBindingTable) const noexcept override;

    private:
        inline SharedPtr<const ICommandQueue> getQueue() const noexcept override {
            return std::static_pointer_cast<const ICommandQueue>(this->queue());
        }

        void releaseSharedState() const override;
    };

    /// <summary>
    /// Implements a Vulkan command queue.
    /// </summary>
    /// <seealso cref="VulkanCommandBuffer" />
    class LITEFX_VULKAN_API VulkanQueue final : public CommandQueue<VulkanCommandBuffer>, public Resource<VkQueue> {
        LITEFX_IMPLEMENTATION(VulkanQueueImpl);
        friend struct SharedObject::Allocator<VulkanQueue>;

    public:
        using base_type = CommandQueue<VulkanCommandBuffer>;
        using base_type::submit;

    private:
        /// <summary>
        /// Initializes the Vulkan command queue.
        /// </summary>
        /// <param name="device">The device, commands get send to.</param>
        /// <param name="type">The type of the command queue.</param>
        /// <param name="priority">The priority, of which commands are issued on the device.</param>
        /// <param name="familyId">The ID of the queue family.</param>
        /// <param name="queueId">The ID of the queue.</param>
        explicit VulkanQueue(const VulkanDevice& device, QueueType type, QueuePriority priority, UInt32 familyId, UInt32 queueId);

    private:
        /// <inheritdoc />
        VulkanQueue(VulkanQueue&&) noexcept = delete;

        /// <inheritdoc />
        VulkanQueue(const VulkanQueue&) = delete;

        /// <inheritdoc />
        VulkanQueue& operator=(VulkanQueue&&) noexcept = delete;

        /// <inheritdoc />
        VulkanQueue& operator=(const VulkanQueue&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanQueue() noexcept override;

    public:
        /// <summary>
        /// Creates a new Vulkan command queue.
        /// </summary>
        /// <param name="device">The device, commands get send to.</param>
        /// <param name="type">The type of the command queue.</param>
        /// <param name="priority">The priority, of which commands are issued on the device.</param>
        /// <param name="familyId">The ID of the queue family.</param>
        /// <param name="queueId">The ID of the queue.</param>
        /// <returns>A pointer to the newly created command queue instance.</returns>
        static inline SharedPtr<VulkanQueue> create(const VulkanDevice& device, QueueType type, QueuePriority priority, UInt32 familyId, UInt32 queueId) {
            return SharedObject::create<VulkanQueue>(device, type, priority, familyId, queueId);
        }

        // VulkanQueue interface.
    public:
        /// <summary>
        /// Returns a pointer to the device that provides this queue or `nullptr`, if the device has already been released.
        /// </summary>
        /// <returns>A pointer to the queue's parent device.</returns>
        SharedPtr<const VulkanDevice> device() const noexcept;

        /// <summary>
        /// Returns the queue family ID.
        /// </summary>
        /// <returns>The queue family ID.</returns>
        UInt32 familyId() const noexcept;

        /// <summary>
        /// Returns the queue ID.
        /// </summary>
        /// <returns>The queue ID.</returns>
        UInt32 queueId() const noexcept;

        /// <summary>
        /// Returns the internal timeline semaphore used to synchronize the queue execution.
        /// </summary>
        /// <returns>The internal timeline semaphore.</returns>
        const VkSemaphore& timelineSemaphore() const noexcept;

        // CommandQueue interface.
    public:
        /// <inheritdoc />
        QueuePriority priority() const noexcept override;

        /// <inheritdoc />
        QueueType type() const noexcept override;

#ifdef LITEFX_BUILD_SUPPORT_DEBUG_MARKERS
    public:
        /// <inheritdoc />
        void beginDebugRegion(const String& label, const Vectors::ByteVector3& color = DEFAULT_DEBUG_COLOR) const noexcept override;

        /// <inheritdoc />
        void endDebugRegion() const noexcept override;

        /// <inheritdoc />
        void setDebugMarker(const String& label, const Vectors::ByteVector3& color = DEFAULT_DEBUG_COLOR) const noexcept override;
#endif // LITEFX_BUILD_SUPPORT_DEBUG_MARKERS

    public:
        /// <inheritdoc />
        SharedPtr<VulkanCommandBuffer> createCommandBuffer(bool beginRecording = false, bool secondary = false) const override;

        /// <inheritdoc />
        UInt64 submit(const SharedPtr<const VulkanCommandBuffer>& commandBuffer) const override;

        /// <inheritdoc />
        UInt64 submit(Enumerable<SharedPtr<const VulkanCommandBuffer>> commandBuffers) const override;

        /// <inheritdoc />
        void waitFor(UInt64 fence) const override;

        /// <inheritdoc />
        void waitFor(const VulkanQueue& queue, UInt64 fence) const noexcept;

        /// <inheritdoc />
        UInt64 currentFence() const noexcept override;

    private:
        inline void waitForQueue(const ICommandQueue& queue, UInt64 fence) const override {
            auto vkQueue = dynamic_cast<const VulkanQueue*>(&queue);

            if (vkQueue == nullptr) [[unlikely]]
                throw InvalidArgumentException("queue", "Cannot wait for queues from other backends.");

            this->waitFor(*vkQueue, fence);
        }
    };

    /// <summary>
    /// Implements a Vulkan <see cref="RenderPipeline" />.
    /// </summary>
    /// <seealso cref="VulkanComputePipeline" />
    /// <seealso cref="VulkanRenderPipelineBuilder" />
    class LITEFX_VULKAN_API VulkanRenderPipeline final : public virtual VulkanPipelineState, public RenderPipeline<VulkanPipelineLayout, VulkanShaderProgram, VulkanInputAssembler, VulkanRasterizer> {
        LITEFX_IMPLEMENTATION(VulkanRenderPipelineImpl);
        LITEFX_BUILDER(VulkanRenderPipelineBuilder);

    public:
        /// <summary>
        /// Initializes a new Vulkan render pipeline.
        /// </summary>
        /// <param name="renderPass">The parent render pass.</param>
        /// <param name="shaderProgram">The shader program used by the pipeline.</param>
        /// <param name="layout">The layout of the pipeline.</param>
        /// <param name="inputAssembler">The input assembler state of the pipeline.</param>
        /// <param name="rasterizer">The rasterizer state of the pipeline.</param>
        /// <param name="samples">The initial multi-sampling level of the render pipeline.</param>
        /// <param name="enableAlphaToCoverage">Whether or not to enable Alpha-to-Coverage multi-sampling.</param>
        /// <param name="name">The optional name of the render pipeline.</param>
        explicit VulkanRenderPipeline(const VulkanRenderPass& renderPass, const SharedPtr<VulkanShaderProgram>& shaderProgram, const SharedPtr<VulkanPipelineLayout>& layout, const SharedPtr<VulkanInputAssembler>& inputAssembler, const SharedPtr<VulkanRasterizer>& rasterizer, MultiSamplingLevel samples = MultiSamplingLevel::x1, bool enableAlphaToCoverage = false, const String& name = "");

        /// <inheritdoc />
        VulkanRenderPipeline(VulkanRenderPipeline&&) noexcept = delete;

        /// <inheritdoc />
        VulkanRenderPipeline(const VulkanRenderPipeline&) = delete;

        /// <inheritdoc />
        VulkanRenderPipeline& operator=(VulkanRenderPipeline&&) noexcept = delete;

        /// <inheritdoc />
        VulkanRenderPipeline& operator=(const VulkanRenderPipeline&) = delete;

        /// <inheritdoc />
        ~VulkanRenderPipeline() noexcept override;

    private:
        /// <summary>
        /// Initializes a new Vulkan render pipeline.
        /// </summary>
        /// <param name="renderPass">The parent render pass.</param>
        /// <param name="name">The optional name of the render pipeline.</param>
        VulkanRenderPipeline(const VulkanRenderPass& renderPass, const String& name = "");

        // Pipeline interface.
    public:
        /// <inheritdoc />
        SharedPtr<const VulkanShaderProgram> program() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const VulkanPipelineLayout> layout() const noexcept override;

        // RenderPipeline interface.
    public:
        /// <inheritdoc />
        SharedPtr<VulkanInputAssembler> inputAssembler() const noexcept override;

        /// <inheritdoc />
        SharedPtr<VulkanRasterizer> rasterizer() const noexcept override;

        /// <inheritdoc />
        bool alphaToCoverage() const noexcept override;

        /// <inheritdoc />
        MultiSamplingLevel samples() const noexcept override;

        /// <inheritdoc />
        void updateSamples(MultiSamplingLevel samples) override;

        // VulkanPipelineState interface.
    public:
        /// <inheritdoc />
        VkPipelineBindPoint pipelineType() const noexcept override;

        /// <inheritdoc />
        void use(const VulkanCommandBuffer& commandBuffer) const override;
    };

    /// <summary>
    /// Implements a Vulkan <see cref="ComputePipeline" />.
    /// </summary>
    /// <seealso cref="VulkanRenderPipeline" />
    /// <seealso cref="VulkanComputePipelineBuilder" />
    class LITEFX_VULKAN_API VulkanComputePipeline final : public virtual VulkanPipelineState, public ComputePipeline<VulkanPipelineLayout, VulkanShaderProgram> {
        LITEFX_IMPLEMENTATION(VulkanComputePipelineImpl);
        LITEFX_BUILDER(VulkanComputePipelineBuilder);

    public:
        /// <summary>
        /// Initializes a new Vulkan compute pipeline.
        /// </summary>
        /// <param name="device">The parent device.</param>
        /// <param name="layout">The layout of the pipeline.</param>
        /// <param name="shaderProgram">The shader program used by the pipeline.</param>
        /// <param name="name">The optional debug name of the render pipeline.</param>
        explicit VulkanComputePipeline(const VulkanDevice& device, const SharedPtr<VulkanPipelineLayout>& layout, const SharedPtr<VulkanShaderProgram>& shaderProgram, const String& name = "");

        /// <inheritdoc />
        VulkanComputePipeline(VulkanComputePipeline&&) noexcept;

        /// <inheritdoc />
        VulkanComputePipeline(const VulkanComputePipeline&) = delete;

        /// <inheritdoc />
        VulkanComputePipeline& operator=(VulkanComputePipeline&&) noexcept;

        /// <inheritdoc />
        VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;

        /// <inheritdoc />
        ~VulkanComputePipeline() noexcept override;

    private:
        /// <summary>
        /// Initializes a new Vulkan compute pipeline.
        /// </summary>
        /// <param name="device">The parent device.</param>
        VulkanComputePipeline(const VulkanDevice& device) noexcept;

        // Pipeline interface.
    public:
        /// <inheritdoc />
        SharedPtr<const VulkanShaderProgram> program() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const VulkanPipelineLayout> layout() const noexcept override;

        // VulkanPipelineState interface.
    public:
        /// <inheritdoc />
        VkPipelineBindPoint pipelineType() const noexcept override;

        /// <inheritdoc />
        void use(const VulkanCommandBuffer& commandBuffer) const override;
    };
    
    /// <summary>
    /// Implements a Vulkan <see cref="RayTracingPipeline" />.
    /// </summary>
    /// <seealso cref="VulkanRenderPipeline" />
    /// <seealso cref="VulkanRayTracingPipelineBuilder" />
    class LITEFX_VULKAN_API VulkanRayTracingPipeline final : public virtual VulkanPipelineState, public RayTracingPipeline<VulkanPipelineLayout, VulkanShaderProgram> {
        LITEFX_IMPLEMENTATION(VulkanRayTracingPipelineImpl);
        LITEFX_BUILDER(VulkanRayTracingPipelineBuilder);

    public:
        /// <summary>
        /// Initializes a new Vulkan ray-tracing pipeline.
        /// </summary>
        /// <param name="device">The parent device.</param>
        /// <param name="layout">The layout of the pipeline.</param>
        /// <param name="shaderProgram">The shader program used by the pipeline.</param>
        /// <param name="shaderRecords">The shader record collection that is used to build the shader binding table for the pipeline.</param>
        /// <param name="maxRecursionDepth">The maximum number of ray bounces.</param>
        /// <param name="maxPayloadSize">The maximum size for ray payloads in the pipeline.</param>
        /// <param name="maxAttributeSize">The maximum size for ray attributes in the pipeline.</param>
        /// <param name="name">The optional debug name of the render pipeline.</param>
        explicit VulkanRayTracingPipeline(const VulkanDevice& device, const SharedPtr<VulkanPipelineLayout>& layout, const SharedPtr<VulkanShaderProgram>& shaderProgram, ShaderRecordCollection&& shaderRecords, UInt32 maxRecursionDepth = 10, UInt32 maxPayloadSize = 0, UInt32 maxAttributeSize = 32, const String& name = ""); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

        /// <inheritdoc />
        VulkanRayTracingPipeline(VulkanRayTracingPipeline&&) noexcept;

        /// <inheritdoc />
        VulkanRayTracingPipeline(const VulkanRayTracingPipeline&) = delete;

        /// <inheritdoc />
        VulkanRayTracingPipeline& operator=(VulkanRayTracingPipeline&&) noexcept;

        /// <inheritdoc />
        VulkanRayTracingPipeline& operator=(const VulkanRayTracingPipeline&) = delete;

        /// <inheritdoc />
        ~VulkanRayTracingPipeline() noexcept override;

    private:
        /// <summary>
        /// Initializes a new Vulkan ray-tracing pipeline.
        /// </summary>
        /// <param name="device">The parent device.</param>
        /// <param name="shaderRecords">The shader record collection that is used to build the shader binding table for the pipeline.</param>
        VulkanRayTracingPipeline(const VulkanDevice& device, ShaderRecordCollection&& shaderRecords) noexcept;

        // Pipeline interface.
    public:
        /// <inheritdoc />
        SharedPtr<const VulkanShaderProgram> program() const noexcept override;

        /// <inheritdoc />
        SharedPtr<const VulkanPipelineLayout> layout() const noexcept override;

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
        SharedPtr<IVulkanBuffer> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups = ShaderBindingGroup::All) const override;

        // VulkanPipelineState interface.
    public:
        /// <inheritdoc />
        VkPipelineBindPoint pipelineType() const noexcept override;

        /// <inheritdoc />
        void use(const VulkanCommandBuffer& commandBuffer) const override;
    };

    /// <summary>
    /// Implements a Vulkan frame buffer.
    /// </summary>
    /// <seealso cref="VulkanRenderPass" />
    class LITEFX_VULKAN_API VulkanFrameBuffer final : public FrameBuffer<IVulkanImage> {
        LITEFX_IMPLEMENTATION(VulkanFrameBufferImpl);
        friend struct SharedObject::Allocator<VulkanFrameBuffer>;

    public:
        using FrameBuffer::addImage;
        using FrameBuffer::mapRenderTarget;
        using FrameBuffer::mapRenderTargets;

    private:
        /// <summary>
        /// Initializes a Vulkan frame buffer.
        /// </summary>
        /// <param name="device">The device the frame buffer is allocated on.</param>
        /// <param name="renderArea">The initial size of the render area.</param>
        /// <param name="name">The name of the frame buffer.</param>
        VulkanFrameBuffer(const VulkanDevice& device, const Size2d& renderArea, StringView name = "");

    private:
        /// <inheritdoc />
        VulkanFrameBuffer(VulkanFrameBuffer&&) noexcept = delete;
        
        /// <inheritdoc />
        VulkanFrameBuffer(const VulkanFrameBuffer&) = delete;

        /// <inheritdoc />
        VulkanFrameBuffer& operator=(VulkanFrameBuffer&&) noexcept = delete;

        /// <inheritdoc />
        VulkanFrameBuffer& operator=(const VulkanFrameBuffer&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanFrameBuffer() noexcept override;

    public:
        /// <summary>
        /// Initializes a Vulkan frame buffer.
        /// </summary>
        /// <param name="device">The device the frame buffer is allocated on.</param>
        /// <param name="renderArea">The initial size of the render area.</param>
        /// <param name="name">The name of the frame buffer.</param>
        /// <returns>A pointer to the newly created frame buffer instance.</returns>
        static inline SharedPtr<VulkanFrameBuffer> create(const VulkanDevice& device, const Size2d& renderArea, StringView name = "") {
            return SharedObject::create<VulkanFrameBuffer>(device, renderArea, name);
        }

        // Vulkan frame buffer interface.
    public:
        /// <summary>
        /// Returns the image view for an image at the specified index.
        /// </summary>
        /// <param name="imageIndex">The index of the image for which the image view should be returned.</param>
        /// <returns>The image view for the image.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the provided image index does not address an image within the frame buffer.</exception>
        VkImageView imageView(UInt32 imageIndex) const;

        /// <summary>
        /// Returns the image view for an image with the specified name.
        /// </summary>
        /// <param name="imageName">The name of the image for which the image view should be returned.</param>
        /// <returns>The image view for the image.</returns>
        /// <exception cref="InvalidArgumentException">Thrown, if the provided image name does refer to an image within the frame buffer.</exception>
        VkImageView imageView(StringView imageName) const;

        /// <summary>
        /// Returns the image view for an image mapped to the specified render target.
        /// </summary>
        /// <param name="renderTarget">The render target for which to return the image image view.</param>
        /// <returns>The image view for the image.</returns>
        /// <exception cref="InvalidArgumentException">Thrown, if the provided render target is not mapped to an image within the frame buffer.</exception>
        VkImageView imageView(const RenderTarget& renderTarget) const;

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
        const Array<SharedPtr<const IVulkanImage>>& images() const override;

        /// <inheritdoc />
        inline const IVulkanImage& operator[](UInt32 index) const override {
            return this->image(index);
        }

        /// <inheritdoc />
        const IVulkanImage& image(UInt32 index) const override;

        /// <inheritdoc />
        inline const IVulkanImage& operator[](const RenderTarget& renderTarget) const override {
            return this->image(renderTarget);
        }

        /// <inheritdoc />
        const IVulkanImage& image(const RenderTarget& renderTarget) const override;

        /// <inheritdoc />
        inline const IVulkanImage& operator[](StringView renderTargetName) const override {
            return this->resolveImage(hash(renderTargetName));
        }

        /// <inheritdoc />
        inline const IVulkanImage& image(StringView renderTargetName) const override {
            return this->resolveImage(hash(renderTargetName));
        }

        /// <inheritdoc />
        const IVulkanImage& resolveImage(UInt64 hash) const override;

        /// <inheritdoc />
        void addImage(const String& name, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) override;

        /// <inheritdoc />
        void addImage(const String& name, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) override;

        /// <inheritdoc />
        void resize(const Size2d& renderArea) override;
    };

    /// <summary>
    /// Implements a Vulkan render pass.
    /// </summary>
    /// <seealso cref="VulkanRenderPassBuilder" />
    class LITEFX_VULKAN_API VulkanRenderPass final : public RenderPass<VulkanQueue, VulkanFrameBuffer> {
        LITEFX_IMPLEMENTATION(VulkanRenderPassImpl);
        LITEFX_BUILDER(VulkanRenderPassBuilder);
        friend struct SharedObject::Allocator<VulkanRenderPass>;

    public:
        using base_type = RenderPass<VulkanQueue, VulkanFrameBuffer>;

    private:
        /// <summary>
        /// Creates and initializes a new Vulkan render pass instance that executes on the default graphics queue.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        explicit VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u);

        /// <summary>
        /// Creates and initializes a new Vulkan render pass instance that executes on the default graphics queue.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="name">The name of the render pass state resource.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        explicit VulkanRenderPass(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u);
        
        /// <summary>
        /// Creates and initializes a new Vulkan render pass instance.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="queue">The command queue to execute the render pass on.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        explicit VulkanRenderPass(const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u);

        /// <summary>
        /// Creates and initializes a new Vulkan render pass instance.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="name">The name of the render pass state resource.</param>
        /// <param name="queue">The command queue to execute the render pass on.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        explicit VulkanRenderPass(const VulkanDevice& device, const String& name, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u);

    private:
        /// <inheritdoc />
        VulkanRenderPass(VulkanRenderPass&&) noexcept = delete;

        /// <inheritdoc />
        VulkanRenderPass(const VulkanRenderPass&) = delete;

        /// <inheritdoc />
        VulkanRenderPass& operator=(VulkanRenderPass&&) noexcept = delete;

        /// <inheritdoc />
        VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanRenderPass() noexcept override;

    public:
        /// <summary>
        /// Creates and initializes a new Vulkan render pass instance that executes on the default graphics queue.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        /// <returns>A pointer to the newly created render pass instance.</returns>
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u) {
            return SharedObject::create<VulkanRenderPass>(device, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers);
        }

        /// <summary>
        /// Creates and initializes a new Vulkan render pass instance that executes on the default graphics queue.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="name">The name of the render pass state resource.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        /// <returns>A pointer to the newly created render pass instance.</returns>
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u) {
            return SharedObject::create<VulkanRenderPass>(device, name, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers);
        }

        /// <summary>
        /// Creates and initializes a new Vulkan render pass instance.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="queue">The command queue to execute the render pass on.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        /// <returns>A pointer to the newly created render pass instance.</returns>
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u) {
            return SharedObject::create<VulkanRenderPass>(device, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers);
        }

        /// <summary>
        /// Creates and initializes a new Vulkan render pass instance.
        /// </summary>
        /// <param name="device">The parent device instance.</param>
        /// <param name="name">The name of the render pass state resource.</param>
        /// <param name="queue">The command queue to execute the render pass on.</param>
        /// <param name="renderTargets">The render targets that are output by the render pass.</param>
        /// <param name="inputAttachments">The input attachments that are read by the render pass.</param>
        /// <param name="inputAttachmentSamplerBinding">The binding point for the input attachment sampler.</param>
        /// <param name="secondaryCommandBuffers">The number of command buffers that can be used for recording multi-threaded commands during the render pass.</param>
        /// <returns>A pointer to the newly created render pass instance.</returns>
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, const String& name, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u) {
            return SharedObject::create<VulkanRenderPass>(device, name, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers);
        }

    private:
        /// <summary>
        /// Creates an uninitialized Vulkan render pass instance.
        /// </summary>
        /// <param name="device">The parent device of the render pass.</param>
        /// <param name="name">The name of the render pass state resource.</param>
        explicit VulkanRenderPass(const VulkanDevice& device, const String& name = "");

        /// <summary>
        /// Creates an uninitialized Vulkan render pass instance.
        /// </summary>
        /// <remarks>
        /// This factory is called by the <see cref="VulkanRenderPassBuilder" /> in order to create a render pass instance without initializing it. The instance is only initialized 
        /// after calling <see cref="VulkanRenderPassBuilder::go" />.
        /// </remarks>
        /// <param name="device">The parent device of the render pass.</param>
        /// <param name="name">The name of the render pass state resource.</param>
        /// <returns>A pointer to the newly created render pass instance.</returns>
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, const String& name = "") {
            return SharedObject::create<VulkanRenderPass>(device, name);
        }

        // RenderPass interface.
    public:
        /// <summary>
        /// Returns a reference of the device that provides this queue.
        /// </summary>
        /// <returns>A reference of the queue's parent device.</returns>
        const VulkanDevice& device() const noexcept /*override*/;

        /// <inheritdoc />
        SharedPtr<const VulkanFrameBuffer> activeFrameBuffer() const noexcept override;

        /// <inheritdoc />
        const VulkanQueue& commandQueue() const noexcept override;

        /// <inheritdoc />
        Enumerable<SharedPtr<const VulkanCommandBuffer>> commandBuffers() const override;

        /// <inheritdoc />
        SharedPtr<const VulkanCommandBuffer> commandBuffer(UInt32 index) const override;

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
        void begin(const VulkanFrameBuffer& frameBuffer) const override;

        /// <inheritdoc />
        UInt64 end() const override;
    };

    /// <summary>
    /// Implements a Vulkan swap chain.
    /// </summary>
    class LITEFX_VULKAN_API VulkanSwapChain final : public SwapChain<IVulkanImage> {
        LITEFX_IMPLEMENTATION(VulkanSwapChainImpl);
        friend class VulkanImage;
        friend class VulkanDevice;

    public:
        using base_type = SwapChain<IVulkanImage>;

    private:
        /// <summary>
        /// Initializes a Vulkan swap chain.
        /// </summary>
        /// <param name="device">The device that owns the swap chain.</param>
        /// <param name="format">The initial surface format.</param>
        /// <param name="renderArea">The initial size of the render area.</param>
        /// <param name="buffers">The initial number of buffers.</param>
        /// <param name="enableVsync">`true` if vertical synchronization should be used, otherwise `false`.</param>
        explicit VulkanSwapChain(const VulkanDevice& device, Format surfaceFormat = Format::B8G8R8A8_SRGB, const Size2d& renderArea = { 800, 600 }, UInt32 buffers = 3, bool enableVsync = false); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    public:
        /// <inheritdoc />
        VulkanSwapChain(VulkanSwapChain&&) noexcept = delete;

        /// <inheritdoc />
        VulkanSwapChain(const VulkanSwapChain&) = delete;

        /// <inheritdoc />
        VulkanSwapChain& operator=(VulkanSwapChain&&) noexcept = delete;

        /// <inheritdoc />
        VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

        /// <inheritdoc />
        ~VulkanSwapChain() noexcept override;

        // Vulkan Swap Chain interface.
    public:
        /// <summary>
        /// Returns the query pool for the current frame.
        /// </summary>
        /// <returns>A reference of the query pool for the current frame.</returns>
        virtual const VkQueryPool& timestampQueryPool() const noexcept;

        // SwapChain interface.
    public:
        /// <inheritdoc />
        const Array<SharedPtr<const TimingEvent>>& timingEvents() const override;

        /// <inheritdoc />
        SharedPtr<const TimingEvent> timingEvent(UInt32 queryId) const override;

        /// <inheritdoc />
        UInt64 readTimingEvent(SharedPtr<const TimingEvent> timingEvent) const override;

        /// <inheritdoc />
        UInt32 resolveQueryId(SharedPtr<const TimingEvent> timingEvent) const override;

        /// <inheritdoc />
        const IGraphicsDevice& device() const override;

        /// <inheritdoc />
        Format surfaceFormat() const noexcept override;

        /// <inheritdoc />
        UInt32 buffers() const noexcept override;

        /// <inheritdoc />
        const Size2d& renderArea() const noexcept override;

        /// <inheritdoc />
        bool verticalSynchronization() const noexcept override;

        /// <inheritdoc />
        IVulkanImage* image(UInt32 backBuffer) const override;

        /// <inheritdoc />
        const IVulkanImage& image() const noexcept override;

        /// <inheritdoc />
        const Array<SharedPtr<IVulkanImage>>& images() const noexcept override;

        /// <inheritdoc />
        void present(UInt64 fence) const override;

    public:
        /// <inheritdoc />
        Enumerable<Format> getSurfaceFormats() const override;

        /// <inheritdoc />
        void addTimingEvent(SharedPtr<const TimingEvent> timingEvent) override;

        /// <inheritdoc />
        void reset(Format surfaceFormat, const Size2d& renderArea, UInt32 buffers, bool enableVsync = false) override;

        /// <inheritdoc />
        [[nodiscard]] UInt32 swapBackBuffer() const override;
    };

    /// <summary>
    /// A graphics factory that produces objects for a <see cref="VulkanDevice" />.
    /// </summary>
    /// <remarks>
    /// Internally this factory implementation is based on <a href="https://gpuopen.com/vulkan-memory-allocator/" target="_blank">Vulkan Memory Allocator</a>.
    /// </remarks>
    class LITEFX_VULKAN_API VulkanGraphicsFactory final : public GraphicsFactory<VulkanDescriptorLayout, IVulkanBuffer, IVulkanVertexBuffer, IVulkanIndexBuffer, IVulkanImage, IVulkanSampler, VulkanBottomLevelAccelerationStructure, VulkanTopLevelAccelerationStructure> {
        LITEFX_IMPLEMENTATION(VulkanGraphicsFactoryImpl);
        friend class VulkanDevice;
        friend struct SharedObject::Allocator<VulkanGraphicsFactory>;

    public:
        using base_type = GraphicsFactory<VulkanDescriptorLayout, IVulkanBuffer, IVulkanVertexBuffer, IVulkanIndexBuffer, IVulkanImage, IVulkanSampler, VulkanBottomLevelAccelerationStructure, VulkanTopLevelAccelerationStructure>;
        using base_type::createBuffer;
        using base_type::createVertexBuffer;
        using base_type::createIndexBuffer;
        using base_type::createTexture;
        using base_type::createTextures;
        using base_type::createSampler;
        using base_type::createSamplers;

    private:
        /// <summary>
        /// Creates a new graphics factory.
        /// </summary>
        /// <param name="device">The device the factory should produce objects for.</param>
        explicit VulkanGraphicsFactory(const VulkanDevice& device);

    private:
        /// <inheritdoc />
        VulkanGraphicsFactory(VulkanGraphicsFactory&&) noexcept = delete;

        /// <inheritdoc />
        VulkanGraphicsFactory(const VulkanGraphicsFactory&) = delete;

        /// <inheritdoc />
        VulkanGraphicsFactory& operator=(VulkanGraphicsFactory&&) noexcept = delete;

        /// <inheritdoc />
        VulkanGraphicsFactory& operator=(const VulkanGraphicsFactory&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanGraphicsFactory() noexcept override;

    private:
        /// <summary>
        /// Creates a new graphics factory.
        /// </summary>
        /// <param name="device">The device the factory should produce objects for.</param>
        static inline SharedPtr<VulkanGraphicsFactory> create(const VulkanDevice& device) {
            return SharedObject::create<VulkanGraphicsFactory>(device);
        }

        /// <summary>
        /// Creates a descriptor heap.
        /// </summary>
        /// <param name="heapSize">The size of the descriptor heap buffer in bytes.</param>
        /// <returns>A buffer that provides memory for the descriptor heap.</returns>
        SharedPtr<IVulkanBuffer> createDescriptorHeap(size_t heapSize) const;

        /// <summary>
        /// Creates a descriptor heap.
        /// </summary>
        /// <param name="name">The name of the descriptor heap.</param>
        /// <param name="heapSize">The size of the descriptor heap buffer in bytes.</param>
        /// <returns>A buffer that provides memory for the descriptor heap.</returns>
        SharedPtr<IVulkanBuffer> createDescriptorHeap(const String& name, size_t heapSize) const;

    public:
        /// <inheritdoc />
        SharedPtr<IVulkanBuffer> createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        SharedPtr<IVulkanBuffer> createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        SharedPtr<IVulkanVertexBuffer> createVertexBuffer(const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        SharedPtr<IVulkanVertexBuffer> createVertexBuffer(const String& name, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        SharedPtr<IVulkanIndexBuffer> createIndexBuffer(const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        SharedPtr<IVulkanIndexBuffer> createIndexBuffer(const String& name, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        SharedPtr<IVulkanImage> createTexture(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        SharedPtr<IVulkanImage> createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        Generator<SharedPtr<IVulkanImage>> createTextures(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const override;

        /// <inheritdoc />
        SharedPtr<IVulkanSampler> createSampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// <inheritdoc />
        SharedPtr<IVulkanSampler> createSampler(const String& name, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// <inheritdoc />
        Generator<SharedPtr<IVulkanSampler>> createSamplers(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// <inheritdoc />
        UniquePtr<VulkanBottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const override;

        /// <inheritdoc />
        UniquePtr<VulkanTopLevelAccelerationStructure> createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const override;
    };

    /// <summary>
    /// Implements a Vulkan graphics device.
    /// </summary>
    class LITEFX_VULKAN_API VulkanDevice final : public GraphicsDevice<VulkanGraphicsFactory, VulkanSurface, VulkanGraphicsAdapter, VulkanSwapChain, VulkanQueue, VulkanRenderPass, VulkanRenderPipeline, VulkanComputePipeline, VulkanRayTracingPipeline, VulkanBarrier>, public Resource<VkDevice> {
        LITEFX_IMPLEMENTATION(VulkanDeviceImpl);
        friend struct SharedObject::Allocator<VulkanDevice>;
        friend class VulkanBackend;

    public:
        /// <summary>
        /// The default size for the global buffer heap in bytes.
        /// </summary>
        /// <remarks>
        /// The default value represents the minimum supported value over all devices registered at GPUinfo: 
        /// https://vulkan.gpuinfo.org/displayextensionproperty.php?extensionname=VK_EXT_descriptor_buffer&extensionproperty=descriptorBufferAddressSpaceSize&platform=all.
        /// </remarks>
        static const size_t DEFAULT_DESCRIPTOR_HEAP_SIZE = 134'217'728;   // equals 128 Mb

    private:
        /// <summary>
        /// Creates a new device instance.
        /// </summary>
        /// <param name="backend">The backend from which the device is created.</param>
        /// <param name="adapter">The adapter the device uses for drawing.</param>
        /// <param name="surface">The surface, the device should draw to.</param>
        /// <param name="features">The features that should be supported by this device.</param>
        /// <param name="extensions">The required extensions the device gets initialized with.</param>
        /// <param name="globalDescriptorHeapSize">The size of the global descriptor heap in bytes.</param>
        explicit VulkanDevice(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, GraphicsDeviceFeatures features = { }, Span<String> extensions = { }, size_t globalDescriptorHeapSize = DEFAULT_DESCRIPTOR_HEAP_SIZE);

    private:
        /// <inheritdoc />
        VulkanDevice(VulkanDevice&&) noexcept = delete;

        /// <inheritdoc />
        VulkanDevice(const VulkanDevice&) = delete;

        /// <inheritdoc />
        VulkanDevice& operator=(VulkanDevice&&) noexcept = delete;

        /// <inheritdoc />
        VulkanDevice& operator=(const VulkanDevice&) = delete;

    public:
        /// <inheritdoc />
        ~VulkanDevice() noexcept override;

        // Factory methods.
    public:
        /// <summary>
        /// Initializes the device instance.
        /// </summary>
        /// <param name="backend">The backend from which the device is created.</param>
        /// <param name="adapter">The adapter the device uses for drawing.</param>
        /// <param name="surface">The surface, the device should draw to.</param>
        /// <param name="features">The features that should be supported by this device.</param>
        /// <param name="extensions">The required extensions the device gets initialized with.</param>
        /// <param name="globalDescriptorHeapSize">The size of the global descriptor heap in bytes.</param>
        /// <returns>A shared pointer to the new device instance.</returns>
        static inline SharedPtr<VulkanDevice> create(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, GraphicsDeviceFeatures features = { }, Span<String> extensions = { }, size_t globalDescriptorHeapSize = DEFAULT_DESCRIPTOR_HEAP_SIZE) {
            return SharedObject::create<VulkanDevice>(backend, adapter, std::move(surface), features, extensions, globalDescriptorHeapSize)->initialize(Format::B8G8R8A8_SRGB, { 800, 600 }, 3, false, features); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        }

        /// <summary>
        /// Initializes the device instance.
        /// </summary>
        /// <param name="backend">The backend from which the device is created.</param>
        /// <param name="adapter">The adapter the device uses for drawing.</param>
        /// <param name="surface">The surface, the device should draw to.</param>
        /// <param name="format">The initial surface format, device uses for drawing.</param>
        /// <param name="renderArea">The initial size of the render area.</param>
        /// <param name="backBuffers">The initial number of back buffers.</param>
        /// <param name="enableVsync">The initial setting for vertical synchronization.</param>
        /// <param name="features">The features that should be supported by this device.</param>
        /// <param name="extensions">The required extensions the device gets initialized with.</param>
        /// <param name="globalDescriptorHeapSize">The size of the global descriptor heap in bytes.</param>
        /// <returns>A shared pointer to the new device instance.</returns>
        static inline SharedPtr<VulkanDevice> create(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync = false, GraphicsDeviceFeatures features = { }, Span<String> extensions = { }, size_t globalDescriptorHeapSize = DEFAULT_DESCRIPTOR_HEAP_SIZE) {
            return SharedObject::create<VulkanDevice>(backend, adapter, std::move(surface), features, extensions, globalDescriptorHeapSize)->initialize(format, renderArea, backBuffers, enableVsync, features);
        }

    private:
        /// <summary>
        /// Initializes the resources owned by the device.
        /// </summary>
        /// <param name="format">The initial surface format, device uses for drawing.</param>
        /// <param name="renderArea">The initial size of the render area.</param>
        /// <param name="backBuffers">The initial number of back buffers.</param>
        /// <param name="enableVsync">The initial setting for vertical synchronization.</param>
        /// <param name="features">The features that should be supported by this device.</param>
        /// <returns>A shared pointer to the current device instance.</returns>
        SharedPtr<VulkanDevice> initialize(Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync = false, GraphicsDeviceFeatures features = { });

        /// <summary>
        /// Releases the device state to prepare it for destruction.
        /// </summary>
        void release() noexcept;

        // Vulkan Device interface.
    private:
        /// <summary>
        /// Sets the debug name for an object.
        /// </summary>
        /// <remarks>
        /// This function sets the debug name for an object to make it easier to identify when using an external debugger. This function will do nothing
        /// in release mode or if the device extension VK_EXT_debug_marker is not available.
        /// </remarks>
        /// <param name="objectType">The type of the object.</param>
        /// <param name="objectHandle">The handle of the object casted to an integer.</param>
        /// <param name="name">The debug name of the object.</param>
        void setDebugName(VkDebugReportObjectTypeEXT objectType, UInt64 objectHandle, StringView name) const noexcept;

    public:
        /// <summary>
        /// Returns the array that stores the extensions that were used to initialize the device.
        /// </summary>
        /// <returns>A reference to the array that stores the extensions that were used to initialize the device.</returns>
        Span<const String> enabledExtensions() const noexcept;

        /// <summary>
        /// Sets the debug name for an object.
        /// </summary>
        /// <remarks>
        /// This function sets the debug name for an object to make it easier to identify when using an external debugger. This function will do nothing
        /// in release mode or if the device extension VK_EXT_debug_marker is not available.
        /// </remarks>
        /// <typeparam name="THandle">The type of the handle. Must be a Vulkan handle type.</typeparam>
        /// <param name="objectHandle">The handle of the object casted to an integer.</param>
        /// <param name="objectType">The type of the object.</param>
        /// <param name="name">The debug name of the object.</param>
        template <typename THandle>
        inline void setDebugName(THandle objectHandle, VkDebugReportObjectTypeEXT objectType, StringView name) const noexcept {
            this->setDebugName(objectType, Vk::handleAddress(objectHandle), name); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        }

        /// <summary>
        /// Returns the indices of all queue families with support for <paramref name="type" />.
        /// </summary>
        /// <param name="type">The type of workload that must be supported by the family in order for it to be returned. Specifying <see cref="QueueType::None" /> will return all available queue families.</param>
        /// <returns>The indices of the queue families that support queue workloads specified by <paramref name="type" />.</returns>
        Enumerable<UInt32> queueFamilyIndices(QueueType type = QueueType::None) const;

        /// <summary>
        /// Resolves the binary size for a descriptor of <paramref name="type" />.
        /// </summary>
        /// <param name="type">The type of the descriptor.</param>
        /// <returns>The size of the descriptor.</returns>
        UInt32 descriptorSize(DescriptorType type) const;

        // GraphicsDevice interface.
    public:
        /// <inheritdoc />
        DeviceState& state() const noexcept override;

        /// <inheritdoc />
        const VulkanSwapChain& swapChain() const noexcept override;

        /// <inheritdoc />
        VulkanSwapChain& swapChain() noexcept override;

        /// <inheritdoc />
        const VulkanSurface& surface() const noexcept override;

        /// <inheritdoc />
        const VulkanGraphicsAdapter& adapter() const noexcept override;

        /// <inheritdoc />
        const VulkanGraphicsFactory& factory() const noexcept override;

        /// <inheritdoc />
        const VulkanQueue& defaultQueue(QueueType type) const override;

        /// <inheritdoc />
        SharedPtr<const VulkanQueue> createQueue(QueueType type, QueuePriority priority = QueuePriority::Normal) override;

        /// <inheritdoc />
        [[nodiscard]] UniquePtr<VulkanBarrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const override;

        /// <inheritdoc />
        [[nodiscard]] SharedPtr<VulkanFrameBuffer> makeFrameBuffer(StringView name, const Size2d& renderArea) const override;

        /// <inheritdoc />
        MultiSamplingLevel maximumMultiSamplingLevel(Format format) const noexcept override;

        /// <inheritdoc />
        double ticksPerMillisecond() const noexcept override;

        /// <inheritdoc />
        void wait() const override;

        /// <inheritdoc />
        void computeAccelerationStructureSizes(const VulkanBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const override;

        /// <inheritdoc />
        void computeAccelerationStructureSizes(const VulkanTopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const override;

        /// <inheritdoc />
        void allocateGlobalDescriptors(const VulkanDescriptorSet& descriptorSet, DescriptorHeapType heapType, UInt32& heapOffset, UInt32& heapSize) const override;

        /// <inheritdoc />
        void releaseGlobalDescriptors(const VulkanDescriptorSet& descriptorSet) const override;

        /// <inheritdoc />
        void updateGlobalDescriptors(const VulkanDescriptorSet& descriptorSet, UInt32 binding, UInt32 offset, UInt32 descriptors) const override;

        /// <inheritdoc />
        void bindDescriptorSet(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet, const VulkanPipelineState& pipeline) const noexcept override;

        /// <inheritdoc />
        void bindGlobalDescriptorHeaps(const VulkanCommandBuffer& commandBuffer) const noexcept override;

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
    public:
        /// <inheritdoc />
        [[nodiscard]] VulkanRenderPassBuilder buildRenderPass(UInt32 commandBuffers = 1) const override;

        /// <inheritdoc />
        [[nodiscard]] VulkanRenderPassBuilder buildRenderPass(const String& name, UInt32 commandBuffers = 1) const override;

        ///// <inheritdoc />
        //[[nodiscard]] VulkanRenderPipelineBuilder buildRenderPipeline(const String& name) const override;

        /// <inheritdoc />
        [[nodiscard]] VulkanRenderPipelineBuilder buildRenderPipeline(const VulkanRenderPass& renderPass, const String& name) const override;

        /// <inheritdoc />
        [[nodiscard]] VulkanComputePipelineBuilder buildComputePipeline(const String& name) const override;

        /// <inheritdoc />
        [[nodiscard]] VulkanRayTracingPipelineBuilder buildRayTracingPipeline(ShaderRecordCollection&& shaderRecords) const override;

        /// <inheritdoc />
        [[nodiscard]] VulkanRayTracingPipelineBuilder buildRayTracingPipeline(const String& name, ShaderRecordCollection&& shaderRecords) const override;
        
        /// <inheritdoc />
        [[nodiscard]] VulkanPipelineLayoutBuilder buildPipelineLayout() const override;

        /// <inheritdoc />
        [[nodiscard]] VulkanInputAssemblerBuilder buildInputAssembler() const override;

        /// <inheritdoc />
        [[nodiscard]] VulkanRasterizerBuilder buildRasterizer() const override;

        /// <inheritdoc />
        [[nodiscard]] VulkanShaderProgramBuilder buildShaderProgram() const override;

        /// <inheritdoc />
        [[nodiscard]] VulkanBarrierBuilder buildBarrier() const override;
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)
    };

    /// <summary>
    /// Defines a rendering backend that creates a Vulkan device.
    /// </summary>
    class LITEFX_VULKAN_API VulkanBackend final : public RenderBackend<VulkanDevice>, public Resource<VkInstance> {
        LITEFX_IMPLEMENTATION(VulkanBackendImpl);

    public:
        /// <summary>
        /// Initializes a new vulkan rendering backend.
        /// </summary>
        /// <param name="app">An instance of the app that owns the backend.</param>
        /// <param name="extensions">A set of instance extensions to enable on the backend instance.</param>
        /// <param name="validationLayers">A set of validation layers to enable on the rendering backend.</param>
        explicit VulkanBackend(const App& app, const Span<String> extensions = { }, const Span<String> validationLayers = { });

        /// <inheritdoc />
        VulkanBackend(VulkanBackend&&) noexcept;

        /// <inheritdoc />
        VulkanBackend(const VulkanBackend&) = delete;

        /// <inheritdoc />
        VulkanBackend& operator=(VulkanBackend&&) noexcept;

        /// <inheritdoc />
        VulkanBackend& operator=(const VulkanBackend&) = delete;

        /// <inheritdoc />
        ~VulkanBackend() noexcept override;

    public:
        /// <summary>
        /// Returns the validation layers that are enabled on the backend.
        /// </summary>
        /// <returns>An array of validation layers that are enabled on the backend.</returns>
        virtual Span<const String> getEnabledValidationLayers() const noexcept;

#ifdef VK_USE_PLATFORM_WIN32_KHR
        /// <summary>
        /// Creates a surface on a window handle.
        /// </summary>
        /// <param name="hwnd">The window handle on which the surface should be created.</param>
        /// <returns>The instance of the created surface.</returns>
        UniquePtr<VulkanSurface> createSurface(const HWND& hwnd) const;
#else
        /// <summary>
        /// A callback that creates a surface from a Vulkan instance.
        /// </summary>
        typedef std::function<VkSurfaceKHR(const VkInstance&)> surface_callback;

        /// <summary>
        /// Creates a surface using the <paramref name="predicate" /> callback.
        /// </summary>
        /// <param name="predicate">A callback that gets called with the backend instance handle and creates the surface instance</param>
        /// <returns>The instance of the created surface.</returns>
        /// <seealso cref="surface_callback" />
        UniquePtr<VulkanSurface> createSurface(surface_callback predicate) const;
#endif // VK_USE_PLATFORM_WIN32_KHR

    public:
        /// <summary>
        /// Returns <c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.
        /// </summary>
        /// <returns><c>true</c>, if all elements of <paramref cref="extensions" /> are contained by the a list of available extensions.</returns>
        /// <seealso cref="getAvailableInstanceExtensions" />
        static bool validateInstanceExtensions(Span<const String> extensions);

        /// <summary>
        /// Returns a list of available extensions.
        /// </summary>
        /// <returns>A list of available extensions.</returns>
        /// <seealso cref="validateInstanceExtensions" />
        static Enumerable<String> getAvailableInstanceExtensions();

        /// <summary>
        /// Returns <c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.
        /// </summary>
        /// <returns><c>true</c>, if all elements of <paramref cref="validationLayers" /> are contained by the a list of available validation layers.</returns>
        /// <seealso cref="getInstanceValidationLayers" />
        static bool validateInstanceLayers(const Span<const String> validationLayers);

        /// <summary>
        /// Returns a list of available validation layers.
        /// </summary>
        /// <returns>A list of available validation layers.</returns>
        /// <seealso cref="validateInstanceLayers" />
        static Enumerable<String> getInstanceValidationLayers();

        // IBackend interface.
    public:
        /// <inheritdoc />
        BackendType type() const noexcept override;

        /// <inheritdoc />
        StringView name() const noexcept override;

    protected:
        /// <inheritdoc />
        void activate() override;

        /// <inheritdoc />
        void deactivate() override;

        // RenderBackend interface.
    public:
        /// <inheritdoc />
        const Array<SharedPtr<const VulkanGraphicsAdapter>>& adapters() const override;

        /// <inheritdoc />
        const VulkanGraphicsAdapter* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const noexcept override;

        /// <inheritdoc />
        void registerDevice(const String& name, SharedPtr<VulkanDevice>&& device) override;

        /// <inheritdoc />
        void releaseDevice(const String& name) override;

        /// <inheritdoc />
        VulkanDevice* device(const String& name) noexcept override;

        /// <inheritdoc />
        const VulkanDevice* device(const String& name) const noexcept override;
    };

}

#pragma warning(pop)