#pragma once

#include <litefx/rendering_api.hpp>
#include <litefx/rendering_formatters.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;
    using namespace LiteFX::Math;

    /// <summary>
    /// A barrier used for GPU resource synchronization.
    /// </summary>
    /// <seealso cref="IBarrier" />
    template <typename TBuffer, typename TImage> requires
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TImage, IImage>
    class Barrier : public IBarrier {
    public:
        using IBarrier::transition;

        using buffer_type = TBuffer;
        using image_type = TImage;

    public:
        constexpr inline virtual ~Barrier() noexcept = default;

    public:
        /// <inheritdoc />
        constexpr inline virtual void transition(const buffer_type& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;

        /// <inheritdoc />
        constexpr inline virtual void transition(const buffer_type& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;

        /// <inheritdoc />
        constexpr inline virtual void transition(const image_type& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;

        /// <inheritdoc />
        constexpr inline virtual void transition(const image_type& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;

        /// <inheritdoc />
        constexpr inline virtual void transition(const image_type& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;

        /// <inheritdoc />
        constexpr inline virtual void transition(const image_type& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;

    private:
        constexpr inline void doTransition(const IBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) override {
            this->transition(dynamic_cast<const buffer_type&>(buffer), accessBefore, accessAfter);
        }

        constexpr inline void doTransition(const IBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) override {
            this->transition(dynamic_cast<const buffer_type&>(buffer), element, accessBefore, accessAfter);
        }

        constexpr inline void doTransition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override {
            this->transition(dynamic_cast<const image_type&>(image), accessBefore, accessAfter, layout);
        }

        constexpr inline void doTransition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override {
            this->transition(dynamic_cast<const image_type&>(image), accessBefore, accessAfter, fromLayout, toLayout);
        }

        constexpr inline void doTransition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override {
            this->transition(dynamic_cast<const image_type&>(image), level, levels, layer, layers, plane, accessBefore, accessAfter, layout);
        }

        constexpr inline void doTransition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override {
            this->transition(dynamic_cast<const image_type&>(image), level, levels, layer, layers, plane, accessBefore, accessAfter, fromLayout, toLayout);
        }
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
    /// From a CPU perspective, think of a descriptor set as an array of pointers to different buffers (i.e. descriptors) for the shader. A descriptor can be bound to a set by 
    /// calling <see cref="DescriptorSet::update" />. Note that this does not automatically ensure, that the buffer memory is visible for the GPU. Instead, a buffer may also 
    /// require a transfer into GPU visible memory, depending on the <see cref="ResourceHeap" />. However, as long as a descriptor within a set is mapped to a buffer, modifying 
    /// this buffer also reflects the change to the shader, without requiring to update the descriptor, similarly to how modifying the object behind a pointer does not require 
    /// the pointer to change.
    /// 
    /// Note, that there might be multiple descriptor set instances of the same <see cref="DescriptorSetLayout" />, pointing to different <see cref="IBuffer" /> instances, 
    /// depending on the number of <i>frames in flight</i>. Since multiple frames can be computed concurrently, it is important to properly synchronize descriptor set updates.
    /// Generally, there are three strategies to choose from, that you can implement or mix in custom flavors, depending on your use case:
    /// 
    /// <list type="bullet">
    ///   <item>
    ///     <term>Naive:</term>
    ///     <description>
    ///     The naive approach most closely matches earlier graphics API concepts. Create one buffer per descriptor and synchronize frames. This basically means that each 
    ///     back buffer swap is synchronized to wait for the graphics pipeline. This way, writing to a buffer ensures, that it is only read within the frame of reference and 
    ///     modifying it does not interfere with other frames. This strategy is memory efficient, but may cause the GPU to stall. It may, however be a valid strategy, for 
    ///     data that is only written once or very infrequently.
    ///     </description>
    ///   </item>
    ///   <item>
    ///     <term>Array of Buffers:</term>
    ///     <description>
    ///     The helper methods for creating and updating constant buffers are able to create buffer arrays. Those arrays can be used to create a buffer for each frame in 
    ///     flight. When binding a buffer to a descriptor, it is possible to bind only one element of the array. This way, each frame has its own buffer and does not 
    ///     interfere with other buffer writes.
    ///     </description>
    ///   </item>
    ///   <item>
    ///     <term>Ring-Buffer:</term>
    ///     <description>
    ///     The most efficient (yet not always applicable) approach involves creating one large buffer array, that is bound to multiple descriptor sets. This ensures that
    ///     the buffer memory stays contiguous and does not get fragmented. However, this requires to know upfront, how many buffers are required for each descriptor, which
    ///     might not always be possible. Thus another flavor of using this technique involves a creating a <i>large enough</i> descriptor array and updating the descriptor
    ///     set with an increasing array element for each object as a ring-buffer. As long as there are enough elements in the buffer, so that no second update interferes 
    ///     with a buffer write in an earlier frame, this method provides the most efficient approach. However, it may be hard or impossible to determine the ideal size of 
    ///     the ring-buffer upfront.
    ///     </description>
    ///   </itemattach
    /// </list>
    /// 
    /// Note that samplers, textures and input attachments currently do not support array binding, since they are typically only updated once or require pipeline 
    /// synchronization anyway.
    /// 
    /// Also note, that another buffer management strategy is currently not available: the <i>Monolithic Buffer</i>. In this strategy, there is only one large buffer for 
    /// <i>all</i> buffers. Differently from the ring buffer strategy, where there is one buffer per descriptor type, a monolithic buffer combines multiple constant buffers, 
    /// containing different data into one giant buffer block. Calling <see cref="RenderPipeline::bind" /> for a descriptor set would then receive an additional dynamic 
    /// offset for each descriptor within the descriptor set.
    /// </remarks>
    /// <typeparam name="TBuffer">The type of the buffer interface. Must inherit from <see cref="IBuffer"/>.</typeparam>
    /// <typeparam name="TImage">The type of the image interface. Must inherit from <see cref="IImage"/>.</typeparam>
    /// <typeparam name="TSampler">The type of the sampler interface. Must inherit from <see cref="ISampler"/>.</typeparam>
    /// <typeparam name="TAccelerationStructure">The type of the acceleration structure interface. Must inherit from <see cref="IAccelerationStructure"/>.</typeparam>
    /// <seealso cref="DescriptorSetLayout" />
    template <typename TBuffer, typename TImage, typename TSampler, typename TAccelerationStructure> requires
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TSampler, ISampler> &&
        std::derived_from<TImage, IImage> &&
        std::derived_from<TAccelerationStructure, IAccelerationStructure>
    class DescriptorSet : public IDescriptorSet {
    public:
        using IDescriptorSet::update;

        using buffer_type = TBuffer;
        using sampler_type = TSampler;
        using image_type = TImage;
        using acceleration_structure_type = TAccelerationStructure;

    public:
        virtual ~DescriptorSet() noexcept = default;

    public:
        /// <inheritdoc />
        virtual void update(UInt32 binding, const buffer_type& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, UInt32 firstDescriptor = 0) const = 0;

        /// <inheritdoc />
        virtual void update(UInt32 binding, const image_type& texture, UInt32 descriptor = 0, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const = 0;

        /// <inheritdoc />
        virtual void update(UInt32 binding, const sampler_type& sampler, UInt32 descriptor = 0) const = 0;

        /// <inheritdoc />
        virtual void update(UInt32 binding, const acceleration_structure_type& accelerationStructure, UInt32 descriptor = 0) const = 0;

    private:
        void doUpdate(UInt32 binding, const IBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor) const override {
            this->update(binding, dynamic_cast<const buffer_type&>(buffer), bufferElement, elements, firstDescriptor);
        }

        void doUpdate(UInt32 binding, const IImage& texture, UInt32 descriptor, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const override {
            this->update(binding, dynamic_cast<const image_type&>(texture), descriptor, firstLevel, levels, firstLayer, layers);
        }

        void doUpdate(UInt32 binding, const ISampler& sampler, UInt32 descriptor) const override {
            this->update(binding, dynamic_cast<const sampler_type&>(sampler), descriptor);
        }

        void doUpdate(UInt32 binding, const IAccelerationStructure& accelerationStructure, UInt32 descriptor) const override {
            this->update(binding, dynamic_cast<const acceleration_structure_type&>(accelerationStructure), descriptor);
        }
    };

    /// <summary>
    /// Describes the layout of a descriptor set.
    /// </summary>
    /// <remarks>
    /// A descriptor set groups together multiple descriptors. This concept is identified by the <c>set</c> keyword in GLSL and <c>space</c> in HLSL.
    /// 
    /// For more information on buffer binding and resource management, refer to the remarks of the <see cref="DescriptorSet" /> interface.
    /// </remarks>
    /// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout"/>.</typeparam>
    /// <typeparam name="TDescriptorSet">The type of the descriptor set. Must implement <see cref="DescriptorSet"/>.</typeparam>
    /// <seealso cref="IDescriptorLayout" />
    /// <seealso cref="DescriptorSet" />
    template <typename TDescriptorLayout, typename TDescriptorSet> requires
        meta::implements<TDescriptorLayout, IDescriptorLayout> &&
        meta::implements<TDescriptorSet, DescriptorSet<typename TDescriptorSet::buffer_type, typename TDescriptorSet::image_type, typename TDescriptorSet::sampler_type, typename TDescriptorSet::acceleration_structure_type>>
    class DescriptorSetLayout : public IDescriptorSetLayout {
    public:
        using IDescriptorSetLayout::free;

        using descriptor_layout_type = TDescriptorLayout;
        using descriptor_set_type = TDescriptorSet;

    public:
        virtual ~DescriptorSetLayout() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Enumerable<const descriptor_layout_type*> descriptors() const noexcept = 0;

        /// <inheritdoc />
        virtual const descriptor_layout_type& descriptor(UInt32 binding) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<descriptor_set_type> allocate(const Enumerable<DescriptorBinding>& bindings = { }) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<descriptor_set_type> allocate(UInt32 descriptors, const Enumerable<DescriptorBinding>& bindings = { }) const = 0;

        /// <inheritdoc />
        virtual Enumerable<UniquePtr<descriptor_set_type>> allocateMultiple(UInt32 descriptorSets, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const = 0;

        /// <inheritdoc />
        virtual Enumerable<UniquePtr<descriptor_set_type>> allocateMultiple(UInt32 descriptorSets, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const = 0;

        /// <inheritdoc />
        virtual Enumerable<UniquePtr<descriptor_set_type>> allocateMultiple(UInt32 descriptorSets, UInt32 descriptors, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const = 0;

        /// <inheritdoc />
        virtual Enumerable<UniquePtr<descriptor_set_type>> allocateMultiple(UInt32 descriptorSets, UInt32 descriptors, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const = 0;

        /// <inheritdoc />
        virtual void free(const descriptor_set_type& descriptorSet) const noexcept = 0;

    private:
        inline Enumerable<const IDescriptorLayout*> getDescriptors() const noexcept override {
            return this->descriptors();
        }

        inline UniquePtr<IDescriptorSet> getDescriptorSet(UInt32 descriptors, const Enumerable<DescriptorBinding>& bindings = { }) const override {
            return this->allocate(descriptors, bindings);
        }

        inline Enumerable<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, const Enumerable<Enumerable<DescriptorBinding>>& bindings = { }) const override {
            return this->allocateMultiple(descriptorSets, descriptors, bindings) | std::views::as_rvalue;
        }

        inline Enumerable<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, std::function<Enumerable<DescriptorBinding>(UInt32)> bindingFactory) const override {
            return this->allocateMultiple(descriptorSets, descriptors, bindingFactory) | std::views::as_rvalue;
        }

        inline void releaseDescriptorSet(const IDescriptorSet& descriptorSet) const noexcept override {
            this->releaseDescriptorSet(dynamic_cast<const descriptor_set_type&>(descriptorSet));
        }
    };

    /// <summary>
    /// Describes the layout of the pipelines push constant ranges.
    /// </summary>
    /// <remarks>
    /// Push constants are very efficient, yet quite limited ways of passing per-draw data to shaders. They are mapped directly, so no buffer must be created or transitioned in order to 
    /// use them. Conceptually a push constant is a single piece of memory that gets transferred to a command buffer directly. Each shader stage has a view into this piece of memory, 
    /// described by an offset and a size. Since the memory is directly dumped in the command buffer, the backing memory can be incrementally updated and there is no need to store an
    /// array of buffers, as long as updates happen sequentially from the same thread. However,  there are certain restrictions when using push constants:
    /// 
    /// <list type="bullet">
    ///     <item>Only one push constant layout per pipeline layout is supported.</item>
    ///     <item>A push constant layout may not define a overall memory size larger than 128 bytes. This is a soft restriction that will issue a warning, however it might be supported on some hardware or for some back-ends.</item>
    ///     <item>The elements and offsets of a push constant memory block must be aligned to 4 bytes.</item>
    ///     <item>Accordingly, the size of a push constant layout must align to 4 bytes.</item>
    ///     <item>Only one <see cref="IPushConstantsRange" /> per shader stage is permitted. Shader stages can be combined together, however in this case, no other ranges must be defined for the stages.</item>
    /// </list>
    /// 
    /// Push constants can be updated by calling <see cref="CommandBuffer::PushConstants" /> and are visible to subsequent draw calls immediately, until another update is performed or
    /// the command buffer is ended.
    /// </remarks>
    /// <typeparam name="TPushConstantsRange">The type of the push constant range. Must implement <see cref="IPushConstantsRange" />.</typeparam>
    /// <seealso cref="IPushConstantsRange" />
    /// <seealso cref="DescriptorSetLayout" />
    template <typename TPushConstantsRange> requires
        meta::implements<TPushConstantsRange, IPushConstantsRange>
    class PushConstantsLayout : public IPushConstantsLayout {
    public:
        using push_constants_range_type = TPushConstantsRange;

    public:
        virtual ~PushConstantsLayout() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Enumerable<const push_constants_range_type*> ranges() const noexcept = 0;

    private:
        inline Enumerable<const IPushConstantsRange*> getRanges() const noexcept override {
            return this->ranges();
        }
    };

    /// <summary>
    /// Represents a shader program, consisting of multiple <see cref="IShaderModule" />s.
    /// </summary>
    /// <typeparam name="TShaderModule">The type of the shader module. Must implement <see cref="IShaderModule"/>.</typeparam>
    /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
    template <typename TShaderModule> requires
        meta::implements<TShaderModule, IShaderModule>
    class ShaderProgram : public IShaderProgram {
    public:
        using shader_module_type = TShaderModule;

    public:
        virtual ~ShaderProgram() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Enumerable<const shader_module_type*> modules() const noexcept = 0;

    private:
        inline virtual Enumerable<const IShaderModule*> getModules() const noexcept {
            return this->modules();
        }
    };
    
    /// <summary>
    /// Represents a the layout of a <see cref="RenderPipeline" />, <see cref="ComputePipeline" /> or <see cref="RayTracingPipeline" />.
    /// </summary>
    /// <typeparam name="TDescriptorSetLayout">The type of the descriptor set layout. Must implement <see cref="DescriptorSetLayout"/>.</typeparam>
    /// <typeparam name="TPushConstantsLayout">The type of the push constants layout. Must implement <see cref="PushConstantsLayout"/>.</typeparam>
    template <typename TDescriptorSetLayout, typename TPushConstantsLayout> requires
        meta::implements<TDescriptorSetLayout, DescriptorSetLayout<typename TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSetLayout::descriptor_set_type>> &&
        meta::implements<TPushConstantsLayout, PushConstantsLayout<typename TPushConstantsLayout::push_constants_range_type>>
    class PipelineLayout : public IPipelineLayout {
    public:
        using descriptor_set_layout_type = TDescriptorSetLayout;
        using push_constants_layout_type = TPushConstantsLayout;

    public:
        virtual ~PipelineLayout() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const descriptor_set_layout_type& descriptorSet(UInt32 space) const = 0;

        /// <inheritdoc />
        virtual Enumerable<const descriptor_set_layout_type*> descriptorSets() const noexcept = 0;

        /// <inheritdoc />
        virtual const push_constants_layout_type* pushConstants() const noexcept = 0;

    private:
        inline Enumerable<const IDescriptorSetLayout*> getDescriptorSets() const noexcept override {
            return this->descriptorSets();
        }
    };

    /// <summary>
    /// Describes a vertex buffer.
    /// </summary>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout"/>.</typeparam>
    template <typename TVertexBufferLayout> requires
        meta::implements<TVertexBufferLayout, IVertexBufferLayout>
    class VertexBuffer : public virtual IVertexBuffer {
    public:
        using vertex_buffer_layout_type = TVertexBufferLayout;

    public:
        virtual ~VertexBuffer() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const vertex_buffer_layout_type& layout() const noexcept = 0;
    };

    /// <summary>
    /// Describes an index buffer.
    /// </summary>
    /// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout"/>.</typeparam>
    template <typename TIndexBufferLayout> requires
        meta::implements<TIndexBufferLayout, IIndexBufferLayout>
    class IndexBuffer : public virtual IIndexBuffer {
    public:
        using index_buffer_layout_type = TIndexBufferLayout;

    public:
        virtual ~IndexBuffer() noexcept = default;

    public:
        /// <inheritdoc/>
        virtual const index_buffer_layout_type& layout() const noexcept = 0;
    };

    /// <summary>
    /// Represents a the input assembler state of a <see cref="RenderPipeline" />.
    /// </summary>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout"/>.</typeparam>
    /// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout"/>.</typeparam>
    template <typename TVertexBufferLayout, typename TIndexBufferLayout> requires
        meta::implements<TVertexBufferLayout, IVertexBufferLayout> &&
        meta::implements<TIndexBufferLayout, IIndexBufferLayout>
    class InputAssembler : public IInputAssembler {
    public:
        using vertex_buffer_layout_type = TVertexBufferLayout;
        using index_buffer_layout_type = TIndexBufferLayout;

    public:
        virtual ~InputAssembler() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Enumerable<const vertex_buffer_layout_type*> vertexBufferLayouts() const noexcept = 0;

        /// <inheritdoc />
        virtual const vertex_buffer_layout_type* vertexBufferLayout(UInt32 binding) const = 0;

        /// <inheritdoc />
        virtual const index_buffer_layout_type* indexBufferLayout() const noexcept = 0;

    private:
        inline Enumerable<const IVertexBufferLayout*> getVertexBufferLayouts() const noexcept override {
            return this->vertexBufferLayouts();
        }
    };

    /// <summary>
    /// Represents a pipeline state.
    /// </summary>
    /// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="PipelineLayout"/>.</typeparam>
    /// <typeparam name="TShaderProgram">The type of the shader program. Must implement <see cref="ShaderProgram"/>.</typeparam>
    /// <seealso cref="RenderPipeline" />
    /// <seealso cref="ComputePipeline" />
    template <typename TPipelineLayout, typename TShaderProgram> requires
        meta::implements<TPipelineLayout, PipelineLayout<typename TPipelineLayout::descriptor_set_layout_type, typename TPipelineLayout::push_constants_layout_type>> &&
        meta::implements<TShaderProgram, ShaderProgram<typename TShaderProgram::shader_module_type>>
    class Pipeline : public virtual IPipeline, public virtual StateResource {
    public:
        using shader_program_type = TShaderProgram;
        using pipeline_layout_type = TPipelineLayout;

    public:
        virtual ~Pipeline() noexcept = default;

    public:
        /// <inheritdoc />
        virtual SharedPtr<const shader_program_type> program() const noexcept = 0;

        /// <inheritdoc />
        virtual SharedPtr<const pipeline_layout_type> layout() const noexcept = 0;

    private:
        inline SharedPtr<const IShaderProgram> getProgram() const noexcept override {
            return std::static_pointer_cast<const IShaderProgram>(this->program());
        }

        inline SharedPtr<const IPipelineLayout> getLayout() const noexcept override {
            return std::static_pointer_cast<const IPipelineLayout>(this->layout());
        }
    };

    /// <summary>
    /// Represents a command buffer, that buffers commands that should be submitted to a <see cref="CommandQueue" />.
    /// </summary>
    /// <typeparam name="TCommandBuffer">The type of the command buffer itself. Must inherit from <see cref="CommandBuffer" />.</typeparam>
    /// <typeparam name="TBuffer">The generic buffer type. Must implement <see cref="IBuffer"/>.</typeparam>
    /// <typeparam name="TVertexBuffer">The vertex buffer type. Must implement <see cref="VertexBuffer"/>.</typeparam>
    /// <typeparam name="TIndexBuffer">The index buffer type. Must implement <see cref="IndexBuffer"/>.</typeparam>
    /// <typeparam name="TImage">The generic image type. Must implement <see cref="IImage"/>.</typeparam>
    /// <typeparam name="TBarrier">The barrier type. Must implement <see cref="Barrier"/>.</typeparam>
    /// <typeparam name="TPipeline">The common pipeline interface type. Must be derived from <see cref="Pipeline"/>.</typeparam>
    /// <typeparam name="TBLAS">The type of the bottom-level acceleration structure. Must implement <see cref="IBottomLevelAccelerationStructure" />.</typeparam>
    /// <typeparam name="TTLAS">The type of the top-level acceleration structure. Must implement <see cref="ITopLevelAccelerationStructure" />.</typeparam>
    template <typename TCommandBuffer, typename TBuffer, typename TVertexBuffer, typename TIndexBuffer, typename TImage, typename TBarrier, typename TPipeline, typename TBLAS, typename TTLAS> requires
        meta::implements<TBarrier, Barrier<TBuffer, TImage>> &&
        //std::derived_from<TCommandBuffer, ICommandBuffer> &&
        std::derived_from<TPipeline, Pipeline<typename TPipeline::pipeline_layout_type, typename TPipeline::shader_program_type>> &&
        std::derived_from<TBLAS, IBottomLevelAccelerationStructure> &&
        std::derived_from<TTLAS, ITopLevelAccelerationStructure>
    class CommandBuffer : public ICommandBuffer {
    public:
        using ICommandBuffer::queue;
        using ICommandBuffer::dispatch;
        using ICommandBuffer::dispatchMesh;
        using ICommandBuffer::draw;
        using ICommandBuffer::drawIndexed;
        using ICommandBuffer::barrier;
        using ICommandBuffer::transfer;
        using ICommandBuffer::generateMipMaps;
        using ICommandBuffer::bind;
        using ICommandBuffer::use;
        using ICommandBuffer::pushConstants;
        using ICommandBuffer::buildAccelerationStructure;
        using ICommandBuffer::updateAccelerationStructure;
        using ICommandBuffer::copyAccelerationStructure;

    public:
        using command_buffer_type = TCommandBuffer;
        using buffer_type = TBuffer;
        using vertex_buffer_type = TVertexBuffer;
        using index_buffer_type = TIndexBuffer;
        using image_type = TImage;
        using barrier_type = TBarrier;
        using bottom_level_acceleration_structure_type = TBLAS;
        using top_level_acceleration_structure_type = TTLAS;

    public:
        using command_buffer_type = TCommandBuffer;
        using buffer_type = TBuffer;
        using vertex_buffer_type = TVertexBuffer;
        using index_buffer_type = TIndexBuffer;
        using image_type = TImage;
        using barrier_type = TBarrier;
        using pipeline_type = TPipeline;
        using pipeline_layout_type = pipeline_type::pipeline_layout_type;
        using descriptor_set_layout_type = pipeline_layout_type::descriptor_set_layout_type;
        using push_constants_layout_type = pipeline_layout_type::push_constants_layout_type;
        using descriptor_set_type = descriptor_set_layout_type::descriptor_set_type;

    public:
        virtual ~CommandBuffer() noexcept = default;

    public:
        /// <inheritdoc />
        virtual UniquePtr<barrier_type> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept = 0;

        /// <inheritdoc />
        virtual void barrier(const barrier_type& barrier) const noexcept = 0;

        /// <inheritdoc />
        virtual void generateMipMaps(image_type& image) noexcept = 0;

        /// <inheritdoc />
        virtual void transfer(buffer_type& source, buffer_type& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(const void* const data, size_t size, buffer_type& target, UInt32 targetElement = 0, UInt32 elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(Span<const void* const> data, size_t elementSize, buffer_type& target, UInt32 firstElement = 0) const = 0;

        /// <inheritdoc />
        virtual void transfer(buffer_type& source, image_type& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(const void* const data, size_t size, image_type& target, UInt32 subresource = 0) const = 0;

        /// <inheritdoc />
        virtual void transfer(Span<const void* const> data, size_t elementSize, image_type& target, UInt32 firstSubresource = 0, UInt32 subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(image_type& source, image_type& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(image_type& source, buffer_type& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(SharedPtr<buffer_type> source, buffer_type& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(SharedPtr<buffer_type> source, image_type& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(SharedPtr<image_type> source, image_type& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(SharedPtr<image_type> source, buffer_type& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void use(const pipeline_type& pipeline) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(const descriptor_set_type& descriptorSet) const = 0;

        /// <inheritdoc />
        virtual void bind(Span<const descriptor_set_type*> descriptorSets) const = 0;

        /// <inheritdoc />
        virtual void bind(const descriptor_set_type& descriptorSet, const pipeline_type& pipeline) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(Span<const descriptor_set_type*> descriptorSets, const pipeline_type& pipeline) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(const vertex_buffer_type& buffer) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(const index_buffer_type& buffer) const noexcept = 0;

        /// <inheritdoc />
        virtual void pushConstants(const push_constants_layout_type& layout, const void* const memory) const noexcept = 0;

        /// <inheritdoc />
        inline virtual void draw(const vertex_buffer_type& vertexBuffer, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->draw(vertexBuffer.elements(), instances, firstVertex, firstInstance);
        }

        /// <inheritdoc />
        inline virtual void drawIndexed(const index_buffer_type& indexBuffer, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const {
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }

        /// <inheritdoc />
        inline virtual void drawIndexed(const vertex_buffer_type& vertexBuffer, const index_buffer_type& indexBuffer, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }

        /// <inheritdoc />
        virtual void execute(SharedPtr<const command_buffer_type> commandBuffer) const = 0;

        /// <inheritdoc />
        virtual void execute(Enumerable<SharedPtr<const command_buffer_type>> commandBuffers) const = 0;

        /// <inheritdoc />
        virtual void buildAccelerationStructure(bottom_level_acceleration_structure_type& blas, const SharedPtr<const buffer_type> scratchBuffer, const buffer_type& buffer, UInt64 offset = 0) const = 0;

        /// <inheritdoc />
        virtual void buildAccelerationStructure(top_level_acceleration_structure_type& tlas, const SharedPtr<const buffer_type> scratchBuffer, const buffer_type& buffer, UInt64 offset = 0) const = 0;

        /// <inheritdoc />
        virtual void updateAccelerationStructure(bottom_level_acceleration_structure_type& blas, const SharedPtr<const buffer_type> scratchBuffer, const buffer_type& buffer, UInt64 offset = 0) const = 0;

        /// <inheritdoc />
        virtual void updateAccelerationStructure(top_level_acceleration_structure_type& tlas, const SharedPtr<const buffer_type> scratchBuffer, const buffer_type& buffer, UInt64 offset = 0) const = 0;

        /// <inheritdoc />
        virtual void copyAccelerationStructure(const bottom_level_acceleration_structure_type& from, const bottom_level_acceleration_structure_type& to, bool compress = false) const noexcept = 0;

        /// <inheritdoc />
        virtual void copyAccelerationStructure(const top_level_acceleration_structure_type& from, const top_level_acceleration_structure_type& to, bool compress = false) const noexcept = 0;

        /// <inheritdoc />
        virtual void traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const buffer_type& rayGenerationShaderBindingTable, const buffer_type* missShaderBindingTable, const buffer_type* hitShaderBindingTable, const buffer_type* callableShaderBindingTable) const noexcept = 0;

        /// <inheritdoc />
        inline void traceRays(const Vector3u& dimensions, const ShaderBindingTableOffsets& offsets, const buffer_type& rayGenerationShaderBindingTable, const buffer_type* missShaderBindingTable, const buffer_type* hitShaderBindingTable, const buffer_type* callableShaderBindingTable) const noexcept {
            this->traceRays(dimensions.x(), dimensions.y(), dimensions.z(), offsets, rayGenerationShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable);
        }

    private:
        inline UniquePtr<IBarrier> getBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept {
            return this->makeBarrier(syncBefore, syncAfter);
        }

        inline void cmdBarrier(const IBarrier& barrier) const noexcept override {
            this->barrier(dynamic_cast<const barrier_type&>(barrier));
        }

        inline void cmdGenerateMipMaps(IImage& image) noexcept override {
            this->generateMipMaps(dynamic_cast<image_type&>(image));
        }

        inline void cmdTransfer(IBuffer& source, IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const override {
            this->transfer(dynamic_cast<buffer_type&>(source), dynamic_cast<buffer_type&>(target), sourceElement, targetElement, elements);
        }
        
        inline void cmdTransfer(IBuffer& source, IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const override {
            this->transfer(dynamic_cast<buffer_type&>(source), dynamic_cast<image_type&>(target), sourceElement, firstSubresource, elements);
        }
        
        inline void cmdTransfer(IImage& source, IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const override {
            this->transfer(dynamic_cast<image_type&>(source), dynamic_cast<image_type&>(target), sourceSubresource, targetSubresource, subresources);
        }

        inline void cmdTransfer(IImage& source, IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const override {
            this->transfer(dynamic_cast<image_type&>(source), dynamic_cast<buffer_type&>(target), firstSubresource, targetElement, subresources);
        }

        inline void cmdTransfer(SharedPtr<IBuffer> source, IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const override {
            this->transfer(std::dynamic_pointer_cast<buffer_type>(source), dynamic_cast<buffer_type&>(target), sourceElement, targetElement, elements);
        }
        
        inline void cmdTransfer(SharedPtr<IBuffer> source, IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const override {
            this->transfer(std::dynamic_pointer_cast<buffer_type>(source), dynamic_cast<image_type&>(target), sourceElement, firstSubresource, elements);
        }
        
        inline void cmdTransfer(SharedPtr<IImage> source, IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const override {
            this->transfer(std::dynamic_pointer_cast<image_type>(source), dynamic_cast<image_type&>(target), sourceSubresource, targetSubresource, subresources);
        }
        
        inline void cmdTransfer(SharedPtr<IImage> source, IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const override {
            this->transfer(std::dynamic_pointer_cast<image_type>(source), dynamic_cast<buffer_type&>(target), firstSubresource, targetElement, subresources);
        }

        inline void cmdTransfer(const void* const data, size_t size, IBuffer& target, UInt32 targetElement, UInt32 elements) const override {
            this->transfer(data, size, dynamic_cast<buffer_type&>(target), targetElement, elements);
        }

        inline void cmdTransfer(Span<const void* const> data, size_t elementSize, IBuffer& target, UInt32 targetElement) const override {
            this->transfer(data, elementSize, dynamic_cast<buffer_type&>(target), targetElement);
        }

        inline void cmdTransfer(const void* const data, size_t size, IImage& target, UInt32 subresource) const override {
            this->transfer(data, size, dynamic_cast<image_type&>(target), subresource);
        }

        inline void cmdTransfer(Span<const void* const> data, size_t elementSize, IImage& target, UInt32 firstSubresource, UInt32 elements) const override {
            this->transfer(data, elementSize, dynamic_cast<image_type&>(target), firstSubresource, elements);
        }

        inline void cmdUse(const IPipeline& pipeline) const noexcept override {
            this->use(dynamic_cast<const pipeline_type&>(pipeline));
        }

        inline void cmdBind(const IDescriptorSet& descriptorSet) const override {
            this->bind(dynamic_cast<const descriptor_set_type&>(descriptorSet));
        }

        inline void cmdBind(Span<const IDescriptorSet*> descriptorSets) const override {
            auto sets = descriptorSets | std::views::transform([](auto set) { return dynamic_cast<const descriptor_set_type*>(set); }) | std::ranges::to<Array<const descriptor_set_type*>>();
            this->bind(Span<const descriptor_set_type*>(sets));
        }

        inline void cmdBind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const noexcept override {
            this->bind(dynamic_cast<const descriptor_set_type&>(descriptorSet), dynamic_cast<const pipeline_type&>(pipeline));
        }

        inline void cmdBind(Span<const IDescriptorSet*> descriptorSets, const IPipeline& pipeline) const noexcept override {
            auto sets = descriptorSets | std::views::transform([](auto set) { return dynamic_cast<const descriptor_set_type*>(set); }) | std::ranges::to<Array<const descriptor_set_type*>>();
            this->bind(Span<const descriptor_set_type*>(sets), dynamic_cast<const pipeline_type&>(pipeline));
        }
        
        inline void cmdBind(const IVertexBuffer& buffer) const noexcept override {
            this->bind(dynamic_cast<const vertex_buffer_type&>(buffer));
        }

        inline void cmdBind(const IIndexBuffer& buffer) const noexcept override {
            this->bind(dynamic_cast<const index_buffer_type&>(buffer));
        }
        
        inline void cmdPushConstants(const IPushConstantsLayout& layout, const void* const memory) const noexcept override {
            this->pushConstants(dynamic_cast<const push_constants_layout_type&>(layout), memory);
        }
        
        inline void cmdDraw(const IVertexBuffer& vertexBuffer, UInt32 instances, UInt32 firstVertex, UInt32 firstInstance) const override {
            this->draw(dynamic_cast<const vertex_buffer_type&>(vertexBuffer), instances, firstVertex, firstInstance);
        }
        
        inline void cmdDrawIndexed(const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const override {
            this->drawIndexed(dynamic_cast<const index_buffer_type&>(indexBuffer), instances, firstIndex, vertexOffset, firstInstance);
        }
        
        inline void cmdDrawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const override {
            this->drawIndexed(dynamic_cast<const vertex_buffer_type&>(vertexBuffer), dynamic_cast<const index_buffer_type&>(indexBuffer), instances, firstIndex, vertexOffset, firstInstance);
        }

        inline void cmdExecute(SharedPtr<const ICommandBuffer> commandBuffer) const override {
            this->execute(std::dynamic_pointer_cast<const command_buffer_type>(commandBuffer));
        }
        
        inline void cmdExecute(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers) const override {
            return this->execute(commandBuffers | std::views::transform([](auto buffer) { return std::dynamic_pointer_cast<const command_buffer_type>(buffer); }));
        }

        void cmdBuildAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset) const override {
            this->buildAccelerationStructure(dynamic_cast<bottom_level_acceleration_structure_type&>(blas), std::dynamic_pointer_cast<const buffer_type>(scratchBuffer), dynamic_cast<const buffer_type&>(buffer), offset);
        }

        void cmdBuildAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset) const override {
            this->buildAccelerationStructure(dynamic_cast<top_level_acceleration_structure_type&>(tlas), std::dynamic_pointer_cast<const buffer_type>(scratchBuffer), dynamic_cast<const buffer_type&>(buffer), offset);
        }

        void cmdUpdateAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset) const override {
            this->updateAccelerationStructure(dynamic_cast<bottom_level_acceleration_structure_type&>(blas), std::dynamic_pointer_cast<const buffer_type>(scratchBuffer), dynamic_cast<const buffer_type&>(buffer), offset);
        }
        
        void cmdUpdateAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer> scratchBuffer, const IBuffer& buffer, UInt64 offset) const override {
            this->updateAccelerationStructure(dynamic_cast<top_level_acceleration_structure_type&>(tlas), std::dynamic_pointer_cast<const buffer_type>(scratchBuffer), dynamic_cast<const buffer_type&>(buffer), offset);
        }

        void cmdCopyAccelerationStructure(const IBottomLevelAccelerationStructure& from, const IBottomLevelAccelerationStructure& to, bool compress) const noexcept override {
            this->copyAccelerationStructure(dynamic_cast<const bottom_level_acceleration_structure_type&>(from), dynamic_cast<const bottom_level_acceleration_structure_type&>(to), compress);
        }

        void cmdCopyAccelerationStructure(const ITopLevelAccelerationStructure& from, const ITopLevelAccelerationStructure& to, bool compress) const noexcept override {
            this->copyAccelerationStructure(dynamic_cast<const top_level_acceleration_structure_type&>(from), dynamic_cast<const top_level_acceleration_structure_type&>(to), compress);
        }

        void cmdTraceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const IBuffer& rayGenerationShaderBindingTable, const IBuffer* missShaderBindingTable, const IBuffer* hitShaderBindingTable, const IBuffer* callableShaderBindingTable) const noexcept override {
            this->traceRays(width, height, depth, offsets, dynamic_cast<const buffer_type&>(rayGenerationShaderBindingTable), dynamic_cast<const buffer_type*>(missShaderBindingTable), dynamic_cast<const buffer_type*>(hitShaderBindingTable), dynamic_cast<const buffer_type*>(callableShaderBindingTable));
        }
    };

    /// <summary>
    /// Represents a graphics <see cref="Pipeline" />.
    /// </summary>
    /// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="PipelineLayout"/>.</typeparam>
    /// <typeparam name="TShaderProgram">The type of the shader program. Must implement <see cref="ShaderProgram"/>.</typeparam>
    /// <typeparam name="TInputAssembler">The type of the input assembler state. Must implement <see cref="InputAssembler"/>.</typeparam>
    /// <typeparam name="TRasterizer">The type of the rasterizer state. Must implement <see cref="Rasterizer"/>.</typeparam>
    /// <seealso cref="RenderPipelineBuilder" />
    template <typename TPipelineLayout, typename TShaderProgram, typename TInputAssembler, typename TRasterizer> requires
        meta::implements<TInputAssembler, InputAssembler<typename TInputAssembler::vertex_buffer_layout_type, typename TInputAssembler::index_buffer_layout_type>> &&
        meta::implements<TRasterizer, Rasterizer>
    class RenderPipeline : public IRenderPipeline, public Pipeline<TPipelineLayout, TShaderProgram> {
    public:
        using input_assembler_type = TInputAssembler;
        using rasterizer_type = TRasterizer;

    public:
        virtual ~RenderPipeline() noexcept = default;

    public:
        /// <inheritdoc />
        virtual SharedPtr<input_assembler_type> inputAssembler() const noexcept = 0;

        /// <inheritdoc />
        virtual SharedPtr<rasterizer_type> rasterizer() const noexcept = 0;

    private:
        inline SharedPtr<IInputAssembler> getInputAssembler() const noexcept override {
            return this->inputAssembler();
        }

        inline SharedPtr<IRasterizer> getRasterizer() const noexcept override {
            return this->rasterizer();
        }
    };

    /// <summary>
    /// Represents a compute <see cref="Pipeline" />.
    /// </summary>
    /// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="PipelineLayout"/>.</typeparam>
    /// <typeparam name="TShaderProgram">The type of the shader program. Must implement <see cref="ShaderProgram"/>.</typeparam>
    /// <seealso cref="ComputePipelineBuilder" />
    template <typename TPipelineLayout, typename TShaderProgram>
    class ComputePipeline : public IComputePipeline, public Pipeline<TPipelineLayout, TShaderProgram> {
    public:
        virtual ~ComputePipeline() noexcept = default;
    };

    /// <summary>
    /// Represents a ray-tracing <see cref="Pipeline" />.
    /// </summary>
    /// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="PipelineLayout"/>.</typeparam>
    /// <typeparam name="TShaderProgram">The type of the shader program. Must implement <see cref="ShaderProgram"/>.</typeparam>
    /// <seealso cref="RayTracingPipelineBuilder" />
    template <typename TPipelineLayout, typename TShaderProgram>
    class RayTracingPipeline : public IRayTracingPipeline, public Pipeline<TPipelineLayout, TShaderProgram> {
    public:
        using base_type = Pipeline<TPipelineLayout, TShaderProgram>;
        using descriptor_set_layout_type = base_type::pipeline_layout_type::descriptor_set_layout_type;
        using descriptor_set_type = descriptor_set_layout_type::descriptor_set_type;
        using descriptor_layout_type = descriptor_set_layout_type::descriptor_layout_type;
        using buffer_type = descriptor_set_type::buffer_type;
        using image_type = descriptor_set_type::image_type;
        using sampler_type = descriptor_set_type::sampler_type;

    public:
        virtual ~RayTracingPipeline() noexcept = default;

    public:
        /// <inheritdoc />
        virtual UniquePtr<buffer_type> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups = ShaderBindingGroup::All) const noexcept = 0;

    private:
        inline UniquePtr<IBuffer> getShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups) const noexcept override {
            return this->allocateShaderBindingTable(offsets, groups);
        }
    };

    /// <summary>
    /// Stores the images for the output attachments for a back buffer of a <see cref="RenderPass" />, as well as a <see cref="CommandBuffer" /> instance, that records draw commands.
    /// </summary>
    /// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="CommandBuffer"/>.</typeparam>
    /// <seealso cref="RenderTarget" />
    template <typename TCommandBuffer> requires
        meta::implements<TCommandBuffer, CommandBuffer<typename TCommandBuffer::command_buffer_type, typename TCommandBuffer::buffer_type, typename TCommandBuffer::vertex_buffer_type, typename TCommandBuffer::index_buffer_type, typename TCommandBuffer::image_type, typename TCommandBuffer::barrier_type, typename TCommandBuffer::pipeline_type, typename TCommandBuffer::bottom_level_acceleration_structure_type, typename TCommandBuffer::top_level_acceleration_structure_type>>
    class FrameBuffer : public IFrameBuffer {
    public:
        using command_buffer_type = TCommandBuffer;
        using image_type = command_buffer_type::image_type;

    public:
        virtual ~FrameBuffer() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Enumerable<SharedPtr<const command_buffer_type>> commandBuffers() const noexcept = 0;

        /// <inheritdoc />
        virtual SharedPtr<const command_buffer_type> commandBuffer(UInt32 index) const = 0;

        /// <inheritdoc />
        virtual Enumerable<const image_type*> images() const noexcept = 0;

    private:
        inline SharedPtr<const ICommandBuffer> getCommandBuffer(UInt32 index) const noexcept override {
            return this->commandBuffer(index);
        }

        inline Enumerable<SharedPtr<const ICommandBuffer>> getCommandBuffers() const noexcept override {
            return this->commandBuffers();
        }

        inline Enumerable<const IImage*> getImages() const noexcept override {
            return this->images();
        }
    };

    /// <summary>
    /// Represents the source for an input attachment mapping.
    /// </summary>
    /// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="FrameBuffer" />.</typeparam>
    template <typename TFrameBuffer> requires
        meta::implements<TFrameBuffer, FrameBuffer<typename TFrameBuffer::command_buffer_type>>
    class RenderPassDependencySource : public IRenderPassDependencySource {
    public:
        using frame_buffer_type = TFrameBuffer;

    public:
        virtual ~RenderPassDependencySource() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const frame_buffer_type& frameBuffer(UInt32 buffer) const = 0;
    };

    /// <summary>
    /// Represents a mapping between a set of <see cref="IRenderTarget" /> instances and the input attachments of a <see cref="RenderPass" />.
    /// </summary>
    /// <typeparam name="TRenderPassDependencySource">The type of the input attachment mapping source. Must implement <see cref="RenderPassDependencySource" />.</typeparam>
    template <typename TRenderPassDependencySource> requires
        meta::implements<TRenderPassDependencySource, RenderPassDependencySource<typename TRenderPassDependencySource::frame_buffer_type>>
    class IRenderPassDependency {
    public:
        using render_pass_dependency_source_type = TRenderPassDependencySource;

    public:
        virtual ~IRenderPassDependency() noexcept = default;

    public:
        /// <summary>
        /// Returns the source of the input attachment render target.
        /// </summary>
        /// <returns>The source of the input attachment render target.</returns>
        virtual const render_pass_dependency_source_type* inputAttachmentSource() const noexcept = 0;

        /// <summary>
        /// Returns a reference of the render target that is mapped to the input attachment.
        /// </summary>
        /// <returns>A reference of the render target that is mapped to the input attachment.</returns>
        virtual const RenderTarget& renderTarget() const noexcept = 0;

        /// <summary>
        /// Returns the binding point for the input attachment binding.
        /// </summary>
        /// <returns>The binding point for the input attachment binding.</returns>
        virtual const DescriptorBindingPoint& binding() const noexcept = 0;
    };

    /// <summary>
    /// Represents a render pass.
    /// </summary>
    /// <remarks>
    /// A render pass is a conceptual layer, that may not have any logical representation within the actual implementation. It is a high-level view on a specific workload on the
    /// GPU, that processes data using different <see cref="RenderPipeline" />s and stores the outputs in the <see cref="IRenderTarget" />s of a <see cref="FrameBuffer" />.
    /// </remarks>
    /// <typeparam name="TRenderPipeline">The type of the render pipeline. Must implement <see cref="RenderPipeline" />.</typeparam>
    /// <typeparam name="TCommandQueue">The type of the command queue. Must implement <see cref="CommandQueue" />.</typeparam>
    /// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="FrameBuffer" />.</typeparam>
    /// <typeparam name="TRenderPassDependency">The type of the input attachment mapping. Must implement <see cref="IRenderPassDependency" />.</typeparam>
    template <typename TRenderPipeline, typename TCommandQueue, typename TFrameBuffer, typename TRenderPassDependency> requires
        meta::implements<TFrameBuffer, FrameBuffer<typename TFrameBuffer::command_buffer_type>> &&
        /*meta::implements<TCommandQueue, CommandQueue<typename TFrameBuffer::command_buffer_type>> &&*/
        meta::implements<TRenderPipeline, RenderPipeline<typename TRenderPipeline::pipeline_layout_type, typename TRenderPipeline::shader_program_type, typename TRenderPipeline::input_assembler_type, typename TRenderPipeline::rasterizer_type>> /*&&
        meta::implements<TRenderPassDependency, IRenderPassDependency<TDerived>>*/
    class RenderPass : public virtual StateResource, public IRenderPass, public RenderPassDependencySource<TFrameBuffer> {
    public:
        using command_queue_type = TCommandQueue;
        using frame_buffer_type = TFrameBuffer;
        using render_pipeline_type = TRenderPipeline;
        using render_pass_dependency_type = TRenderPassDependency;
        using pipeline_layout_type = render_pipeline_type::pipeline_layout_type;
        using descriptor_set_layout_type = pipeline_layout_type::descriptor_set_layout_type;
        using descriptor_set_type = descriptor_set_layout_type::descriptor_set_type;

    public:
        virtual ~RenderPass() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const frame_buffer_type& activeFrameBuffer() const = 0;

        /// <inheritdoc />
        virtual const command_queue_type& commandQueue() const noexcept = 0;

        /// <inheritdoc />
        virtual Enumerable<const frame_buffer_type*> frameBuffers() const noexcept = 0;

        /// <inheritdoc />
        virtual Enumerable<const render_pipeline_type*> pipelines() const noexcept = 0;

        /// <inheritdoc />
        virtual Span<const render_pass_dependency_type> inputAttachments() const noexcept = 0;

    private:
        inline Enumerable<const IFrameBuffer*> getFrameBuffers() const noexcept override {
            return this->frameBuffers();
        }

        inline Enumerable<const IRenderPipeline*> getPipelines() const noexcept override {
            return this->pipelines();
        }

        inline const ICommandQueue& getCommandQueue() const noexcept override {
            return this->commandQueue();
        }
    };

    /// <summary>
    /// Represents a swap chain, i.e. a chain of multiple <see cref="IImage" /> instances, that can be presented to a <see cref="ISurface" />.
    /// </summary>
    /// <typeparam name="TImageInterface">The type of the image interface. Must inherit from <see cref="IImage"/>.</typeparam>
    template <typename TImageInterface, typename TFrameBuffer> requires
        meta::implements<TFrameBuffer, FrameBuffer<typename TFrameBuffer::command_buffer_type>> &&
        std::derived_from<TImageInterface, IImage>
    class SwapChain : public ISwapChain {
    public:
        using image_interface_type = TImageInterface;
        using frame_buffer_type = TFrameBuffer;

    public:
        virtual ~SwapChain() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Enumerable<image_interface_type*> images() const noexcept = 0;

        /// <summary>
        /// Queues a present that gets executed after <paramref name="frameBuffer" /> signals its readiness.
        /// </summary>
        /// <param name="frameBuffer">The frame buffer for which the present should wait.</param>
        virtual void present(const frame_buffer_type& frameBuffer) const = 0;

        /// <inheritdoc />
        inline void present(const IFrameBuffer& frameBuffer) const override {
            this->present(dynamic_cast<const frame_buffer_type&>(frameBuffer));
        }

    private:
        inline Enumerable<IImage*> getImages() const noexcept override {
            return this->images();
        }
    };

    /// <summary>
    /// Represents a command queue.
    /// </summary>
    /// <typeparam name="TCommandBuffer">The type of the command buffer for this queue. Must implement <see cref="CommandBuffer"/>.</typeparam>
    template <typename TCommandBuffer> requires
        meta::implements<TCommandBuffer, CommandBuffer<typename TCommandBuffer::command_buffer_type, typename TCommandBuffer::buffer_type, typename TCommandBuffer::vertex_buffer_type, typename TCommandBuffer::index_buffer_type, typename TCommandBuffer::image_type, typename TCommandBuffer::barrier_type, typename TCommandBuffer::pipeline_type, typename TCommandBuffer::bottom_level_acceleration_structure_type, typename TCommandBuffer::top_level_acceleration_structure_type>>
    class CommandQueue : public ICommandQueue {
    public:
        using ICommandQueue::submit;

        using command_buffer_type = TCommandBuffer;

    public:
        virtual ~CommandQueue() noexcept = default;

    public:
        /// <inheritdoc />
        virtual SharedPtr<command_buffer_type> createCommandBuffer(bool beginRecording = false, bool secondary = false) const = 0;

        /// <inheritdoc />
        inline virtual UInt64 submit(SharedPtr<command_buffer_type> commandBuffer) const {
            return this->submit(std::static_pointer_cast<const command_buffer_type>(commandBuffer));
        }

        /// <inheritdoc />
        virtual UInt64 submit(SharedPtr<const command_buffer_type> commandBuffer) const = 0;

        /// <inheritdoc />
        inline virtual UInt64 submit(const Enumerable<SharedPtr<command_buffer_type>>& commandBuffers) const {
            return this->submit(commandBuffers | std::ranges::to<Enumerable<SharedPtr<const command_buffer_type>>>());
        }

        /// <inheritdoc />
        virtual UInt64 submit(const Enumerable<SharedPtr<const command_buffer_type>>& commandBuffers) const = 0;

    private:
        inline SharedPtr<ICommandBuffer> getCommandBuffer(bool beginRecording, bool secondary) const override {
            return this->createCommandBuffer(beginRecording, secondary);
        }

        inline UInt64 submitCommandBuffer(SharedPtr<const ICommandBuffer> commandBuffer) const override {
            return this->submit(std::dynamic_pointer_cast<const command_buffer_type>(commandBuffer));
        }

        inline UInt64 submitCommandBuffers(const Enumerable<SharedPtr<const ICommandBuffer>>& commandBuffers) const override {
            return this->submit(commandBuffers | std::views::transform([](auto buffer) { return std::dynamic_pointer_cast<const command_buffer_type>(buffer); }) | std::ranges::to<Enumerable<SharedPtr<const command_buffer_type>>>());
        }
    };

    /// <summary>
    /// Describes a factory that creates objects for a <see cref="GraphicsDevice" />.
    /// </summary>
    /// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout" />.</typeparam>
    /// <typeparam name="TVertexBuffer">The type of the vertex buffer. Must implement <see cref="VertexBuffer" />.</typeparam>
    /// <typeparam name="TIndexBuffer">The type of the index buffer. Must implement <see cref="IndexBuffer" />.</typeparam>
    /// <typeparam name="TImage">The type of the image. Must inherit from <see cref="IImage"/>.</typeparam>
    /// <typeparam name="TBuffer">The type of the buffer. Must inherit from <see cref="IBuffer"/>.</typeparam>
    /// <typeparam name="TSampler">The type of the sampler. Must inherit from <see cref="ISampler"/>.</typeparam>
    /// <typeparam name="TBLAS">The type of the bottom-level acceleration structure. Must implement <see cref="IBottomLevelAccelerationStructure" />.</typeparam>
    /// <typeparam name="TTLAS">The type of the top-level acceleration structure. Must implement <see cref="ITopLevelAccelerationStructure" />.</typeparam>
    template <typename TDescriptorLayout, typename TBuffer, typename TVertexBuffer, typename TIndexBuffer, typename TImage, typename TSampler, typename TBLAS, typename TTLAS> requires
        meta::implements<TDescriptorLayout, IDescriptorLayout> &&
        std::derived_from<TVertexBuffer, VertexBuffer<typename TVertexBuffer::vertex_buffer_layout_type>> &&
        std::derived_from<TIndexBuffer, IndexBuffer<typename TIndexBuffer::index_buffer_layout_type>> &&
        std::derived_from<TImage, IImage> &&
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TSampler, ISampler> &&
        std::derived_from<TBLAS, IBottomLevelAccelerationStructure> &&
        std::derived_from<TTLAS, ITopLevelAccelerationStructure>
    class GraphicsFactory : public IGraphicsFactory {
    public:
        using IGraphicsFactory::createBuffer;
        using IGraphicsFactory::createVertexBuffer;
        using IGraphicsFactory::createIndexBuffer;
        using IGraphicsFactory::createAttachment;
        using IGraphicsFactory::createTexture;
        using IGraphicsFactory::createTextures;
        using IGraphicsFactory::createSampler;
        using IGraphicsFactory::createSamplers;

        using descriptor_layout_type = TDescriptorLayout;
        using vertex_buffer_type = TVertexBuffer;
        using vertex_buffer_layout_type = vertex_buffer_type::vertex_buffer_layout_type;
        using index_buffer_type = TIndexBuffer;
        using index_buffer_layout_type = index_buffer_type::index_buffer_layout_type;
        using buffer_type = TBuffer;
        using image_type = TImage;
        using sampler_type = TSampler;
        using bottom_level_acceleration_structure_type = TBLAS;
        using top_level_acceleration_structure_type = TTLAS;

    public:
        virtual ~GraphicsFactory() noexcept = default;

    public:
        /// <inheritdoc />
        virtual UniquePtr<TBuffer> createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const = 0;
        
        /// <inheritdoc />
        virtual UniquePtr<TBuffer> createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TVertexBuffer> createVertexBuffer(const vertex_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TVertexBuffer> createVertexBuffer(const String& name, const vertex_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TIndexBuffer> createIndexBuffer(const index_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TIndexBuffer> createIndexBuffer(const String& name, const index_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createAttachment(const RenderTarget& target, const Size2d& size, MultiSamplingLevel samples = MultiSamplingLevel::x1) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createAttachment(const String& name, const RenderTarget& target, const Size2d& size, MultiSamplingLevel samples = MultiSamplingLevel::x1) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createTexture(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const = 0;

        /// <inheritdoc />
        virtual Enumerable<UniquePtr<TImage>> createTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 layers = 1, UInt32 levels = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TSampler> createSampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TSampler> createSampler(const String& name, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const = 0;

        /// <inheritdoc />
        virtual Enumerable<UniquePtr<TSampler>> createSamplers(UInt32 elements, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const = 0;

        /// <inheritdoc />
        inline UniquePtr<TBLAS> createBottomLevelAccelerationStructure(AccelerationStructureFlags flags) const {
            return this->createBottomLevelAccelerationStructure("", flags);
        }

        /// <inheritdoc />
        virtual UniquePtr<TBLAS> createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const = 0;

        /// <inheritdoc />
        inline UniquePtr<TTLAS> createTopLevelAccelerationStructure(AccelerationStructureFlags flags) const {
            return this->createTopLevelAccelerationStructure("", flags);
        }

        /// <inheritdoc />
        virtual UniquePtr<TTLAS> createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const = 0;

    private:
        inline UniquePtr<IBuffer> getBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage) const override {
            return this->createBuffer(type, heap, elementSize, elements, usage);
        }

        inline UniquePtr<IBuffer> getBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage) const override {
            return this->createBuffer(name, type, heap, elementSize, elements, usage);
        }

        inline UniquePtr<IVertexBuffer> getVertexBuffer(const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const override {
            return this->createVertexBuffer(dynamic_cast<const vertex_buffer_layout_type&>(layout), heap, elements, usage);
        }

        inline UniquePtr<IVertexBuffer> getVertexBuffer(const String& name, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const override {
            return this->createVertexBuffer(name, dynamic_cast<const vertex_buffer_layout_type&>(layout), heap, elements, usage);
        }
        
        inline UniquePtr<IIndexBuffer> getIndexBuffer(const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const override {
            return this->createIndexBuffer(dynamic_cast<const index_buffer_layout_type&>(layout), heap, elements, usage);
        }

        inline UniquePtr<IIndexBuffer> getIndexBuffer(const String& name, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage) const override {
            return this->createIndexBuffer(name, dynamic_cast<const index_buffer_layout_type&>(layout), heap, elements, usage);
        }

        inline UniquePtr<IImage> getAttachment(const RenderTarget& target, const Size2d& size, MultiSamplingLevel samples) const override {
            return this->createAttachment(target, size, samples);
        }

        inline UniquePtr<IImage> getAttachment(const String& name, const RenderTarget& target, const Size2d& size, MultiSamplingLevel samples) const override {
            return this->createAttachment(name, target, size, samples);
        }
        
        inline UniquePtr<IImage> getTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const override {
            return this->createTexture(format, size, dimension, levels, layers, samples, usage);
        }

        inline UniquePtr<IImage> getTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage) const override {
            return this->createTexture(name, format, size, dimension, levels, layers, samples, usage);
        }

        inline Enumerable<UniquePtr<IImage>> getTextures(UInt32 elements, Format format, const Size3d& size, ImageDimensions dimension, UInt32 layers, UInt32 levels, MultiSamplingLevel samples, ResourceUsage usage) const override {
            return this->createTextures(elements, format, size, dimension, layers, levels, samples, usage) | std::views::as_rvalue;
        }
        
        inline UniquePtr<ISampler> getSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const override {
            return this->createSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

        inline UniquePtr<ISampler> getSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const override {
            return this->createSampler(name, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }
        
        inline Enumerable<UniquePtr<ISampler>> getSamplers(UInt32 elements, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const override {
            return this->createSamplers(elements, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy) | std::views::as_rvalue;
        }

        inline UniquePtr<IBottomLevelAccelerationStructure> getBlas(StringView name, AccelerationStructureFlags flags) const override {
            return this->createBottomLevelAccelerationStructure(name, flags);
        }

        inline UniquePtr<ITopLevelAccelerationStructure> getTlas(StringView name, AccelerationStructureFlags flags) const override {
            return this->createTopLevelAccelerationStructure(name, flags);
        }
    };

    /// <summary>
    /// Represents the graphics device that a rendering back-end is doing work on.
    /// </summary>
    /// <remarks>
    /// The graphics device is the central instance of a renderer. It has two major roles. First, it maintains the <see cref="GraphicsFactory" /> instance, that is used to facilitate
    /// common objects. Second, it owns the device state, which contains objects required for communication between your application and the graphics driver. Most notably, those objects
    /// contain the <see cref="SwapChain" /> instance and the <see cref="CommandQueue" /> instances used for data and command transfer.
    /// </remarks>
    /// <typeparam name="TFactory">The type of the graphics factory. Must implement <see cref="GraphicsFactory" />.</typeparam>
    /// <typeparam name="TSurface">The type of the surface. Must implement <see cref="ISurface" />.</typeparam>
    /// <typeparam name="TGraphicsAdapter">The type of the graphics adapter. Must implement <see cref="IGraphicsAdapter" />.</typeparam>
    /// <typeparam name="TSwapChain">The type of the swap chain. Must implement <see cref="SwapChain" />.</typeparam>
    /// <typeparam name="TCommandQueue">The type of the command queue. Must implement <see cref="CommandQueue" />.</typeparam>
    /// <typeparam name="TRenderPass">The type of the render pass. Must implement <see cref="RenderPass" />.</typeparam>
    /// <typeparam name="TComputePipeline">The type of the compute pipeline. Must implement <see cref="ComputePipeline" />.</typeparam>
    /// <typeparam name="TRayTracingPipeline">The type of the ray-tracing pipeline. Must implement <see cref="RayTracingPipeline" />.</typeparam>
    /// <typeparam name="TBarrier">The type of the memory barrier. Must implement <see cref="Barrier" />.</typeparam>
    template <typename TFactory, typename TSurface, typename TGraphicsAdapter, typename TSwapChain, typename TCommandQueue, typename TRenderPass, typename TComputePipeline, typename TRayTracingPipeline, typename TBarrier> requires
        meta::implements<TSurface, ISurface> &&
        meta::implements<TGraphicsAdapter, IGraphicsAdapter> &&
        meta::implements<TSwapChain, SwapChain<typename TFactory::image_type, typename TRenderPass::frame_buffer_type>> &&
        meta::implements<TCommandQueue, CommandQueue<typename TCommandQueue::command_buffer_type>> &&
        meta::implements<TFactory, GraphicsFactory<typename TFactory::descriptor_layout_type, typename TFactory::buffer_type, typename TFactory::vertex_buffer_type, typename TFactory::index_buffer_type, typename TFactory::image_type, typename TFactory::sampler_type, typename TFactory::bottom_level_acceleration_structure_type, typename TFactory::top_level_acceleration_structure_type>> &&
        meta::implements<TRenderPass, RenderPass<typename TRenderPass::render_pipeline_type, TCommandQueue, typename TRenderPass::frame_buffer_type, typename TRenderPass::render_pass_dependency_type>> &&
        meta::implements<TComputePipeline, ComputePipeline<typename TComputePipeline::pipeline_layout_type, typename TComputePipeline::shader_program_type>> &&
        meta::implements<TRayTracingPipeline, RayTracingPipeline<typename TRayTracingPipeline::pipeline_layout_type, typename TRayTracingPipeline::shader_program_type>> &&
        meta::implements<TBarrier, Barrier<typename TFactory::buffer_type, typename TFactory::image_type>>
    class GraphicsDevice : public IGraphicsDevice {
    public:
        using surface_type = TSurface;
        using adapter_type = TGraphicsAdapter;
        using swap_chain_type = TSwapChain;
        using command_queue_type = TCommandQueue;
        using command_buffer_type = command_queue_type::command_buffer_type;
        using factory_type = TFactory;
        using barrier_type = TBarrier;
        using descriptor_layout_type = factory_type::descriptor_layout_type;
        using vertex_buffer_type = factory_type::vertex_buffer_type;
        using index_buffer_type = factory_type::index_buffer_type;
        using buffer_type = factory_type::buffer_type;
        using image_type = factory_type::image_type;
        using sampler_type = factory_type::sampler_type;
        using bottom_level_acceleration_structure_type = factory_type::bottom_level_acceleration_structure_type;
        using top_level_acceleration_structure_type = factory_type::top_level_acceleration_structure_type;
        using render_pass_type = TRenderPass;
        using frame_buffer_type = render_pass_type::frame_buffer_type;
        using render_pipeline_type = render_pass_type::render_pipeline_type;
        using compute_pipeline_type = TComputePipeline;
        using ray_tracing_pipeline_type = TRayTracingPipeline;
        using pipeline_layout_type = render_pipeline_type::pipeline_layout_type;
        using shader_program_type = render_pipeline_type::shader_program_type;
        using input_assembler_type = render_pipeline_type::input_assembler_type;
        using rasterizer_type = render_pipeline_type::rasterizer_type;
        using shader_program_type = render_pipeline_type::shader_program_type;

    public:
        virtual ~GraphicsDevice() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const surface_type& surface() const noexcept = 0;

        /// <inheritdoc />
        virtual const adapter_type& adapter() const noexcept = 0;

        /// <inheritdoc />
        virtual const swap_chain_type& swapChain() const noexcept = 0;

        /// <inheritdoc />
        virtual swap_chain_type& swapChain() noexcept = 0;

        /// <inheritdoc />
        virtual const factory_type& factory() const noexcept = 0;

        /// <inheritdoc />
        virtual const command_queue_type& defaultQueue(QueueType type) const = 0;

        /// <inheritdoc />
        virtual const command_queue_type* createQueue(QueueType type, QueuePriority priority = QueuePriority::Normal) noexcept = 0;

        /// <inheritdoc />
        virtual [[nodiscard]] UniquePtr<barrier_type> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept = 0;

        /// <inheritdoc />
        virtual [[nodiscard]] UniquePtr<frame_buffer_type> makeFrameBuffer(const Size2d& renderArea) const noexcept = 0;

    private:
        inline UniquePtr<IBarrier> getNewBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const noexcept override {
            return this->makeBarrier(syncBefore, syncAfter);
        }

        inline UniquePtr<IFrameBuffer> getNewFrameBuffer(const Size2d& renderArea) const noexcept override {
            return this->makeFrameBuffer(renderArea);
        }

        inline const ICommandQueue& getDefaultQueue(QueueType type) const {
            return this->defaultQueue(type);
        }

        inline const ICommandQueue* getNewQueue(QueueType type, QueuePriority priority) noexcept {
            return this->createQueue(type, priority);
        }

    public:
        /// <inheritdoc />
        virtual void computeAccelerationStructureSizes(const bottom_level_acceleration_structure_type& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const = 0;

        /// <inheritdoc />
        virtual void computeAccelerationStructureSizes(const top_level_acceleration_structure_type& tlas, UInt64 & bufferSize, UInt64 & scratchSize, bool forUpdate = false) const = 0;

    private:
        inline void getAccelerationStructureSizes(const IBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const {
            this->computeAccelerationStructureSizes(dynamic_cast<const bottom_level_acceleration_structure_type&>(blas), bufferSize, scratchSize, forUpdate);
        }

        inline void getAccelerationStructureSizes(const ITopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const {
            this->computeAccelerationStructureSizes(dynamic_cast<const top_level_acceleration_structure_type&>(tlas), bufferSize, scratchSize, forUpdate);
        }

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
    public:
        using render_pass_builder_type = render_pass_type::builder_type;
        using render_pipeline_builder_type = render_pipeline_type::builder_type;
        using compute_pipeline_builder_type = compute_pipeline_type::builder_type;
        using ray_tracing_pipeline_builder_type = ray_tracing_pipeline_type::builder_type;
        using pipeline_layout_builder_type = pipeline_layout_type::builder_type;
        using input_assembler_builder_type = input_assembler_type::builder_type;
        using rasterizer_builder_type = rasterizer_type::builder_type;
        using shader_program_builder_type = shader_program_type::builder_type;
        using barrier_builder_Type = barrier_type::builder_type;

        /// <summary>
        /// Returns a builder for a <see cref="RenderPass" />.
        /// </summary>
        /// <param name="samples">The number of samples, the render targets of the render pass should be sampled with.</param>
        /// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
        /// <returns>An instance of a builder that is used to create a new render pass.</returns>
        [[nodiscard]] virtual render_pass_builder_type buildRenderPass(MultiSamplingLevel samples = MultiSamplingLevel::x1, UInt32 commandBuffers = 1) const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="RenderPass" />.
        /// </summary>
        /// <param name="name">The name of the render pass.</param>
        /// <param name="samples">The number of samples, the render targets of the render pass should be sampled with.</param>
        /// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
        /// <returns>An instance of a builder that is used to create a new render pass.</returns>
        [[nodiscard]] virtual render_pass_builder_type buildRenderPass(const String& name, MultiSamplingLevel samples = MultiSamplingLevel::x1, UInt32 commandBuffers = 1) const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="ComputePipeline" />.
        /// </summary>
        /// <param name="name">The name of the compute pipeline.</param>
        /// <returns>An instance of a builder that is used to create a new compute pipeline.</returns>
        [[nodiscard]] virtual compute_pipeline_builder_type buildComputePipeline(const String& name) const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="RenderPipeline" />.
        /// </summary>
        /// <param name="name">The name of the render pipeline.</param>
        /// <returns>An instance of a builder that is used to create a new render pipeline.</returns>
        //[[nodiscard]] virtual render_pipeline_builder_type buildRenderPipeline(const String& name) const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="RenderPipeline" />.
        /// </summary>
        /// <param name="renderPass">The parent render pass of the pipeline.</param>
        /// <param name="name">The name of the render pipeline.</param>
        /// <returns>An instance of a builder that is used to create a new render pipeline.</returns>
        [[nodiscard]] virtual render_pipeline_builder_type buildRenderPipeline(const render_pass_type& renderPass, const String& name) const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="RayTracingPipeline" />.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="shaderRecords">The shader record collection that is used to build the shader binding table for the pipeline.</param>
        /// <returns>An instance of a builder that is used to create a new ray-tracing pipeline.</returns>
        [[nodiscard]] virtual ray_tracing_pipeline_builder_type buildRayTracingPipeline(ShaderRecordCollection&& shaderRecords) const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="RayTracingPipeline" />.
        /// </summary>
        /// <remarks>
        /// This method is only supported if the <see cref="GraphicsDeviceFeature::RayTracing" /> feature is enabled.
        /// </remarks>
        /// <param name="name">The name of the ray-tracing pipeline.</param>
        /// <param name="shaderRecords">The shader record collection that is used to build the shader binding table for the pipeline.</param>
        /// <returns>An instance of a builder that is used to create a new ray-tracing pipeline.</returns>
        [[nodiscard]] virtual ray_tracing_pipeline_builder_type buildRayTracingPipeline(const String& name, ShaderRecordCollection&& shaderRecords) const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="PipelineLayout" />.
        /// </summary>
        /// <returns>An instance of a builder that is used to create a new pipeline layout.</returns>
        [[nodiscard]] virtual pipeline_layout_builder_type buildPipelineLayout() const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="InputAssembler" />.
        /// </summary>
        /// <returns>An instance of a builder that is used to create a new input assembler.</returns>
        [[nodiscard]] virtual input_assembler_builder_type buildInputAssembler() const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="Rasterizer" />.
        /// </summary>
        /// <returns>An instance of a builder that is used to create a new rasterizer.</returns>
        [[nodiscard]] virtual rasterizer_builder_type buildRasterizer() const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="ShaderProgram" />.
        /// </summary>
        /// <returns>An instance of a builder that is used to create a new shader program.</returns>
        [[nodiscard]] virtual shader_program_builder_type buildShaderProgram() const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="Barrier" />.
        /// </summary>
        /// <returns>An instance of a builder that is used to create a new barrier.</returns>
        [[nodiscard]] virtual barrier_builder_Type buildBarrier() const = 0;
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)
    };

    /// <summary>
    /// Defines a back-end, that provides a device instance for a certain surface and graphics adapter.
    /// </summary>
    /// <typeparam name="TGraphicsDevice">The type of the graphics device. Must implement <see cref="GraphicsDevice" />.</typeparam>
    template <typename TGraphicsDevice> requires
        meta::implements<TGraphicsDevice, GraphicsDevice<typename TGraphicsDevice::factory_type, typename TGraphicsDevice::surface_type, typename TGraphicsDevice::adapter_type, typename TGraphicsDevice::swap_chain_type, typename TGraphicsDevice::command_queue_type, typename TGraphicsDevice::render_pass_type, typename TGraphicsDevice::compute_pipeline_type, typename TGraphicsDevice::ray_tracing_pipeline_type, typename TGraphicsDevice::barrier_type>>
    class RenderBackend : public IRenderBackend {
    public:
        using device_type = TGraphicsDevice;
        using surface_type = device_type::surface_type;
        using adapter_type = device_type::adapter_type;
        using swap_chain_type = device_type::swap_chain_type;
        using command_queue_type = device_type::command_queue_type;
        using command_buffer_type = device_type::command_buffer_type;
        using factory_type = device_type::factory_type;
        using barrier_type = device_type::barrier_type;
        using descriptor_layout_type = factory_type::descriptor_layout_type;
        using vertex_buffer_type = factory_type::vertex_buffer_type;
        using index_buffer_type = factory_type::index_buffer_type;
        using buffer_type = factory_type::buffer_type;
        using image_type = factory_type::image_type;
        using sampler_type = factory_type::sampler_type;
        using frame_buffer_type = device_type::frame_buffer_type;
        using render_pass_type = device_type::render_pass_type;
        using pipeline_layout_type = device_type::pipeline_layout_type;
        using render_pipeline_type = device_type::render_pipeline_type;
        using compute_pipeline_type = device_type::compute_pipeline_type;
        using ray_tracing_pipeline_type = device_type::ray_tracing_pipeline_type;
        using shader_program_type = device_type::shader_program_type;
        using input_assembler_type = device_type::input_assembler_type;
        using rasterizer_type = device_type::rasterizer_type;

    public:
        virtual ~RenderBackend() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Enumerable<const adapter_type*> listAdapters() const = 0;

        /// <inheritdoc />
        virtual const adapter_type* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const = 0;

        /// <inheritdoc />
        virtual void registerDevice(String name, UniquePtr<device_type>&& device) = 0;

        /// <summary>
        /// Creates a new graphics device.
        /// </summary>
        /// <param name="_args">The arguments that are passed to the graphics device constructor.</param>
        /// <returns>A pointer of the created graphics device instance.</returns>
        template <typename TSelf, typename ...TArgs>
        inline device_type* createDevice(this TSelf&& self, String name, const adapter_type& adapter, UniquePtr<surface_type>&& surface, TArgs&&... _args) {
            auto device = makeUnique<device_type>(self, adapter, std::move(surface), std::forward<TArgs>(_args)...);
            auto devicePointer = device.get();
            self.registerDevice(name, std::move(device));
            return devicePointer;
        }

        /// <summary>
        /// Destroys and removes a device from the backend.
        /// </summary>
        /// <param name="name">The name of the device.</param>
        virtual void releaseDevice(const String& name) = 0;

        /// <inheritdoc />
        virtual device_type* device(const String& name) noexcept = 0;

        /// <inheritdoc />
        virtual const device_type* device(const String& name) const noexcept = 0;

        /// <inheritdoc />
        inline virtual const device_type* operator[](const String& name) const noexcept {
            return this->device(name);
        };

        /// <inheritdoc />
        inline virtual device_type* operator[](const String& name) noexcept {
            return this->device(name);
        };

        // IRenderBackend interface
    private:
        inline Enumerable<const IGraphicsAdapter*> getAdapters() const override {
            return this->listAdapters();
        }
    };
}