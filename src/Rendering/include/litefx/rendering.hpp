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
        virtual ~Barrier() noexcept = default;

    public:
        /// <inheritdoc />
        virtual void transition(buffer_type& buffer, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter) = 0;

        /// <inheritdoc />
        virtual void transition(buffer_type& buffer, const UInt32& element, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter) = 0;

        /// <inheritdoc />
        virtual void transition(image_type& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout) = 0;

        /// <inheritdoc />
        virtual void transition(image_type& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout) = 0;

        /// <inheritdoc />
        virtual void transition(image_type& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout) = 0;

        /// <inheritdoc />
        virtual void transition(image_type& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout) = 0;

    private:
        virtual void doTransition(IBuffer& buffer, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter) override {
            this->transition(dynamic_cast<buffer_type&>(buffer), accessBefore, accessAfter);
        }

        virtual void doTransition(IBuffer& buffer, const UInt32& element, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter) override {
            this->transition(dynamic_cast<buffer_type&>(buffer), element, accessBefore, accessAfter);
        }

        virtual void doTransition(IImage& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout) override {
            this->transition(dynamic_cast<image_type&>(image), accessBefore, accessAfter, layout);
        }

        virtual void doTransition(IImage& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout) override {
            this->transition(dynamic_cast<image_type&>(image), accessBefore, accessAfter, fromLayout, toLayout);
        }

        virtual void doTransition(IImage& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout) override {
            this->transition(dynamic_cast<image_type&>(image), level, levels, layer, layers, plane, accessBefore, accessAfter, layout);
        }

        virtual void doTransition(IImage& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout) override {
            this->transition(dynamic_cast<image_type&>(image), level, levels, layer, layers, plane, accessBefore, accessAfter, fromLayout, toLayout);
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
    /// require a transfer into GPU visible memory, depending on the <see cref="BufferUsage" />. However, as long as a descriptor within a set is mapped to a buffer, modifying 
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
    ///   </item>
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
    /// <seealso cref="DescriptorSetLayout" />
    template <typename TBuffer, typename TImage, typename TSampler> requires
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TSampler, ISampler> &&
        std::derived_from<TImage, IImage>
    class DescriptorSet : public IDescriptorSet {
    public:
        using IDescriptorSet::attach;
        using IDescriptorSet::update;

        using buffer_type = TBuffer;
        using sampler_type = TSampler;
        using image_type = TImage;

    public:
        virtual ~DescriptorSet() noexcept = default;

    public:
        /// <inheritdoc />
        virtual void update(const UInt32& binding, const buffer_type& buffer, const UInt32& bufferElement = 0, const UInt32& elements = 0, const UInt32& firstDescriptor = 0) const = 0;

        /// <inheritdoc />
        virtual void update(const UInt32& binding, const image_type& texture, const UInt32& descriptor = 0, const UInt32& firstLevel = 0, const UInt32& levels = 0, const UInt32& firstLayer = 0, const UInt32& layers = 0) const = 0;

        /// <inheritdoc />
        virtual void update(const UInt32& binding, const sampler_type& sampler, const UInt32& descriptor = 0) const = 0;

        /// <inheritdoc />
        virtual void attach(const UInt32& binding, const image_type& image) const = 0;

    private:
        virtual void doUpdate(const UInt32& binding, const IBuffer& buffer, const UInt32& bufferElement, const UInt32& elements, const UInt32& firstDescriptor) const override {
            this->update(binding, dynamic_cast<const buffer_type&>(buffer), bufferElement, elements, firstDescriptor);
        }

        virtual void doUpdate(const UInt32& binding, const IImage& texture, const UInt32& descriptor, const UInt32& firstLevel, const UInt32& levels, const UInt32& firstLayer, const UInt32& layers) const  override {
            this->update(binding, dynamic_cast<const image_type&>(texture), descriptor, firstLevel, levels, firstLayer, layers);
        }

        virtual void doUpdate(const UInt32& binding, const ISampler& sampler, const UInt32& descriptor) const  override {
            this->update(binding, dynamic_cast<const sampler_type&>(sampler), descriptor);
        }

        virtual void doAttach(const UInt32& binding, const IImage& image) const  override {
            this->attach(binding, dynamic_cast<const image_type&>(image));
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
        rtti::implements<TDescriptorLayout, IDescriptorLayout> &&
        rtti::implements<TDescriptorSet, DescriptorSet<typename TDescriptorSet::buffer_type, typename TDescriptorSet::image_type, typename TDescriptorSet::sampler_type>>
    class DescriptorSetLayout : public IDescriptorSetLayout {
    public:
        using IDescriptorSetLayout::free;

        using descriptor_layout_type = TDescriptorLayout;
        using descriptor_set_type = TDescriptorSet;

    public:
        virtual ~DescriptorSetLayout() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const descriptor_layout_type*> descriptors() const noexcept = 0;

        /// <inheritdoc />
        virtual const descriptor_layout_type& descriptor(const UInt32& binding) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<descriptor_set_type> allocate(const Array<DescriptorBinding>& bindings = { }) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<descriptor_set_type> allocate(const UInt32& descriptors, const Array<DescriptorBinding>& bindings = { }) const = 0;

        /// <inheritdoc />
        virtual Array<UniquePtr<descriptor_set_type>> allocateMultiple(const UInt32& descriptorSets, const Array<Array<DescriptorBinding>>& bindings = { }) const = 0;

        /// <inheritdoc />
        virtual Array<UniquePtr<descriptor_set_type>> allocateMultiple(const UInt32& descriptorSets, std::function<Array<DescriptorBinding>(const UInt32&)> bindingFactory) const = 0;

        /// <inheritdoc />
        virtual Array<UniquePtr<descriptor_set_type>> allocateMultiple(const UInt32& descriptorSets, const UInt32& descriptors, const Array<Array<DescriptorBinding>>& bindings = { }) const = 0;

        /// <inheritdoc />
        virtual Array<UniquePtr<descriptor_set_type>> allocateMultiple(const UInt32& descriptorSets, const UInt32& descriptors, std::function<Array<DescriptorBinding>(const UInt32&)> bindingFactory) const = 0;

        /// <inheritdoc />
        virtual void free(const descriptor_set_type& descriptorSet) const noexcept = 0;

    private:
        virtual Array<const IDescriptorLayout*> getDescriptors() const noexcept override {
            auto descriptors = this->descriptors();
            return Array<const IDescriptorLayout*>(descriptors.begin(), descriptors.end());
        }

        virtual UniquePtr<IDescriptorSet> getDescriptorSet(const UInt32& descriptors, const Array<DescriptorBinding>& bindings = { }) const override {
            return this->allocate(descriptors, bindings);
        }

        virtual Array<UniquePtr<IDescriptorSet>> getDescriptorSets(const UInt32& descriptorSets, const UInt32& descriptors, const Array<Array<DescriptorBinding>>& bindings = { }) const override {
            auto sets = this->allocateMultiple(descriptorSets, descriptors, bindings);
            Array<UniquePtr<IDescriptorSet>> results;
            results.reserve(sets.size());
            std::move(sets.begin(), sets.end(), std::inserter(results, results.end()));
            return results;
        }

        virtual Array<UniquePtr<IDescriptorSet>> getDescriptorSets(const UInt32& descriptorSets, const UInt32& descriptors, std::function<Array<DescriptorBinding>(const UInt32&)> bindingFactory) const override {
            auto sets = this->allocateMultiple(descriptorSets, descriptors, bindingFactory);
            Array<UniquePtr<IDescriptorSet>> results;
            results.reserve(sets.size());
            std::move(sets.begin(), sets.end(), std::inserter(results, results.end()));
            return results;
        }

        virtual void releaseDescriptorSet(const IDescriptorSet& descriptorSet) const noexcept override {
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
        rtti::implements<TPushConstantsRange, IPushConstantsRange>
    class PushConstantsLayout : public IPushConstantsLayout {
    public:
        using push_constants_range_type = TPushConstantsRange;

    public:
        virtual ~PushConstantsLayout() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const push_constants_range_type*> ranges() const noexcept = 0;

    private:
        virtual Array<const IPushConstantsRange*> getRanges() const noexcept override {
            auto ranges = this->ranges();
            return Array<const IPushConstantsRange*>(ranges.begin(), ranges.end());
        }
    };

    /// <summary>
    /// Represents a shader program, consisting of multiple <see cref="IShaderModule" />s.
    /// </summary>
    /// <typeparam name="TShaderModule">The type of the shader module. Must implement <see cref="IShaderModule"/>.</typeparam>
    /// <seealso href="https://github.com/crud89/LiteFX/wiki/Shader-Development" />
    template <typename TShaderModule> requires
        rtti::implements<TShaderModule, IShaderModule>
    class ShaderProgram : public IShaderProgram {
    public:
        using shader_module_type = TShaderModule;

    public:
        virtual ~ShaderProgram() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const shader_module_type*> modules() const noexcept = 0;

    private:
        virtual Array<const IShaderModule*> getModules() const noexcept {
            auto modules = this->modules();
            return Array<const IShaderModule*>(modules.begin(), modules.end());
        }
    };
    
    /// <summary>
    /// Represents a the layout of a <see cref="RenderPipeline" /> or a <see cref="ComputePipeline" />.
    /// </summary>
    /// <typeparam name="TDescriptorSetLayout">The type of the descriptor set layout. Must implement <see cref="DescriptorSetLayout"/>.</typeparam>
    /// <typeparam name="TPushConstantsLayout">The type of the push constants layout. Must implement <see cref="PushConstantsLayout"/>.</typeparam>
    template <typename TDescriptorSetLayout, typename TPushConstantsLayout> requires
        rtti::implements<TDescriptorSetLayout, DescriptorSetLayout<typename TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSetLayout::descriptor_set_type>> &&
        rtti::implements<TPushConstantsLayout, PushConstantsLayout<typename TPushConstantsLayout::push_constants_range_type>>
    class PipelineLayout : public IPipelineLayout {
    public:
        using descriptor_set_layout_type = TDescriptorSetLayout;
        using push_constants_layout_type = TPushConstantsLayout;

    public:
        virtual ~PipelineLayout() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const descriptor_set_layout_type& descriptorSet(const UInt32& space) const = 0;

        /// <inheritdoc />
        virtual Array<const descriptor_set_layout_type*> descriptorSets() const noexcept = 0;

        /// <inheritdoc />
        virtual const push_constants_layout_type* pushConstants() const noexcept = 0;

    private:
        virtual Array<const IDescriptorSetLayout*> getDescriptorSets() const noexcept override {
            auto layouts = this->descriptorSets();
            return Array<const IDescriptorSetLayout*>(layouts.begin(), layouts.end());
        }
    };

    /// <summary>
    /// Describes a vertex buffer.
    /// </summary>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout"/>.</typeparam>
    template <typename TVertexBufferLayout> requires
        rtti::implements<TVertexBufferLayout, IVertexBufferLayout>
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
        rtti::implements<TIndexBufferLayout, IIndexBufferLayout>
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
        rtti::implements<TVertexBufferLayout, IVertexBufferLayout> &&
        rtti::implements<TIndexBufferLayout, IIndexBufferLayout>
    class InputAssembler : public IInputAssembler {
    public:
        using vertex_buffer_layout_type = TVertexBufferLayout;
        using index_buffer_layout_type = TIndexBufferLayout;

    public:
        virtual ~InputAssembler() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const vertex_buffer_layout_type*> vertexBufferLayouts() const noexcept = 0;

        /// <inheritdoc />
        virtual const vertex_buffer_layout_type& vertexBufferLayout(const UInt32& binding) const = 0;

        /// <inheritdoc />
        virtual const index_buffer_layout_type& indexBufferLayout() const = 0;

    private:
        virtual Array<const IVertexBufferLayout*> getVertexBufferLayouts() const noexcept override {
            auto layouts = this->vertexBufferLayouts();
            return Array<const IVertexBufferLayout*>(layouts.begin(), layouts.end());
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
        rtti::implements<TPipelineLayout, PipelineLayout<typename TPipelineLayout::descriptor_set_layout_type, typename TPipelineLayout::push_constants_layout_type>> &&
        rtti::implements<TShaderProgram, ShaderProgram<typename TShaderProgram::shader_module_type>>
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
        virtual SharedPtr<const IShaderProgram> getProgram() const noexcept override {
            return std::static_pointer_cast<const IShaderProgram>(this->program());
        }

        virtual SharedPtr<const IPipelineLayout> getLayout() const noexcept override {
            return std::static_pointer_cast<const IPipelineLayout>(this->layout());
        }
    };

    /// <summary>
    /// Represents a command buffer, that buffers commands that should be submitted to a <see cref="CommandQueue" />.
    /// </summary>
    /// <typeparam name="TBuffer">The generic buffer type. Must implement <see cref="IBuffer"/>.</typeparam>
    /// <typeparam name="TVertexBuffer">The vertex buffer type. Must implement <see cref="VertexBuffer"/>.</typeparam>
    /// <typeparam name="TIndexBuffer">The index buffer type. Must implement <see cref="IndexBuffer"/>.</typeparam>
    /// <typeparam name="TImage">The generic image type. Must implement <see cref="IImage"/>.</typeparam>
    /// <typeparam name="TBarrier">The barrier type. Must implement <see cref="Barrier"/>.</typeparam>
    /// <typeparam name="TPipeline">The common pipeline interface type. Must be derived from <see cref="Pipeline"/>.</typeparam>
    template <typename TBuffer, typename TVertexBuffer, typename TIndexBuffer, typename TImage, typename TBarrier, typename TPipeline> requires
        rtti::implements<TBarrier, Barrier<TBuffer, TImage>> &&
        std::derived_from<TPipeline, Pipeline<typename TPipeline::pipeline_layout_type, typename TPipeline::shader_program_type>>
    class CommandBuffer : public ICommandBuffer {
    public:
        using ICommandBuffer::dispatch;
        using ICommandBuffer::draw;
        using ICommandBuffer::drawIndexed;
        using ICommandBuffer::barrier;
        using ICommandBuffer::transfer;
        using ICommandBuffer::generateMipMaps;
        using ICommandBuffer::bind;
        using ICommandBuffer::use;
        using ICommandBuffer::pushConstants;

    public:
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
        virtual void barrier(const barrier_type& barrier) const noexcept = 0;

        /// <inheritdoc />
        virtual void generateMipMaps(image_type& image) noexcept = 0;

        /// <inheritdoc />
        virtual void transfer(buffer_type& source, buffer_type& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(buffer_type& source, image_type& target, const UInt32& sourceElement = 0, const UInt32& firstSubresource = 0, const UInt32& elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(image_type& source, image_type& target, const UInt32& sourceSubresource = 0, const UInt32& targetSubresource = 0, const UInt32& subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(image_type& source, buffer_type& target, const UInt32& firstSubresource = 0, const UInt32& targetElement = 0, const UInt32& subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(SharedPtr<buffer_type> source, buffer_type& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(SharedPtr<buffer_type> source, image_type& target, const UInt32& sourceElement = 0, const UInt32& firstSubresource = 0, const UInt32& elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(SharedPtr<image_type> source, image_type& target, const UInt32& sourceSubresource = 0, const UInt32& targetSubresource = 0, const UInt32& subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(SharedPtr<image_type> source, buffer_type& target, const UInt32& firstSubresource = 0, const UInt32& targetElement = 0, const UInt32& subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void use(const pipeline_type& pipeline) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(const descriptor_set_type& descriptorSet, const pipeline_type& pipeline) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(const vertex_buffer_type& buffer) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(const index_buffer_type& buffer) const noexcept = 0;

        /// <inheritdoc />
        virtual void pushConstants(const push_constants_layout_type& layout, const void* const memory) const noexcept = 0;

        /// <inheritdoc />
        virtual void draw(const vertex_buffer_type& vertexBuffer, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->draw(vertexBuffer.elements(), instances, firstVertex, firstInstance);
        }

        /// <inheritdoc />
        virtual void drawIndexed(const index_buffer_type& indexBuffer, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const {
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }

        /// <inheritdoc />
        virtual void drawIndexed(const vertex_buffer_type& vertexBuffer, const index_buffer_type& indexBuffer, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }

    private:
        virtual void cmdBarrier(const IBarrier& barrier) const noexcept override { 
            this->barrier(dynamic_cast<const barrier_type&>(barrier));
        }

        virtual void cmdGenerateMipMaps(IImage& image) noexcept override { 
            this->generateMipMaps(dynamic_cast<image_type&>(image));
        }

        virtual void cmdTransfer(IBuffer& source, IBuffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const override { 
            this->transfer(dynamic_cast<buffer_type&>(source), dynamic_cast<buffer_type&>(target), sourceElement, targetElement, elements);
        }
        
        virtual void cmdTransfer(IBuffer& source, IImage& target, const UInt32& sourceElement, const UInt32& firstSubresource, const UInt32& elements) const override { 
            this->transfer(dynamic_cast<buffer_type&>(source), dynamic_cast<image_type&>(target), sourceElement, firstSubresource, elements);
        }
        
        virtual void cmdTransfer(IImage& source, IImage& target, const UInt32& sourceSubresource, const UInt32& targetSubresource, const UInt32& subresources) const override {
            this->transfer(dynamic_cast<image_type&>(source), dynamic_cast<image_type&>(target), sourceSubresource, targetSubresource, subresources);
        }

        virtual void cmdTransfer(IImage& source, IBuffer& target, const UInt32& firstSubresource, const UInt32& targetElement, const UInt32& subresources) const override {
            this->transfer(dynamic_cast<image_type&>(source), dynamic_cast<buffer_type&>(target), firstSubresource, targetElement, subresources);
        }

        virtual void cmdTransfer(SharedPtr<IBuffer> source, IBuffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const override {
            this->transfer(std::dynamic_pointer_cast<buffer_type>(source), dynamic_cast<buffer_type&>(target), sourceElement, targetElement, elements);
        }
        
        virtual void cmdTransfer(SharedPtr<IBuffer> source, IImage& target, const UInt32& sourceElement, const UInt32& firstSubresource, const UInt32& elements) const override {
            this->transfer(std::dynamic_pointer_cast<buffer_type>(source), dynamic_cast<image_type&>(target), sourceElement, firstSubresource, elements);
        }
        
        virtual void cmdTransfer(SharedPtr<IImage> source, IImage& target, const UInt32& sourceSubresource, const UInt32& targetSubresource, const UInt32& subresources) const override {
            this->transfer(std::dynamic_pointer_cast<image_type>(source), dynamic_cast<image_type&>(target), sourceSubresource, targetSubresource, subresources);
        }
        
        virtual void cmdTransfer(SharedPtr<IImage> source, IBuffer& target, const UInt32& firstSubresource, const UInt32& targetElement, const UInt32& subresources) const override {
            this->transfer(std::dynamic_pointer_cast<image_type>(source), dynamic_cast<buffer_type&>(target), firstSubresource, targetElement, subresources);
        }

        virtual void cmdUse(const IPipeline& pipeline) const noexcept override { 
            this->use(dynamic_cast<const pipeline_type&>(pipeline));
        }

        virtual void cmdBind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const noexcept override { 
            this->bind(dynamic_cast<const descriptor_set_type&>(descriptorSet), dynamic_cast<const pipeline_type&>(pipeline));
        }
        
        virtual void cmdBind(const IVertexBuffer& buffer) const noexcept override { 
            this->bind(dynamic_cast<const vertex_buffer_type&>(buffer));
        }

        virtual void cmdBind(const IIndexBuffer& buffer) const noexcept override { 
            this->bind(dynamic_cast<const index_buffer_type&>(buffer));
        }
        
        virtual void cmdPushConstants(const IPushConstantsLayout& layout, const void* const memory) const noexcept override { 
            this->pushConstants(dynamic_cast<const push_constants_layout_type&>(layout), memory);
        }
        
        virtual void cmdDraw(const IVertexBuffer& vertexBuffer, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const override { 
            this->draw(dynamic_cast<const vertex_buffer_type&>(vertexBuffer), instances, firstVertex, firstInstance);
        }
        
        virtual void cmdDrawIndexed(const IIndexBuffer& indexBuffer, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const override { 
            this->drawIndexed(dynamic_cast<const index_buffer_type&>(indexBuffer), instances, firstIndex, vertexOffset, firstInstance);
        }
        
        virtual void cmdDrawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const override { 
            this->drawIndexed(dynamic_cast<const vertex_buffer_type&>(vertexBuffer), dynamic_cast<const index_buffer_type&>(indexBuffer), instances, firstIndex, vertexOffset, firstInstance);
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
        rtti::implements<TInputAssembler, InputAssembler<typename TInputAssembler::vertex_buffer_layout_type, typename TInputAssembler::index_buffer_layout_type>> &&
        rtti::implements<TRasterizer, Rasterizer>
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
        virtual SharedPtr<IInputAssembler> getInputAssembler() const noexcept override {
            return this->inputAssembler();
        }

        virtual SharedPtr<IRasterizer> getRasterizer() const noexcept override {
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
    /// Stores the images for the output attachments for a back buffer of a <see cref="RenderPass" />, as well as a <see cref="CommandBuffer" /> instance, that records draw commands.
    /// </summary>
    /// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="CommandBuffer"/>.</typeparam>
    /// <seealso cref="RenderTarget" />
    template <typename TCommandBuffer> requires
        rtti::implements<TCommandBuffer, CommandBuffer<typename TCommandBuffer::buffer_type, typename TCommandBuffer::vertex_buffer_type, typename TCommandBuffer::index_buffer_type, typename TCommandBuffer::image_type, typename TCommandBuffer::barrier_type, typename TCommandBuffer::pipeline_type>>
    class FrameBuffer : public IFrameBuffer {
    public:
        using command_buffer_type = TCommandBuffer;
        using image_type = command_buffer_type::image_type;

    public:
        virtual ~FrameBuffer() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<SharedPtr<const command_buffer_type>> commandBuffers() const noexcept = 0;

        /// <inheritdoc />
        virtual SharedPtr<const command_buffer_type> commandBuffer(const UInt32& index) const = 0;

        /// <inheritdoc />
        virtual Array<const image_type*> images() const noexcept = 0;

        /// <inheritdoc />
        virtual const image_type& image(const UInt32& location) const = 0;

    private:
        virtual SharedPtr<const ICommandBuffer> getCommandBuffer(const UInt32& index) const noexcept override {
            return this->commandBuffer(index);
        }

        virtual Array<SharedPtr<const ICommandBuffer>> getCommandBuffers() const noexcept override {
            auto commandBuffers = this->commandBuffers();
            return Array<SharedPtr<const ICommandBuffer>>(commandBuffers.begin(), commandBuffers.end());
        }

        virtual Array<const IImage*> getImages() const noexcept override {
            auto images = this->images();
            return Array<const IImage*>(images.begin(), images.end());
        }
    };

    /// <summary>
    /// Represents the source for an input attachment mapping.
    /// </summary>
    /// <remarks>
    /// This interface is implemented by a <see cref="RenderPass" /> to return the frame buffer for a given back buffer. It is called by a <see cref="FrameBuffer" /> 
    /// during initialization or re-creation, in order to resolve input attachment dependencies.
    /// </remarks>
    /// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="FrameBuffer" />.</typeparam>
    template <typename TFrameBuffer> requires
        rtti::implements<TFrameBuffer, FrameBuffer<typename TFrameBuffer::command_buffer_type>>
    class IInputAttachmentMappingSource {
    public:
        using frame_buffer_type = TFrameBuffer;

    public:
        virtual ~IInputAttachmentMappingSource() noexcept = default;

    public:
        /// <summary>
        /// Returns the frame buffer with the index provided in <paramref name="buffer" />.
        /// </summary>
        /// <param name="buffer">The index of a frame buffer within the source.</param>
        /// <returns>The frame buffer with the index provided in <paramref name="buffer" />.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the <paramref name="buffer" /> does not map to a frame buffer within the source.</exception>
        virtual const frame_buffer_type& frameBuffer(const UInt32& buffer) const = 0;
    };

    /// <summary>
    /// Represents a mapping between a set of <see cref="IRenderTarget" /> instances and the input attachments of a <see cref="RenderPass" />.
    /// </summary>
    /// <typeparam name="TInputAttachmentMappingSource">The type of the input attachment mapping source. Must implement <see cref="IInputAttachmentMappingSource" />.</typeparam>
    template <typename TInputAttachmentMappingSource> requires
        rtti::implements<TInputAttachmentMappingSource, IInputAttachmentMappingSource<typename TInputAttachmentMappingSource::frame_buffer_type>>
    class IInputAttachmentMapping {
    public:
        using input_attachment_mapping_source_type = TInputAttachmentMappingSource;

    public:
        virtual ~IInputAttachmentMapping() noexcept = default;

    public:
        /// <summary>
        /// Returns the source of the input attachment render target.
        /// </summary>
        /// <returns>The source of the input attachment render target.</returns>
        virtual const input_attachment_mapping_source_type* inputAttachmentSource() const noexcept = 0;

        /// <summary>
        /// Returns a reference of the render target that is mapped to the input attachment.
        /// </summary>
        /// <returns>A reference of the render target that is mapped to the input attachment.</returns>
        virtual const RenderTarget& renderTarget() const noexcept = 0;

        /// <summary>
        /// Returns the location of the input attachment, the render target will be bound to.
        /// </summary>
        /// <remarks>
        /// The locations of all input attachments for a frame buffer must be within a continuous domain, starting at <c>0</c>. A frame buffer validates the locations
        /// when it is initialized and will raise an exception, if a location is either not mapped or assigned multiple times.
        /// </remarks>
        /// <returns>The location of the input attachment, the render target will be bound to.</returns>
        virtual const UInt32& location() const noexcept = 0;
    };

    /// <summary>
    /// Represents a render pass.
    /// </summary>
    /// <remarks>
    /// A render pass is a conceptual layer, that may not have any logical representation within the actual implementation. It is a high-level view on a specific workload on the
    /// GPU, that processes data using different <see cref="RenderPipeline" />s and stores the outputs in the <see cref="IRenderTarget" />s of a <see cref="FrameBuffer" />.
    /// </remarks>
    /// <typeparam name="TRenderPipeline">The type of the render pipeline. Must implement <see cref="RenderPipeline" />.</typeparam>
    /// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="FrameBuffer" />.</typeparam>
    /// <typeparam name="TInputAttachmentMapping">The type of the input attachment mapping. Must implement <see cref="IInputAttachmentMapping" />.</typeparam>
    template <typename TRenderPipeline, typename TFrameBuffer, typename TInputAttachmentMapping> requires
        rtti::implements<TFrameBuffer, FrameBuffer<typename TFrameBuffer::command_buffer_type>> &&
        rtti::implements<TRenderPipeline, RenderPipeline<typename TRenderPipeline::pipeline_layout_type, typename TRenderPipeline::shader_program_type, typename TRenderPipeline::input_assembler_type, typename TRenderPipeline::rasterizer_type>> /*&&
        rtti::implements<TInputAttachmentMapping, IInputAttachmentMapping<TDerived>>*/
    class RenderPass : public virtual StateResource, public IRenderPass, public IInputAttachmentMappingSource<TFrameBuffer> {
    public:
        using IRenderPass::updateAttachments;

        using frame_buffer_type = TFrameBuffer;
        using render_pipeline_type = TRenderPipeline;
        using input_attachment_mapping_type = TInputAttachmentMapping;
        using pipeline_layout_type = render_pipeline_type::pipeline_layout_type;
        using descriptor_set_layout_type = pipeline_layout_type::descriptor_set_layout_type;
        using descriptor_set_type = descriptor_set_layout_type::descriptor_set_type;

    public:
        virtual ~RenderPass() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const frame_buffer_type& activeFrameBuffer() const = 0;

        /// <inheritdoc />
        virtual Array<const frame_buffer_type*> frameBuffers() const noexcept = 0;

        /// <inheritdoc />
        virtual Array<const render_pipeline_type*> pipelines() const noexcept = 0;

        /// <inheritdoc />
        virtual Span<const input_attachment_mapping_type> inputAttachments() const noexcept = 0;

        /// <inheritdoc />
        virtual void updateAttachments(const descriptor_set_type& descriptorSet) const = 0;

    private:
        virtual Array<const IFrameBuffer*> getFrameBuffers() const noexcept override {
            auto frameBuffers = this->frameBuffers();
            return Array<const IFrameBuffer*>(frameBuffers.begin(), frameBuffers.end());
        }

        virtual Array<const IRenderPipeline*> getPipelines() const noexcept override {
            auto pipelines = this->pipelines();
            return Array<const IRenderPipeline*>(pipelines.begin(), pipelines.end());
        }

        virtual void setAttachments(const IDescriptorSet& descriptorSet) const override {
            this->updateAttachments(dynamic_cast<const descriptor_set_type&>(descriptorSet));
        }
    };

    /// <summary>
    /// Represents a swap chain, i.e. a chain of multiple <see cref="IImage" /> instances, that can be presented to a <see cref="ISurface" />.
    /// </summary>
    /// <typeparam name="TImageInterface">The type of the image interface. Must inherit from <see cref="IImage"/>.</typeparam>
    template <typename TImageInterface, typename TFrameBuffer> requires
        rtti::implements<TFrameBuffer, FrameBuffer<typename TFrameBuffer::command_buffer_type>> &&
        std::derived_from<TImageInterface, IImage>
    class SwapChain : public ISwapChain {
    public:
        using image_interface_type = TImageInterface;
        using frame_buffer_type = TFrameBuffer;

    public:
        virtual ~SwapChain() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const image_interface_type*> images() const noexcept = 0;

        /// <summary>
        /// Queues a present that gets executed after <paramref name="frameBuffer" /> signals its readiness.
        /// </summary>
        /// <param name="frameBuffer">The frame buffer for which the present should wait.</param>
        virtual void present(const frame_buffer_type& frameBuffer) const = 0;

        /// <inheritdoc />
        virtual void present(const IFrameBuffer& frameBuffer) const override {
            this->present(dynamic_cast<const frame_buffer_type&>(frameBuffer));
        }

    private:
        virtual Array<const IImage*> getImages() const noexcept override {
            auto images = this->images();
            return Array<const IImage*>(images.begin(), images.end());
        }
    };

    /// <summary>
    /// Represents a command queue.
    /// </summary>
    /// <typeparam name="TCommandBuffer">The type of the command buffer for this queue. Must implement <see cref="CommandBuffer"/>.</typeparam>
    template <typename TCommandBuffer> requires
        rtti::implements<TCommandBuffer, CommandBuffer<typename TCommandBuffer::buffer_type, typename TCommandBuffer::vertex_buffer_type, typename TCommandBuffer::index_buffer_type, typename TCommandBuffer::image_type, typename TCommandBuffer::barrier_type, typename TCommandBuffer::pipeline_type>>
    class CommandQueue : public ICommandQueue {
    public:
        using ICommandQueue::submit;

        using command_buffer_type = TCommandBuffer;

    public:
        virtual ~CommandQueue() noexcept = default;

    public:
        /// <inheritdoc />
        virtual SharedPtr<command_buffer_type> createCommandBuffer(const bool& beginRecording = false) const = 0;

        /// <inheritdoc />
        virtual UInt64 submit(SharedPtr<command_buffer_type> commandBuffer) const {
            return this->submit(std::static_pointer_cast<const command_buffer_type>(commandBuffer));
        }

        /// <inheritdoc />
        virtual UInt64 submit(SharedPtr<const command_buffer_type> commandBuffer) const = 0;

        /// <inheritdoc />
        virtual UInt64 submit(const Array<SharedPtr<command_buffer_type>>& commandBuffers) const {
            return this->submit(commandBuffers | std::ranges::to<Array<SharedPtr<const command_buffer_type>>>());
        }

        /// <inheritdoc />
        virtual UInt64 submit(const Array<SharedPtr<const command_buffer_type>>& commandBuffers) const = 0;

    private:
        virtual SharedPtr<ICommandBuffer> getCommandBuffer(const bool& beginRecording) const override {
            return this->createCommandBuffer(beginRecording);
        }

        virtual UInt64 submitCommandBuffer(SharedPtr<const ICommandBuffer> commandBuffer) const override {
            return this->submit(std::dynamic_pointer_cast<const command_buffer_type>(commandBuffer));
        }

        virtual UInt64 submitCommandBuffers(const Array<SharedPtr<const ICommandBuffer>>& commandBuffers) const override {
            Array<SharedPtr<const command_buffer_type>> buffers = commandBuffers |
                std::views::transform([](auto buffer) { return std::dynamic_pointer_cast<const command_buffer_type>(buffer); }) |
                std::ranges::to<Array<SharedPtr<const command_buffer_type>>>();

            return this->submit(buffers);
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
    template <typename TDescriptorLayout, typename TBuffer, typename TVertexBuffer, typename TIndexBuffer, typename TImage, typename TSampler> requires
        rtti::implements<TDescriptorLayout, IDescriptorLayout> &&
        std::derived_from<TVertexBuffer, VertexBuffer<typename TVertexBuffer::vertex_buffer_layout_type>> &&
        std::derived_from<TIndexBuffer, IndexBuffer<typename TIndexBuffer::index_buffer_layout_type>> &&
        std::derived_from<TImage, IImage> &&
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TSampler, ISampler>
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

    public:
        virtual ~GraphicsFactory() noexcept = default;

    public:
        /// <inheritdoc />
        virtual UniquePtr<TBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1, const bool& allowWrite = false) const = 0;
        
        /// <inheritdoc />
        virtual UniquePtr<TBuffer> createBuffer(const String& name, const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1, const bool& allowWrite = false) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TVertexBuffer> createVertexBuffer(const vertex_buffer_layout_type& layout, const BufferUsage& usage, const UInt32& elements = 1) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TVertexBuffer> createVertexBuffer(const String& name, const vertex_buffer_layout_type& layout, const BufferUsage& usage, const UInt32& elements = 1) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TIndexBuffer> createIndexBuffer(const index_buffer_layout_type& layout, const BufferUsage& usage, const UInt32& elements) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TIndexBuffer> createIndexBuffer(const String& name, const index_buffer_layout_type& layout, const BufferUsage& usage, const UInt32& elements) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createAttachment(const String& name, const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createTexture(const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createTexture(const String& name, const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const = 0;

        /// <inheritdoc />
        virtual Array<UniquePtr<TImage>> createTextures(const UInt32& elements, const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32 & layers = 1, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TSampler> createSampler(const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TSampler> createSampler(const String& name, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const = 0;

        /// <inheritdoc />
        virtual Array<UniquePtr<TSampler>> createSamplers(const UInt32& elements, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const = 0;

    private:
        virtual UniquePtr<IBuffer> getBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements, const bool& allowWrite) const override { 
            return this->createBuffer(type, usage, elementSize, elements, allowWrite);
        }

        virtual UniquePtr<IBuffer> getBuffer(const String& name, const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements, const bool& allowWrite) const override {
            return this->createBuffer(name, type, usage, elementSize, elements, allowWrite);
        }

        virtual UniquePtr<IVertexBuffer> getVertexBuffer(const IVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override { 
            return this->createVertexBuffer(dynamic_cast<const vertex_buffer_layout_type&>(layout), usage, elements);
        }

        virtual UniquePtr<IVertexBuffer> getVertexBuffer(const String& name, const IVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override {
            return this->createVertexBuffer(name, dynamic_cast<const vertex_buffer_layout_type&>(layout), usage, elements);
        }
        
        virtual UniquePtr<IIndexBuffer> getIndexBuffer(const IIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override {
            return this->createIndexBuffer(dynamic_cast<const index_buffer_layout_type&>(layout), usage, elements);
        }

        virtual UniquePtr<IIndexBuffer> getIndexBuffer(const String& name, const IIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override {
            return this->createIndexBuffer(name, dynamic_cast<const index_buffer_layout_type&>(layout), usage, elements);
        }

        virtual UniquePtr<IImage> getAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const override { 
            return this->createAttachment(format, size, samples);
        }

        virtual UniquePtr<IImage> getAttachment(const String& name, const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const override {
            return this->createAttachment(name, format, size, samples);
        }
        
        virtual UniquePtr<IImage> getTexture(const Format& format, const Size3d& size, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& allowWrite) const override { 
            return this->createTexture(format, size, dimension, levels, layers, samples, allowWrite);
        }

        virtual UniquePtr<IImage> getTexture(const String& name, const Format& format, const Size3d& size, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& allowWrite) const override {
            return this->createTexture(name, format, size, dimension, levels, layers, samples, allowWrite);
        }

        virtual Array<UniquePtr<IImage>> getTextures(const UInt32& elements, const Format& format, const Size3d& size, const ImageDimensions& dimension, const UInt32& layers, const UInt32& levels, const MultiSamplingLevel& samples, const bool& allowWrite) const override { 
            auto textures = this->getTextures(elements, format, size, dimension, layers, levels, samples, allowWrite);
            Array<UniquePtr<IImage>> results;
            results.reserve(textures.size());
            std::move(std::begin(textures), std::end(textures), std::inserter(results, std::end(results)));
            return results;
        }
        
        virtual UniquePtr<ISampler> getSampler(const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const override { 
            return this->createSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

        virtual UniquePtr<ISampler> getSampler(const String& name, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const override {
            return this->createSampler(name, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }
        
        virtual Array<UniquePtr<ISampler>> getSamplers(const UInt32& elements, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const override {
            auto samplers = this->createSamplers(elements, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
            Array<UniquePtr<ISampler>> results;
            results.reserve(samplers.size());
            std::move(std::begin(samplers), std::end(samplers), std::inserter(results, std::end(results)));
            return results;
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
    /// <typeparam name="TBarrier">The type of the memory barrier. Must implement <see cref="Barrier" />.</typeparam>
    template <typename TFactory, typename TSurface, typename TGraphicsAdapter, typename TSwapChain, typename TCommandQueue, typename TRenderPass, typename TComputePipeline, typename TBarrier> requires
        rtti::implements<TSurface, ISurface> &&
        rtti::implements<TGraphicsAdapter, IGraphicsAdapter> &&
        rtti::implements<TSwapChain, SwapChain<typename TFactory::image_type, typename TRenderPass::frame_buffer_type>> &&
        rtti::implements<TCommandQueue, CommandQueue<typename TCommandQueue::command_buffer_type>> &&
        rtti::implements<TFactory, GraphicsFactory<typename TFactory::descriptor_layout_type, typename TFactory::buffer_type, typename TFactory::vertex_buffer_type, typename TFactory::index_buffer_type, typename TFactory::image_type, typename TFactory::sampler_type>> &&
        rtti::implements<TRenderPass, RenderPass<typename TRenderPass::render_pipeline_type, typename TRenderPass::frame_buffer_type, typename TRenderPass::input_attachment_mapping_type>> &&
        rtti::implements<TComputePipeline, ComputePipeline<typename TComputePipeline::pipeline_layout_type, typename TComputePipeline::shader_program_type>> &&
        rtti::implements<TBarrier, Barrier<typename TFactory::buffer_type, typename TFactory::image_type>>
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
        using render_pass_type = TRenderPass;
        using frame_buffer_type = render_pass_type::frame_buffer_type;
        using render_pipeline_type = render_pass_type::render_pipeline_type;
        using compute_pipeline_type = TComputePipeline;
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
        virtual const command_queue_type& graphicsQueue() const noexcept = 0;

        /// <inheritdoc />
        virtual const command_queue_type& transferQueue() const noexcept = 0;

        /// <inheritdoc />
        virtual const command_queue_type& bufferQueue() const noexcept = 0;

        /// <inheritdoc />
        virtual const command_queue_type& computeQueue() const noexcept = 0;

        /// <inheritdoc />
        [[nodiscard]] virtual UniquePtr<barrier_type> makeBarrier(const PipelineStage& syncBefore, const PipelineStage& syncAfter) const noexcept = 0;

    private:
        virtual UniquePtr<IBarrier> getNewBarrier(const PipelineStage& syncBefore, const PipelineStage& syncAfter) const noexcept override {
            return this->makeBarrier(syncBefore, syncAfter);
        }

#if defined(BUILD_DEFINE_BUILDERS)
    public:
        using render_pass_builder_type = render_pass_type::builder_type;
        using render_pipeline_builder_type = render_pipeline_type::builder_type;
        using compute_pipeline_builder_type = compute_pipeline_type::builder_type;
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
        [[nodiscard]] virtual render_pass_builder_type buildRenderPass(const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const UInt32& commandBuffers = 1) const = 0;

        /// <summary>
        /// Returns a builder for a <see cref="RenderPass" />.
        /// </summary>
        /// <param name="name">The name of the render pass.</param>
        /// <param name="samples">The number of samples, the render targets of the render pass should be sampled with.</param>
        /// <param name="commandBuffers">The number of command buffers in each frame buffer.</param>
        /// <returns>An instance of a builder that is used to create a new render pass.</returns>
        [[nodiscard]] virtual render_pass_builder_type buildRenderPass(const String& name, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const UInt32& commandBuffers = 1) const = 0;

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
#endif // defined(BUILD_DEFINE_BUILDERS)
    };

    /// <summary>
    /// Defines a back-end, that provides a device instance for a certain surface and graphics adapter.
    /// </summary>
    /// <typeparam name="TBackend">The type of the backend derived from the interface. Must implement <see cref="IRenderBackend" />.</typeparam>
    /// <typeparam name="TGraphicsDevice">The type of the graphics device. Must implement <see cref="GraphicsDevice" />.</typeparam>
    template <typename TBackend, typename TGraphicsDevice> requires
        rtti::implements<TGraphicsDevice, GraphicsDevice<typename TGraphicsDevice::factory_type, typename TGraphicsDevice::surface_type, typename TGraphicsDevice::adapter_type, typename TGraphicsDevice::swap_chain_type, typename TGraphicsDevice::command_queue_type, typename TGraphicsDevice::render_pass_type, typename TGraphicsDevice::compute_pipeline_type, typename TGraphicsDevice::barrier_type>>
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
        using shader_program_type = device_type::shader_program_type;
        using input_assembler_type = device_type::input_assembler_type;
        using rasterizer_type = device_type::rasterizer_type;

    public:
        virtual ~RenderBackend() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const adapter_type*> listAdapters() const = 0;

        /// <inheritdoc />
        virtual const adapter_type* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const = 0;

        /// <inheritdoc />
        virtual void registerDevice(String name, UniquePtr<device_type>&& device) = 0;

        /// <summary>
        /// Creates a new graphics device.
        /// </summary>
        /// <param name="_args">The arguments that are passed to the graphics device constructor.</param>
        /// <returns>A pointer of the created graphics device instance.</returns>
        template <typename ...TArgs>
        device_type* createDevice(String name, const adapter_type& adapter, UniquePtr<surface_type>&& surface, TArgs&&... _args) {
            auto device = makeUnique<device_type>(static_cast<const TBackend&>(*this), adapter, std::move(surface), std::forward<TArgs>(_args)...);
            auto devicePointer = device.get();
            this->registerDevice(name, std::move(device));
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
        virtual const device_type* operator[](const String& name) const noexcept {
            return this->device(name);
        };

        /// <inheritdoc />
        virtual device_type* operator[](const String& name) noexcept {
            return this->device(name);
        };

        // IRenderBackend interface
    private:
        virtual Array<const IGraphicsAdapter*> getAdapters() const override {
            auto adapters = this->listAdapters();
            return Array<const IGraphicsAdapter*>(adapters.begin(), adapters.end());
        }
    };
}