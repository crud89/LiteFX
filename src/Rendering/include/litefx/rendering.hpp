#pragma once

#include <litefx/rendering_api.hpp>
#include <litefx/rendering_formatters.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;
    using namespace LiteFX::Math;

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TVertexBufferLayout, typename TParent> requires
        rtti::implements<TVertexBufferLayout, IVertexBufferLayout>
    class VertexBufferLayoutBuilder : public Builder<TDerived, TVertexBufferLayout, TParent> {
    public:
        using Builder<TDerived, TVertexBufferLayout, TParent>::Builder;

    public:
        virtual TDerived& addAttribute(UniquePtr<BufferAttribute>&& attribute) = 0;
    };

    /// <summary>
    /// A barrier that transitions a set of resources backed by <see cref="IDeviceMemory" /> into different <see cref="ResourceState" />.
    /// </summary>
    /// <remarks>
    /// It is recommended to insert multiple transitions into one single barrier. This can be done by calling <see cref="transition" /> multiple times. 
    /// </remarks>
    template <typename TBuffer, typename TImage> requires
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TImage, IImage>
    class Barrier : public IBarrier {
    public:
        using buffer_type = TBuffer;
        using image_type = TImage;

    public:
        virtual ~Barrier() noexcept = default;

    public:
        /// <inheritdoc />
        virtual void transition(TBuffer& buffer, const ResourceState& targetState) = 0;

        /// <inheritdoc />
        virtual void transition(TBuffer& buffer, const UInt32& element, const ResourceState& targetState) = 0;

        /// <inheritdoc />
        virtual void transition(TBuffer& buffer, const ResourceState& sourceState, const ResourceState& targetState) = 0;

        /// <inheritdoc />
        virtual void transition(TBuffer& buffer, const ResourceState& sourceState, const UInt32& element, const ResourceState& targetState) = 0;

        /// <inheritdoc />
        virtual void transition(TImage& image, const ResourceState& targetState) = 0;

        /// <inheritdoc />
        virtual void transition(TImage& image, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) = 0;

        /// <inheritdoc />
        virtual void transition(TImage& image, const ResourceState& sourceState, const ResourceState& targetState) = 0;

        /// <inheritdoc />
        virtual void transition(TImage& image, const ResourceState& sourceState, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) = 0;

        /// <inheritdoc />
        virtual void waitFor(const TBuffer& buffer) = 0;

        /// <inheritdoc />
        virtual void waitFor(const TImage& image) = 0;

    private:
        virtual void doTransition(IBuffer& buffer, const ResourceState& targetState) override { 
            this->transition(dynamic_cast<TBuffer&>(buffer), targetState);
        }

        virtual void doTransition(IBuffer& buffer, const UInt32& element, const ResourceState& targetState) override {
            this->transition(dynamic_cast<TBuffer&>(buffer), element, targetState);
        }

        virtual void doTransition(IBuffer& buffer, const ResourceState& sourceState, const ResourceState& targetState) override {
            this->transition(dynamic_cast<TBuffer&>(buffer), sourceState, targetState);
        }

        virtual void doTransition(IBuffer& buffer, const ResourceState& sourceState, const UInt32& element, const ResourceState& targetState) override {
            this->transition(dynamic_cast<TBuffer&>(buffer), sourceState, element, targetState);
        }

        virtual void doTransition(IImage& image, const ResourceState& targetState) override {
            this->transition(dynamic_cast<TImage&>(image), targetState);
        }

        virtual void doTransition(IImage& image, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) override {
            this->transition(dynamic_cast<TImage&>(image), level, layer, plane, targetState);
        }

        virtual void doTransition(IImage& image, const ResourceState& sourceState, const ResourceState& targetState) override {
            this->transition(dynamic_cast<TImage&>(image), sourceState, targetState);
        }

        virtual void doTransition(IImage& image, const ResourceState& sourceState, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) override {
            this->transition(dynamic_cast<TImage&>(image), sourceState, level, layer, plane, targetState);
        }

        virtual void doWaitFor(const IBuffer& buffer) override {
            this->waitFor(dynamic_cast<const TBuffer&>(buffer));
        }

        virtual void doWaitFor(const IImage& image) override {
            this->waitFor(dynamic_cast<const TImage&>(image));
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
        using buffer_type = TBuffer;
        using sampler_type = TSampler;
        using image_type = TImage;

    public:
        virtual ~DescriptorSet() noexcept = default;

    public:
        /// <inheritdoc />
        virtual void update(const UInt32& binding, const TBuffer& buffer, const UInt32& bufferElement = 0, const UInt32& elements = 1, const UInt32& firstDescriptor = 0) const = 0;

        /// <inheritdoc />
        virtual void update(const UInt32& binding, const TImage& texture, const UInt32& descriptor = 0, const UInt32& firstLevel = 0, const UInt32& levels = 0, const UInt32& firstLayer = 0, const UInt32& layers = 0) const = 0;

        /// <inheritdoc />
        virtual void update(const UInt32& binding, const TSampler& sampler, const UInt32& descriptor = 0) const = 0;

        /// <inheritdoc />
        virtual void attach(const UInt32& binding, const TImage& image) const = 0;

    private:
        virtual void doUpdate(const UInt32& binding, const IBuffer& buffer, const UInt32& bufferElement, const UInt32& elements, const UInt32& firstDescriptor) const override {
            this->update(binding, dynamic_cast<const TBuffer&>(buffer), bufferElement, elements, firstDescriptor);
        }

        virtual void doUpdate(const UInt32& binding, const IImage& texture, const UInt32& descriptor, const UInt32& firstLevel, const UInt32& levels, const UInt32& firstLayer, const UInt32& layers) const  override {
            this->update(binding, dynamic_cast<const TImage&>(texture), descriptor, firstLevel, levels, firstLayer, layers);
        }

        virtual void doUpdate(const UInt32& binding, const ISampler& sampler, const UInt32& descriptor) const  override {
            this->update(binding, dynamic_cast<const TSampler&>(sampler), descriptor);
        }

        virtual void doAttach(const UInt32& binding, const IImage& image) const  override {
            this->attach(binding, dynamic_cast<const TImage&>(image));
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
    template <typename TDescriptorLayout, typename TDescriptorSet, typename TBuffer = TDescriptorSet::buffer_type, typename TSampler = TDescriptorSet::sampler_type, typename TImage = TDescriptorSet::image_type> requires
        rtti::implements<TDescriptorLayout, IDescriptorLayout> &&
        rtti::implements<TDescriptorSet, DescriptorSet<TBuffer, TImage, TSampler>>
    class DescriptorSetLayout : public IDescriptorSetLayout {
    public:
        using descriptor_layout_type = TDescriptorLayout;
        using descriptor_set_type = TDescriptorSet;

    public:
        virtual ~DescriptorSetLayout() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const TDescriptorLayout*> descriptors() const noexcept = 0;

        /// <inheritdoc />
        virtual const TDescriptorLayout& descriptor(const UInt32& binding) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TDescriptorSet> allocate() const noexcept = 0;

        /// <inheritdoc />
        virtual Array<UniquePtr<TDescriptorSet>> allocate(const UInt32& descriptorSets) const noexcept = 0;

        /// <inheritdoc />
        virtual void free(const TDescriptorSet& descriptorSet) const noexcept = 0;

    private:
        virtual Array<const IDescriptorLayout*> getDescriptors() const noexcept override {
            auto descriptors = this->descriptors();
            return Array<const IDescriptorLayout*>(descriptors.begin(), descriptors.end());
        }

        virtual UniquePtr<IDescriptorSet> getDescriptorSet() const noexcept override {
            return this->allocate();
        }

        virtual Array<UniquePtr<IDescriptorSet>> getDescriptorSets(const UInt32& descriptorSets) const noexcept override {
            auto sets = this->allocate(descriptorSets);
            Array<UniquePtr<IDescriptorSet>> results;
            results.reserve(sets.size());
            std::move(sets.begin(), sets.end(), std::inserter(results, results.end()));
            return results;
        }

        virtual void releaseDescriptorSet(const IDescriptorSet& descriptorSet) const noexcept override {
            this->releaseDescriptorSet(dynamic_cast<const TDescriptorSet&>(descriptorSet));
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TDescriptorSetLayout, typename TParent, typename TDescriptorLayout = TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSet = TDescriptorSetLayout::descriptor_set_type> requires
        rtti::implements<TDescriptorSetLayout, DescriptorSetLayout<TDescriptorLayout, TDescriptorSet>>
    class DescriptorSetLayoutBuilder : public Builder<TDerived, TDescriptorSetLayout, TParent> {
    public:
        using Builder<TDerived, TDescriptorSetLayout, TParent>::Builder;

    public:
        virtual TDerived& addDescriptor(UniquePtr<TDescriptorLayout>&& layout) = 0;
        virtual TDerived& addDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors = 1) = 0;

    public:
        /// <summary>
        /// Adds an uniform/constant buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptorSize">The size of a single descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        virtual TDerived& addUniform(const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors = 1) {
            return this->addDescriptor(DescriptorType::Uniform, binding, descriptorSize, descriptors);
        }

        /// <summary>
        /// Adds a texel buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        virtual TDerived& addBuffer(const UInt32& binding, const UInt32& descriptors = 1, const bool& writable = false) {
            return this->addDescriptor(writable ? DescriptorType::WritableBuffer : DescriptorType::Buffer, binding, 0, descriptors);
        }

        /// <summary>
        /// Adds a storage/structured buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        virtual TDerived& addStorage(const UInt32& binding, const UInt32& descriptors = 1, const bool& writable = false) {
            return this->addDescriptor(writable ? DescriptorType::WritableStorage : DescriptorType::Storage, binding, 0, descriptors);
        }

        /// <summary>
        /// Adds an image/texture descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        virtual TDerived& addImage(const UInt32& binding, const UInt32& descriptors = 1, const bool& writable = false) {
            return this->addDescriptor(writable ? DescriptorType::WritableTexture : DescriptorType::Texture, binding, 0, descriptors);
        }

        /// <summary>
        /// Adds an input attachment descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        virtual TDerived& addInputAttachment(const UInt32& binding) {
            return this->addDescriptor(DescriptorType::InputAttachment, binding, 0);
        }

        /// <summary>
        /// Adds a sampler descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        virtual TDerived& addSampler(const UInt32& binding, const UInt32& descriptors = 1) {
            return this->addDescriptor(DescriptorType::Sampler, binding, 0, descriptors);
        }

        virtual void use(UniquePtr<TDescriptorLayout>&& layout) {
            this->addDescriptor(std::move(layout));
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
        virtual Array<const TPushConstantsRange*> ranges() const noexcept = 0;

    private:
        virtual Array<const IPushConstantsRange*> getRanges() const noexcept override {
            auto ranges = this->ranges();
            return Array<const IPushConstantsRange*>(ranges.begin(), ranges.end());
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TPushConstantsLayout, typename TParent, typename TPushConstantsRange = TPushConstantsLayout::push_constants_range_type> requires
        rtti::implements<TPushConstantsLayout, PushConstantsLayout<TPushConstantsRange>>
    class PushConstantsLayoutBuilder : public Builder<TDerived, TPushConstantsLayout, TParent> {
    public:
        using Builder<TDerived, TPushConstantsLayout, TParent>::Builder;

    public:
        virtual TDerived& addRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) = 0;
    };

    /// <summary>
    /// Represents a shader program, consisting of multiple <see cref="IShaderModule" />s.
    /// </summary>
    /// <typeparam name="TShaderModule">The type of the shader module. Must implement <see cref="IShaderModule"/>.</typeparam>
    template <typename TShaderModule> requires
        rtti::implements<TShaderModule, IShaderModule>
    class ShaderProgram : public IShaderProgram {
    public:
        using shader_module_type = TShaderModule;

    public:
        virtual ~ShaderProgram() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const TShaderModule*> modules() const noexcept = 0;

    private:
        virtual Array<const IShaderModule*> getModules() const noexcept {
            auto modules = this->modules();
            return Array<const IShaderModule*>(modules.begin(), modules.end());
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TShaderProgram, typename TParent, typename TShaderModule = typename TShaderProgram::shader_module_type> requires
        rtti::implements<TShaderProgram, ShaderProgram<TShaderModule>>
    class ShaderProgramBuilder : public Builder<TDerived, TShaderProgram, TParent> {
    public:
        using Builder<TDerived, TShaderProgram, TParent>::Builder;

    public:
        virtual TDerived& addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint = "main") = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TShaderProgram, typename TParent>
    class GraphicsShaderProgramBuilder : public ShaderProgramBuilder<TDerived, TShaderProgram, TParent> {
    public:
        using ShaderProgramBuilder<TDerived, TShaderProgram, TParent>::ShaderProgramBuilder;

    public:
        virtual TDerived& addVertexShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual TDerived& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TShaderProgram, typename TParent>
    class ComputeShaderProgramBuilder : public ShaderProgramBuilder<TDerived, TShaderProgram, TParent> {
    public:
        using ShaderProgramBuilder<TDerived, TShaderProgram, TParent>::ShaderProgramBuilder;

    public:
        virtual TDerived& addComputeShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
    };
    
    /// <summary>
    /// Represents a the layout of a <see cref="RenderPipeline" />.
    /// </summary>
    /// <typeparam name="TDescriptorSetLayout">The type of the descriptor set layout. Must implement <see cref="DescriptorSetLayout"/>.</typeparam>
    /// <typeparam name="TPushConstantsLayout">The type of the push constants layout. Must implement <see cref="PushConstantsLayout"/>.</typeparam>
    /// <typeparam name="TShaderProgram">The type of the shader program. Must implement <see cref="ShaderProgram"/>.</typeparam>
    template <typename TDescriptorSetLayout, typename TPushConstantsLayout, typename TShaderProgram, typename TDescriptorLayout = TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSet = TDescriptorSetLayout::descriptor_set_type, typename TPushConstantsRange = TPushConstantsLayout::push_constants_range_type, typename TShaderModule = TShaderProgram::shader_module_type> requires
        rtti::implements<TDescriptorSetLayout, DescriptorSetLayout<TDescriptorLayout, TDescriptorSet>> &&
        rtti::implements<TPushConstantsLayout, PushConstantsLayout<TPushConstantsRange>> &&
        rtti::implements<TShaderProgram, ShaderProgram<TShaderModule>>
    class PipelineLayout : public IPipelineLayout {
    public:
        using descriptor_set_layout_type = TDescriptorSetLayout;
        using push_constants_layout_type = TPushConstantsLayout;
        using shader_program_type = TShaderProgram;
        using descriptor_set_type = TDescriptorSet;

    public:
        virtual ~PipelineLayout() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const TShaderProgram& program() const noexcept = 0;

        /// <inheritdoc />
        virtual const TDescriptorSetLayout& descriptorSet(const UInt32& space) const = 0;

        /// <inheritdoc />
        virtual Array<const TDescriptorSetLayout*> descriptorSets() const noexcept = 0;

        /// <inheritdoc />
        virtual const TPushConstantsLayout* pushConstants() const noexcept = 0;

    private:
        virtual Array<const IDescriptorSetLayout*> getDescriptorSets() const noexcept override {
            auto layouts = this->descriptorSets();
            return Array<const IDescriptorSetLayout*>(layouts.begin(), layouts.end());
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TPipelineLayout, typename TParent, typename TDescriptorSetLayout = TPipelineLayout::descriptor_set_layout_type, typename TPushConstantsLayout = TPipelineLayout::push_constants_layout_type, typename TShaderProgram = TPipelineLayout::shader_program_type> requires
        rtti::implements<TPipelineLayout, PipelineLayout<TDescriptorSetLayout, TPushConstantsLayout, TShaderProgram>>
    class PipelineLayoutBuilder : public Builder<TDerived, TPipelineLayout, TParent> {
    public:
        using Builder<TDerived, TPipelineLayout, TParent>::Builder;

    public:
        virtual void use(UniquePtr<TShaderProgram>&& program) = 0;
        virtual void use(UniquePtr<TDescriptorSetLayout>&& layout) = 0;
        virtual void use(UniquePtr<TPushConstantsLayout>&& layout) = 0;
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
        virtual const TVertexBufferLayout& layout() const noexcept = 0;
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
        virtual const TIndexBufferLayout& layout() const noexcept = 0;
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
        virtual Array<const TVertexBufferLayout*> vertexBufferLayouts() const noexcept = 0;

        /// <inheritdoc />
        virtual const TVertexBufferLayout& vertexBufferLayout(const UInt32& binding) const = 0;

        /// <inheritdoc />
        virtual const TIndexBufferLayout& indexBufferLayout() const = 0;

    private:
        virtual Array<const IVertexBufferLayout*> getVertexBufferLayouts() const noexcept override {
            auto layouts = this->vertexBufferLayouts();
            return Array<const IVertexBufferLayout*>(layouts.begin(), layouts.end());
        }
    };

    /// <summary>
    /// Builds a <see cref="InputAssembler" />.
    /// </summary>
    template <typename TDerived, typename TInputAssembler, typename TParent, typename TVertexBufferLayout = TInputAssembler::vertex_buffer_layout_type, typename TIndexBufferLayout = TInputAssembler::index_buffer_layout_type> requires
        rtti::implements<TInputAssembler, InputAssembler<TVertexBufferLayout, TIndexBufferLayout>>
    class InputAssemblerBuilder : public Builder<TDerived, TInputAssembler, TParent, SharedPtr<TInputAssembler>> {
    public:
        using Builder<TDerived, TInputAssembler, TParent, SharedPtr<TInputAssembler>>::Builder;

    public:
        /// <summary>
        /// Specifies the topology to initialize the input assembler with.
        /// </summary>
        /// <param name="topology">The topology to initialize the input assembler with.</param>
        virtual TDerived& withTopology(const PrimitiveTopology& topology) = 0;

        /// <summary>
        /// Adds a vertex buffer layout to the input assembler. Can be called multiple times.
        /// </summary>
        /// <param name="layout">The layout to add to the input assembler.</param>
        virtual void use(UniquePtr<TVertexBufferLayout>&& layout) = 0;

        /// <summary>
        /// Adds an index buffer layout to the input assembler. Can only be called once.
        /// </summary>
        /// <param name="layout"></param>
        /// <exception cref="RuntimeException">Thrown if another index buffer layout has already been specified.</excpetion>
        virtual void use(UniquePtr<TIndexBufferLayout>&& layout) = 0;
    };

    /// <summary>
    /// Represents a pipeline state.
    /// </summary>
    /// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="PipelineLayout"/>.</typeparam>
    /// <typeparam name="TShaderProgram">The type of the shader program. Must implement <see cref="ShaderProgram"/>.</typeparam>
    /// <typeparam name="TDescriptorSetLayout">The type of the descriptor set layout. Must implement <see cref="DescriptorSetLayout"/>.</typeparam>
    /// <typeparam name="TDescriptorSet">The type of the descriptor set. Must implement <see cref="DescriptorSet"/>.</typeparam>
    /// <seealso cref="RenderPipeline" />
    /// <seealso cref="ComputePipeline" />
    template <typename TPipelineLayout, typename TDescriptorSetLayout = typename TPipelineLayout::descriptor_set_layout_type, typename TPushConstantsLayout = typename TPipelineLayout::push_constants_layout_type, typename TShaderProgram = typename TPipelineLayout::shader_program_type, typename TDescriptorSet = typename TDescriptorSetLayout::descriptor_set_type> requires 
        rtti::implements<TPipelineLayout, PipelineLayout<TDescriptorSetLayout, TPushConstantsLayout, TShaderProgram>>
    class Pipeline : public IPipeline {
    public:
        using pipeline_layout_type = TPipelineLayout;

    public:
        virtual ~Pipeline() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const TPipelineLayout& layout() const noexcept = 0;
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
    template <typename TBuffer, typename TVertexBuffer, typename TIndexBuffer, typename TImage, typename TBarrier, typename TPipeline, typename TPipelineLayout = TPipeline::pipeline_layout_type, typename TDescriptorSet = TPipelineLayout::descriptor_set_type, typename TPushConstantsLayout = TPipelineLayout::push_constants_layout_type> requires
        rtti::implements<TBarrier, Barrier<TBuffer, TImage>> &&
        std::derived_from<TPipeline, Pipeline<TPipelineLayout>>
    class CommandBuffer : public ICommandBuffer {
        using ICommandBuffer::begin;
        using ICommandBuffer::end;
        using ICommandBuffer::dispatch;
        using ICommandBuffer::draw;
        using ICommandBuffer::drawIndexed;

    public:
        using buffer_type = TBuffer;
        using vertex_buffer_type = TVertexBuffer;
        using index_buffer_type = TIndexBuffer;
        using image_type = TImage;
        using barrier_type = TBarrier;
        using pipeline_type = TPipeline;

    public:
        virtual ~CommandBuffer() noexcept = default;

    public:
        /// <inheritdoc />
        virtual void barrier(const TBarrier& barrier, const bool& invert = false) const noexcept = 0;

        /// <inheritdoc />
        virtual void generateMipMaps(TImage& image) noexcept = 0;

        /// <inheritdoc />
        virtual void transfer(const TBuffer& source, const TBuffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(const TBuffer& source, const TImage& target, const UInt32& sourceElement = 0, const UInt32& firstSubresource = 0, const UInt32& elements = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(const TImage& source, const TImage& target, const UInt32& sourceSubresource = 0, const UInt32& targetSubresource = 0, const UInt32& subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void transfer(const TImage& source, const TBuffer& target, const UInt32& firstSubresource = 0, const UInt32& targetElement = 0, const UInt32& subresources = 1) const = 0;

        /// <inheritdoc />
        virtual void use(const TPipeline& pipeline) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(const TDescriptorSet& descriptorSet) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(const TVertexBuffer& buffer) const noexcept = 0;

        /// <inheritdoc />
        virtual void bind(const TIndexBuffer& buffer) const noexcept = 0;

        /// <inheritdoc />
        virtual void pushConstants(const TPushConstantsLayout& layout, const void* const memory) const noexcept = 0;

        /// <inheritdoc />
        virtual void draw(const TVertexBuffer& vertexBuffer, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->draw(vertexBuffer.elements(), instances, firstVertex, firstInstance);
        }

        /// <inheritdoc />
        virtual void drawIndexed(const TIndexBuffer& indexBuffer, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const {
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }

        /// <inheritdoc />
        virtual void drawIndexed(const TVertexBuffer& vertexBuffer, const TIndexBuffer& indexBuffer, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }

    private:
        virtual void cmdBarrier(const IBarrier& barrier, const bool& invert) const noexcept override { 
            this->barrier(dynamic_cast<const TBarrier&>(barrier), invert);
        }

        virtual void cmdGenerateMipMaps(IImage& image) noexcept override { 
            this->generateMipMaps(dynamic_cast<TImage&>(image));
        }

        virtual void cmdTransfer(const IBuffer& source, const IBuffer& target, const UInt32& sourceElement, const UInt32& targetElement, const UInt32& elements) const override { 
            this->transfer(dynamic_cast<const TBuffer&>(source), dynamic_cast<const TBuffer&>(target), sourceElement, targetElement, elements);
        }
        
        virtual void cmdTransfer(const IBuffer& source, const IImage& target, const UInt32& sourceElement, const UInt32& firstSubresource, const UInt32& elements) const override { 
            this->transfer(dynamic_cast<const TBuffer&>(source), dynamic_cast<const TImage&>(target), sourceElement, firstSubresource, elements);
        }
        
        virtual void cmdTransfer(const IImage& source, const IImage& target, const UInt32& sourceSubresource, const UInt32& targetSubresource, const UInt32& subresources) const override {
            this->transfer(dynamic_cast<const TImage&>(source), dynamic_cast<const TImage&>(target), sourceSubresource, targetSubresource, subresources);
        }

        virtual void cmdTransfer(const IImage& source, const IBuffer& target, const UInt32& firstSubresource, const UInt32& targetElement, const UInt32& subresources) const override {
            this->transfer(dynamic_cast<const TImage&>(source), dynamic_cast<const TBuffer&>(target), firstSubresource, targetElement, subresources);
        }

        virtual void cmdUse(const IPipeline& pipeline) const noexcept override { 
            this->use(dynamic_cast<const TPipeline&>(pipeline));
        }

        virtual void cmdBind(const IDescriptorSet& descriptorSet) const noexcept override { 
            this->bind(dynamic_cast<const TDescriptorSet&>(descriptorSet));
        }
        
        virtual void cmdBind(const IVertexBuffer& buffer) const noexcept override { 
            this->bind(dynamic_cast<const TVertexBuffer&>(buffer));
        }

        virtual void cmdBind(const IIndexBuffer& buffer) const noexcept override { 
            this->bind(dynamic_cast<const TVertexBuffer&>(buffer));
        }
        
        virtual void cmdPushConstants(const IPushConstantsLayout& layout, const void* const memory) const noexcept override { 
            this->pushConstants(dynamic_cast<const TPushConstantsLayout&>(layout), memory);
        }
        
        virtual void cmdDraw(const IVertexBuffer& vertexBuffer, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const override { 
            this->draw(dynamic_cast<const TVertexBuffer&>(vertexBuffer), instances, firstVertex, firstInstance);
        }
        
        virtual void cmdDrawIndexed(const IIndexBuffer& indexBuffer, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const override { 
            this->drawIndexed(dynamic_cast<const TIndexBuffer&>(indexBuffer), instances, firstIndex, vertexOffset, firstInstance);
        }
        
        virtual void cmdDrawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const override { 
            this->drawIndexed(dynamic_cast<const TVertexBuffer&>(vertexBuffer), dynamic_cast<const TIndexBuffer&>(indexBuffer), instances, firstIndex, vertexOffset, firstInstance);
        }
    };

    /// <summary>
    /// Represents a graphics <see cref="Pipeline" />.
    /// </summary>
    /// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="PipelineLayout"/>.</typeparam>
    /// <typeparam name="TInputAssembler">The type of the input assembler state. Must implement <see cref="InputAssembler"/>.</typeparam>
    /// <typeparam name="TVertexBufferInterface">The type of the vertex buffer interface. Must inherit from <see cref="VertexBuffer"/>.</typeparam>
    /// <typeparam name="TIndexBufferInterface">The type of the index buffer interface. Must inherit from <see cref="IndexBuffer"/>.</typeparam>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout"/>.</typeparam>
    /// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout"/>.</typeparam>
    /// <seealso cref="IRenderPipelineBuilder" />
    template <typename TPipelineLayout, typename TInputAssembler, typename TVertexBufferInterface, typename TIndexBufferInterface, typename TVertexBufferLayout = TVertexBufferInterface::vertex_buffer_layout_type, typename TIndexBufferLayout = TIndexBufferInterface::index_buffer_layout_type> requires
        rtti::implements<TInputAssembler, InputAssembler<TVertexBufferLayout, TIndexBufferLayout>> &&
        std::derived_from<TVertexBufferInterface, VertexBuffer<TVertexBufferLayout>> &&
        std::derived_from<TIndexBufferInterface, IndexBuffer<TIndexBufferLayout>>
    class RenderPipeline : public IRenderPipeline, public Pipeline<TPipelineLayout> {
    public:
        using vertex_buffer_interface_type = TVertexBufferInterface;
        using index_buffer_interface_type = TIndexBufferInterface;
        using input_assembler_type = TInputAssembler;

    public:
        virtual ~RenderPipeline() noexcept = default;

    public:
        /// <inheritdoc />
        virtual SharedPtr<TInputAssembler> inputAssembler() const noexcept = 0;

    private:
        virtual SharedPtr<IInputAssembler> getInputAssembler() const noexcept override {
            return this->inputAssembler();
        }
    };

    /// <summary>
    /// Describes the interface of a render pipeline builder.
    /// </summary>
    /// <seealso cref="RenderPipeline" />
    template <typename TDerived, typename TRenderPipeline, typename TInputAssembler = TRenderPipeline::input_assembler_type, typename TPipelineLayout = TRenderPipeline::pipeline_layout_type, typename TVertexBufferInterface = TRenderPipeline::vertex_buffer_interface_type, typename TIndexBufferInterface = TRenderPipeline::index_buffer_interface_type> requires
        rtti::implements<TRenderPipeline, RenderPipeline<TPipelineLayout, TInputAssembler, TVertexBufferInterface, TIndexBufferInterface>>
    class RenderPipelineBuilder : public Builder<TDerived, TRenderPipeline> {
    public:
        using Builder<TDerived, TRenderPipeline>::Builder;

    public:
        /// <summary>
        /// Uses the provided pipeline layout to initialize the render pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="layout">The pipeline layout to initialize the render pipeline with.</param>
        virtual void use(UniquePtr<TPipelineLayout>&& layout) = 0;

        /// <summary>
        /// Uses the provided rasterizer state to initialize the render pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="rasterizer">The rasterizer state to initialize the render pipeline with.</param>
        virtual void use(SharedPtr<IRasterizer> rasterizer) = 0;

        /// <summary>
        /// Uses the provided input assembler state to initialize the render pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="inputAssembler">The input assembler state to initialize the render pipeline with.</param>
        virtual void use(SharedPtr<TInputAssembler> inputAssembler) = 0;

        /// <summary>
        /// Uses the provided viewport to initialize the render pipeline. Can be invoked multiple times.
        /// </summary>
        /// <param name="viewport">A viewport to initialize the render pipeline with.</param>
        virtual void use(SharedPtr<IViewport> viewport) = 0;

        /// <summary>
        /// Uses the provided scissor to initialize the render pipeline. Can be invoked multiple times.
        /// </summary>
        /// <param name="scissor">A scissor to initialize the render pipeline with.</param>
        virtual void use(SharedPtr<IScissor> scissor) = 0;

        /// <summary>
        /// Enables <i>Alpha-to-Coverage</i> multi-sampling on the pipeline.
        /// </summary>
        /// <remarks>
        /// For more information on <i>Alpha-to-Coverage</i> multi-sampling see the remarks of <see cref="IRenderPipeline::alphaToCoverage" />.
        /// </remarks>
        /// <param name="enable">Whether or not to use <i>Alpha-to-Coverage</i> multi-sampling.</param>
        virtual TDerived& enableAlphaToCoverage(const bool& enable = true) = 0;
    };

    /// <summary>
    /// Represents a compute <see cref="Pipeline" />.
    /// </summary>
    /// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="PipelineLayout"/>.</typeparam>
    /// <seealso cref="IComputePipelineBuilder" />
    template <typename TPipelineLayout>
    class ComputePipeline : public IComputePipeline, public Pipeline<TPipelineLayout> {
    public:
        virtual ~ComputePipeline() noexcept = default;
    };

    /// <summary>
    /// Describes the interface of a render pipeline builder.
    /// </summary>
    /// <seealso cref="ComputePipeline" />
    template <typename TDerived, typename TComputePipeline, typename TPipelineLayout = TComputePipeline::pipeline_layout_type> requires
        rtti::implements<TComputePipeline, ComputePipeline<TPipelineLayout>>
    class ComputePipelineBuilder : public Builder<TDerived, TComputePipeline> {
    public:
        using Builder<TDerived, TComputePipeline>::Builder;

    public:
        /// <summary>
        /// Uses the provided pipeline layout to initialize the compute pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="layout">The pipeline layout to initialize the compute pipeline with.</param>
        virtual void use(UniquePtr<TPipelineLayout>&& layout) = 0;
    };

    /// <summary>
    /// Stores the images for the output attachments for a back buffer of a <see cref="RenderPass" />, as well as a <see cref="CommandBuffer" /> instance, that records draw commands.
    /// </summary>
    /// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="CommandBuffer"/>.</typeparam>
    /// <seealso cref="RenderTarget" />
    template <typename TCommandBuffer, typename TBuffer = TCommandBuffer::buffer_type, typename TVertexBuffer = TCommandBuffer::vertex_buffer_type, typename TIndexBuffer = TCommandBuffer::index_buffer_type, typename TImage = TCommandBuffer::image_type, typename TBarrier = TCommandBuffer::barrier_type, typename TPipeline = TCommandBuffer::pipeline_type> requires
        rtti::implements<TCommandBuffer, CommandBuffer<TBuffer, TVertexBuffer, TIndexBuffer, TImage, TBarrier, TPipeline>>
    class FrameBuffer : public IFrameBuffer {
    public:
        using command_buffer_type = TCommandBuffer;

    public:
        virtual ~FrameBuffer() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const TCommandBuffer*> commandBuffers() const noexcept = 0;

        /// <inheritdoc />
        virtual const TCommandBuffer& commandBuffer(const UInt32& index) const = 0;

        /// <inheritdoc />
        virtual Array<const TImage*> images() const noexcept = 0;

        /// <inheritdoc />
        virtual const TImage& image(const UInt32& location) const = 0;

    private:
        virtual Array<const ICommandBuffer*> getCommandBuffers() const noexcept override {
            auto commandBuffers = this->commandBuffers();
            return Array<const ICommandBuffer*>(commandBuffers.begin(), commandBuffers.end());
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
    /// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="CommandBuffer"/>.</typeparam>
    template <typename TFrameBuffer, typename TCommandBuffer = typename TFrameBuffer::command_buffer_type> requires
        rtti::implements<TFrameBuffer, FrameBuffer<TCommandBuffer>>
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
        virtual const TFrameBuffer& frameBuffer(const UInt32& buffer) const = 0;
    };

    /// <summary>
    /// Represents a mapping between a set of <see cref="IRenderTarget" /> instances and the input attachments of a <see cref="RenderPass" />.
    /// </summary>
    /// <typeparam name="TInputAttachmentMappingSource">The type of the input attachment mapping source. Must implement <see cref="IInputAttachmentMappingSource" />.</typeparam>
    template <typename TInputAttachmentMappingSource, typename TFrameBuffer = TInputAttachmentMappingSource::frame_buffer_type> requires
        rtti::implements<TInputAttachmentMappingSource, IInputAttachmentMappingSource<TFrameBuffer>>
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
        virtual const TInputAttachmentMappingSource* inputAttachmentSource() const noexcept = 0;

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
    /// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="PipelineLayout" />.</typeparam>
    /// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="FrameBuffer" />.</typeparam>
    /// <typeparam name="TInputAttachmentMapping">The type of the input attachment mapping. Must implement <see cref="IInputAttachmentMapping" />.</typeparam>
    /// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="CommandBuffer"/>.</typeparam>
    // TODO: Add concepts to constrain render pipeline and input attachments properly.
    template <typename TRenderPipeline, typename TFrameBuffer, typename TInputAttachmentMapping, typename TPipelineLayout = TRenderPipeline::pipeline_layout_type, typename TDescriptorSet = TPipelineLayout::descriptor_set_type, typename TCommandBuffer = TFrameBuffer::command_buffer_type> requires
        rtti::implements<TFrameBuffer, FrameBuffer<TCommandBuffer>> /*&&
        rtti::implements<TRenderPipeline, RenderPipeline<TPipelineLayout>> &&
        rtti::implements<TInputAttachmentMapping, IInputAttachmentMapping<TDerived>>*/
    class RenderPass : public virtual StateResource, public IRenderPass, public IInputAttachmentMappingSource<TFrameBuffer> {
    public:
        using frame_buffer_type = TFrameBuffer;
        using render_pipeline_type = TRenderPipeline;
        using input_attachment_mapping_type = TInputAttachmentMapping;

    public:
        virtual ~RenderPass() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const TFrameBuffer& activeFrameBuffer() const = 0;

        /// <inheritdoc />
        virtual Array<const TFrameBuffer*> frameBuffers() const noexcept = 0;

        /// <inheritdoc />
        virtual const TRenderPipeline& pipeline(const UInt32& id) const = 0;

        /// <inheritdoc />
        virtual Array<const TRenderPipeline*> pipelines() const noexcept = 0;

        /// <inheritdoc />
        virtual Span<const TInputAttachmentMapping> inputAttachments() const noexcept = 0;

        /// <inheritdoc />
        virtual void updateAttachments(const TDescriptorSet& descriptorSet) const = 0;

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
            this->updateAttachments(dynamic_cast<const TDescriptorSet&>(descriptorSet));
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TRenderPass, typename TRenderPipeline = TRenderPass::render_pipeline_type, typename TFrameBuffer = TRenderPass::frame_buffer_type, typename TInputAttachmentMapping = typename TRenderPass::input_attachment_mapping_type> requires
        rtti::implements<TRenderPass, RenderPass<TRenderPipeline, TFrameBuffer, TInputAttachmentMapping>>
    class RenderPassBuilder : public Builder<TDerived, TRenderPass> {
    public:
        using Builder<TDerived, TRenderPass>::Builder;

    public:
        virtual void use(RenderTarget&& target) = 0;
        virtual void use(TInputAttachmentMapping&& inputAttachment) = 0;

    public:
        virtual TDerived& commandBuffers(const UInt32& count) = 0;
        virtual TDerived& renderTarget(const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;
        virtual TDerived& renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;
        virtual TDerived& renderTarget(TInputAttachmentMapping& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;
        virtual TDerived& renderTarget(TInputAttachmentMapping& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;
        virtual TDerived& setMultiSamplingLevel(const MultiSamplingLevel& samples = MultiSamplingLevel::x4) = 0;
        virtual TDerived& inputAttachment(const TInputAttachmentMapping& inputAttachment) = 0;
        virtual TDerived& inputAttachment(const UInt32& inputLocation, const TRenderPass& renderPass, const UInt32& outputLocation) = 0;
        virtual TDerived& inputAttachment(const UInt32& inputLocation, const TRenderPass& renderPass, const RenderTarget& renderTarget) = 0;
    };

    /// <summary>
    /// Represents a swap chain, i.e. a chain of multiple <see cref="IImage" /> instances, that can be presented to a <see cref="ISurface" />.
    /// </summary>
    /// <typeparam name="TImageInterface">The type of the image interface. Must inherit from <see cref="IImage"/>.</typeparam>
    template <typename TImageInterface> requires
        std::derived_from<TImageInterface, IImage>
    class SwapChain : public ISwapChain {
    public:
        using image_interface_type = TImageInterface;

    public:
        virtual ~SwapChain() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const TImageInterface*> images() const noexcept = 0;

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
    template <typename TCommandBuffer, typename TBuffer = TCommandBuffer::buffer_type, typename TVertexBuffer = TCommandBuffer::vertex_buffer_type, typename TIndexBuffer = TCommandBuffer::index_buffer_type, typename TImage = TCommandBuffer::image_type, typename TBarrier = TCommandBuffer::barrier_type, typename TPipeline = TCommandBuffer::pipeline_type> requires
        rtti::implements<TCommandBuffer, CommandBuffer<TBuffer, TVertexBuffer, TIndexBuffer, TImage, TBarrier, TPipeline>>
    class CommandQueue : public ICommandQueue {
    public:
        using command_buffer_type = TCommandBuffer;

    public:
        virtual ~CommandQueue() noexcept = default;

    public:
        /// <inheritdoc />
        virtual UniquePtr<TCommandBuffer> createCommandBuffer(const bool& beginRecording = false) const = 0;

        /// <inheritdoc />
        virtual UInt64 submit(const TCommandBuffer& commandBuffer) const = 0;

        /// <inheritdoc />
        virtual UInt64 submit(const Array<const TCommandBuffer*>& commandBuffers) const = 0;

    private:
        virtual UniquePtr<ICommandBuffer> getCommandBuffer(const bool& beginRecording) const override {
            return this->createCommandBuffer(beginRecording);
        }

        virtual UInt64 submitCommandBuffer(const ICommandBuffer& commandBuffer) const override {
            return this->submit(dynamic_cast<const TCommandBuffer&>(commandBuffer));
        }

        virtual UInt64 submitCommandBuffers(const Array<const ICommandBuffer*>& commandBuffers) const override {
            Array<const TCommandBuffer*> buffers;
            buffers.reserve(commandBuffers.size());
            std::transform(commandBuffers.begin(), commandBuffers.end(), buffers.begin(), [](auto buffer) { return dynamic_cast<const TCommandBuffer*>(buffer); });
            return this->submit(buffers);
        }
    };

    /// <summary>
    /// Describes a factory that creates objects for a <see cref="GraphicsDevice" />.
    /// </summary>
    /// <remarks>
    /// Initial resource states depend on the provided <see cref="BufferUsage" />. *Staging* and *Dynamic* resources are always initialized in <see cref="ResourceState::GenericRead" /> 
    /// state, while *Resource* and *Readback* resources are initialized in <see cref="ResourceState::CopyDestination" /> state. Images (and attachments) can only be used as *Resource*,
    /// so they are always created as copy destination. Images require a transfer from a buffer, followed by an explicit <see cref="Barrier" /> into the state required by the shader.
    /// Attachments are implicitly transitioned at the beginning and end of a render pass, so you typically do not need to create an explicit barrier for them.
    /// </remarks>
    /// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout" />.</typeparam>
    /// <typeparam name="TVertexBuffer">The type of the vertex buffer. Must implement <see cref="VertexBuffer" />.</typeparam>
    /// <typeparam name="TIndexBuffer">The type of the index buffer. Must implement <see cref="IndexBuffer" />.</typeparam>
    /// <typeparam name="TImage">The type of the image. Must inherit from <see cref="IImage"/>.</typeparam>
    /// <typeparam name="TBuffer">The type of the buffer. Must inherit from <see cref="IBuffer"/>.</typeparam>
    /// <typeparam name="TSampler">The type of the sampler. Must inherit from <see cref="ISampler"/>.</typeparam>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout"/>.</typeparam>
    /// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout"/>.</typeparam>
    template <typename TDescriptorLayout, typename TBuffer, typename TVertexBuffer, typename TIndexBuffer, typename TImage, typename TSampler, typename TVertexBufferLayout = TVertexBuffer::vertex_buffer_layout_type, typename TIndexBufferLayout = TIndexBuffer::index_buffer_layout_type> requires
        rtti::implements<TDescriptorLayout, IDescriptorLayout> &&
        std::derived_from<TVertexBuffer, VertexBuffer<TVertexBufferLayout>> &&
        std::derived_from<TIndexBuffer, IndexBuffer<TIndexBufferLayout>> &&
        std::derived_from<TImage, IImage> &&
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TSampler, ISampler>
    class GraphicsFactory : public IGraphicsFactory {
    public:
        using descriptor_layout_type = TDescriptorLayout;
        using vertex_buffer_type = TVertexBuffer;
        using index_buffer_type = TIndexBuffer;
        using buffer_type = TBuffer;
        using image_type = TImage;
        using sampler_type = TSampler;

    public:
        virtual ~GraphicsFactory() noexcept = default;

    public:
        /// <inheritdoc />
        virtual UniquePtr<TBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1, const bool& allowWrite = false) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TVertexBuffer> createVertexBuffer(const TVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TIndexBuffer> createIndexBuffer(const TIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TImage> createTexture(const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const = 0;

        /// <inheritdoc />
        virtual Array<UniquePtr<TImage>> createTextures(const UInt32& elements, const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32 & layers = 1, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const = 0;

        /// <inheritdoc />
        virtual UniquePtr<TSampler> createSampler(const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const = 0;

        /// <inheritdoc />
        virtual Array<UniquePtr<TSampler>> createSamplers(const UInt32& elements, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const = 0;

    private:
        virtual UniquePtr<IBuffer> getBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements, const bool& allowWrite) const override { 
            return this->createBuffer(type, usage, elementSize, elements, allowWrite);
        }

        virtual UniquePtr<IVertexBuffer> getVertexBuffer(const IVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override { 
            return this->createVertexBuffer(dynamic_cast<const TVertexBufferLayout&>(layout), usage, elements);
        }
        
        virtual UniquePtr<IIndexBuffer> getIndexBuffer(const IIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const override {
            return this->createIndexBuffer(dynamic_cast<const TIndexBufferLayout&>(layout), usage, elements);
        }

        virtual UniquePtr<IImage> getAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples) const override { 
            return this->createAttachment(format, size, samples);
        }
        
        virtual UniquePtr<IImage> getTexture(const Format& format, const Size3d& size, const ImageDimensions& dimension, const UInt32& levels, const UInt32& layers, const MultiSamplingLevel& samples, const bool& allowWrite) const override { 
            return this->createTexture(format, size, dimension, levels, layers, samples, allowWrite);
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
    /// <typeparam name="TRenderPipeline">The type of the render pipeline. Must implement <see cref="RenderPipeline" />.</typeparam>
    /// <typeparam name="TImage">The type of the swap chain image. Must inherit from <see cref="IImage" />.</typeparam>
    /// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="FrameBuffer" />.</typeparam>
    /// <typeparam name="TInputAttachmentMapping">The type of the input attachment mapping. Must implement <see cref="IInputAttachmentMapping" />.</typeparam>
    /// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="CommandBuffer" />.</typeparam>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout" />.</typeparam>
    /// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout" />.</typeparam>
    /// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout" />.</typeparam>
    template <typename TFactory, typename TSurface, typename TGraphicsAdapter, typename TSwapChain, typename TCommandQueue, typename TRenderPass, typename TFrameBuffer = TRenderPass::frame_buffer_type, typename TRenderPipeline = TRenderPass::render_pipeline_type, typename TInputAttachmentMapping = TRenderPass::input_attachment_mapping_type, typename TCommandBuffer = TCommandQueue::command_buffer_type, typename TImage = TFactory::image_type, typename TVertexBuffer = TFactory::vertex_buffer_type, typename TIndexBuffer = TFactory::index_buffer_type, typename TDescriptorLayout = TFactory::descriptor_layout_type, typename TBuffer = TFactory::buffer_type, typename TSampler = TFactory::sampler_type> requires
        rtti::implements<TSurface, ISurface> &&
        rtti::implements<TGraphicsAdapter, IGraphicsAdapter> &&
        rtti::implements<TSwapChain, SwapChain<TImage>> &&
        rtti::implements<TCommandQueue, CommandQueue<TCommandBuffer>> &&
        rtti::implements<TFactory, GraphicsFactory<TDescriptorLayout, TBuffer, TVertexBuffer, TIndexBuffer, TImage, TSampler>> &&
        rtti::implements<TRenderPass, RenderPass<TRenderPipeline, TFrameBuffer, TInputAttachmentMapping>>
    class GraphicsDevice : public IGraphicsDevice {
    public:
        using surface_type = TSurface;
        using adapter_type = TGraphicsAdapter;
        using swap_chain_type = TSwapChain;
        using command_queue_type = TCommandQueue;
        using factory_type = TFactory;
        using frame_buffer_type = TFrameBuffer;
        using render_pass_type = TRenderPass;

    public:
        virtual ~GraphicsDevice() noexcept = default;

    public:
        /// <inheritdoc />
        virtual const TSurface& surface() const noexcept = 0;

        /// <inheritdoc />
        virtual const TGraphicsAdapter& adapter() const noexcept = 0;

        /// <inheritdoc />
        virtual const TSwapChain& swapChain() const noexcept = 0;

        /// <inheritdoc />
        virtual const TFactory& factory() const noexcept = 0;

        /// <inheritdoc />
        virtual const TCommandQueue& graphicsQueue() const noexcept = 0;

        /// <inheritdoc />
        virtual const TCommandQueue& transferQueue() const noexcept = 0;

        /// <inheritdoc />
        virtual const TCommandQueue& bufferQueue() const noexcept = 0;

        /// <inheritdoc />
        virtual const TCommandQueue& computeQueue() const noexcept = 0;
    };

    /// <summary>
    /// Defines a back-end, that provides a device instance for a certain surface and graphics adapter.
    /// </summary>
    /// <typeparam name="TGraphicsAdapter">The type of the backend derived from the interface. Must implement <see cref="IRenderBackend" />.</typeparam>
    /// <typeparam name="TGraphicsAdapter">The type of the graphics adapter. Must implement <see cref="IGraphicsAdapter" />.</typeparam>
    /// <typeparam name="TSurface">The type of the surface. Must implement <see cref="ISurface" />.</typeparam>
    /// <typeparam name="TSwapChain">The type of the swap chain. Must implement <see cref="SwapChain" />.</typeparam>
    /// <typeparam name="TGraphicsDevice">The type of the graphics device. Must implement <see cref="GraphicsDevice" />.</typeparam>
    /// <typeparam name="TCommandQueue">The type of the command queue. Must implement <see cref="CommandQueue" />.</typeparam>
    /// <typeparam name="TFactory">The type of the graphics factory. Must implement <see cref="GraphicsFactory" />.</typeparam>
    template <typename TBackend, typename TGraphicsDevice, typename TGraphicsAdapter = TGraphicsDevice::adapter_type, typename TSurface = TGraphicsDevice::surface_type, typename TSwapChain = TGraphicsDevice::swap_chain_type, typename TFrameBuffer = TGraphicsDevice::frame_buffer_type, typename TCommandQueue = TGraphicsDevice::command_queue_type, typename TFactory = TGraphicsDevice::factory_type, typename TRenderPass = TGraphicsDevice::render_pass_type> requires
        rtti::implements<TGraphicsDevice, GraphicsDevice<TFactory, TSurface, TGraphicsAdapter, TSwapChain, TCommandQueue, TRenderPass>>
    class RenderBackend : public IRenderBackend {
    public:
        virtual ~RenderBackend() noexcept = default;

    public:
        /// <inheritdoc />
        virtual Array<const TGraphicsAdapter*> listAdapters() const = 0;

        /// <inheritdoc />
        virtual const TGraphicsAdapter* findAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const = 0;

        /// <inheritdoc />
        virtual void registerDevice(String name, UniquePtr<TGraphicsDevice>&& device) = 0;

        /// <summary>
        /// Creates a new graphics device.
        /// </summary>
        /// <param name="_args">The arguments that are passed to the graphics device constructor.</param>
        /// <returns>A pointer of the created graphics device instance.</returns>
        template <typename ...TArgs>
        void createDevice(String name, const TGraphicsAdapter& adapter, UniquePtr<TSurface>&& surface, TArgs&&... _args) {
            this->registerDevice(name, std::move(makeUnique<TGraphicsDevice>(static_cast<const TBackend&>(*this), adapter, std::move(surface), std::forward<TArgs>(_args)...)));
        }

        /// <summary>
        /// Destroys and removes a device from the backend.
        /// </summary>
        /// <param name="name">The name of the device.</param>
        virtual void releaseDevice(const String& name) = 0;

        /// <inheritdoc />
        virtual TGraphicsDevice* device(const String& name) noexcept = 0;

        /// <inheritdoc />
        virtual const TGraphicsDevice* device(const String& name) const noexcept = 0;

        /// <inheritdoc />
        virtual const TGraphicsDevice* operator[](const String& name) const noexcept {
            return this->device(name);
        };

        /// <inheritdoc />
        virtual TGraphicsDevice* operator[](const String& name) noexcept {
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