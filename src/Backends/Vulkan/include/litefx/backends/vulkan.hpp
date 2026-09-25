#pragma once

#include <litefx/rendering.hpp>

#include "vulkan_api.hpp"
#include "vulkan_formatters.hpp"

#pragma warning(push)
#pragma warning(disable:4250) // Base class members are inherited via dominance.

// NOLINTBEGIN(bugprone-derived-method-shadowing-base-method)

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Math;
    using namespace LiteFX::Rendering;

    /// @brief Implements a Vulkan vertex buffer layout.
    ///
    /// @see VulkanVertexBuffer
    /// @see VulkanIndexBufferLayout
    /// @see VulkanVertexBufferLayoutBuilder
    class LITEFX_VULKAN_API VulkanVertexBufferLayout final : public IVertexBufferLayout {
        LITEFX_IMPLEMENTATION(VulkanVertexBufferLayoutImpl);
        LITEFX_BUILDER(VulkanVertexBufferLayoutBuilder);
        friend struct SharedObject::Allocator<VulkanVertexBufferLayout>;

    private:
        /// @brief Initializes a new vertex buffer layout.
        ///
        /// @param vertexSize The overall size of a single vertex.
        /// @param binding The binding point of the vertex buffers using this layout.
        /// @param inputRate The rate at which the vertex buffer is made available to the vertex shader.
        explicit VulkanVertexBufferLayout(size_t vertexSize, UInt32 binding = 0, VertexBufferInputRate inputRate = VertexBufferInputRate::Vertex);

        /// @brief Initializes a new vertex buffer layout.
        ///
        /// @param vertexSize The overall size of a single vertex.
        /// @param attributes The vertex attributes.
        /// @param binding The binding point of the vertex buffers using this layout.
        /// @param inputRate The rate at which the vertex buffer is made available to the vertex shader.
        explicit VulkanVertexBufferLayout(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding = 0, VertexBufferInputRate inputRate = VertexBufferInputRate::Vertex);

    private:
        /// @copydoc IVertexBufferLayout::IVertexBufferLayout(IVertexBufferLayout&&)
        VulkanVertexBufferLayout(VulkanVertexBufferLayout&&) noexcept = delete;

        /// @copydoc IVertexBufferLayout::IVertexBufferLayout(const IVertexBufferLayout&)
        VulkanVertexBufferLayout(const VulkanVertexBufferLayout&);

        /// @copydoc IVertexBufferLayout::operator=(IVertexBufferLayout&&)
        VulkanVertexBufferLayout& operator=(VulkanVertexBufferLayout&&) noexcept = delete;

        /// @copydoc IVertexBufferLayout::operator=(const IVertexBufferLayout&)
        VulkanVertexBufferLayout& operator=(const VulkanVertexBufferLayout&) = delete;

    public:
        /// @copydoc IVertexBufferLayout::~IVertexBufferLayout
        ~VulkanVertexBufferLayout() noexcept override;

    public:
        /// @brief Creates a new vertex buffer layout.
        ///
        /// @param vertexSize The overall size of a single vertex.
        /// @param binding The binding point of the vertex buffers using this layout.
        /// @param inputRate The rate at which the vertex buffer is made available to the vertex shader.
        /// @return A shared pointer to the newly created vertex buffer layout.
        static inline auto create(size_t vertexSize, UInt32 binding = 0, VertexBufferInputRate inputRate = VertexBufferInputRate::Vertex) {
            return SharedObject::create<VulkanVertexBufferLayout>(vertexSize, binding, inputRate);
        }

        /// @brief Creates a new vertex buffer layout.
        ///
        /// @param vertexSize The overall size of a single vertex.
        /// @param binding The binding point of the vertex buffers using this layout.
        /// @param attributes The vertex attributes.
        /// @param inputRate The rate at which the vertex buffer is made available to the vertex shader.
        /// @return A shared pointer to the newly created vertex buffer layout.
        static inline auto create(size_t vertexSize, const Enumerable<BufferAttribute>& attributes, UInt32 binding = 0, VertexBufferInputRate inputRate = VertexBufferInputRate::Vertex) {
            return SharedObject::create<VulkanVertexBufferLayout>(vertexSize, attributes, binding, inputRate);
        }

        /// @brief Creates a copy of a vertex buffer layout.
        ///
        /// @param other The vertex buffer layout to copy.
        /// @return A shared pointer to the newly created vertex buffer layout.
        static inline auto create(const VulkanVertexBufferLayout& other) {
            return SharedObject::create<VulkanVertexBufferLayout>(other);
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

    /// @brief Implements a Vulkan index buffer layout.
    ///
    /// @see VulkanIndexBuffer
    /// @see VulkanVertexBufferLayout
    class LITEFX_VULKAN_API VulkanIndexBufferLayout final : public IIndexBufferLayout {
        LITEFX_IMPLEMENTATION(VulkanIndexBufferLayoutImpl);
        friend struct SharedObject::Allocator<VulkanIndexBufferLayout>;

    private:
        /// @brief Initializes a new index buffer layout
        ///
        /// @param type The type of the indices within the index buffer.
        explicit VulkanIndexBufferLayout(IndexType type);

    private:
        /// @copydoc IIndexBufferLayout::IIndexBufferLayout(IIndexBufferLayout&&)
        VulkanIndexBufferLayout(VulkanIndexBufferLayout&&) noexcept = delete;

        /// @copydoc IIndexBufferLayout::IIndexBufferLayout(const IIndexBufferLayout&)
        VulkanIndexBufferLayout(const VulkanIndexBufferLayout&);

        /// @copydoc IIndexBufferLayout::operator=(IIndexBufferLayout&&)
        VulkanIndexBufferLayout& operator=(VulkanIndexBufferLayout&&) noexcept = delete;

        /// @copydoc IIndexBufferLayout::operator=(const IIndexBufferLayout&)
        VulkanIndexBufferLayout& operator=(const VulkanIndexBufferLayout&) = delete;

    public:
        /// @copydoc IIndexBufferLayout::~IIndexBufferLayout
        ~VulkanIndexBufferLayout() noexcept override;

    public:
        /// @brief Creates a new index buffer layout
        ///
        /// @param type The type of the indices within the index buffer.
        /// @return A shared pointer to the newly created index buffer layout instance.
        static inline auto create(IndexType type) {
            return SharedObject::create<VulkanIndexBufferLayout>(type);
        }

        /// @brief Creates a copy of an index buffer layout.
        ///
        /// @param other The index buffer layout to copy.
        /// @return A shared pointer to the newly created index buffer layout instance.
        static inline auto create(const VulkanIndexBufferLayout& other) {
            return SharedObject::create<VulkanIndexBufferLayout>(other);
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

    /// @brief Represents the base interface for a Vulkan buffer implementation.
    ///
    /// @see VulkanDescriptorSet
    /// @see IVulkanImage
    /// @see IVulkanVertexBuffer
    /// @see IVulkanIndexBuffer
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

    /// @brief Represents a Vulkan vertex buffer.
    ///
    /// @see VulkanVertexBufferLayout
    /// @see IVulkanBuffer
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

    /// @brief Represents a Vulkan index buffer.
    ///
    /// @see VulkanIndexBufferLayout
    /// @see IVulkanBuffer
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

    /// @brief Represents a Vulkan sampled image or the base interface for a texture.
    ///
    /// @see VulkanDescriptorLayout
    /// @see VulkanDescriptorSet
    /// @see VulkanDescriptorSetLayout
    /// @see IVulkanBuffer
    /// @see IVulkanSampler
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
        /// @brief Returns the image resource aspect mask for all sub-resources.
        ///
        /// @return The image resource aspect mask.
        virtual VkImageAspectFlags aspectMask() const noexcept = 0;

        /// @brief Returns the image resource aspect mask for a single sub-resource.
        ///
        /// @param plane The sub-resource identifier to query the aspect mask from.
        /// @return The image resource aspect mask.
        virtual VkImageAspectFlags aspectMask(UInt32 plane) const = 0;
    };

    /// @brief Represents a Vulkan sampler.
    ///
    /// @see VulkanDescriptorLayout
    /// @see VulkanDescriptorSet
    /// @see VulkanDescriptorSetLayout
    /// @see IVulkanImage
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

    /// @brief Represents the base interface for a Vulkan acceleration structure implementation.
    ///
    /// @see VulkanDescriptorSet
    /// @see VulkanBottomLevelAccelerationStructure
    /// @see VulkanTopevelAccelerationStructure
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

    /// @brief Implements a Vulkan bottom-level acceleration structure (BLAS).
    ///
    /// @see VulkanTopLevelAccelerationStructure
    class LITEFX_VULKAN_API VulkanBottomLevelAccelerationStructure final : public IBottomLevelAccelerationStructure, public virtual IVulkanAccelerationStructure, public virtual StateResource, public virtual Resource<VkAccelerationStructureKHR> {
        LITEFX_IMPLEMENTATION(VulkanBottomLevelAccelerationStructureImpl);
        friend class VulkanDevice;
        friend class VulkanCommandBuffer;

        using IAccelerationStructure::build;
        using IAccelerationStructure::update;
        using IBottomLevelAccelerationStructure::copy;

    public:
        /// @brief Initializes a new Vulkan bottom-level acceleration structure (BLAS).
        ///
        /// @param flags The flags that define how the acceleration structure is built.
        /// @param name The name of the acceleration structure resource.
        /// @throws InvalidArgumentException Thrown if the provided @p flags contain an unsupported combination of flags.
        /// @see AccelerationStructureFlags
        explicit VulkanBottomLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None, StringView name = "");

        /// @copydoc IBottomLevelAccelerationStructure::IBottomLevelAccelerationStructure(IBottomLevelAccelerationStructure&&)
        VulkanBottomLevelAccelerationStructure(VulkanBottomLevelAccelerationStructure&&) noexcept;

        /// @copydoc IBottomLevelAccelerationStructure::IBottomLevelAccelerationStructure(const IBottomLevelAccelerationStructure&)
        VulkanBottomLevelAccelerationStructure(const VulkanBottomLevelAccelerationStructure&) = delete;

        /// @copydoc IBottomLevelAccelerationStructure::operator=(IBottomLevelAccelerationStructure&&)
        VulkanBottomLevelAccelerationStructure& operator=(VulkanBottomLevelAccelerationStructure&&) noexcept;

        /// @copydoc IBottomLevelAccelerationStructure::operator=(const IBottomLevelAccelerationStructure&)
        VulkanBottomLevelAccelerationStructure& operator=(const VulkanBottomLevelAccelerationStructure&) = delete;
        
        /// @copydoc IBottomLevelAccelerationStructure::~IBottomLevelAccelerationStructure
        ~VulkanBottomLevelAccelerationStructure() noexcept override;

        // IAccelerationStructure interface.
    public:
        /// @copydoc IAccelerationStructure::flags
        AccelerationStructureFlags flags() const noexcept override;

        /// @copydoc IAccelerationStructure::buffer
        SharedPtr<const IVulkanBuffer> buffer() const noexcept;

        /// @copydoc IAccelerationStructure::build
        void build(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer = nullptr, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// @copydoc IAccelerationStructure::update
        void update(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer = nullptr, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// @copydoc IBottomLevelAccelerationStructure::copy
        void copy(const VulkanCommandBuffer& commandBuffer, VulkanBottomLevelAccelerationStructure& destination, bool compress = false, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const;

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
        Array<std::pair<UInt32, VkAccelerationStructureGeometryKHR>> buildInfo() const;
        void updateState(const VulkanDevice* device, VkAccelerationStructureKHR handle) noexcept;

    private:
        SharedPtr<const IBuffer> getBuffer() const noexcept override;
        void doBuild(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doUpdate(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize) override;
        void doCopy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress, const SharedPtr<const IBuffer>& buffer, UInt64 offset, bool copyBuildInfo) const override;
    };

    /// @brief Implements a Vulkan top-level acceleration structure (TLAS).
    ///
    /// @see VulkanBottomLevelAccelerationStructure
    class LITEFX_VULKAN_API VulkanTopLevelAccelerationStructure final : public ITopLevelAccelerationStructure, public virtual IVulkanAccelerationStructure, public virtual StateResource, public virtual Resource<VkAccelerationStructureKHR> {
        LITEFX_IMPLEMENTATION(VulkanTopLevelAccelerationStructureImpl);
        friend class VulkanDevice;
        friend class VulkanCommandBuffer;

        using IAccelerationStructure::build;
        using IAccelerationStructure::update;
        using ITopLevelAccelerationStructure::copy;

    public:
        /// @brief Initializes a new Vulkan top-level acceleration structure (BLAS).
        ///
        /// @param flags The flags that define how the acceleration structure is built.
        /// @param name The name of the acceleration structure resource.
        /// @throws InvalidArgumentException Thrown if the provided @p flags contain an unsupported combination of flags.
        /// @see AccelerationStructureFlags
        explicit VulkanTopLevelAccelerationStructure(AccelerationStructureFlags flags = AccelerationStructureFlags::None, StringView name = "");

        /// @copydoc ITopLevelAccelerationStructure::ITopLevelAccelerationStructure(ITopLevelAccelerationStructure&&)
        VulkanTopLevelAccelerationStructure(VulkanTopLevelAccelerationStructure&&) noexcept;

        /// @copydoc ITopLevelAccelerationStructure::ITopLevelAccelerationStructure(const ITopLevelAccelerationStructure&)
        VulkanTopLevelAccelerationStructure(const VulkanTopLevelAccelerationStructure&) = delete;

        /// @copydoc ITopLevelAccelerationStructure::operator=(ITopLevelAccelerationStructure&&)
        VulkanTopLevelAccelerationStructure& operator=(VulkanTopLevelAccelerationStructure&&) noexcept;

        /// @copydoc ITopLevelAccelerationStructure::operator=(const ITopLevelAccelerationStructure&)
        VulkanTopLevelAccelerationStructure& operator=(const VulkanTopLevelAccelerationStructure&) = delete;

        /// @copydoc ITopLevelAccelerationStructure::~ITopLevelAccelerationStructure
        ~VulkanTopLevelAccelerationStructure() noexcept override;

        // IAccelerationStructure interface.
    public:
        /// @copydoc IAccelerationStructure::flags
        AccelerationStructureFlags flags() const noexcept override;

        /// @copydoc IAccelerationStructure::buffer
        SharedPtr<const IVulkanBuffer> buffer() const noexcept;

        /// @copydoc IAccelerationStructure::build
        void build(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer = nullptr, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// @copydoc IAccelerationStructure::update
        void update(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer = nullptr, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, UInt64 maxSize = 0);

        /// @copydoc ITopLevelAccelerationStructure::copy
        void copy(const VulkanCommandBuffer& commandBuffer, VulkanTopLevelAccelerationStructure& destination, bool compress = false, const SharedPtr<const IVulkanBuffer>& buffer = nullptr, UInt64 offset = 0, bool copyBuildInfo = true) const;

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

    /// @brief Implements a Vulkan resource barrier.
    ///
    /// @see VulkanCommandBuffer
    /// @see IVulkanBuffer
    /// @see IVulkanImage
    /// @see Barrier
    class LITEFX_VULKAN_API VulkanBarrier final : public Barrier<IVulkanBuffer, IVulkanImage> {
        LITEFX_IMPLEMENTATION(VulkanBarrierImpl);
        LITEFX_BUILDER(VulkanBarrierBuilder);

    public:
        using base_type = Barrier<IVulkanBuffer, IVulkanImage>;
        using base_type::transition;

    public:
        /// @brief Initializes a new Vulkan barrier.
        ///
        /// @param syncBefore The pipeline stage(s) all previous commands have to finish before the barrier is executed.
        /// @param syncAfter The pipeline stage(s) all subsequent commands are blocked at until the barrier is executed.
        explicit VulkanBarrier(PipelineStage syncBefore, PipelineStage syncAfter) noexcept;

        /// @copydoc Barrier::Barrier(Barrier&&)
        VulkanBarrier(VulkanBarrier&&) noexcept;

        /// @copydoc Barrier::Barrier(const Barrier&)
        VulkanBarrier(const VulkanBarrier&);

        /// @copydoc Barrier::operator=(Barrier&&)
        VulkanBarrier& operator=(VulkanBarrier&&) noexcept;

        /// @copydoc Barrier::operator=(const Barrier&)
        VulkanBarrier& operator=(const VulkanBarrier&);

        /// @copydoc Barrier::~Barrier
        ~VulkanBarrier() noexcept override;

    private:
        explicit VulkanBarrier() noexcept;
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
        void transition(const IVulkanBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// @copydoc Barrier::transition(const buffer_type&, UInt32, ResourceAccess, ResourceAccess)
        void transition(const IVulkanBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) override;

        /// @copydoc Barrier::transition(const image_type&, ResourceAccess, ResourceAccess, ImageLayout)
        void transition(const IVulkanImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override;

        /// @copydoc Barrier::transition(const image_type&, ResourceAccess, ResourceAccess, ImageLayout, ImageLayout)
        void transition(const IVulkanImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override;

        /// @copydoc Barrier::transition(const image_type&, UInt32, UInt32, UInt32, UInt32, UInt32, ResourceAccess, ResourceAccess, ImageLayout)
        void transition(const IVulkanImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override;

        /// @copydoc Barrier::transition(const image_type&, UInt32, UInt32, UInt32, UInt32, UInt32, ResourceAccess, ResourceAccess, ImageLayout, ImageLayout)
        void transition(const IVulkanImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override;

    public:
        /// @brief Adds the barrier to a command buffer and updates the resource target states.
        ///
        /// @param commandBuffer The command buffer to add the barriers to.
        /// @throws RuntimeException Thrown, if any of the contained barriers is a image barrier that targets a sub-resource range that does not share the same @ref ImageLayout in all sub-resources.
        void execute(const VulkanCommandBuffer& commandBuffer) const;
    };

    /// @brief Implements a Vulkan @ref IShaderModule.
    ///
    /// @see VulkanShaderProgram
    /// @see VulkanDevice
    /// @see https://github.com/crud89/LiteFX/wiki/Shader-Development
    class LITEFX_VULKAN_API VulkanShaderModule final : public IShaderModule, public Resource<VkShaderModule> {
        LITEFX_IMPLEMENTATION(VulkanShaderModuleImpl);

    public:
        /// @brief Initializes a new Vulkan shader module.
        ///
        /// @param device The parent device, this shader module has been created from.
        /// @param type The shader stage, this module is used in.
        /// @param fileName The file name of the module source.
        /// @param entryPoint The name of the module entry point.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data for ray-tracing shaders.
        explicit VulkanShaderModule(const VulkanDevice& device, ShaderStage type, const String& fileName, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt);

        /// @brief Initializes a new Vulkan shader module.
        ///
        /// @param device The parent device, this shader module has been created from.
        /// @param type The shader stage, this module is used in.
        /// @param stream The file stream of the module source.
        /// @param name The file name of the module source.
        /// @param entryPoint The name of the module entry point.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data for ray-tracing shaders.
        explicit VulkanShaderModule(const VulkanDevice& device, ShaderStage type, std::istream& stream, const String& name, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt);

        /// @copydoc IShaderModule::IShaderModule(IShaderModule&&)
        VulkanShaderModule(VulkanShaderModule&&) noexcept;

        /// @copydoc IShaderModule::IShaderModule(const IShaderModule&)
        VulkanShaderModule(const VulkanShaderModule&) = delete;

        /// @copydoc IShaderModule::operator=(IShaderModule&&)
        VulkanShaderModule& operator=(VulkanShaderModule&&) noexcept;

        /// @copydoc IShaderModule::operator=(const IShaderModule&)
        VulkanShaderModule& operator=(const VulkanShaderModule&) = delete;

        /// @copydoc IShaderModule::~IShaderModule
        ~VulkanShaderModule() noexcept override;

        // ShaderModule interface.
    public:
        /// @copydoc IShaderModule::fileName
        const String& fileName() const noexcept override;

        /// @copydoc IShaderModule::entryPoint
        const String& entryPoint() const noexcept override;

        /// @copydoc IShaderModule::type
        ShaderStage type() const noexcept override;

        /// @copydoc IShaderModule::shaderLocalDescriptor
        const Optional<DescriptorBindingPoint>& shaderLocalDescriptor() const noexcept override;

    public:
        /// @brief Returns the shader byte code.
        ///
        /// @return The shader byte code.
        const Array<UInt32>& bytecode() const noexcept;

        /// @brief Returns the shader stage creation info for convenience.
        ///
        /// @return The shader stage creation info for convenience.
        VkPipelineShaderStageCreateInfo shaderStageDefinition() const;
    };

    /// @brief Implements a Vulkan @ref ShaderProgram.
    ///
    /// @see VulkanShaderProgramBuilder
    /// @see VulkanShaderModule
    /// @see https://github.com/crud89/LiteFX/wiki/Shader-Development
    class LITEFX_VULKAN_API VulkanShaderProgram final : public ShaderProgram<VulkanShaderModule> {
        LITEFX_IMPLEMENTATION(VulkanShaderProgramImpl);
        LITEFX_BUILDER(VulkanShaderProgramBuilder);
        friend struct SharedObject::Allocator<VulkanShaderProgram>;

    private:
        /// @brief Initializes a new Vulkan shader program.
        ///
        /// @param device The parent device of the shader program.
        /// @param modules The shader modules used by the shader program.
        explicit VulkanShaderProgram(const VulkanDevice& device, Enumerable<UniquePtr<VulkanShaderModule>>&& modules);

        /// @brief Initializes a new Vulkan shader program.
        ///
        /// @param device The parent device of the shader program.
        explicit VulkanShaderProgram(const VulkanDevice& device);

    public:
        /// @copydoc ShaderProgram::ShaderProgram(ShaderProgram&&)
        VulkanShaderProgram(VulkanShaderProgram&&) noexcept = delete;

        /// @copydoc ShaderProgram::ShaderProgram(const ShaderProgram&)
        VulkanShaderProgram(const VulkanShaderProgram&) = delete;

        /// @copydoc ShaderProgram::operator=(ShaderProgram&&)
        VulkanShaderProgram& operator=(VulkanShaderProgram&&) noexcept = delete;

        /// @copydoc ShaderProgram::operator=(const ShaderProgram&)
        VulkanShaderProgram& operator=(const VulkanShaderProgram&) = delete;

    public:
        /// @copydoc ShaderProgram::~ShaderProgram
        ~VulkanShaderProgram() noexcept override;

        // Factory method.
    public:
        /// @brief Creates a new Vulkan shader program.
        ///
        /// @param device The parent device of the shader program.
        /// @param modules The shader modules used by the shader program.
        /// @return A shared pointer to the newly created shader program instance.
        static inline auto create(const VulkanDevice& device, Enumerable<UniquePtr<VulkanShaderModule>>&& modules) {
            return SharedObject::create<VulkanShaderProgram>(device, std::move(modules));
        }

    private:
        /// @brief Creates a new Vulkan shader program.
        ///
        /// @return A shared pointer to the newly created shader program instance.
        static inline auto create(const VulkanDevice& device) {
            return SharedObject::create<VulkanShaderProgram>(device);
        }

    public:
        /// @copydoc ShaderProgram::modules
        const Array<UniquePtr<const VulkanShaderModule>>& modules() const noexcept override;

        /// @copydoc IShaderProgram::reflectPipelineLayout
        SharedPtr<VulkanPipelineLayout> reflectPipelineLayout(Enumerable<PipelineBindingHint> hints = {}) const;

    private:
        SharedPtr<IPipelineLayout> parsePipelineLayout(Enumerable<PipelineBindingHint> hints) const override {
            return std::static_pointer_cast<IPipelineLayout>(this->reflectPipelineLayout(hints));
        }
    };

    /// @brief Implements a Vulkan @ref DescriptorSet.
    ///
    /// @see VulkanDescriptorSetLayout
    class LITEFX_VULKAN_API VulkanDescriptorSet final : public DescriptorSet<IVulkanBuffer, IVulkanImage, IVulkanSampler, IVulkanAccelerationStructure> {
        LITEFX_IMPLEMENTATION(VulkanDescriptorSetImpl);
        friend class VulkanDescriptorSetLayout;

    public:
        using base_type = DescriptorSet<IVulkanBuffer, IVulkanImage, IVulkanSampler, IVulkanAccelerationStructure>;
        using base_type::update;

    private:
        /// @brief Initializes the descriptor set from a cached buffer. This is only called from the descriptor set layout.
        ///
        /// @param layout The parent layout of the descriptor set.
        /// @param buffer The buffer to take over.
        explicit VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, Array<Byte>&& buffer);

    public:
        /// @brief Initializes a new descriptor set.
        ///
        /// @param layout The parent descriptor set layout.
        /// @param unboundedArraySize The size of the unbounded runtime array, if available.
        explicit VulkanDescriptorSet(const VulkanDescriptorSetLayout& layout, UInt32 unboundedArraySize = std::numeric_limits<UInt32>::max());

        /// @copydoc DescriptorSet::DescriptorSet(DescriptorSet&&)
        VulkanDescriptorSet(VulkanDescriptorSet&&) noexcept = delete;

        /// @copydoc DescriptorSet::DescriptorSet(const DescriptorSet&)
        VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;

        /// @copydoc DescriptorSet::operator=(DescriptorSet&&)
        VulkanDescriptorSet& operator=(VulkanDescriptorSet&&) noexcept = delete;

        /// @copydoc DescriptorSet::operator=(const DescriptorSet&)
        VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete;

        /// @copydoc DescriptorSet::~DescriptorSet
        ~VulkanDescriptorSet() noexcept override;

    public:
        /// @brief Returns the parent descriptor set layout.
        ///
        /// @return The parent descriptor set layout.
        const VulkanDescriptorSetLayout& layout() const noexcept;

    private:
        /// @brief Releases the underlying buffer of the descriptor set and returns it to the caller (usually the parent descriptor set layout).
        ///
        /// @return The underlying descriptor buffer.
        Array<Byte>&& releaseBuffer() const noexcept;

    public:
        /// @brief Returns a view over the underlying descriptor buffer.
        ///
        /// @return A view over the underlying descriptor buffer.
        Span<const Byte> descriptorBuffer() const noexcept;

    public:
        /// @copydoc IDescriptorSet::globalHeapAllocation
        VirtualAllocator::Allocation globalHeapAllocation(DescriptorHeapType heapType) const noexcept override;

        /// @copydoc DescriptorSet::bindToHeap(DescriptorType, UInt32, const buffer_type&, UInt32, UInt32, Format) const
        UInt32 bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IVulkanBuffer& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, Format texelFormat = Format::None) const override;

        /// @copydoc DescriptorSet::bindToHeap(DescriptorType, UInt32, const image_type&, UInt32, UInt32, UInt32, UInt32) const
        UInt32 bindToHeap(DescriptorType bindingType, UInt32 descriptor, const IVulkanImage& image, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const override;

        /// @copydoc DescriptorSet::bindToHeap(UInt32, const sampler_type&) const
        UInt32 bindToHeap(UInt32 descriptor, const IVulkanSampler& sampler) const override;

        /// @copydoc DescriptorSet::update(UInt32, const buffer_type&, UInt32, UInt32, UInt32, Format) const
        void update(UInt32 binding, const IVulkanBuffer& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, UInt32 firstDescriptor = 0, Format texelFormat = Format::None) const override;

        /// @copydoc DescriptorSet::update(UInt32, const image_type&, UInt32, UInt32, UInt32, UInt32, UInt32) const
        void update(UInt32 binding, const IVulkanImage& texture, UInt32 descriptor = 0, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const override;

        /// @copydoc DescriptorSet::update(UInt32, const sampler_type&, UInt32) const
        void update(UInt32 binding, const IVulkanSampler& sampler, UInt32 descriptor = 0) const override;

        /// @copydoc DescriptorSet::update(UInt32, const acceleration_structure_type&, UInt32) const
        void update(UInt32 binding, const IVulkanAccelerationStructure& accelerationStructure, UInt32 descriptor = 0) const override;
    };

    /// @brief Implements a Vulkan @ref IDescriptorLayout
    ///
    /// @see IVulkanBuffer
    /// @see IVulkanImage
    /// @see IVulkanSampler
    /// @see VulkanDescriptorSet
    /// @see VulkanDescriptorSetLayout
    class LITEFX_VULKAN_API VulkanDescriptorLayout final : public IDescriptorLayout {
        LITEFX_IMPLEMENTATION(VulkanDescriptorLayoutImpl);

    public:
        /// @brief Initializes a new Vulkan descriptor layout.
        ///
        /// @param type The type of the descriptor.
        /// @param binding The binding point for the descriptor.
        /// @param elementSize The size of the descriptor.
        /// @param descriptors The number of descriptors in the descriptor array. If @p unbounded is set, this value sets the upper limit for the array size.
        /// @param unbounded If set to `true`, the descriptor will be defined as a runtime-allocated, unbounded array.
        /// @see descriptors
        VulkanDescriptorLayout(DescriptorType type, UInt32 binding, size_t elementSize, UInt32 descriptors = 1, bool unbounded = false);

        /// @brief Initializes a new Vulkan descriptor layout for a static sampler.
        ///
        /// @param staticSampler The static sampler to initialize the state with.
        /// @param binding The binding point for the descriptor.
        VulkanDescriptorLayout(const IVulkanSampler& staticSampler, UInt32 binding);

        /// @brief Initializes a new Vulkan descriptor layout for an input attachment.
        ///
        /// @param binding The binding point for the descriptor.
        /// @param inputAttachmentIndex If @p type equals @ref DescriptorType::InputAttachment this value specifies the index of the input attachment. Otherwise, the value is ignored.
        VulkanDescriptorLayout(UInt32 binding, UInt32 inputAttachmentIndex);

        /// @copydoc IDescriptorLayout::IDescriptorLayout(IDescriptorLayout&&)
        VulkanDescriptorLayout(VulkanDescriptorLayout&&) noexcept;

        /// @copydoc IDescriptorLayout::IDescriptorLayout(const IDescriptorLayout&)
        VulkanDescriptorLayout(const VulkanDescriptorLayout&);

        /// @copydoc IDescriptorLayout::operator=(IDescriptorLayout&&)
        VulkanDescriptorLayout& operator=(VulkanDescriptorLayout&&) noexcept;

        /// @copydoc IDescriptorLayout::operator=(const IDescriptorLayout&)
        VulkanDescriptorLayout& operator=(const VulkanDescriptorLayout&);

        /// @copydoc IDescriptorLayout::~IDescriptorLayout
        ~VulkanDescriptorLayout() noexcept override;

        // IDescriptorLayout interface.
    public:
        /// @copydoc IDescriptorLayout::descriptorType
        DescriptorType descriptorType() const noexcept override;

        /// @copydoc IDescriptorLayout::descriptors
        UInt32 descriptors() const noexcept override;

        /// @copydoc IDescriptorLayout::unbounded
        bool unbounded() const noexcept override;

        /// @copydoc IDescriptorLayout::staticSampler
        const IVulkanSampler* staticSampler() const noexcept override;

        // IBufferLayout interface.
    public:
        /// @copydoc IBufferLayout::elementSize
        size_t elementSize() const noexcept override;

        /// @copydoc IBufferLayout::binding
        UInt32 binding() const noexcept override;

        /// @copydoc IBufferLayout::type
        BufferType type() const noexcept override;

        // VulkanDescriptorLayout
    public:
        /// @brief The index of the input attachment mapped to this descriptor.
        ///
        /// If @ref descriptorType returns @ref DescriptorType::InputAttachment this value refers to the index of the input attachment within a set of input attachments of a @ref VulkanRenderPass. Otherwise, the
        /// value is ignored.
        ///
        /// @return The index of the input attachment mapped to this descriptor.
        UInt32 inputAttachmentIndex() const noexcept;
    };

    /// @brief Implements a Vulkan @ref DescriptorSetLayout.
    ///
    /// @see VulkanDescriptorSet
    /// @see VulkanDescriptorSetLayoutBuilder
    class LITEFX_VULKAN_API VulkanDescriptorSetLayout final : public DescriptorSetLayout<VulkanDescriptorLayout, VulkanDescriptorSet>, public Resource<VkDescriptorSetLayout> {
        LITEFX_IMPLEMENTATION(VulkanDescriptorSetLayoutImpl);
        LITEFX_BUILDER(VulkanDescriptorSetLayoutBuilder);
        friend struct SharedObject::Allocator<VulkanDescriptorSetLayout>;

    public:
        using base_type = DescriptorSetLayout<VulkanDescriptorLayout, VulkanDescriptorSet>;
        using base_type::free;
        using base_type::allocate;

    private:
        /// @brief Initializes a Vulkan descriptor set layout.
        ///
        /// @param device The parent device, the pipeline layout has been created from.
        /// @param descriptorLayouts The descriptor layouts of the descriptors within the descriptor set.
        /// @param space The space or set id of the descriptor set.
        /// @param stages The shader stages, the descriptor sets are bound to.
        explicit VulkanDescriptorSetLayout(const VulkanDevice& device, const Enumerable<VulkanDescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages);
        
        /// @brief Initializes a Vulkan descriptor set layout.
        ///
        /// @param device The parent device, the pipeline layout has been created from.
        explicit VulkanDescriptorSetLayout(const VulkanDevice& device);

    private:
        /// @copydoc DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&&)
        VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&&) noexcept = delete;

        /// @copydoc DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayout&)
        VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout& other);

        /// @copydoc DescriptorSetLayout::operator=(DescriptorSetLayout&&)
        VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&&) noexcept = delete;

        /// @copydoc DescriptorSetLayout::operator=(const DescriptorSetLayout&)
        VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

    public:
        /// @copydoc DescriptorSetLayout::~DescriptorSetLayout
        ~VulkanDescriptorSetLayout() noexcept override;

    public:
        /// @brief Creates a Vulkan descriptor set layout.
        ///
        /// @param device The device, the descriptor set layout is created on.
        /// @param descriptorLayouts The descriptor layouts of the descriptors within the descriptor set.
        /// @param space The space or set id of the descriptor set.
        /// @param stages The shader stages, the descriptor sets are bound to.
        /// @return Returns a shared pointer to the newly created descriptor set layout.
        static inline auto create(const VulkanDevice& device, const Enumerable<VulkanDescriptorLayout>& descriptorLayouts, UInt32 space, ShaderStage stages) {
            return SharedObject::create<VulkanDescriptorSetLayout>(device, descriptorLayouts, space, stages);
        }

        /// @brief Creates a copy of a Vulkan descriptor set layout.
        ///
        /// @param other The descriptor set layout to copy.
        /// @return Returns a shared pointer to the newly created descriptor set layout.
        static inline auto create(const VulkanDescriptorSetLayout& other) {
            return SharedObject::create<VulkanDescriptorSetLayout>(other);
        }

    private:
        /// @brief Creates a Vulkan descriptor set layout.
        ///
        /// @param device The device, the descriptor set layout is created on.
        /// @return Returns a shared pointer to the newly created descriptor set layout.
        static inline auto create(const VulkanDevice& device) {
            return SharedObject::create<VulkanDescriptorSetLayout>(device);
        }

    public:
        /// @brief Returns a reference of the device, the pipeline layout has been created from.
        ///
        /// @return A reference of the device, the pipeline layout has been created from.
        const VulkanDevice& device() const noexcept;

        /// @brief Returns the maximum allowed size for an unbounded array in a descriptor set created with this layout, or `0` if the layout does not contain an unbounded array.
        ///
        /// @return The maximum allowed size for an unbounded array in a descriptor set created with this layout
        UInt32 maxUnboundedArraySize() const noexcept;

    public:
        /// @copydoc DescriptorSetLayout::descriptors
        const Array<VulkanDescriptorLayout>& descriptors() const noexcept override;

        /// @copydoc DescriptorSetLayout::descriptor
        const VulkanDescriptorLayout& descriptor(UInt32 binding) const override;

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
        UniquePtr<VulkanDescriptorSet> allocate(UInt32 descriptors, std::initializer_list<DescriptorBinding> bindings) const override;

        /// @copydoc DescriptorSetLayout::allocate(UInt32, Span<DescriptorBinding>) const
        UniquePtr<VulkanDescriptorSet> allocate(UInt32 descriptors, Span<DescriptorBinding> bindings) const override;

        /// @copydoc DescriptorSetLayout::allocate(UInt32, Generator<DescriptorBinding>) const
        UniquePtr<VulkanDescriptorSet> allocate(UInt32 descriptors, Generator<DescriptorBinding> bindings) const override;

        /// @copydoc DescriptorSetLayout::allocate(UInt32, UInt32, std::initializer_list<std::initializer_list<DescriptorBinding>>) const
        Generator<UniquePtr<VulkanDescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::initializer_list<std::initializer_list<DescriptorBinding>> bindings = { }) const override;

#ifdef __cpp_lib_mdspan
        /// @copydoc DescriptorSetLayout::allocate(UInt32, UInt32, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>>) const
        Generator<UniquePtr<VulkanDescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const override;
#endif

        /// @copydoc DescriptorSetLayout::allocate(UInt32, UInt32, std::function<Generator<DescriptorBinding>(UInt32)>) const
        Generator<UniquePtr<VulkanDescriptorSet>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const override;

        /// @copydoc DescriptorSetLayout::free
        void free(const VulkanDescriptorSet& descriptorSet) const override;
    };

    /// @brief Implements the Vulkan @ref IPushConstantsRange.
    ///
    /// @see VulkanPushConstantsLayout
    class LITEFX_VULKAN_API VulkanPushConstantsRange final : public IPushConstantsRange {
        LITEFX_IMPLEMENTATION(VulkanPushConstantsRangeImpl);

    public:
        /// @brief Initializes a new push constants range.
        ///
        /// @param shaderStage The shader stages, that access the push constants from the range.
        /// @param offset The offset relative to the parent push constants backing memory that marks the beginning of the range.
        /// @param size The size of the push constants range.
        /// @param space The space from which the push constants of the range will be accessible in the shader.
        /// @param binding The register from which the push constants of the range will be accessible in the shader.
        explicit VulkanPushConstantsRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding);

        /// @copydoc IPushConstantsRange::IPushConstantsRange(IPushConstantsRange&&)
        VulkanPushConstantsRange(VulkanPushConstantsRange&&) noexcept;

        /// @copydoc IPushConstantsRange::IPushConstantsRange(const IPushConstantsRange&)
        VulkanPushConstantsRange(const VulkanPushConstantsRange&);

        /// @copydoc IPushConstantsRange::operator=(IPushConstantsRange&&)
        VulkanPushConstantsRange& operator=(VulkanPushConstantsRange&&) noexcept;

        /// @copydoc IPushConstantsRange::operator=(const IPushConstantsRange&)
        VulkanPushConstantsRange& operator=(const VulkanPushConstantsRange&);

        /// @copydoc IPushConstantsRange::~IPushConstantsRange
        ~VulkanPushConstantsRange() noexcept override;

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

    /// @brief Implements the Vulkan @ref PushConstantsLayout.
    ///
    /// @see VulkanPushConstantsRange
    /// @see VulkanPushConstantsLayoutBuilder
    /// @see VulkanPushConstantsLayoutBuilder
    class LITEFX_VULKAN_API VulkanPushConstantsLayout final : public PushConstantsLayout<VulkanPushConstantsRange> {
        LITEFX_IMPLEMENTATION(VulkanPushConstantsLayoutImpl);
        LITEFX_BUILDER(VulkanPushConstantsLayoutBuilder);
        friend class VulkanPipelineLayout;

    public:
        /// @brief Initializes a new push constants layout.
        ///
        /// @param ranges The ranges contained by the layout.
        /// @param size The overall size (in bytes) of the push constants backing memory.
        explicit VulkanPushConstantsLayout(Enumerable<UniquePtr<VulkanPushConstantsRange>>&& ranges, UInt32 size);
        
        /// @copydoc PushConstantsLayout::PushConstantsLayout(PushConstantsLayout&&)
        VulkanPushConstantsLayout(VulkanPushConstantsLayout&&) noexcept;

        /// @copydoc PushConstantsLayout::PushConstantsLayout(const PushConstantsLayout&)
        VulkanPushConstantsLayout(const VulkanPushConstantsLayout&) = delete;

        /// @copydoc PushConstantsLayout::operator=(PushConstantsLayout&&)
        VulkanPushConstantsLayout& operator=(VulkanPushConstantsLayout&&) noexcept;

        /// @copydoc PushConstantsLayout::operator=(const PushConstantsLayout&)
        VulkanPushConstantsLayout& operator=(const VulkanPushConstantsLayout&) = delete;

        /// @copydoc PushConstantsLayout::~PushConstantsLayout
        ~VulkanPushConstantsLayout() noexcept override;

    private:
        /// @brief Initializes a new push constants layout.
        ///
        /// @param size The overall size (in bytes) of the push constants backing memory.
        explicit VulkanPushConstantsLayout(UInt32 size);
    
    public:
        /// @copydoc IPushConstantsLayout::size
        UInt32 size() const noexcept override;

        /// @copydoc PushConstantsLayout::ranges
        const Array<UniquePtr<VulkanPushConstantsRange>>& ranges() const override;
    };

    /// @brief Implements a Vulkan @ref PipelineLayout.
    ///
    /// @see VulkanPipelineLayoutBuilder
    class LITEFX_VULKAN_API VulkanPipelineLayout final : public PipelineLayout<VulkanDescriptorSetLayout, VulkanPushConstantsLayout>, public Resource<VkPipelineLayout> {
        LITEFX_IMPLEMENTATION(VulkanPipelineLayoutImpl);
        LITEFX_BUILDER(VulkanPipelineLayoutBuilder);
        friend struct SharedObject::Allocator<VulkanPipelineLayout>;

    private:
        /// @brief Initializes a new Vulkan render pipeline layout.
        ///
        /// @param device The parent device, the layout is created from.
        /// @param descriptorSetLayouts The descriptor set layouts used by the pipeline.
        /// @param pushConstantsLayout The push constants layout used by the pipeline.
        explicit VulkanPipelineLayout(const VulkanDevice& device, const Enumerable<SharedPtr<VulkanDescriptorSetLayout>>& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout);

        /// @brief Initializes a new Vulkan render pipeline layout.
        ///
        /// @param device The parent device, the layout is created from.
        explicit VulkanPipelineLayout(const VulkanDevice& device) noexcept;

    private:
        /// @copydoc PipelineLayout::PipelineLayout(PipelineLayout&&)
        VulkanPipelineLayout(VulkanPipelineLayout&&) noexcept = delete;

        /// @copydoc PipelineLayout::PipelineLayout(const PipelineLayout&)
        VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;

        /// @copydoc PipelineLayout::operator=(PipelineLayout&&)
        VulkanPipelineLayout& operator=(VulkanPipelineLayout&&) noexcept = delete;

        /// @copydoc PipelineLayout::operator=(const PipelineLayout&)
        VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;

    public:
        /// @copydoc PipelineLayout::~PipelineLayout
        ~VulkanPipelineLayout() noexcept override;

    public:
        /// @brief Creates a new Vulkan render pipeline layout.
        ///
        /// @param device The parent device, the layout is created from.
        /// @param descriptorSetLayouts The descriptor set layouts used by the pipeline.
        /// @param pushConstantsLayout The push constants layout used by the pipeline.
        /// @return A shared pointer to the newly created pipeline layout instance.
        static inline auto create(const VulkanDevice& device, const Enumerable<SharedPtr<VulkanDescriptorSetLayout>>& descriptorSetLayouts, UniquePtr<VulkanPushConstantsLayout>&& pushConstantsLayout) {
            return SharedObject::create<VulkanPipelineLayout>(device, descriptorSetLayouts, std::move(pushConstantsLayout));
        }

    private:
        /// @brief Creates a new Vulkan render pipeline layout.
        ///
        /// @param device The parent device, the layout is created from.
        /// @return A shared pointer to the newly created pipeline layout instance.
        static inline auto create(const VulkanDevice& device) {
            return SharedObject::create<VulkanPipelineLayout>(device);
        }

        // PipelineLayout interface.
    public:
        /// @brief Returns a reference to the parent device.
        ///
        /// @return A reference to the parent device.
        const VulkanDevice& device() const noexcept /*override*/;

        /// @copydoc PipelineLayout::descriptorSet
        const VulkanDescriptorSetLayout& descriptorSet(UInt32 space) const override;

        /// @copydoc PipelineLayout::descriptorSets
        const Array<SharedPtr<const VulkanDescriptorSetLayout>>& descriptorSets() const override;

        /// @copydoc PipelineLayout::pushConstants
        const VulkanPushConstantsLayout* pushConstants() const noexcept override;

        /// @copydoc IPipelineLayout::dynamicResourceHeapAccess
        bool dynamicResourceHeapAccess() const override;

        /// @copydoc IPipelineLayout::dynamicSamplerHeapAccess
        bool dynamicSamplerHeapAccess() const override;
    };

    /// @brief Implements the Vulkan input assembler state.
    ///
    /// @see VulkanInputAssemblerBuilder
    class LITEFX_VULKAN_API VulkanInputAssembler final : public InputAssembler<VulkanVertexBufferLayout, VulkanIndexBufferLayout> {
        LITEFX_IMPLEMENTATION(VulkanInputAssemblerImpl);
        LITEFX_BUILDER(VulkanInputAssemblerBuilder);
        friend struct SharedObject::Allocator<VulkanInputAssembler>;

	private:
        /// @brief Initializes a new Vulkan input assembler state.
        ///
        /// @param vertexBufferLayouts The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.
        /// @param controlPoints The number of control points in a patch primitive. Ignored if @p primitiveTopology is not `PrimitiveTopology::PatchList`. Must be a value between 1 and 32.
        explicit VulkanInputAssembler(PrimitiveTopology primitiveTopology, UInt32 controlPoints = 1);

		/// @brief Initializes a new Vulkan input assembler state.
		///
		/// @param vertexBufferLayouts The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.
		/// @param indexBufferLayout The index buffer layout.
		/// @param primitiveTopology The primitive topology.
        /// @param controlPoints The number of control points in a patch primitive. Ignored if @p primitiveTopology is not `PrimitiveTopology::PatchList`. Must be a value between 1 and 32.
		explicit VulkanInputAssembler(Enumerable<SharedPtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, SharedPtr<VulkanIndexBufferLayout>&& indexBufferLayout = nullptr, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList, UInt32 controlPoints = 1);

        /// @brief Initializes a new Vulkan input assembler state.
        explicit VulkanInputAssembler();

    private:
        /// @copydoc InputAssembler::InputAssembler(InputAssembler&&)
        VulkanInputAssembler(VulkanInputAssembler&&) noexcept;

        /// @copydoc InputAssembler::InputAssembler(const InputAssembler&)
		VulkanInputAssembler(const VulkanInputAssembler&);

        /// @copydoc InputAssembler::operator=(InputAssembler&&)
        VulkanInputAssembler& operator=(VulkanInputAssembler&&) noexcept;

        /// @copydoc InputAssembler::operator=(const InputAssembler&)
        VulkanInputAssembler& operator=(const VulkanInputAssembler&);

    public:
        /// @copydoc InputAssembler::~InputAssembler
		~VulkanInputAssembler() noexcept override;

    public:
        /// @brief Creates a new Vulkan input assembler state.
        ///
        /// @param primitiveTopology The primitive topology.
        /// @param controlPoints The number of control points in a patch primitive. Ignored if @p primitiveTopology is not `PrimitiveTopology::PatchList`. Must be a value between 1 and 32.
        /// @return A shared pointer to the newly created input assembler instance.
        static inline auto create(PrimitiveTopology primitiveTopology, UInt32 controlPoints = 1) {
            return SharedObject::create<VulkanInputAssembler>(primitiveTopology, controlPoints);
        }

        /// @brief Creates a new Vulkan input assembler state.
        ///
        /// @param vertexBufferLayouts The vertex buffer layouts supported by the input assembler state. Each layout must have a unique binding.
        /// @param indexBufferLayout The index buffer layout.
        /// @param primitiveTopology The primitive topology.
        /// @param controlPoints The number of control points in a patch primitive. Ignored if @p primitiveTopology is not `PrimitiveTopology::PatchList`. Must be a value between 1 and 32.
        /// @return A shared pointer to the newly created input assembler instance.
        static inline auto create(Enumerable<SharedPtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, SharedPtr<VulkanIndexBufferLayout>&& indexBufferLayout = nullptr, PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList, UInt32 controlPoints = 1) {
            return SharedObject::create<VulkanInputAssembler>(std::move(vertexBufferLayouts), std::move(indexBufferLayout), primitiveTopology, controlPoints);
        }

        /// @brief Creates a new Vulkan input assembler state as a copy from another one.
        ///
        /// @param other The input assembler state to copy.
        /// @return A shared pointer to the newly created input assembler instance.
        static inline auto create(const VulkanInputAssembler& other) {
            return SharedObject::create<VulkanInputAssembler>(other);
        }

    private:
        /// @brief Creates a new Vulkan input assembler state.
        ///
        /// @return A shared pointer to the newly created input assembler instance.
        static inline auto create() {
            return SharedObject::create<VulkanInputAssembler>();
        }

    public:
        /// @copydoc InputAssembler::vertexBufferLayouts
        Enumerable<const VulkanVertexBufferLayout&> vertexBufferLayouts() const override;

		/// @copydoc InputAssembler::vertexBufferLayout
		const VulkanVertexBufferLayout& vertexBufferLayout(UInt32 binding) const override;

		/// @copydoc InputAssembler::indexBufferLayout
		const VulkanIndexBufferLayout* indexBufferLayout() const noexcept override;

        /// @copydoc IInputAssembler::topology
        PrimitiveTopology topology() const noexcept override;

        /// @copydoc IInputAssembler::controlPoints
        UInt32 controlPoints() const noexcept override;
    };

    /// @brief Implements a Vulkan @ref IRasterizer.
    ///
    /// @see VulkanRasterizerBuilder
    class LITEFX_VULKAN_API VulkanRasterizer final : public Rasterizer {
        LITEFX_BUILDER(VulkanRasterizerBuilder);
        friend struct SharedObject::Allocator<VulkanRasterizer>;

    private:
        /// @brief Initializes a new Vulkan rasterizer state.
        ///
        /// @param polygonMode The polygon mode used by the pipeline.
        /// @param cullMode The cull mode used by the pipeline.
        /// @param cullOrder The cull order used by the pipeline.
        /// @param lineWidth The line width used by the pipeline.
        /// @param depthClip The depth clip toggle of the rasterizer state.
        /// @param depthStencilState The rasterizer depth/stencil state.
        /// @param conservativeRasterization Toggles the use of conservative rasterization in the rasterizer.
        explicit VulkanRasterizer(PolygonMode polygonMode, CullMode cullMode, CullOrder cullOrder, Float lineWidth = 1.f, bool depthClip = true, const DepthStencilState& depthStencilState = {}, bool conservativeRasterization = false) noexcept;

        /// @brief Initializes a new Vulkan rasterizer state.
        explicit VulkanRasterizer() noexcept;

    private:
        /// @copydoc Rasterizer::Rasterizer(Rasterizer&&)
        VulkanRasterizer(VulkanRasterizer&&) noexcept = delete;

        /// @copydoc Rasterizer::Rasterizer(const Rasterizer&)
        VulkanRasterizer(const VulkanRasterizer&) = default;

        /// @copydoc Rasterizer::operator=(Rasterizer&&)
        VulkanRasterizer& operator=(VulkanRasterizer&&) noexcept = delete;

        /// @copydoc Rasterizer::operator=(const Rasterizer&)
        VulkanRasterizer& operator=(const VulkanRasterizer&) = delete;

    public:
        /// @copydoc Rasterizer::~Rasterizer
        ~VulkanRasterizer() noexcept override;

    public:
        /// @brief Creates a new Vulkan rasterizer state.
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
            return SharedObject::create<VulkanRasterizer>(polygonMode, cullMode, cullOrder, lineWidth, depthClip, depthStencilState, conservativeRasterization);
        }

        /// @brief Creates a new Vulkan rasterizer state by copying an existing one.
        ///
        /// @param other The rasterizer state to copy.
        /// @return A shared pointer to the newly created rasterizer instance.
        static inline auto create(const VulkanRasterizer& other) {
            return SharedObject::create<VulkanRasterizer>(other);
        }

    private:
        /// @brief Creates a new Vulkan rasterizer state.
        ///
        /// @return A shared pointer to the newly created rasterizer instance.
        static inline auto create() {
            return SharedObject::create<VulkanRasterizer>();
        }

    public:
        /// @brief Sets the line width on the rasterizer.
        ///
        /// Note that updating the line width requires the "wide lines" feature to be available. If it is not, the line width **must** be `1.0`. This constraint is not enforced by the engine and you are
        /// responsible of making sure that it is fulfilled.
        ///
        /// Furthermore, note that the DirectX 12 back-end does have any representation for the line width concept. Thus you should only use the line width, if you plan to only support Vulkan.
        ///
        /// @return A reference to the line width.
        /// @see https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#features-wideLines
        void updateLineWidth(Float lineWidth) noexcept;
    };

    /// @brief Defines the base class for Vulkan pipeline state objects.
    ///
    /// @see VulkanRenderPipeline
    /// @see VulkanComputePipeline
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
        /// @brief Returns the type of the pipeline.
        ///
        /// @return The type of the pipeline.
        virtual VkPipelineBindPoint pipelineType() const noexcept = 0;

        /// @brief Sets the current pipeline state on the @p commandBuffer.
        ///
        /// @param commandBuffer The command buffer to set the current pipeline state on.
        virtual void use(const VulkanCommandBuffer& commandBuffer) const = 0;
    };

    /// @brief Records commands for a @ref VulkanQueue
    ///
    /// @see VulkanQueue
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
        /// @brief Initializes a command buffer from a command queue.
        ///
        /// @param queue The parent command queue, the buffer gets submitted to.
        /// @param begin If set to `true`, the command buffer automatically starts recording by calling @ref begin.
        /// @param primary `true`, if the command buffer is a primary command buffer.
        explicit VulkanCommandBuffer(const VulkanQueue& queue, bool begin = false, bool primary = true);

    private:
        /// @copydoc CommandBuffer::CommandBuffer(CommandBuffer&&)
        VulkanCommandBuffer(VulkanCommandBuffer&&) noexcept = delete;

        /// @copydoc CommandBuffer::CommandBuffer(const CommandBuffer&)
        VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;

        /// @copydoc CommandBuffer::operator=(CommandBuffer&&)
        VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) noexcept = delete;

        /// @copydoc CommandBuffer::operator=(const CommandBuffer&)
        VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
        
    public:
        /// @copydoc CommandBuffer::~CommandBuffer
        ~VulkanCommandBuffer() noexcept override;

        // Factory method.
    public:
        /// @brief Initializes a command buffer from a command queue.
        ///
        /// @param queue The parent command queue, the buffer gets submitted to.
        /// @param begin If set to `true`, the command buffer automatically starts recording by calling @ref begin.
        /// @param primary `true`, if the command buffer is a primary command buffer.
        static inline SharedPtr<VulkanCommandBuffer> create(const VulkanQueue& queue, bool begin = false, bool primary = true) {
            return SharedObject::create<VulkanCommandBuffer>(queue, begin, primary);
        }

        // Vulkan Command Buffer interface.
    public:
        /// @brief Begins the command buffer as a secondary command buffer that inherits the state of @p renderPass.
        ///
        /// @param renderPass The render pass state to inherit.
        void begin(const VulkanRenderPass& renderPass) const;

        // CommandBuffer interface.
    public:
        /// @copydoc ICommandBuffer::queue
        SharedPtr<const VulkanQueue> queue() const noexcept;

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
        [[nodiscard]] UniquePtr<VulkanBarrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const override;

        /// @copydoc CommandBuffer::barrier
        void barrier(const VulkanBarrier& barrier) const noexcept override;

        /// @copydoc CommandBuffer::transfer(const buffer_type&, const buffer_type&, UInt32, UInt32, UInt32) const
        void transfer(const IVulkanBuffer& source, const IVulkanBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// @copydoc CommandBuffer::transfer(const void* const, size_t, const buffer_type&, UInt32, UInt32) const
        void transfer(const void* const data, size_t size, const IVulkanBuffer& target, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// @copydoc CommandBuffer::transfer(Span<const void* const>, size_t, const buffer_type&, UInt32) const
        void transfer(Span<const void* const> data, size_t elementSize, const IVulkanBuffer& target, UInt32 firstElement = 0) const override;

        /// @copydoc CommandBuffer::transfer(const buffer_type&, const image_type&, UInt32, UInt32, UInt32) const
        void transfer(const IVulkanBuffer& source, const IVulkanImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::transfer(const void* const, size_t, const image_type&, UInt32) const
        void transfer(const void* const data, size_t size, const IVulkanImage& target, UInt32 subresource = 0) const override;

        /// @copydoc CommandBuffer::transfer(Span<const void* const>, size_t, const image_type&, UInt32, UInt32) const
        void transfer(Span<const void* const> data, size_t elementSize, const IVulkanImage& target, UInt32 firstSubresource = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::transfer(const image_type&, const image_type&, UInt32, UInt32, UInt32) const
        void transfer(const IVulkanImage& source, const IVulkanImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::transfer(const image_type&, const buffer_type&, UInt32, UInt32, UInt32) const
        void transfer(const IVulkanImage& source, const IVulkanBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::transfer(const SharedPtr<const buffer_type>&, const buffer_type&, UInt32, UInt32, UInt32) const
        void transfer(const SharedPtr<const IVulkanBuffer>& source, const IVulkanBuffer& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const override;

        /// @copydoc CommandBuffer::transfer(const SharedPtr<const buffer_type>&, const image_type&, UInt32, UInt32, UInt32) const
        void transfer(const SharedPtr<const IVulkanBuffer>& source, const IVulkanImage& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const override;

        /// @copydoc CommandBuffer::transfer(const SharedPtr<const image_type>&, const image_type&, UInt32, UInt32, UInt32) const
        void transfer(const SharedPtr<const IVulkanImage>& source, const IVulkanImage& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::transfer(const SharedPtr<const image_type>&, const buffer_type&, UInt32, UInt32, UInt32) const
        void transfer(const SharedPtr<const IVulkanImage>& source, const IVulkanBuffer& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const override;

        /// @copydoc CommandBuffer::use
        void use(const VulkanPipelineState& pipeline) const noexcept override;

		/// @copydoc CommandBuffer::bind(const descriptor_set_type&) const
		void bind(const VulkanDescriptorSet& descriptorSet) const override;

        /// @copydoc CommandBuffer::bind(Span<const descriptor_set_type*>) const
        void bind(Span<const VulkanDescriptorSet*> descriptorSets) const override;

		/// @copydoc CommandBuffer::bind(const descriptor_set_type&, const pipeline_type&) const
		void bind(const VulkanDescriptorSet& descriptorSet, const VulkanPipelineState& pipeline) const override;

        /// @copydoc CommandBuffer::bind(Span<const descriptor_set_type*>, const pipeline_type&) const
        void bind(Span<const VulkanDescriptorSet*> descriptorSets, const VulkanPipelineState& pipeline) const override;

        /// @copydoc CommandBuffer::bind(const vertex_buffer_type&) const
        void bind(const IVulkanVertexBuffer& buffer) const noexcept override;

        /// @copydoc CommandBuffer::bind(const index_buffer_type&) const
        void bind(const IVulkanIndexBuffer& buffer) const noexcept override;

        /// @copydoc ICommandBuffer::dispatch(const Vector3u&) const
        void dispatch(const Vector3u& threadCount) const noexcept override;

        /// @copydoc CommandBuffer::dispatchIndirect
        void dispatchIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// @copydoc ICommandBuffer::dispatchMesh(const Vector3u&) const
        void dispatchMesh(const Vector3u& threadCount) const noexcept override;

        /// @copydoc CommandBuffer::dispatchMeshIndirect(const buffer_type&, UInt32, UInt64) const
        void dispatchMeshIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// @copydoc CommandBuffer::dispatchMeshIndirect(const buffer_type&, const buffer_type&, UInt64, UInt64, UInt32) const
        void dispatchMeshIndirect(const IVulkanBuffer& batchBuffer, const IVulkanBuffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// @copydoc ICommandBuffer::draw(UInt32, UInt32, UInt32, UInt32) const
        void draw(UInt32 vertices, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const noexcept override;

        /// @copydoc CommandBuffer::drawIndirect(const buffer_type&, UInt32, UInt64) const
        void drawIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// @copydoc CommandBuffer::drawIndirect(const buffer_type&, const buffer_type&, UInt64, UInt64, UInt32) const
        void drawIndirect(const IVulkanBuffer& batchBuffer, const IVulkanBuffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// @copydoc ICommandBuffer::drawIndexed(UInt32, UInt32, UInt32, Int32, UInt32) const
        void drawIndexed(UInt32 indices, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const noexcept override;

        /// @copydoc CommandBuffer::drawIndexedIndirect(const buffer_type&, UInt32, UInt64) const
        void drawIndexedIndirect(const IVulkanBuffer& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept override;

        /// @copydoc CommandBuffer::drawIndexedIndirect(const buffer_type&, const buffer_type&, UInt64, UInt64, UInt32) const
        void drawIndexedIndirect(const IVulkanBuffer& batchBuffer, const IVulkanBuffer& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept override;

        /// @copydoc CommandBuffer::pushConstants
        void pushConstants(const VulkanPushConstantsLayout& layout, const void* const memory) const override;

        /// @copydoc ICommandBuffer::writeTimingEvent
        void writeTimingEvent(const SharedPtr<const TimingEvent>& timingEvent) const override;

        /// @copydoc ICommandBuffer::execute(const SharedPtr<const ICommandBuffer>&) const
        void execute(const SharedPtr<const VulkanCommandBuffer>& commandBuffer) const override;

        /// @copydoc ICommandBuffer::execute(Enumerable<SharedPtr<const ICommandBuffer>>) const
        void execute(Enumerable<SharedPtr<const VulkanCommandBuffer>> commandBuffers) const override;

        /// @copydoc CommandBuffer::buildAccelerationStructure(bottom_level_acceleration_structure_type&, const SharedPtr<const buffer_type>&, const buffer_type&, UInt64) const
        void buildAccelerationStructure(VulkanBottomLevelAccelerationStructure& blas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const override;

        /// @copydoc CommandBuffer::buildAccelerationStructure(top_level_acceleration_structure_type&, const SharedPtr<const buffer_type>&, const buffer_type&, UInt64) const
        void buildAccelerationStructure(VulkanTopLevelAccelerationStructure& tlas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const override;

        /// @copydoc CommandBuffer::updateAccelerationStructure(bottom_level_acceleration_structure_type&, const SharedPtr<const buffer_type>&, const buffer_type&, UInt64) const
        void updateAccelerationStructure(VulkanBottomLevelAccelerationStructure& blas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const override;

        /// @copydoc CommandBuffer::updateAccelerationStructure(top_level_acceleration_structure_type&, const SharedPtr<const buffer_type>&, const buffer_type&, UInt64) const
        void updateAccelerationStructure(VulkanTopLevelAccelerationStructure& tlas, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const IVulkanBuffer& buffer, UInt64 offset) const override;

        /// @copydoc CommandBuffer::copyAccelerationStructure(const bottom_level_acceleration_structure_type&, const bottom_level_acceleration_structure_type&, bool) const
        void copyAccelerationStructure(const VulkanBottomLevelAccelerationStructure& from, const VulkanBottomLevelAccelerationStructure& to, bool compress = false) const noexcept override;

        /// @copydoc CommandBuffer::copyAccelerationStructure(const top_level_acceleration_structure_type&, const top_level_acceleration_structure_type&, bool) const
        void copyAccelerationStructure(const VulkanTopLevelAccelerationStructure& from, const VulkanTopLevelAccelerationStructure& to, bool compress = false) const noexcept override;

        /// @copydoc CommandBuffer::traceRays(UInt32, UInt32, UInt32, const ShaderBindingTableOffsets&, const buffer_type&, const buffer_type*, const buffer_type*, const buffer_type*) const
        void traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IVulkanBuffer& rayGenerationShaderBindingTable, const IVulkanBuffer* missShaderBindingTable, const IVulkanBuffer* hitShaderBindingTable, const IVulkanBuffer* callableShaderBindingTable) const noexcept override;

    private:
        inline SharedPtr<const ICommandQueue> getQueue() const noexcept override {
            return std::static_pointer_cast<const ICommandQueue>(this->queue());
        }

        void releaseSharedState() const override;
    };

    /// @brief Implements a Vulkan command queue.
    ///
    /// @see VulkanCommandBuffer
    class LITEFX_VULKAN_API VulkanQueue final : public CommandQueue<VulkanCommandBuffer>, public Resource<VkQueue> {
        LITEFX_IMPLEMENTATION(VulkanQueueImpl);
        friend struct SharedObject::Allocator<VulkanQueue>;

    public:
        using base_type = CommandQueue<VulkanCommandBuffer>;
        using base_type::submit;

    private:
        /// @brief Initializes the Vulkan command queue.
        ///
        /// @param device The device, commands get send to.
        /// @param type The type of the command queue.
        /// @param priority The priority, of which commands are issued on the device.
        /// @param familyId The ID of the queue family.
        /// @param queueId The ID of the queue.
        explicit VulkanQueue(const VulkanDevice& device, QueueType type, QueuePriority priority, UInt32 familyId, UInt32 queueId);

    private:
        /// @copydoc CommandQueue::CommandQueue(CommandQueue&&)
        VulkanQueue(VulkanQueue&&) noexcept = delete;

        /// @copydoc CommandQueue::CommandQueue(const CommandQueue&)
        VulkanQueue(const VulkanQueue&) = delete;

        /// @copydoc CommandQueue::operator=(CommandQueue&&)
        VulkanQueue& operator=(VulkanQueue&&) noexcept = delete;

        /// @copydoc CommandQueue::operator=(const CommandQueue&)
        VulkanQueue& operator=(const VulkanQueue&) = delete;

    public:
        /// @copydoc CommandQueue::~CommandQueue
        ~VulkanQueue() noexcept override;

    public:
        /// @brief Creates a new Vulkan command queue.
        ///
        /// @param device The device, commands get send to.
        /// @param type The type of the command queue.
        /// @param priority The priority, of which commands are issued on the device.
        /// @param familyId The ID of the queue family.
        /// @param queueId The ID of the queue.
        /// @return A pointer to the newly created command queue instance.
        static inline SharedPtr<VulkanQueue> create(const VulkanDevice& device, QueueType type, QueuePriority priority, UInt32 familyId, UInt32 queueId) {
            return SharedObject::create<VulkanQueue>(device, type, priority, familyId, queueId);
        }

        // VulkanQueue interface.
    public:
        /// @brief Returns a pointer to the device that provides this queue or `nullptr`, if the device has already been released.
        ///
        /// @return A pointer to the queue's parent device.
        SharedPtr<const VulkanDevice> device() const noexcept;

        /// @brief Returns the queue family ID.
        ///
        /// @return The queue family ID.
        UInt32 familyId() const noexcept;

        /// @brief Returns the queue ID.
        ///
        /// @return The queue ID.
        UInt32 queueId() const noexcept;

        /// @brief Returns the internal timeline semaphore used to synchronize the queue execution.
        ///
        /// @return The internal timeline semaphore.
        const VkSemaphore& timelineSemaphore() const noexcept;

        // CommandQueue interface.
    public:
        /// @copydoc ICommandQueue::priority
        QueuePriority priority() const noexcept override;

        /// @copydoc ICommandQueue::type
        QueueType type() const noexcept override;

#ifdef LITEFX_BUILD_SUPPORT_DEBUG_MARKERS
    public:
        /// @copydoc ICommandQueue::beginDebugRegion
        void beginDebugRegion(const String& label, const Vectors::ByteVector3& color = DEFAULT_DEBUG_COLOR) const noexcept override;

        /// @copydoc ICommandQueue::endDebugRegion
        void endDebugRegion() const noexcept override;

        /// @copydoc ICommandQueue::setDebugMarker
        void setDebugMarker(const String& label, const Vectors::ByteVector3& color = DEFAULT_DEBUG_COLOR) const noexcept override;
#endif // LITEFX_BUILD_SUPPORT_DEBUG_MARKERS

    public:
        /// @copydoc CommandQueue::createCommandBuffer
        SharedPtr<VulkanCommandBuffer> createCommandBuffer(bool beginRecording = false, bool secondary = false) const override;

        /// @copydoc CommandQueue::submit(const SharedPtr<const command_buffer_type>&) const
        UInt64 submit(const SharedPtr<const VulkanCommandBuffer>& commandBuffer) const override;

        /// @copydoc CommandQueue::submit(Enumerable<SharedPtr<const command_buffer_type>>) const
        UInt64 submit(Enumerable<SharedPtr<const VulkanCommandBuffer>> commandBuffers) const override;

        /// @copydoc ICommandQueue::waitFor(UInt64) const
        void waitFor(UInt64 fence) const override;

        /// @copydoc ICommandQueue::waitFor(const ICommandQueue&, UInt64) const
        void waitFor(const VulkanQueue& queue, UInt64 fence) const noexcept;

        /// @copydoc ICommandQueue::currentFence
        UInt64 currentFence() const noexcept override;
        
        /// @copydoc ICommandQueue::lastCompletedFence
        UInt64 lastCompletedFence() const noexcept override;

    private:
        inline void waitForQueue(const ICommandQueue& queue, UInt64 fence) const override {
            auto vkQueue = dynamic_cast<const VulkanQueue*>(&queue);

            if (vkQueue == nullptr) [[unlikely]]
                throw InvalidArgumentException("queue", "Cannot wait for queues from other backends.");

            this->waitFor(*vkQueue, fence);
        }
    };

    /// @brief Implements a Vulkan @ref RenderPipeline.
    ///
    /// @see VulkanComputePipeline
    /// @see VulkanRenderPipelineBuilder
    class LITEFX_VULKAN_API VulkanRenderPipeline final : public virtual VulkanPipelineState, public RenderPipeline<VulkanPipelineLayout, VulkanShaderProgram, VulkanInputAssembler, VulkanRasterizer> {
        LITEFX_IMPLEMENTATION(VulkanRenderPipelineImpl);
        LITEFX_BUILDER(VulkanRenderPipelineBuilder);

    public:
        /// @brief Initializes a new Vulkan render pipeline.
        ///
        /// @param renderPass The parent render pass.
        /// @param layout The layout of the pipeline.
        /// @param shaderProgram The shader program used by the pipeline.
        /// @param inputAssembler The input assembler state of the pipeline.
        /// @param rasterizer The rasterizer state of the pipeline.
        /// @param samples The initial multi-sampling level of the render pipeline.
        /// @param enableAlphaToCoverage Whether or not to enable Alpha-to-Coverage multi-sampling.
        /// @param name The optional name of the render pipeline.
        explicit VulkanRenderPipeline(const VulkanRenderPass& renderPass, const SharedPtr<VulkanPipelineLayout>& layout, const SharedPtr<VulkanShaderProgram>& shaderProgram, const SharedPtr<VulkanInputAssembler>& inputAssembler, const SharedPtr<VulkanRasterizer>& rasterizer, MultiSamplingLevel samples = MultiSamplingLevel::x1, bool enableAlphaToCoverage = false, const String& name = "");
        
        /// @copydoc VulkanPipelineState::VulkanPipelineState(VulkanPipelineState&&)
        VulkanRenderPipeline(VulkanRenderPipeline&&) noexcept = delete;

        /// @copydoc VulkanPipelineState::VulkanPipelineState(const VulkanPipelineState&)
        VulkanRenderPipeline(const VulkanRenderPipeline&) = delete;

        /// @copydoc VulkanPipelineState::operator=(VulkanPipelineState&&)
        VulkanRenderPipeline& operator=(VulkanRenderPipeline&&) noexcept = delete;

        /// @copydoc VulkanPipelineState::operator=(const VulkanPipelineState&)
        VulkanRenderPipeline& operator=(const VulkanRenderPipeline&) = delete;

        /// @copydoc VulkanPipelineState::~VulkanPipelineState
        ~VulkanRenderPipeline() noexcept override;

    private:
        /// @brief Initializes a new Vulkan render pipeline.
        ///
        /// @param renderPass The parent render pass.
        /// @param name The optional name of the render pipeline.
        VulkanRenderPipeline(const VulkanRenderPass& renderPass, const String& name = "");

        // Pipeline interface.
    public:
        /// @copydoc Pipeline::program
        SharedPtr<const VulkanShaderProgram> program() const noexcept override;

        /// @copydoc Pipeline::layout
        SharedPtr<const VulkanPipelineLayout> layout() const noexcept override;

        // RenderPipeline interface.
    public:
        /// @copydoc RenderPipeline::inputAssembler
        SharedPtr<VulkanInputAssembler> inputAssembler() const noexcept override;

        /// @copydoc RenderPipeline::rasterizer
        SharedPtr<VulkanRasterizer> rasterizer() const noexcept override;

        /// @copydoc IRenderPipeline::alphaToCoverage
        bool alphaToCoverage() const noexcept override;

        /// @copydoc IRenderPipeline::samples
        MultiSamplingLevel samples() const noexcept override;

        /// @copydoc IRenderPipeline::updateSamples
        void updateSamples(MultiSamplingLevel samples) override;

        // VulkanPipelineState interface.
    public:
        /// @copydoc VulkanPipelineState::pipelineType
        VkPipelineBindPoint pipelineType() const noexcept override;

        /// @copydoc VulkanPipelineState::use
        void use(const VulkanCommandBuffer& commandBuffer) const override;
    };

    /// @brief Implements a Vulkan @ref ComputePipeline.
    ///
    /// @see VulkanRenderPipeline
    /// @see VulkanComputePipelineBuilder
    class LITEFX_VULKAN_API VulkanComputePipeline final : public virtual VulkanPipelineState, public ComputePipeline<VulkanPipelineLayout, VulkanShaderProgram> {
        LITEFX_IMPLEMENTATION(VulkanComputePipelineImpl);
        LITEFX_BUILDER(VulkanComputePipelineBuilder);

    public:
        /// @brief Initializes a new Vulkan compute pipeline.
        ///
        /// @param device The parent device.
        /// @param layout The layout of the pipeline.
        /// @param shaderProgram The shader program used by the pipeline.
        /// @param name The optional debug name of the render pipeline.
        explicit VulkanComputePipeline(const VulkanDevice& device, const SharedPtr<VulkanPipelineLayout>& layout, const SharedPtr<VulkanShaderProgram>& shaderProgram, const String& name = "");

        /// @copydoc VulkanPipelineState::VulkanPipelineState(VulkanPipelineState&&)
        VulkanComputePipeline(VulkanComputePipeline&&) noexcept;

        /// @copydoc VulkanPipelineState::VulkanPipelineState(const VulkanPipelineState&)
        VulkanComputePipeline(const VulkanComputePipeline&) = delete;

        /// @copydoc VulkanPipelineState::operator=(VulkanPipelineState&&)
        VulkanComputePipeline& operator=(VulkanComputePipeline&&) noexcept;

        /// @copydoc VulkanPipelineState::operator=(const VulkanPipelineState&)
        VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;

        /// @copydoc VulkanPipelineState::~VulkanPipelineState
        ~VulkanComputePipeline() noexcept override;

    private:
        /// @brief Initializes a new Vulkan compute pipeline.
        ///
        /// @param device The parent device.
        VulkanComputePipeline(const VulkanDevice& device) noexcept;

        // Pipeline interface.
    public:
        /// @copydoc Pipeline::program
        SharedPtr<const VulkanShaderProgram> program() const noexcept override;

        /// @copydoc Pipeline::layout
        SharedPtr<const VulkanPipelineLayout> layout() const noexcept override;

        // VulkanPipelineState interface.
    public:
        /// @copydoc VulkanPipelineState::pipelineType
        VkPipelineBindPoint pipelineType() const noexcept override;

        /// @copydoc VulkanPipelineState::use
        void use(const VulkanCommandBuffer& commandBuffer) const override;
    };
    
    /// @brief Implements a Vulkan @ref RayTracingPipeline.
    ///
    /// @see VulkanRenderPipeline
    /// @see VulkanRayTracingPipelineBuilder
    class LITEFX_VULKAN_API VulkanRayTracingPipeline final : public virtual VulkanPipelineState, public RayTracingPipeline<VulkanPipelineLayout, VulkanShaderProgram> {
        LITEFX_IMPLEMENTATION(VulkanRayTracingPipelineImpl);
        LITEFX_BUILDER(VulkanRayTracingPipelineBuilder);

    public:
        /// @brief Initializes a new Vulkan ray-tracing pipeline.
        ///
        /// @param device The parent device.
        /// @param layout The layout of the pipeline.
        /// @param shaderProgram The shader program used by the pipeline.
        /// @param shaderRecords The shader record collection that is used to build the shader binding table for the pipeline.
        /// @param maxRecursionDepth The maximum number of ray bounces.
        /// @param maxPayloadSize The maximum size for ray payloads in the pipeline.
        /// @param maxAttributeSize The maximum size for ray attributes in the pipeline.
        /// @param name The optional debug name of the render pipeline.
        explicit VulkanRayTracingPipeline(const VulkanDevice& device, const SharedPtr<VulkanPipelineLayout>& layout, const SharedPtr<VulkanShaderProgram>& shaderProgram, ShaderRecordCollection&& shaderRecords, UInt32 maxRecursionDepth = 10, UInt32 maxPayloadSize = 0, UInt32 maxAttributeSize = 32, const String& name = ""); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

        /// @copydoc VulkanPipelineState::VulkanPipelineState(VulkanPipelineState&&)
        VulkanRayTracingPipeline(VulkanRayTracingPipeline&&) noexcept;

        /// @copydoc VulkanPipelineState::VulkanPipelineState(const VulkanPipelineState&)
        VulkanRayTracingPipeline(const VulkanRayTracingPipeline&) = delete;

        /// @copydoc VulkanPipelineState::operator=(VulkanPipelineState&&)
        VulkanRayTracingPipeline& operator=(VulkanRayTracingPipeline&&) noexcept;

        /// @copydoc VulkanPipelineState::operator=(const VulkanPipelineState&)
        VulkanRayTracingPipeline& operator=(const VulkanRayTracingPipeline&) = delete;

        /// @copydoc VulkanPipelineState::~VulkanPipelineState
        ~VulkanRayTracingPipeline() noexcept override;

    private:
        /// @brief Initializes a new Vulkan ray-tracing pipeline.
        ///
        /// @param device The parent device.
        /// @param shaderRecords The shader record collection that is used to build the shader binding table for the pipeline.
        VulkanRayTracingPipeline(const VulkanDevice& device, ShaderRecordCollection&& shaderRecords) noexcept;

        // Pipeline interface.
    public:
        /// @copydoc Pipeline::program
        SharedPtr<const VulkanShaderProgram> program() const noexcept override;

        /// @copydoc Pipeline::layout
        SharedPtr<const VulkanPipelineLayout> layout() const noexcept override;

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
        SharedPtr<IVulkanBuffer> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups = ShaderBindingGroup::All) const override;

        // VulkanPipelineState interface.
    public:
        /// @copydoc VulkanPipelineState::pipelineType
        VkPipelineBindPoint pipelineType() const noexcept override;

        /// @copydoc VulkanPipelineState::use
        void use(const VulkanCommandBuffer& commandBuffer) const override;
    };

    /// @brief Implements a Vulkan frame buffer.
    ///
    /// @see VulkanRenderPass
    class LITEFX_VULKAN_API VulkanFrameBuffer final : public FrameBuffer<IVulkanImage> {
        LITEFX_IMPLEMENTATION(VulkanFrameBufferImpl);
        friend struct SharedObject::Allocator<VulkanFrameBuffer>;

    public:
        using FrameBuffer::allocation_callback_type;
        using FrameBuffer::addImage;
        using FrameBuffer::mapRenderTarget;
        using FrameBuffer::mapRenderTargets;

    private:
        /// @brief Initializes a Vulkan frame buffer.
        ///
        /// @param device The device the frame buffer is allocated on.
        /// @param renderArea The initial size of the render area.
        /// @param name The name of the frame buffer.
        VulkanFrameBuffer(const VulkanDevice& device, const Size2d& renderArea, StringView name = "");

        /// @brief Initializes a Vulkan frame buffer.
        ///
        /// @param device The device the frame buffer is allocated on.
        /// @param renderArea The initial size of the render area.
        /// @param allocationCallback A callback that gets invoked, when the frame buffer allocates a new image.
        /// @param name The name of the frame buffer.
        /// @see IFrameBuffer::allocation_callback_type
        VulkanFrameBuffer(const VulkanDevice& device, const Size2d& renderArea, allocation_callback_type allocationCallback, StringView name = "");

    private:
        /// @copydoc FrameBuffer::FrameBuffer(FrameBuffer&&)
        VulkanFrameBuffer(VulkanFrameBuffer&&) noexcept = delete;
        
        /// @copydoc FrameBuffer::FrameBuffer(const FrameBuffer&)
        VulkanFrameBuffer(const VulkanFrameBuffer&) = delete;

        /// @copydoc FrameBuffer::operator=(FrameBuffer&&)
        VulkanFrameBuffer& operator=(VulkanFrameBuffer&&) noexcept = delete;

        /// @copydoc FrameBuffer::operator=(const FrameBuffer&)
        VulkanFrameBuffer& operator=(const VulkanFrameBuffer&) = delete;

    public:
        /// @copydoc FrameBuffer::~FrameBuffer
        ~VulkanFrameBuffer() noexcept override;

    public:
        /// @brief Initializes a Vulkan frame buffer.
        ///
        /// @param device The device the frame buffer is allocated on.
        /// @param renderArea The initial size of the render area.
        /// @param name The name of the frame buffer.
        /// @return A pointer to the newly created frame buffer instance.
        static inline SharedPtr<VulkanFrameBuffer> create(const VulkanDevice& device, const Size2d& renderArea, StringView name = "") {
            return SharedObject::create<VulkanFrameBuffer>(device, renderArea, name);
        }

        /// @brief Initializes a Vulkan frame buffer.
        ///
        /// @param device The device the frame buffer is allocated on.
        /// @param renderArea The initial size of the render area.
        /// @param allocationCallback A callback that gets invoked, when the frame buffer allocates a new image.
        /// @param name The name of the frame buffer.
        /// @return A pointer to the newly created frame buffer instance.
        static inline SharedPtr<VulkanFrameBuffer> create(const VulkanDevice& device, const Size2d& renderArea, allocation_callback_type allocationCallback, StringView name = "") {
            return SharedObject::create<VulkanFrameBuffer>(device, renderArea, std::move(allocationCallback), name);
        }

        // Vulkan frame buffer interface.
    public:
        /// @brief Returns the image view for an image at the specified index.
        ///
        /// @param imageIndex The index of the image for which the image view should be returned.
        /// @return The image view for the image.
        /// @throws ArgumentOutOfRangeException Thrown, if the provided image index does not address an image within the frame buffer.
        VkImageView imageView(UInt32 imageIndex) const;

        /// @brief Returns the image view for an image with the specified name.
        ///
        /// @param imageName The name of the image for which the image view should be returned.
        /// @return The image view for the image.
        /// @throws InvalidArgumentException Thrown, if the provided image name does refer to an image within the frame buffer.
        VkImageView imageView(StringView imageName) const;

        /// @brief Returns the image view for an image mapped to the specified render target.
        ///
        /// @param renderTarget The render target for which to return the image image view.
        /// @return The image view for the image.
        /// @throws InvalidArgumentException Thrown, if the provided render target is not mapped to an image within the frame buffer.
        VkImageView imageView(const RenderTarget& renderTarget) const;

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
        const Array<SharedPtr<const IVulkanImage>>& images() const override;

        /// @copydoc IFrameBuffer::operator[](UInt32) const
        inline const IVulkanImage& operator[](UInt32 index) const override {
            return this->image(index);
        }

        /// @copydoc IFrameBuffer::image(UInt32) const
        const IVulkanImage& image(UInt32 index) const override;

        /// @copydoc IFrameBuffer::operator[](const RenderTarget&) const
        inline const IVulkanImage& operator[](const RenderTarget& renderTarget) const override {
            return this->image(renderTarget);
        }

        /// @copydoc IFrameBuffer::image(const RenderTarget&) const
        const IVulkanImage& image(const RenderTarget& renderTarget) const override;

        /// @copydoc IFrameBuffer::operator[](StringView) const
        inline const IVulkanImage& operator[](StringView renderTargetName) const override {
            return this->resolveImage(hash(renderTargetName));
        }

        /// @copydoc IFrameBuffer::image(StringView) const
        inline const IVulkanImage& image(StringView renderTargetName) const override {
            return this->resolveImage(hash(renderTargetName));
        }

        /// @copydoc IFrameBuffer::resolveImage
        const IVulkanImage& resolveImage(UInt64 hash) const override;

        /// @copydoc IFrameBuffer::addImage(const String&, Format, MultiSamplingLevel, ResourceUsage)
        void addImage(const String& name, Format format, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) override;

        /// @copydoc IFrameBuffer::addImage(const String&, const RenderTarget&, MultiSamplingLevel, ResourceUsage)
        void addImage(const String& name, const RenderTarget& renderTarget, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::FrameBufferImage) override;

        /// @copydoc IFrameBuffer::resize
        void resize(const Size2d& renderArea) override;
    };

    /// @brief Implements a Vulkan render pass.
    ///
    /// @see VulkanRenderPassBuilder
    class LITEFX_VULKAN_API VulkanRenderPass final : public RenderPass<VulkanQueue, VulkanFrameBuffer> {
        LITEFX_IMPLEMENTATION(VulkanRenderPassImpl);
        LITEFX_BUILDER(VulkanRenderPassBuilder);
        friend struct SharedObject::Allocator<VulkanRenderPass>;

    public:
        using base_type = RenderPass<VulkanQueue, VulkanFrameBuffer>;

    private:
        /// @brief Creates and initializes a new Vulkan render pass instance that executes on the default graphics queue.
        ///
        /// @param device The parent device instance.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        explicit VulkanRenderPass(const VulkanDevice& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000);

        /// @brief Creates and initializes a new Vulkan render pass instance that executes on the default graphics queue.
        ///
        /// @param device The parent device instance.
        /// @param name The name of the render pass state resource.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        explicit VulkanRenderPass(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000);

        /// @brief Creates and initializes a new Vulkan render pass instance.
        ///
        /// @param device The parent device instance.
        /// @param queue The command queue to execute the render pass on.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        explicit VulkanRenderPass(const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000);

        /// @brief Creates and initializes a new Vulkan render pass instance.
        ///
        /// @param device The parent device instance.
        /// @param name The name of the render pass state resource.
        /// @param queue The command queue to execute the render pass on.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        explicit VulkanRenderPass(const VulkanDevice& device, const String& name, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000);

    private:
        /// @copydoc RenderPass::RenderPass(RenderPass&&)
        VulkanRenderPass(VulkanRenderPass&&) noexcept = delete;

        /// @copydoc RenderPass::RenderPass(const RenderPass&)
        VulkanRenderPass(const VulkanRenderPass&) = delete;

        /// @copydoc RenderPass::operator=(RenderPass&&)
        VulkanRenderPass& operator=(VulkanRenderPass&&) noexcept = delete;

        /// @copydoc RenderPass::operator=(const RenderPass&)
        VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;

    public:
        /// @copydoc RenderPass::~RenderPass
        ~VulkanRenderPass() noexcept override;

    public:
        /// @brief Creates and initializes a new Vulkan render pass instance that executes on the default graphics queue.
        ///
        /// @param device The parent device instance.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000) {
            return SharedObject::create<VulkanRenderPass>(device, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask);
        }

        /// @brief Creates and initializes a new Vulkan render pass instance that executes on the default graphics queue.
        ///
        /// @param device The parent device instance.
        /// @param name The name of the render pass state resource.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, const String& name, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000) {
            return SharedObject::create<VulkanRenderPass>(device, name, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask);
        }

        /// @brief Creates and initializes a new Vulkan render pass instance.
        ///
        /// @param device The parent device instance.
        /// @param queue The command queue to execute the render pass on.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000) {
            return SharedObject::create<VulkanRenderPass>(device, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask);
        }

        /// @brief Creates and initializes a new Vulkan render pass instance.
        ///
        /// @param device The parent device instance.
        /// @param name The name of the render pass state resource.
        /// @param queue The command queue to execute the render pass on.
        /// @param renderTargets The render targets that are output by the render pass.
        /// @param inputAttachments The input attachments that are read by the render pass.
        /// @param inputAttachmentSamplerBinding The binding point for the input attachment sampler.
        /// @param secondaryCommandBuffers The number of command buffers that can be used for recording multi-threaded commands during the render pass.
        /// @param viewMask A mask that identifies the enabled view instances for this render pass.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, const String& name, const VulkanQueue& queue, Span<RenderTarget> renderTargets, Span<RenderPassDependency> inputAttachments = { }, Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding = std::nullopt, UInt32 secondaryCommandBuffers = 1u, UInt32 viewMask = 0b0000) {
            return SharedObject::create<VulkanRenderPass>(device, name, queue, renderTargets, inputAttachments, inputAttachmentSamplerBinding, secondaryCommandBuffers, viewMask);
        }

    private:
        /// @brief Creates an uninitialized Vulkan render pass instance.
        ///
        /// @param device The parent device of the render pass.
        /// @param name The name of the render pass state resource.
        explicit VulkanRenderPass(const VulkanDevice& device, const String& name = "");

        /// @brief Creates an uninitialized Vulkan render pass instance.
        ///
        /// This factory is called by the @ref VulkanRenderPassBuilder in order to create a render pass instance without initializing it. The instance is only initialized after calling @ref
        /// VulkanRenderPassBuilder::go.
        ///
        /// @param device The parent device of the render pass.
        /// @param name The name of the render pass state resource.
        /// @return A pointer to the newly created render pass instance.
        static inline SharedPtr<VulkanRenderPass> create(const VulkanDevice& device, const String& name = "") {
            return SharedObject::create<VulkanRenderPass>(device, name);
        }

        // RenderPass interface.
    public:
        /// @brief Returns a reference of the device that provides this queue.
        ///
        /// @return A reference of the queue's parent device.
        const VulkanDevice& device() const noexcept /*override*/;

        /// @copydoc RenderPass::activeFrameBuffer
        SharedPtr<const VulkanFrameBuffer> activeFrameBuffer() const noexcept override;

        /// @copydoc RenderPass::commandQueue
        const VulkanQueue& commandQueue() const noexcept override;

        /// @copydoc RenderPass::commandBuffers
        Enumerable<SharedPtr<const VulkanCommandBuffer>> commandBuffers() const override;

        /// @copydoc RenderPass::commandBuffer
        SharedPtr<const VulkanCommandBuffer> commandBuffer(UInt32 index) const override;

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
        void begin(const VulkanFrameBuffer& frameBuffer) const override;

        /// @copydoc IRenderPass::end
        UInt64 end() const override;

        /// @copydoc IRenderPass::viewMask
        UInt32 viewMask() const noexcept override;
    };

    /// @brief Implements a Vulkan swap chain.
    class LITEFX_VULKAN_API VulkanSwapChain final : public SwapChain<IVulkanImage> {
        LITEFX_IMPLEMENTATION(VulkanSwapChainImpl);
        friend class VulkanImage;
        friend class VulkanDevice;

    public:
        using base_type = SwapChain<IVulkanImage>;

    private:
        /// @brief Initializes a Vulkan swap chain.
        ///
        /// @param device The device that owns the swap chain.
        /// @param format The initial surface format.
        /// @param renderArea The initial size of the render area.
        /// @param buffers The initial number of buffers.
        /// @param enableVsync `true` if vertical synchronization should be used, otherwise `false`.
        explicit VulkanSwapChain(const VulkanDevice& device, Format surfaceFormat = Format::B8G8R8A8_SRGB, const Size2d& renderArea = { 800, 600 }, UInt32 buffers = 3, bool enableVsync = false); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    public:
        /// @copydoc SwapChain::SwapChain(SwapChain&&)
        VulkanSwapChain(VulkanSwapChain&&) noexcept = delete;

        /// @copydoc SwapChain::SwapChain(const SwapChain&)
        VulkanSwapChain(const VulkanSwapChain&) = delete;

        /// @copydoc SwapChain::operator=(SwapChain&&)
        VulkanSwapChain& operator=(VulkanSwapChain&&) noexcept = delete;

        /// @copydoc SwapChain::operator=(const SwapChain&)
        VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

        /// @copydoc SwapChain::~SwapChain
        ~VulkanSwapChain() noexcept override;

        // Vulkan Swap Chain interface.
    public:
        /// @brief Returns the query pool for the current frame.
        ///
        /// @return A reference of the query pool for the current frame.
        const VkQueryPool& timestampQueryPool() const noexcept;

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
        IVulkanImage* image(UInt32 backBuffer) const override;

        /// @copydoc ISwapChain::image() const
        const IVulkanImage& image() const noexcept override;

        /// @copydoc SwapChain::images
        const Array<SharedPtr<IVulkanImage>>& images() const noexcept override;

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
    };

    /// @brief A graphics factory that produces objects for a @ref VulkanDevice.
    ///
    /// Internally this factory implementation is based on [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/).
    class LITEFX_VULKAN_API VulkanGraphicsFactory final : public GraphicsFactory<VulkanDescriptorLayout, IVulkanBuffer, IVulkanVertexBuffer, IVulkanIndexBuffer, IVulkanImage, IVulkanSampler, VulkanBottomLevelAccelerationStructure, VulkanTopLevelAccelerationStructure> {
        LITEFX_IMPLEMENTATION(VulkanGraphicsFactoryImpl);
        friend class VulkanDevice;
        friend struct SharedObject::Allocator<VulkanGraphicsFactory>;

    public:
        using base_type = GraphicsFactory<VulkanDescriptorLayout, IVulkanBuffer, IVulkanVertexBuffer, IVulkanIndexBuffer, IVulkanImage, IVulkanSampler, VulkanBottomLevelAccelerationStructure, VulkanTopLevelAccelerationStructure>;
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
        explicit VulkanGraphicsFactory(const VulkanDevice& device);

    private:
        /// @copydoc GraphicsFactory::GraphicsFactory(GraphicsFactory&&)
        VulkanGraphicsFactory(VulkanGraphicsFactory&&) noexcept = delete;

        /// @copydoc GraphicsFactory::GraphicsFactory(const GraphicsFactory&)
        VulkanGraphicsFactory(const VulkanGraphicsFactory&) = delete;

        /// @copydoc GraphicsFactory::operator=(GraphicsFactory&&)
        VulkanGraphicsFactory& operator=(VulkanGraphicsFactory&&) noexcept = delete;

        /// @copydoc GraphicsFactory::operator=(const GraphicsFactory&)
        VulkanGraphicsFactory& operator=(const VulkanGraphicsFactory&) = delete;

    public:
        /// @copydoc GraphicsFactory::~GraphicsFactory
        ~VulkanGraphicsFactory() noexcept override;

    private:
        /// @brief Creates a new graphics factory.
        ///
        /// @param device The device the factory should produce objects for.
        static inline SharedPtr<VulkanGraphicsFactory> create(const VulkanDevice& device) {
            return SharedObject::create<VulkanGraphicsFactory>(device);
        }

        /// @brief Creates a descriptor heap.
        ///
        /// @param heapSize The size of the descriptor heap buffer in bytes.
        /// @return A buffer that provides memory for the descriptor heap.
        SharedPtr<IVulkanBuffer> createDescriptorHeap(size_t heapSize) const;

        /// @brief Creates a descriptor heap.
        ///
        /// @param name The name of the descriptor heap.
        /// @param heapSize The size of the descriptor heap buffer in bytes.
        /// @return A buffer that provides memory for the descriptor heap.
        SharedPtr<IVulkanBuffer> createDescriptorHeap(const String& name, size_t heapSize) const;

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
        SharedPtr<IVulkanBuffer> createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createBuffer(const String&, BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IVulkanBuffer> createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createVertexBuffer(const vertex_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IVulkanVertexBuffer> createVertexBuffer(const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createVertexBuffer(const String&, const vertex_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IVulkanVertexBuffer> createVertexBuffer(const String& name, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createIndexBuffer(const index_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IVulkanIndexBuffer> createIndexBuffer(const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createIndexBuffer(const String&, const index_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        SharedPtr<IVulkanIndexBuffer> createIndexBuffer(const String& name, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createTexture(Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior) const
        SharedPtr<IVulkanImage> createTexture(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createTexture(const String&, Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior) const
        SharedPtr<IVulkanImage> createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateBuffer(SharedPtr<TBuffer>&, BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateBuffer(SharedPtr<IVulkanBuffer>& buffer, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateBuffer(SharedPtr<TBuffer>&, const String&, BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateBuffer(SharedPtr<IVulkanBuffer>& buffer, const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateVertexBuffer(SharedPtr<TVertexBuffer>&, const vertex_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateVertexBuffer(SharedPtr<IVulkanVertexBuffer>& buffer, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateVertexBuffer(SharedPtr<TVertexBuffer>&, const String&, const vertex_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateVertexBuffer(SharedPtr<IVulkanVertexBuffer>& buffer, const String& name, const VulkanVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateIndexBuffer(SharedPtr<TIndexBuffer>&, const index_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateIndexBuffer(SharedPtr<IVulkanIndexBuffer>& buffer, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateIndexBuffer(SharedPtr<TIndexBuffer>&, const String&, const index_buffer_layout_type&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior) const
        bool tryCreateIndexBuffer(SharedPtr<IVulkanIndexBuffer>& buffer, const String& name, const VulkanIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateTexture(SharedPtr<TImage>&, Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior) const
        bool tryCreateTexture(SharedPtr<IVulkanImage>& image, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::tryCreateTexture(SharedPtr<TImage>&, const String&, Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior) const
        bool tryCreateTexture(SharedPtr<IVulkanImage>& image, const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createTextures
        Generator<SharedPtr<IVulkanImage>> createTextures(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const override;

        /// @copydoc GraphicsFactory::createSampler(FilterMode, FilterMode, BorderMode, BorderMode, BorderMode, MipMapMode, Float, Float, Float, Float) const
        SharedPtr<IVulkanSampler> createSampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// @copydoc GraphicsFactory::createSampler(const String&, FilterMode, FilterMode, BorderMode, BorderMode, BorderMode, MipMapMode, Float, Float, Float, Float) const
        SharedPtr<IVulkanSampler> createSampler(const String& name, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// @copydoc GraphicsFactory::createSamplers
        Generator<SharedPtr<IVulkanSampler>> createSamplers(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const override;

        /// @copydoc GraphicsFactory::createBottomLevelAccelerationStructure(StringView, AccelerationStructureFlags) const
        UniquePtr<VulkanBottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const override;

        /// @copydoc GraphicsFactory::createTopLevelAccelerationStructure(StringView, AccelerationStructureFlags) const
        UniquePtr<VulkanTopLevelAccelerationStructure> createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags = AccelerationStructureFlags::None) const override;

        /// @copydoc IGraphicsFactory::supportsResizableBaseAddressRegister
        bool supportsResizableBaseAddressRegister() const noexcept override;

        /// @copydoc IGraphicsFactory::memoryStatistics
        Array<MemoryHeapStatistics> memoryStatistics() const override;

        /// @copydoc IGraphicsFactory::detailedMemoryStatistics
        DetailedMemoryStatistics detailedMemoryStatistics() const override;
    };

    /// @brief Implements a Vulkan graphics device.
    class LITEFX_VULKAN_API VulkanDevice final : public GraphicsDevice<VulkanGraphicsFactory, VulkanSurface, VulkanGraphicsAdapter, VulkanSwapChain, VulkanQueue, VulkanRenderPass, VulkanRenderPipeline, VulkanComputePipeline, VulkanRayTracingPipeline, VulkanBarrier>, public Resource<VkDevice> {
        LITEFX_IMPLEMENTATION(VulkanDeviceImpl);
        friend struct SharedObject::Allocator<VulkanDevice>;
        friend class VulkanBackend;

    public:
        /// @brief The default size for the global buffer heap in bytes.
        ///
        /// The default value represents the minimum supported value over all devices registered at GPUinfo:
        /// https://vulkan.gpuinfo.org/displayextensionproperty.php?extensionname=VK_EXT_descriptor_buffer&extensionproperty=descriptorBufferAddressSpaceSize&platform=all.
        static const size_t DEFAULT_DESCRIPTOR_HEAP_SIZE = 134'217'728;   // equals 128 Mb

    private:
        /// @brief Creates a new device instance.
        ///
        /// @param backend The backend from which the device is created.
        /// @param adapter The adapter the device uses for drawing.
        /// @param surface The surface, the device should draw to.
        /// @param features The features that should be supported by this device.
        /// @param extensions The required extensions the device gets initialized with.
        /// @param globalDescriptorHeapSize The size of the global descriptor heap in bytes.
        explicit VulkanDevice(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, GraphicsDeviceFeatures features = { }, Span<String> extensions = { }, size_t globalDescriptorHeapSize = DEFAULT_DESCRIPTOR_HEAP_SIZE);

    private:
        /// @copydoc GraphicsDevice::GraphicsDevice(GraphicsDevice&&)
        VulkanDevice(VulkanDevice&&) noexcept = delete;

        /// @copydoc GraphicsDevice::GraphicsDevice(const GraphicsDevice&)
        VulkanDevice(const VulkanDevice&) = delete;

        /// @copydoc GraphicsDevice::operator=(GraphicsDevice&&)
        VulkanDevice& operator=(VulkanDevice&&) noexcept = delete;

        /// @copydoc GraphicsDevice::operator=(const GraphicsDevice&)
        VulkanDevice& operator=(const VulkanDevice&) = delete;

    public:
        /// @copydoc GraphicsDevice::~GraphicsDevice
        ~VulkanDevice() noexcept override;

        // Factory methods.
    public:
        /// @brief Initializes the device instance.
        ///
        /// Providing an extension chain using @p deviceExtensionObjects allows to customize which extensions to load and enable. User-defined extensions provided this way will be picked up and patched with the
        /// required settings accordingly. Settings enabled by the user will not be disabled this way, with the exception of features that are controlled by the @p features property.
        ///
        /// @param backend The backend from which the device is created.
        /// @param adapter The adapter the device uses for drawing.
        /// @param surface The surface, the device should draw to.
        /// @param features The features that should be supported by this device.
        /// @param extensions The required extensions the device gets initialized with.
        /// @param deviceExtensionObjects A pointer to additional extension objects (such as device features) that are stored in the devices's `pNext` chain during device creation.
        /// @param globalDescriptorHeapSize The size of the global descriptor heap in bytes.
        /// @return A shared pointer to the new device instance.
        static inline SharedPtr<VulkanDevice> create(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, GraphicsDeviceFeatures features = { }, Span<String> extensions = { }, void* deviceExtensionObjects = nullptr, size_t globalDescriptorHeapSize = DEFAULT_DESCRIPTOR_HEAP_SIZE) {
            return SharedObject::create<VulkanDevice>(backend, adapter, std::move(surface), features, extensions, globalDescriptorHeapSize)->initialize(Format::B8G8R8A8_SRGB, { 800, 600 }, 3, false, features, deviceExtensionObjects); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        }

        /// @brief Initializes the device instance.
        ///
        /// Providing an extension chain using @p deviceExtensionObjects allows to customize which extensions to load and enable. User-defined extensions provided this way will be picked up and patched with the
        /// required settings accordingly. Settings enabled by the user will not be disabled this way, with the exception of features that are controlled by the @p features property.
        ///
        /// @param backend The backend from which the device is created.
        /// @param adapter The adapter the device uses for drawing.
        /// @param surface The surface, the device should draw to.
        /// @param format The initial surface format, device uses for drawing.
        /// @param renderArea The initial size of the render area.
        /// @param backBuffers The initial number of back buffers.
        /// @param enableVsync The initial setting for vertical synchronization.
        /// @param features The features that should be supported by this device.
        /// @param extensions The required extensions the device gets initialized with.
        /// @param deviceExtensionObjects A pointer to additional extension objects (such as device features) that are stored in the devices's `pNext` chain during device creation.
        /// @param globalDescriptorHeapSize The size of the global descriptor heap in bytes.
        /// @return A shared pointer to the new device instance.
        static inline SharedPtr<VulkanDevice> create(const VulkanBackend& backend, const VulkanGraphicsAdapter& adapter, UniquePtr<VulkanSurface>&& surface, Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync = false, GraphicsDeviceFeatures features = { }, Span<String> extensions = { }, void* deviceExtensionObjects = nullptr, size_t globalDescriptorHeapSize = DEFAULT_DESCRIPTOR_HEAP_SIZE) {
            return SharedObject::create<VulkanDevice>(backend, adapter, std::move(surface), features, extensions, globalDescriptorHeapSize)->initialize(format, renderArea, backBuffers, enableVsync, features, deviceExtensionObjects);
        }

    private:
        /// @brief Initializes the resources owned by the device.
        ///
        /// @param format The initial surface format, device uses for drawing.
        /// @param renderArea The initial size of the render area.
        /// @param backBuffers The initial number of back buffers.
        /// @param enableVsync The initial setting for vertical synchronization.
        /// @param features The features that should be supported by this device.
        /// @param deviceExtensionObjects A pointer to additional extension objects (such as device features) that are stored in the devices's `pNext` chain during device creation.
        /// @return A shared pointer to the current device instance.
        SharedPtr<VulkanDevice> initialize(Format format, const Size2d& renderArea, UInt32 backBuffers, bool enableVsync = false, GraphicsDeviceFeatures features = { }, void* deviceExtensionObjects = nullptr);

        /// @brief Releases the device state to prepare it for destruction.
        void release() noexcept;

        // Vulkan Device interface.
    private:
        /// @brief Sets the debug name for an object.
        ///
        /// This function sets the debug name for an object to make it easier to identify when using an external debugger. This function will do nothing in release mode or if the device extension
        /// VK_EXT_debug_marker is not available.
        ///
        /// @param objectType The type of the object.
        /// @param objectHandle The handle of the object casted to an integer.
        /// @param name The debug name of the object.
        void setDebugName(VkDebugReportObjectTypeEXT objectType, UInt64 objectHandle, StringView name) const;

    public:
        /// @brief Returns the array that stores the extensions that were used to initialize the device.
        ///
        /// @return A reference to the array that stores the extensions that were used to initialize the device.
        Span<const String> enabledExtensions() const noexcept;

        /// @brief Sets the debug name for an object.
        ///
        /// This function sets the debug name for an object to make it easier to identify when using an external debugger. This function will do nothing in release mode or if the device extension
        /// VK_EXT_debug_marker is not available.
        ///
        /// @tparam THandle The type of the handle. Must be a Vulkan handle type.
        /// @param objectHandle The handle of the object casted to an integer.
        /// @param objectType The type of the object.
        /// @param name The debug name of the object.
        template <typename THandle>
        inline void setDebugName(THandle objectHandle, VkDebugReportObjectTypeEXT objectType, StringView name) const {
            this->setDebugName(objectType, Vk::handleAddress(objectHandle), name); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        }

        /// @brief Returns the indices of all queue families with support for @p type.
        ///
        /// @param type The type of workload that must be supported by the family in order for it to be returned. Specifying @ref QueueType::None will return all available queue families.
        /// @return The indices of the queue families that support queue workloads specified by @p type.
        Enumerable<UInt32> queueFamilyIndices(QueueType type = QueueType::None) const;

        /// @brief Resolves the binary size for a descriptor of @p type.
        ///
        /// @param type The type of the descriptor.
        /// @return The size of the descriptor.
        UInt32 descriptorSize(DescriptorType type) const;

        // GraphicsDevice interface.
    public:
        /// @copydoc IGraphicsDevice::state
        DeviceState& state() const noexcept override;

        /// @copydoc GraphicsDevice::swapChain() const
        const VulkanSwapChain& swapChain() const noexcept override;

        /// @copydoc GraphicsDevice::swapChain() const
        VulkanSwapChain& swapChain() noexcept override;

        /// @copydoc GraphicsDevice::surface
        const VulkanSurface& surface() const noexcept override;

        /// @copydoc GraphicsDevice::adapter
        const VulkanGraphicsAdapter& adapter() const noexcept override;

        /// @copydoc GraphicsDevice::factory
        const VulkanGraphicsFactory& factory() const noexcept override;

        /// @copydoc GraphicsDevice::defaultQueue
        const VulkanQueue& defaultQueue(QueueType type) const override;

        /// @copydoc GraphicsDevice::createQueue
        SharedPtr<const VulkanQueue> createQueue(QueueType type, QueuePriority priority = QueuePriority::Normal) override;

        /// @copydoc GraphicsDevice::makeBarrier
        [[nodiscard]] UniquePtr<VulkanBarrier> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const override;

        /// @copydoc GraphicsDevice::makeFrameBuffer(StringView, const Size2d&) const
        [[nodiscard]] SharedPtr<VulkanFrameBuffer> makeFrameBuffer(StringView name, const Size2d& renderArea) const override;

        /// @copydoc GraphicsDevice::makeFrameBuffer(StringView, const Size2d&, frame_buffer_type::allocation_callback_type) const
        [[nodiscard]] SharedPtr<VulkanFrameBuffer> makeFrameBuffer(StringView name, const Size2d& renderArea, VulkanFrameBuffer::allocation_callback_type allocationCallback) const override;

        /// @copydoc IGraphicsDevice::maximumMultiSamplingLevel
        MultiSamplingLevel maximumMultiSamplingLevel(Format format) const override;

        /// @copydoc IGraphicsDevice::ticksPerMillisecond
        double ticksPerMillisecond() const noexcept override;

        /// @copydoc IGraphicsDevice::wait
        void wait() const override;

        /// @copydoc GraphicsDevice::computeAccelerationStructureSizes(const bottom_level_acceleration_structure_type&, UInt64&, UInt64&, bool) const
        void computeAccelerationStructureSizes(const VulkanBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const override;

        /// @copydoc GraphicsDevice::computeAccelerationStructureSizes(const top_level_acceleration_structure_type&, UInt64&, UInt64&, bool) const
        void computeAccelerationStructureSizes(const VulkanTopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const override;

        /// @copydoc GraphicsDevice::allocateGlobalDescriptors
        [[nodiscard]] VirtualAllocator::Allocation allocateGlobalDescriptors(const VulkanDescriptorSet& descriptorSet, DescriptorHeapType heapType) const override;

        /// @copydoc GraphicsDevice::releaseGlobalDescriptors
        void releaseGlobalDescriptors(const VulkanDescriptorSet& descriptorSet) const override;

        /// @copydoc GraphicsDevice::updateGlobalDescriptors
        void updateGlobalDescriptors(const VulkanDescriptorSet& descriptorSet, UInt32 binding, UInt32 offset, UInt32 descriptors) const override;

        /// @copydoc GraphicsDevice::bindDescriptorSet
        void bindDescriptorSet(const VulkanCommandBuffer& commandBuffer, const VulkanDescriptorSet& descriptorSet, const VulkanPipelineState& pipeline) const override;

        /// @copydoc GraphicsDevice::bindGlobalDescriptorHeaps
        void bindGlobalDescriptorHeaps(const VulkanCommandBuffer& commandBuffer) const noexcept override;

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
    public:
        /// @copydoc GraphicsDevice::buildRenderPass(UInt32) const
        [[nodiscard]] VulkanRenderPassBuilder buildRenderPass(UInt32 commandBuffers = 1) const override;

        /// @copydoc GraphicsDevice::buildRenderPass(const String&, UInt32) const
        [[nodiscard]] VulkanRenderPassBuilder buildRenderPass(const String& name, UInt32 commandBuffers = 1) const override;

        ///// <inheritdoc cref="GraphicsDevice::buildRenderPipeline(const String&) const" />
        //[[nodiscard]] VulkanRenderPipelineBuilder buildRenderPipeline(const String& name) const override;

        /// @copydoc GraphicsDevice::buildRenderPipeline
        [[nodiscard]] VulkanRenderPipelineBuilder buildRenderPipeline(const VulkanRenderPass& renderPass, const String& name) const override;

        /// @copydoc GraphicsDevice::buildComputePipeline
        [[nodiscard]] VulkanComputePipelineBuilder buildComputePipeline(const String& name) const override;

        /// @copydoc GraphicsDevice::buildRayTracingPipeline(ShaderRecordCollection&&) const
        [[nodiscard]] VulkanRayTracingPipelineBuilder buildRayTracingPipeline(ShaderRecordCollection&& shaderRecords) const override;

        /// @copydoc GraphicsDevice::buildRayTracingPipeline(const String&, ShaderRecordCollection&&) const
        [[nodiscard]] VulkanRayTracingPipelineBuilder buildRayTracingPipeline(const String& name, ShaderRecordCollection&& shaderRecords) const override;
        
        /// @copydoc GraphicsDevice::buildPipelineLayout
        [[nodiscard]] VulkanPipelineLayoutBuilder buildPipelineLayout() const override;

        /// @copydoc GraphicsDevice::buildInputAssembler
        [[nodiscard]] VulkanInputAssemblerBuilder buildInputAssembler() const override;

        /// @copydoc GraphicsDevice::buildRasterizer
        [[nodiscard]] VulkanRasterizerBuilder buildRasterizer() const override;

        /// @copydoc GraphicsDevice::buildShaderProgram
        [[nodiscard]] VulkanShaderProgramBuilder buildShaderProgram() const override;

        /// @copydoc GraphicsDevice::buildBarrier
        [[nodiscard]] VulkanBarrierBuilder buildBarrier() const override;
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)
    };

    /// @brief Defines a rendering backend that creates a Vulkan device.
    class LITEFX_VULKAN_API VulkanBackend final : public RenderBackend<VulkanDevice>, public Resource<VkInstance> {
        LITEFX_IMPLEMENTATION(VulkanBackendImpl);

    public:
        /// @brief Initializes a new vulkan rendering backend.
        ///
        /// @param app An instance of the app that owns the backend.
        /// @param extensions A set of instance extensions to enable on the backend instance.
        /// @param validationLayers A set of validation layers to enable on the rendering backend.
        /// @param instanceExtensionObjects A pointer to additional extension objects that are stored in the instance's `pNext` chain during instance creation.
        explicit VulkanBackend(const App& app, const Span<String> extensions = { }, const Span<String> validationLayers = { }, void* instanceExtensionObjects = nullptr);

        /// @copydoc RenderBackend::RenderBackend(RenderBackend&&)
        VulkanBackend(VulkanBackend&&) noexcept;

        /// @copydoc RenderBackend::RenderBackend(const RenderBackend&)
        VulkanBackend(const VulkanBackend&) = delete;

        /// @copydoc RenderBackend::operator=(RenderBackend&&)
        VulkanBackend& operator=(VulkanBackend&&) noexcept;

        /// @copydoc RenderBackend::operator=(const RenderBackend&)
        VulkanBackend& operator=(const VulkanBackend&) = delete;

        /// @copydoc RenderBackend::~RenderBackend
        ~VulkanBackend() noexcept override;

    public:
        /// @brief Returns the validation layers that are enabled on the backend.
        ///
        /// @return An array of validation layers that are enabled on the backend.
        Span<const String> getEnabledValidationLayers() const noexcept;

#ifdef VK_USE_PLATFORM_WIN32_KHR
        /// @brief Creates a surface on a window handle.
        ///
        /// @param hwnd The window handle on which the surface should be created.
        /// @return The instance of the created surface.
        UniquePtr<VulkanSurface> createSurface(const HWND& hwnd) const;
#else
        /// @brief A callback that creates a surface from a Vulkan instance.
        typedef std::function<VkSurfaceKHR(const VkInstance&)> surface_callback;

        /// @brief Creates a surface using the @p predicate callback.
        ///
        /// @param predicate A callback that gets called with the backend instance handle and creates the surface instance
        /// @return The instance of the created surface.
        /// @see surface_callback
        UniquePtr<VulkanSurface> createSurface(surface_callback predicate) const;
#endif // VK_USE_PLATFORM_WIN32_KHR

    public:
        /// @brief Returns `true`, if all elements of @p are contained by the a list of available extensions.
        ///
        /// @return `true`, if all elements of @p are contained by the a list of available extensions.
        /// @see getAvailableInstanceExtensions
        static bool validateInstanceExtensions(Span<const String> extensions);

        /// @brief Returns a list of available extensions.
        ///
        /// @return A list of available extensions.
        /// @see validateInstanceExtensions
        static Enumerable<String> getAvailableInstanceExtensions();

        /// @brief Returns `true`, if all elements of @p are contained by the a list of available validation layers.
        ///
        /// @return `true`, if all elements of @p are contained by the a list of available validation layers.
        /// @see getInstanceValidationLayers
        static bool validateInstanceLayers(const Span<const String> validationLayers);

        /// @brief Returns a list of available validation layers.
        ///
        /// @return A list of available validation layers.
        /// @see validateInstanceLayers
        static Enumerable<String> getInstanceValidationLayers();

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
        const Array<SharedPtr<const VulkanGraphicsAdapter>>& adapters() const override;

        /// @copydoc RenderBackend::findAdapter(const Optional<UInt64>&) const
        const VulkanGraphicsAdapter* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const override;

        /// @copydoc RenderBackend::findAdapter(GpuPreference) const
        const VulkanGraphicsAdapter* findAdapter(GpuPreference preference) const override;

        /// @copydoc RenderBackend::registerDevice
        void registerDevice(const String& name, SharedPtr<VulkanDevice>&& device) override;

        /// @copydoc RenderBackend::releaseDevice
        void releaseDevice(const String& name) override;

        /// @copydoc RenderBackend::device(const String&)
        VulkanDevice* device(const String& name) override;

        /// @copydoc RenderBackend::device(const String&)
        const VulkanDevice* device(const String& name) const override;
    };

}

// NOLINTEND(bugprone-derived-method-shadowing-base-method)

#pragma warning(pop)