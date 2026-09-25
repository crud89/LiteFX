#pragma once

#include <litefx/rendering_api.hpp>
#include <litefx/rendering_formatters.hpp>

// NOLINTBEGIN(bugprone-derived-method-shadowing-base-method)

namespace LiteFX::Rendering {
    using namespace LiteFX;
    using namespace LiteFX::Math;

    /// @brief A barrier used for GPU resource synchronization.
    ///
    /// @see IBarrier
    template <typename TBuffer, typename TImage> requires
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TImage, IImage>
    class Barrier : public IBarrier {
    public:
        using IBarrier::transition;

        using buffer_type = TBuffer;
        using image_type = TImage;

    protected:
        Barrier() noexcept = default;
        Barrier(const Barrier&) = default;
        Barrier(Barrier&&) noexcept = default;
        Barrier& operator=(const Barrier&) = default;
        Barrier& operator=(Barrier&&) noexcept = default;

    public:
        constexpr ~Barrier() noexcept override = default;

    public:
        /// @copydoc IBarrier::transition(const IBuffer&, ResourceAccess, ResourceAccess)
        constexpr virtual void transition(const buffer_type& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;

        /// @copydoc IBarrier::transition(const IBuffer&, UInt32, ResourceAccess, ResourceAccess)
        constexpr virtual void transition(const buffer_type& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) = 0;

        /// @copydoc IBarrier::transition(const IImage&, ResourceAccess, ResourceAccess, ImageLayout)
        constexpr virtual void transition(const image_type& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;

        /// @copydoc IBarrier::transition(const IImage&, ResourceAccess, ResourceAccess, ImageLayout, ImageLayout)
        constexpr virtual void transition(const image_type& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;

        /// @copydoc IBarrier::transition(const IImage&, UInt32, UInt32, UInt32, UInt32, ResourceAccess, ResourceAccess, ImageLayout)
        constexpr virtual void transition(const image_type& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) = 0;

        /// @copydoc IBarrier::transition(const IImage&, UInt32, UInt32, UInt32, UInt32, ResourceAccess, ResourceAccess, ImageLayout, ImageLayout)
        constexpr virtual void transition(const image_type& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) = 0;

    private:
        constexpr void doTransition(const IBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter) override {
            this->transition(dynamic_cast<const buffer_type&>(buffer), accessBefore, accessAfter);
        }

        constexpr void doTransition(const IBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter) override {
            this->transition(dynamic_cast<const buffer_type&>(buffer), element, accessBefore, accessAfter);
        }

        constexpr void doTransition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override {
            this->transition(dynamic_cast<const image_type&>(image), accessBefore, accessAfter, layout);
        }

        constexpr void doTransition(const IImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override {
            this->transition(dynamic_cast<const image_type&>(image), accessBefore, accessAfter, fromLayout, toLayout);
        }

        constexpr void doTransition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout) override {
            this->transition(dynamic_cast<const image_type&>(image), level, levels, layer, layers, plane, accessBefore, accessAfter, layout);
        }

        constexpr void doTransition(const IImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout) override {
            this->transition(dynamic_cast<const image_type&>(image), level, levels, layer, layers, plane, accessBefore, accessAfter, fromLayout, toLayout);
        }
    };

    /// @brief Defines a set of descriptors.
    ///
    /// Descriptors can be grouped into multiple descriptor sets. It is generally a good practice to group descriptors based on the frequency of the updates they receive. For example, it typically makes sense
    /// to store the camera buffer in a descriptor set, since it only needs to be updated once per frame for each camera, whilst the object or material data should be stored in separate descriptor sets, that
    /// are possibly updated before each draw call. However, other scenarios employing multiple descriptor sets are also possible.
    ///
    /// From a shader perspective, a descriptor set is identified by a `set` (GLSL) or `space` (HLSL), whilst a descriptor is addressed by a `binding` (GLSL) or `register` (HLSL). Descriptor sets are read
    /// from GPU-visible memory, depending on how they are bound during the current draw call.
    ///
    /// From a CPU perspective, think of a descriptor set as an array of pointers to different buffers (i.e. descriptors) for the shader. A descriptor can be bound to a set by calling @ref
    /// DescriptorSet::update. Note that this does not automatically ensure, that the buffer memory is visible for the GPU. Instead, a buffer may also require a transfer into GPU visible memory, depending on
    /// the @ref ResourceHeap. However, as long as a descriptor within a set is mapped to a buffer, modifying this buffer also reflects the change to the shader, without requiring to update the descriptor,
    /// similarly to how modifying the object behind a pointer does not require the pointer to change.
    ///
    /// Note, that there might be multiple descriptor set instances of the same @ref DescriptorSetLayout, pointing to different @ref IBuffer instances, depending on the number of *frames in flight*. Since
    /// multiple frames can be computed concurrently, it is important to properly synchronize descriptor set updates. Generally, there are three strategies to choose from, that you can implement or mix in
    /// custom flavors, depending on your use case:
    ///
    /// - **Naive:**: The naive approach most closely matches earlier graphics API concepts. Create one buffer per descriptor and synchronize frames. This basically means that each back buffer swap is synchronized to wait for the graphics pipeline. This way, writing to a buffer ensures, that it is only read within the frame of reference and modifying it does not interfere with other frames. This strategy is memory efficient, but may cause the GPU to stall. It may, however be a valid strategy, for data that is only written once or very infrequently.
    /// - **Array of Buffers:**: The helper methods for creating and updating constant buffers are able to create buffer arrays. Those arrays can be used to create a buffer for each frame in flight. When binding a buffer to a descriptor, it is possible to bind only one element of the array. This way, each frame has its own buffer and does not interfere with other buffer writes.
    /// - **Ring-Buffer:**: The most efficient (yet not always applicable) approach involves creating one large buffer array, that is bound to multiple descriptor sets. This ensures that the buffer memory stays contiguous and does not get fragmented. However, this requires to know upfront, how many buffers are required for each descriptor, which might not always be possible. Thus another flavor of using this technique involves a creating a *large enough* descriptor array and updating the descriptor set with an increasing array element for each object as a ring-buffer. As long as there are enough elements in the buffer, so that no second update interferes with a buffer write in an earlier frame, this method provides the most efficient approach. However, it may be hard or impossible to determine the ideal size of the ring-buffer upfront.
    ///
    /// Note that samplers, textures and input attachments currently do not support array binding, since they are typically only updated once or require pipeline synchronization anyway.
    ///
    /// Also note, that another buffer management strategy is currently not available: the *Monolithic Buffer*. In this strategy, there is only one large buffer for *all* buffers. Differently from the ring
    /// buffer strategy, where there is one buffer per descriptor type, a monolithic buffer combines multiple constant buffers, containing different data into one giant buffer block. Calling @ref
    /// RenderPipeline::bind for a descriptor set would then receive an additional dynamic offset for each descriptor within the descriptor set.
    ///
    /// @tparam TBuffer The type of the buffer interface. Must inherit from @ref IBuffer.
    /// @tparam TImage The type of the image interface. Must inherit from @ref IImage.
    /// @tparam TSampler The type of the sampler interface. Must inherit from @ref ISampler.
    /// @tparam TAccelerationStructure The type of the acceleration structure interface. Must inherit from @ref IAccelerationStructure.
    /// @see DescriptorSetLayout
    template <typename TBuffer, typename TImage, typename TSampler, typename TAccelerationStructure> requires
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TSampler, ISampler> &&
        std::derived_from<TImage, IImage> &&
        std::derived_from<TAccelerationStructure, IAccelerationStructure>
    class DescriptorSet : public IDescriptorSet {
    public:
        using IDescriptorSet::bindToHeap;
        using IDescriptorSet::update;

        using buffer_type = TBuffer;
        using sampler_type = TSampler;
        using image_type = TImage;
        using acceleration_structure_type = TAccelerationStructure;

    protected:
        DescriptorSet() noexcept = default;
        DescriptorSet(const DescriptorSet&) = default;
        DescriptorSet(DescriptorSet&&) noexcept = default;
        DescriptorSet& operator=(const DescriptorSet&) = default;
        DescriptorSet& operator=(DescriptorSet&&) noexcept = default;

    public:
        ~DescriptorSet() noexcept override = default;

    public:
        /// @copydoc IDescriptorSet::bindToHeap(DescriptorType, UInt32, const IBuffer&, UInt32, UInt32, Format)
        virtual UInt32 bindToHeap(DescriptorType bindingType, UInt32 descriptor, const buffer_type& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, Format texelFormat = Format::None) const = 0;

        /// @copydoc IDescriptorSet::bindToHeap(DescriptorType, UInt32, const IImage&, UInt32, UInt32, Format)
        virtual UInt32 bindToHeap(DescriptorType bindingType, UInt32 descriptor, const image_type& image, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const = 0;

        /// @copydoc IDescriptorSet::bindToHeap(UInt32, const ISampler&)
        virtual UInt32 bindToHeap(UInt32 descriptor, const sampler_type& sampler) const = 0;

        /// @copydoc IDescriptorSet::update(UInt32, const IBuffer&, UInt32, UInt32, UInt32, Format)
        virtual void update(UInt32 binding, const buffer_type& buffer, UInt32 bufferElement = 0, UInt32 elements = 0, UInt32 firstDescriptor = 0, Format texelFormat = Format::None) const = 0;

        /// @copydoc IDescriptorSet::update(UInt32, const IImage&, UInt32, UInt32, UInt32, UInt32, UInt32)
        virtual void update(UInt32 binding, const image_type& texture, UInt32 descriptor = 0, UInt32 firstLevel = 0, UInt32 levels = 0, UInt32 firstLayer = 0, UInt32 layers = 0) const = 0;

        /// @copydoc IDescriptorSet::update(UInt32, const ISampler&, UInt32)
        virtual void update(UInt32 binding, const sampler_type& sampler, UInt32 descriptor = 0) const = 0;

        /// @copydoc IDescriptorSet::update(UInt32, const IAccelerationStructure&, UInt32)
        virtual void update(UInt32 binding, const acceleration_structure_type& accelerationStructure, UInt32 descriptor = 0) const = 0;

    private:
        UInt32 doBind(DescriptorType bindingType, UInt32 descriptor, const IBuffer& buffer, UInt32 bufferElement, UInt32 elements, Format texelFormat) const override {
            return this->bindToHeap(bindingType, descriptor, dynamic_cast<const buffer_type&>(buffer), bufferElement, elements, texelFormat);
        }

        UInt32 doBind(DescriptorType bindingType, UInt32 descriptor, const IImage& image, UInt32 firstLevel, UInt32 levels, UInt32 firstLayer, UInt32 layers) const override {
            return this->bindToHeap(bindingType, descriptor, dynamic_cast<const image_type&>(image), firstLevel, levels, firstLayer, layers);
        }

        UInt32 doBind(UInt32 descriptor, const ISampler& sampler) const override {
            return this->bindToHeap(descriptor, dynamic_cast<const sampler_type&>(sampler));
        }

        void doUpdate(UInt32 binding, const IBuffer& buffer, UInt32 bufferElement, UInt32 elements, UInt32 firstDescriptor, Format texelFormat) const override {
            this->update(binding, dynamic_cast<const buffer_type&>(buffer), bufferElement, elements, firstDescriptor, texelFormat);
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

    /// @brief Describes the layout of a descriptor set.
    ///
    /// A descriptor set groups together multiple descriptors. This concept is identified by the `set` keyword in GLSL and `space` in HLSL.
    ///
    /// For more information on buffer binding and resource management, refer to the remarks of the @ref DescriptorSet interface.
    ///
    /// @tparam TDescriptorLayout The type of the descriptor layout. Must implement @ref IDescriptorLayout.
    /// @tparam TDescriptorSet The type of the descriptor set. Must implement @ref DescriptorSet.
    /// @see IDescriptorLayout
    /// @see DescriptorSet
    template <typename TDescriptorLayout, typename TDescriptorSet> requires
        meta::implements<TDescriptorLayout, IDescriptorLayout> &&
        meta::implements<TDescriptorSet, DescriptorSet<typename TDescriptorSet::buffer_type, typename TDescriptorSet::image_type, typename TDescriptorSet::sampler_type, typename TDescriptorSet::acceleration_structure_type>>
    class DescriptorSetLayout : public IDescriptorSetLayout {
    public:
        using IDescriptorSetLayout::free;

        using descriptor_layout_type = TDescriptorLayout;
        using descriptor_set_type = TDescriptorSet;

    protected:
        DescriptorSetLayout() noexcept = default;
        DescriptorSetLayout(const DescriptorSetLayout&) = default;
        DescriptorSetLayout(DescriptorSetLayout&&) noexcept = default;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = default;
        DescriptorSetLayout& operator=(DescriptorSetLayout&&) noexcept = default;

    public:
        ~DescriptorSetLayout() noexcept override = default;

    public:
        /// @copydoc IDescriptorSetLayout::descriptors()
        virtual const Array<descriptor_layout_type>& descriptors() const noexcept = 0;

        /// @copydoc IDescriptorSetLayout::descriptor(UInt32)
        const descriptor_layout_type& descriptor(UInt32 binding) const override = 0;

        /// @copydoc IDescriptorSetLayout::allocate(std::initializer_list<DescriptorBinding>)
        virtual inline UniquePtr<descriptor_set_type> allocate(std::initializer_list<DescriptorBinding> bindings = { }) const {
            return this->allocate(0, bindings);
        }

        /// @copydoc IDescriptorSetLayout::allocate(Span<DescriptorBinding>)
        virtual inline UniquePtr<descriptor_set_type> allocate(Span<DescriptorBinding> bindings) const {
            return this->allocate(0, bindings);
        }

        /// @copydoc IDescriptorSetLayout::allocate(Generator<DescriptorBinding>)
        virtual inline UniquePtr<descriptor_set_type> allocate(Generator<DescriptorBinding> bindings) const {
            return this->allocate(0, std::move(bindings));
        }

        /// @copydoc IDescriptorSetLayout::allocate(UInt32, std::initializer_list<DescriptorBinding>)
        virtual UniquePtr<descriptor_set_type> allocate(UInt32 descriptors, std::initializer_list<DescriptorBinding> bindings) const = 0;

        /// @copydoc IDescriptorSetLayout::allocate(UInt32, Span<DescriptorBinding>)
        virtual UniquePtr<descriptor_set_type> allocate(UInt32 descriptors, Span<DescriptorBinding> bindings) const = 0;

        /// @copydoc IDescriptorSetLayout::allocate(UInt32, Generator<DescriptorBinding>)
        virtual UniquePtr<descriptor_set_type> allocate(UInt32 descriptors, Generator<DescriptorBinding> bindings) const = 0;

        /// @copydoc IDescriptorSetLayout::allocate(UInt32, std::initializer_list<std::initializer_list<DescriptorBinding>>)
        virtual inline Generator<UniquePtr<descriptor_set_type>> allocate(UInt32 descriptorSets, std::initializer_list<std::initializer_list<DescriptorBinding>> bindings = { }) const {
            return this->allocate(descriptorSets, 0, bindings);
        }

#ifdef __cpp_lib_mdspan
        /// @copydoc IDescriptorSetLayout::allocate(UInt32, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>>)
        virtual inline Generator<UniquePtr<descriptor_set_type>> allocate(UInt32 descriptorSets, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const {
            return this->allocate(descriptorSets, 0, bindings);
        }
#endif

        /// @copydoc IDescriptorSetLayout::allocate(UInt32, std::function<Generator<DescriptorBinding>(UInt32)>)
        virtual inline Generator<UniquePtr<descriptor_set_type>> allocate(UInt32 descriptorSets, std::function<Generator<DescriptorBinding>(UInt32)> bindings) const {
            return this->allocate(descriptorSets, 0, std::move(bindings));
        }

        /// @copydoc IDescriptorSetLayout::allocate(UInt32, UInt32, std::initializer_list<std::initializer_list<DescriptorBinding>>)
        virtual Generator<UniquePtr<descriptor_set_type>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::initializer_list<std::initializer_list<DescriptorBinding>> bindings = { }) const = 0;

#ifdef __cpp_lib_mdspan
        /// @copydoc IDescriptorSetLayout::allocate(UInt32, UInt32, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>>)
        virtual Generator<UniquePtr<descriptor_set_type>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const = 0;
#endif

        /// @copydoc IDescriptorSetLayout::allocate(UInt32, UInt32, std::function<Generator<DescriptorBinding>(UInt32)>)
        virtual Generator<UniquePtr<descriptor_set_type>> allocate(UInt32 descriptorSets, UInt32 descriptors, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const = 0;

        /// @copydoc IDescriptorSetLayout::free(const IDescriptorSet&)
        virtual void free(const descriptor_set_type& descriptorSet) const = 0;

    private:
        inline Enumerable<const IDescriptorLayout&> getDescriptors() const noexcept override {
            return this->descriptors();
        }

        inline UniquePtr<IDescriptorSet> getDescriptorSet(UInt32 descriptors, std::initializer_list<DescriptorBinding> bindings) const override {
            return this->allocate(descriptors, bindings);
        }

        inline UniquePtr<IDescriptorSet> getDescriptorSet(UInt32 descriptors, Span<DescriptorBinding> bindings) const override {
            return this->allocate(descriptors, bindings);
        }

        inline UniquePtr<IDescriptorSet> getDescriptorSet(UInt32 descriptors, Generator<DescriptorBinding> bindings) const override {
            return this->allocate(descriptors, std::move(bindings));
        }

        inline Generator<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, std::initializer_list<std::initializer_list<DescriptorBinding>> bindings) const override {
            co_yield std::ranges::elements_of(this->allocate(descriptorSets, descriptors, bindings) | std::views::transform([](auto set) -> UniquePtr<IDescriptorSet> { return set; }));
        }

#ifdef __cpp_lib_mdspan
        inline Generator<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, std::mdspan<DescriptorBinding, std::dextents<size_t, 2>> bindings) const override {
            co_yield std::ranges::elements_of(this->allocate(descriptorSets, descriptors, bindings) | std::views::transform([](auto set) -> UniquePtr<IDescriptorSet> { return set; }));
        }
#endif

        inline Generator<UniquePtr<IDescriptorSet>> getDescriptorSets(UInt32 descriptorSets, UInt32 descriptors, std::function<Generator<DescriptorBinding>(UInt32)> bindingFactory) const override {
            co_yield std::ranges::elements_of(this->allocate(descriptorSets, descriptors, std::move(bindingFactory)) | std::views::transform([](auto set) -> UniquePtr<IDescriptorSet> { return set; }));
        }

        inline void releaseDescriptorSet(const IDescriptorSet& descriptorSet) const override {
            this->free(dynamic_cast<const descriptor_set_type&>(descriptorSet));
        }
    };

    /// @brief Describes the layout of the pipelines push constant ranges.
    ///
    /// Push constants are very efficient, yet quite limited ways of passing per-draw data to shaders. They are mapped directly, so no buffer must be created or transitioned in order to use them. Conceptually
    /// a push constant is a single piece of memory that gets transferred to a command buffer directly. Each shader stage has a view into this piece of memory, described by an offset and a size. Since the
    /// memory is directly dumped in the command buffer, the backing memory can be incrementally updated and there is no need to store an array of buffers, as long as updates happen sequentially from the same
    /// thread. However, there are certain restrictions when using push constants:
    ///
    /// - Only one push constant layout per pipeline layout is supported.
    /// - A push constant layout may not define a overall memory size larger than 128 bytes. This is a soft restriction that will issue a warning, however it might be supported on some hardware or for some back-ends.
    /// - The elements and offsets of a push constant memory block must be aligned to 4 bytes.
    /// - Accordingly, the size of a push constant layout must align to 4 bytes.
    /// - Only one @ref IPushConstantsRange per shader stage is permitted. Shader stages can be combined together, however in this case, no other ranges must be defined for the stages.
    ///
    /// Push constants can be updated by calling @ref CommandBuffer::PushConstants and are visible to subsequent draw calls immediately, until another update is performed or the command buffer is ended.
    ///
    /// @tparam TPushConstantsRange The type of the push constant range. Must implement @ref IPushConstantsRange.
    /// @see IPushConstantsRange
    /// @see DescriptorSetLayout
    template <typename TPushConstantsRange> requires
        meta::implements<TPushConstantsRange, IPushConstantsRange>
    class PushConstantsLayout : public IPushConstantsLayout {
    public:
        using push_constants_range_type = TPushConstantsRange;

    protected:
        PushConstantsLayout() noexcept = default;
        PushConstantsLayout(const PushConstantsLayout&) = default;
        PushConstantsLayout(PushConstantsLayout&&) noexcept = default;
        PushConstantsLayout& operator=(const PushConstantsLayout&) = default;
        PushConstantsLayout& operator=(PushConstantsLayout&&) noexcept = default;

    public:
        ~PushConstantsLayout() noexcept override = default;

    public:
        /// @copydoc IPushConstantsLayout::ranges()
        virtual const Array<UniquePtr<push_constants_range_type>>& ranges() const = 0;

    private:
        inline Enumerable<const IPushConstantsRange&> getRanges() const override {
            return this->ranges() | std::views::transform([](auto& ptr) -> const IPushConstantsRange& { return *ptr; });
        }
    };

    /// @brief Represents a shader program, consisting of multiple @ref IShaderModules.
    ///
    /// @tparam TShaderModule The type of the shader module. Must implement @ref IShaderModule.
    /// @see https://github.com/crud89/LiteFX/wiki/Shader-Development
    template <typename TShaderModule> requires
        meta::implements<TShaderModule, IShaderModule>
    class ShaderProgram : public IShaderProgram {
    public:
        using shader_module_type = TShaderModule;

    protected:
        ShaderProgram() noexcept = default;
        ShaderProgram(const ShaderProgram&) = default;
        ShaderProgram(ShaderProgram&&) noexcept = default;
        ShaderProgram& operator=(const ShaderProgram&) = default;
        ShaderProgram& operator=(ShaderProgram&&) noexcept = default;

    public:
        ~ShaderProgram() noexcept override = default;

    public:
        /// @copydoc IShaderProgram::modules()
        virtual const Array<UniquePtr<const shader_module_type>>& modules() const noexcept = 0;

    private:
        inline Enumerable<const IShaderModule&> getModules() const override {
            return this->modules() | std::views::transform([](const auto& m) -> const IShaderModule& { return *m; });
        }
    };
    
    /// @brief Represents a the layout of a @ref RenderPipeline, @ref ComputePipeline or @ref RayTracingPipeline.
    ///
    /// @tparam TDescriptorSetLayout The type of the descriptor set layout. Must implement @ref DescriptorSetLayout.
    /// @tparam TPushConstantsLayout The type of the push constants layout. Must implement @ref PushConstantsLayout.
    template <typename TDescriptorSetLayout, typename TPushConstantsLayout> requires
        meta::implements<TDescriptorSetLayout, DescriptorSetLayout<typename TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSetLayout::descriptor_set_type>> &&
        meta::implements<TPushConstantsLayout, PushConstantsLayout<typename TPushConstantsLayout::push_constants_range_type>>
    class PipelineLayout : public IPipelineLayout {
    public:
        using descriptor_set_layout_type = TDescriptorSetLayout;
        using push_constants_layout_type = TPushConstantsLayout;

    protected:
        PipelineLayout() noexcept = default;
        PipelineLayout(const PipelineLayout&) = default;
        PipelineLayout(PipelineLayout&&) noexcept = default;
        PipelineLayout& operator=(const PipelineLayout&) = default;
        PipelineLayout& operator=(PipelineLayout&&) noexcept = default;

    public:
        ~PipelineLayout() noexcept override = default;

    public:
        /// @copydoc IPipelineLayout::descriptorSet
        const descriptor_set_layout_type& descriptorSet(UInt32 space) const override = 0;

        /// @copydoc IPipelineLayout::descriptorSets()
        virtual const Array<SharedPtr<const descriptor_set_layout_type>>& descriptorSets() const = 0;

        /// @copydoc IPipelineLayout::pushConstants
        const push_constants_layout_type* pushConstants() const noexcept override = 0;

    private:
        inline Enumerable<SharedPtr<const IDescriptorSetLayout>> getDescriptorSets() const override {
            return this->descriptorSets();
        }
    };

    /// @brief Describes a vertex buffer.
    ///
    /// @tparam TVertexBufferLayout The type of the vertex buffer layout. Must implement @ref IVertexBufferLayout.
    template <typename TVertexBufferLayout> requires
        meta::implements<TVertexBufferLayout, IVertexBufferLayout>
    class VertexBuffer : public virtual IVertexBuffer {
    public:
        using vertex_buffer_layout_type = TVertexBufferLayout;
    protected:
        VertexBuffer() noexcept = default;
        VertexBuffer(const VertexBuffer&) = default;
        VertexBuffer(VertexBuffer&&) noexcept = default;
        VertexBuffer& operator=(const VertexBuffer&) = default;
        VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

    public:
        ~VertexBuffer() noexcept override = default;

    public:
        /// @copydoc IVertexBuffer::layout
        const vertex_buffer_layout_type& layout() const noexcept override = 0;
    };

    /// @brief Describes an index buffer.
    ///
    /// @tparam TIndexBufferLayout The type of the index buffer layout. Must implement @ref IIndexBufferLayout.
    template <typename TIndexBufferLayout> requires
        meta::implements<TIndexBufferLayout, IIndexBufferLayout>
    class IndexBuffer : public virtual IIndexBuffer {
    public:
        using index_buffer_layout_type = TIndexBufferLayout;
    
    protected:
        IndexBuffer() noexcept = default;
        IndexBuffer(const IndexBuffer&) = default;
        IndexBuffer(IndexBuffer&&) noexcept = default;
        IndexBuffer& operator=(const IndexBuffer&) = default;
        IndexBuffer& operator=(IndexBuffer&&) noexcept = default;
    
    public:
        ~IndexBuffer() noexcept override = default;

    public:
        /// @copydoc IIndexBuffer::layout
        const index_buffer_layout_type& layout() const noexcept override = 0;
    };

    /// @brief Represents a the input assembler state of a @ref RenderPipeline.
    ///
    /// @tparam TVertexBufferLayout The type of the vertex buffer layout. Must implement @ref IVertexBufferLayout.
    /// @tparam TIndexBufferLayout The type of the index buffer layout. Must implement @ref IIndexBufferLayout.
    template <typename TVertexBufferLayout, typename TIndexBufferLayout> requires
        meta::implements<TVertexBufferLayout, IVertexBufferLayout> &&
        meta::implements<TIndexBufferLayout, IIndexBufferLayout>
    class InputAssembler : public IInputAssembler {
    public:
        using vertex_buffer_layout_type = TVertexBufferLayout;
        using index_buffer_layout_type = TIndexBufferLayout;

    protected:
        InputAssembler() noexcept = default;
        InputAssembler(const InputAssembler&) = default;
        InputAssembler(InputAssembler&&) noexcept = default;
        InputAssembler& operator=(const InputAssembler&) = default;
        InputAssembler& operator=(InputAssembler&&) noexcept = default;

    public:
        ~InputAssembler() noexcept override = default;

    public:
        /// @copydoc IInputAssembler::vertexBufferLayouts()
        virtual Enumerable<const vertex_buffer_layout_type&> vertexBufferLayouts() const = 0;

        /// @copydoc IInputAssembler::vertexBufferLayout
        const vertex_buffer_layout_type& vertexBufferLayout(UInt32 binding) const override = 0;

        /// @copydoc IInputAssembler::indexBufferLayout
        const index_buffer_layout_type* indexBufferLayout() const noexcept override = 0;

    private:
        inline Enumerable<const IVertexBufferLayout&> getVertexBufferLayouts() const override {
            return this->vertexBufferLayouts();
        }
    };

    /// @brief Represents a pipeline state.
    ///
    /// @tparam TPipelineLayout The type of the render pipeline layout. Must implement @ref PipelineLayout.
    /// @tparam TShaderProgram The type of the shader program. Must implement @ref ShaderProgram.
    /// @see RenderPipeline
    /// @see ComputePipeline
    template <typename TPipelineLayout, typename TShaderProgram> requires
        meta::implements<TPipelineLayout, PipelineLayout<typename TPipelineLayout::descriptor_set_layout_type, typename TPipelineLayout::push_constants_layout_type>> &&
        meta::implements<TShaderProgram, ShaderProgram<typename TShaderProgram::shader_module_type>>
    class Pipeline : public virtual IPipeline, public virtual StateResource {
    public:
        using shader_program_type = TShaderProgram;
        using pipeline_layout_type = TPipelineLayout;

    protected:
        Pipeline() noexcept = default;
        Pipeline(const Pipeline&) = default;
        Pipeline(Pipeline&&) noexcept = default;
        Pipeline& operator=(const Pipeline&) = default;
        Pipeline& operator=(Pipeline&&) noexcept = default;

    public:
        ~Pipeline() noexcept override = default;

    public:
        /// @copydoc IPipeline::program()
        virtual SharedPtr<const shader_program_type> program() const noexcept = 0;

        /// @copydoc IPipeline::layout()
        virtual SharedPtr<const pipeline_layout_type> layout() const noexcept = 0;

    private:
        inline SharedPtr<const IShaderProgram> getProgram() const noexcept override {
            return std::static_pointer_cast<const IShaderProgram>(this->program());
        }

        inline SharedPtr<const IPipelineLayout> getLayout() const noexcept override {
            return std::static_pointer_cast<const IPipelineLayout>(this->layout());
        }
    };

    /// @brief Represents a command buffer, that buffers commands that should be submitted to a @ref CommandQueue.
    ///
    /// @tparam TCommandBuffer The type of the command buffer itself. Must inherit from @ref CommandBuffer.
    /// @tparam TBuffer The generic buffer type. Must implement @ref IBuffer.
    /// @tparam TVertexBuffer The vertex buffer type. Must implement @ref VertexBuffer.
    /// @tparam TIndexBuffer The index buffer type. Must implement @ref IndexBuffer.
    /// @tparam TImage The generic image type. Must implement @ref IImage.
    /// @tparam TBarrier The barrier type. Must implement @ref Barrier.
    /// @tparam TPipeline The common pipeline interface type. Must be derived from @ref Pipeline.
    /// @tparam TBLAS The type of the bottom-level acceleration structure. Must implement @ref IBottomLevelAccelerationStructure.
    /// @tparam TTLAS The type of the top-level acceleration structure. Must implement @ref ITopLevelAccelerationStructure.
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
        using ICommandBuffer::dispatchIndirect;

        using ICommandBuffer::dispatchMesh;
        using ICommandBuffer::draw;
        using ICommandBuffer::drawIndirect;
        using ICommandBuffer::drawIndexed;
        using ICommandBuffer::drawIndexedIndirect;
        using ICommandBuffer::barrier;
        using ICommandBuffer::transfer;
        using ICommandBuffer::bind;
        using ICommandBuffer::use;
        using ICommandBuffer::pushConstants;
        using ICommandBuffer::buildAccelerationStructure;
        using ICommandBuffer::updateAccelerationStructure;
        using ICommandBuffer::copyAccelerationStructure;

        friend TCommandBuffer;

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
        using bottom_level_acceleration_structure_type = TBLAS;
        using top_level_acceleration_structure_type = TTLAS;

    private:
        CommandBuffer() noexcept = default;
        CommandBuffer(CommandBuffer&&) noexcept = default;
        CommandBuffer(const CommandBuffer&) = default;
        CommandBuffer& operator=(const CommandBuffer&) = default;
        CommandBuffer& operator=(CommandBuffer&&) noexcept = default;

    public:
        ~CommandBuffer() noexcept override = default;

    public:
        /// @copydoc ICommandBuffer::makeBarrier(PipelineStage, PipelineStage)
        virtual UniquePtr<barrier_type> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const = 0;

        /// @copydoc ICommandBuffer::barrier(const IBarrier&)
        virtual void barrier(const barrier_type& barrier) const noexcept = 0;

        /// @copydoc ICommandBuffer::transfer(const IBuffer&, const IBuffer&, UInt32, UInt32, UInt32)
        virtual void transfer(const buffer_type& source, const buffer_type& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const = 0;

        /// @copydoc ICommandBuffer::transfer(const void* const, size_t, const IBuffer&, UInt32, UInt32)
        virtual void transfer(const void* const data, size_t size, const buffer_type& target, UInt32 targetElement = 0, UInt32 elements = 1) const = 0;

        /// @copydoc ICommandBuffer::transfer(Span<const void* const>, size_t, const IBuffer&, UInt32)
        virtual void transfer(Span<const void* const> data, size_t elementSize, const buffer_type& target, UInt32 firstElement = 0) const = 0;

        /// @copydoc ICommandBuffer::transfer(const IBuffer&, const IImage&, UInt32, UInt32, UInt32)
        virtual void transfer(const buffer_type& source, const image_type& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const = 0;

        /// @copydoc ICommandBuffer::transfer(const void* const, size_t, const IImage&, UInt32)
        virtual void transfer(const void* const data, size_t size, const image_type& target, UInt32 subresource = 0) const = 0;

        /// @copydoc ICommandBuffer::transfer(Span<const void* const>, size_t, const IImage&, UInt32, UInt32)
        virtual void transfer(Span<const void* const> data, size_t elementSize, const image_type& target, UInt32 firstSubresource = 0, UInt32 subresources = 1) const = 0;

        /// @copydoc ICommandBuffer::transfer(const IImage&, const IImage&, UInt32, UInt32, UInt32)
        virtual void transfer(const image_type& source, const image_type& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const = 0;

        /// @copydoc ICommandBuffer::transfer(const IImage&, const IBuffer&, UInt32, UInt32, UInt32)
        virtual void transfer(const image_type& source, const buffer_type& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const = 0;

        /// @copydoc ICommandBuffer::transfer(const SharedPtr<const IBuffer>&, const IBuffer&, UInt32, UInt32, UInt32)
        virtual void transfer(const SharedPtr<const buffer_type>& source, const buffer_type& target, UInt32 sourceElement = 0, UInt32 targetElement = 0, UInt32 elements = 1) const = 0;

        /// @copydoc ICommandBuffer::transfer(const SharedPtr<const IBuffer>&, const IImage&, UInt32, UInt32, UInt32)
        virtual void transfer(const SharedPtr<const buffer_type>& source, const image_type& target, UInt32 sourceElement = 0, UInt32 firstSubresource = 0, UInt32 elements = 1) const = 0;

        /// @copydoc ICommandBuffer::transfer(const SharedPtr<const IImage>&, const IImage&, UInt32, UInt32, UInt32)
        virtual void transfer(const SharedPtr<const image_type>& source, const image_type& target, UInt32 sourceSubresource = 0, UInt32 targetSubresource = 0, UInt32 subresources = 1) const = 0;

        /// @copydoc ICommandBuffer::transfer(const SharedPtr<const IImage>&, const IBuffer&, UInt32, UInt32, UInt32)
        virtual void transfer(const SharedPtr<const image_type>& source, const buffer_type& target, UInt32 firstSubresource = 0, UInt32 targetElement = 0, UInt32 subresources = 1) const = 0;

        /// @copydoc ICommandBuffer::use(const IPipeline&)
        virtual void use(const pipeline_type& pipeline) const noexcept = 0;

        /// @copydoc ICommandBuffer::bind(const IDescriptorSet&)
        virtual void bind(const descriptor_set_type& descriptorSet) const = 0;

        /// @copydoc ICommandBuffer::bind(Span<const IDescriptorSet*>)
        virtual void bind(Span<const descriptor_set_type*> descriptorSets) const = 0;

        /// @copydoc ICommandBuffer::bind(const IDescriptorSet&, const IPipeline&)
        virtual void bind(const descriptor_set_type& descriptorSet, const pipeline_type& pipeline) const = 0;

        /// @copydoc ICommandBuffer::bind(Span<const IDescriptorSet*>, const IPipeline&)
        virtual void bind(Span<const descriptor_set_type*> descriptorSets, const pipeline_type& pipeline) const = 0;

        /// @copydoc ICommandBuffer::bind(const IVertexBuffer&)
        virtual void bind(const vertex_buffer_type& buffer) const noexcept = 0;

        /// @copydoc ICommandBuffer::bind(const IIndexBuffer&)
        virtual void bind(const index_buffer_type& buffer) const noexcept = 0;

        /// @copydoc ICommandBuffer::pushConstants(const IPushConstantsLayout&, const void* const)
        virtual void pushConstants(const push_constants_layout_type& layout, const void* const memory) const = 0;

        /// @copydoc ICommandBuffer::dispatchIndirect(const IBuffer&, UInt32, UInt64)
        virtual void dispatchIndirect(const buffer_type& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept = 0;

        /// @copydoc ICommandBuffer::dispatchMeshIndirect(const IBuffer&, UInt32, UInt64)
        virtual void dispatchMeshIndirect(const buffer_type& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept = 0;

        /// @copydoc ICommandBuffer::dispatchMeshIndirect(const IBuffer&, const IBuffer&, UInt64, UInt64, UInt32)
        virtual void dispatchMeshIndirect(const buffer_type& batchBuffer, const buffer_type& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept = 0;

        /// @copydoc ICommandBuffer::drawIndirect(const IBuffer&, UInt32, UInt64)
        virtual void drawIndirect(const buffer_type& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept = 0;

        /// @copydoc ICommandBuffer::drawIndirect(const IBuffer&, const IBuffer&, UInt64, UInt64, UInt32)
        virtual void drawIndirect(const buffer_type& batchBuffer, const buffer_type& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept = 0;

        /// @copydoc ICommandBuffer::drawIndexedIndirect(const IBuffer&, UInt32, UInt64)
        virtual void drawIndexedIndirect(const buffer_type& batchBuffer, UInt32 batchCount, UInt64 offset = 0) const noexcept = 0;

        /// @copydoc ICommandBuffer::drawIndexedIndirect(const IBuffer&, const IBuffer&, UInt64, UInt64, UInt32)
        virtual void drawIndexedIndirect(const buffer_type& batchBuffer, const buffer_type& countBuffer, UInt64 offset = 0, UInt64 countOffset = 0, UInt32 maxBatches = std::numeric_limits<UInt32>::max()) const noexcept = 0;
        
        /// @copydoc ICommandBuffer::draw(const IVertexBuffer&, UInt32, UInt32, UInt32)
        virtual inline void draw(const vertex_buffer_type& vertexBuffer, UInt32 instances = 1, UInt32 firstVertex = 0, UInt32 firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->draw(vertexBuffer.elements(), instances, firstVertex, firstInstance);
        }

        /// @copydoc ICommandBuffer::drawIndexed(const IVertexBuffer&, UInt32, UInt32, UInt32, UInt32)
        virtual inline void drawIndexed(const index_buffer_type& indexBuffer, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const {
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }

        /// @copydoc ICommandBuffer::drawIndexed(const IVertexBuffer&, const IIndexBuffer&, UInt32, UInt32, UInt32, UInt32)
        virtual inline void drawIndexed(const vertex_buffer_type& vertexBuffer, const index_buffer_type& indexBuffer, UInt32 instances = 1, UInt32 firstIndex = 0, Int32 vertexOffset = 0, UInt32 firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }

        /// @copydoc ICommandBuffer::execute(const SharedPtr<const ICommandBuffer>&)
        virtual void execute(const SharedPtr<const command_buffer_type>& commandBuffer) const = 0;

        /// @copydoc ICommandBuffer::execute(Enumerable<SharedPtr<const ICommandBuffer>>)
        virtual void execute(Enumerable<SharedPtr<const command_buffer_type>> commandBuffers) const = 0;

        /// @copydoc ICommandBuffer::buildAccelerationStructure(IBottomLevelAccelerationStructure&, const SharedPtr<const IBuffer>&, const IBuffer&, UInt64)
        virtual void buildAccelerationStructure(bottom_level_acceleration_structure_type& blas, const SharedPtr<const buffer_type>& scratchBuffer, const buffer_type& buffer, UInt64 offset = 0) const = 0;

        /// @copydoc ICommandBuffer::buildAccelerationStructure(ITopLevelAccelerationStructure&, const SharedPtr<const IBuffer>&, const IBuffer&, UInt64)
        virtual void buildAccelerationStructure(top_level_acceleration_structure_type& tlas, const SharedPtr<const buffer_type>& scratchBuffer, const buffer_type& buffer, UInt64 offset = 0) const = 0;

        /// @copydoc ICommandBuffer::updateAccelerationStructure(IBottomLevelAccelerationStructure&, const SharedPtr<const IBuffer>&, const IBuffer&, UInt64)
        virtual void updateAccelerationStructure(bottom_level_acceleration_structure_type& blas, const SharedPtr<const buffer_type>& scratchBuffer, const buffer_type& buffer, UInt64 offset = 0) const = 0;

        /// @copydoc ICommandBuffer::updateAccelerationStructure(ITopLevelAccelerationStructure&, const SharedPtr<const IBuffer>&, const IBuffer&, UInt64)
        virtual void updateAccelerationStructure(top_level_acceleration_structure_type& tlas, const SharedPtr<const buffer_type>& scratchBuffer, const buffer_type& buffer, UInt64 offset = 0) const = 0;

        /// @copydoc ICommandBuffer::copyAccelerationStructure(const IBottomLevelAccelerationStructure&, const IBottomLevelAccelerationStructure&, bool)
        virtual void copyAccelerationStructure(const bottom_level_acceleration_structure_type& from, const bottom_level_acceleration_structure_type& to, bool compress = false) const noexcept = 0;

        /// @copydoc ICommandBuffer::copyAccelerationStructure(const ITopLevelAccelerationStructure&, const ITopLevelAccelerationStructure&, bool)
        virtual void copyAccelerationStructure(const top_level_acceleration_structure_type& from, const top_level_acceleration_structure_type& to, bool compress = false) const noexcept = 0;

        /// @copydoc ICommandBuffer::traceRays(UInt32, UInt32, UInt32, const ShaderBindingTableOffsets&, const IBuffer&, const IBuffer*, const IBuffer*, const IBuffer*)
        virtual void traceRays(UInt32 width, UInt32 height, UInt32 depth, const ShaderBindingTableOffsets& offsets, const buffer_type& rayGenerationShaderBindingTable, const buffer_type* missShaderBindingTable, const buffer_type* hitShaderBindingTable, const buffer_type* callableShaderBindingTable) const noexcept = 0;

        /// @copydoc ICommandBuffer::traceRays(const Vector3u&, const ShaderBindingTableOffsets&, const IBuffer&, const IBuffer*, const IBuffer*, const IBuffer*)
        inline void traceRays(const Vector3u& dimensions, const ShaderBindingTableOffsets& offsets, const buffer_type& rayGenerationShaderBindingTable, const buffer_type* missShaderBindingTable, const buffer_type* hitShaderBindingTable, const buffer_type* callableShaderBindingTable) const noexcept {
            this->traceRays(dimensions.x(), dimensions.y(), dimensions.z(), offsets, rayGenerationShaderBindingTable, missShaderBindingTable, hitShaderBindingTable, callableShaderBindingTable);
        }

    private:
        inline UniquePtr<IBarrier> getBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const override {
            return this->makeBarrier(syncBefore, syncAfter);
        }

        inline void cmdBarrier(const IBarrier& barrier) const noexcept override {
            this->barrier(dynamic_cast<const barrier_type&>(barrier));
        }

        inline void cmdTransfer(const IBuffer& source, const IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const override {
            this->transfer(dynamic_cast<const buffer_type&>(source), dynamic_cast<const buffer_type&>(target), sourceElement, targetElement, elements);
        }
        
        inline void cmdTransfer(const IBuffer& source, const IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const override {
            this->transfer(dynamic_cast<const buffer_type&>(source), dynamic_cast<const image_type&>(target), sourceElement, firstSubresource, elements);
        }
        
        inline void cmdTransfer(const IImage& source, const IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const override {
            this->transfer(dynamic_cast<const image_type&>(source), dynamic_cast<const image_type&>(target), sourceSubresource, targetSubresource, subresources);
        }

        inline void cmdTransfer(const IImage& source, const IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const override {
            this->transfer(dynamic_cast<const image_type&>(source), dynamic_cast<const buffer_type&>(target), firstSubresource, targetElement, subresources);
        }

        inline void cmdTransfer(const SharedPtr<const IBuffer>& source, const IBuffer& target, UInt32 sourceElement, UInt32 targetElement, UInt32 elements) const override {
            this->transfer(std::dynamic_pointer_cast<const buffer_type>(source), dynamic_cast<const buffer_type&>(target), sourceElement, targetElement, elements);
        }
        
        inline void cmdTransfer(const SharedPtr<const IBuffer>& source, const IImage& target, UInt32 sourceElement, UInt32 firstSubresource, UInt32 elements) const override {
            this->transfer(std::dynamic_pointer_cast<const buffer_type>(source), dynamic_cast<const image_type&>(target), sourceElement, firstSubresource, elements);
        }
        
        inline void cmdTransfer(const SharedPtr<const IImage>& source, const IImage& target, UInt32 sourceSubresource, UInt32 targetSubresource, UInt32 subresources) const override {
            this->transfer(std::dynamic_pointer_cast<const image_type>(source), dynamic_cast<const image_type&>(target), sourceSubresource, targetSubresource, subresources);
        }
        
        inline void cmdTransfer(const SharedPtr<const IImage>& source, const IBuffer& target, UInt32 firstSubresource, UInt32 targetElement, UInt32 subresources) const override {
            this->transfer(std::dynamic_pointer_cast<const image_type>(source), dynamic_cast<const buffer_type&>(target), firstSubresource, targetElement, subresources);
        }

        inline void cmdTransfer(const void* const data, size_t size, const IBuffer& target, UInt32 targetElement, UInt32 elements) const override {
            this->transfer(data, size, dynamic_cast<const buffer_type&>(target), targetElement, elements);
        }

        inline void cmdTransfer(Span<const void* const> data, size_t elementSize, const IBuffer& target, UInt32 targetElement) const override {
            this->transfer(data, elementSize, dynamic_cast<const buffer_type&>(target), targetElement);
        }

        inline void cmdTransfer(const void* const data, size_t size, const IImage& target, UInt32 subresource) const override {
            this->transfer(data, size, dynamic_cast<const image_type&>(target), subresource);
        }

        inline void cmdTransfer(Span<const void* const> data, size_t elementSize, const IImage& target, UInt32 firstSubresource, UInt32 elements) const override {
            this->transfer(data, elementSize, dynamic_cast<const image_type&>(target), firstSubresource, elements);
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

        inline void cmdBind(const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const override {
            this->bind(dynamic_cast<const descriptor_set_type&>(descriptorSet), dynamic_cast<const pipeline_type&>(pipeline));
        }

        inline void cmdBind(Span<const IDescriptorSet*> descriptorSets, const IPipeline& pipeline) const override {
            auto sets = descriptorSets | std::views::transform([](auto set) { return dynamic_cast<const descriptor_set_type*>(set); }) | std::ranges::to<Array<const descriptor_set_type*>>();
            this->bind(Span<const descriptor_set_type*>(sets), dynamic_cast<const pipeline_type&>(pipeline));
        }
        
        inline void cmdBind(const IVertexBuffer& buffer) const override {
            this->bind(dynamic_cast<const vertex_buffer_type&>(buffer));
        }

        inline void cmdBind(const IIndexBuffer& buffer) const override {
            this->bind(dynamic_cast<const index_buffer_type&>(buffer));
        }
        
        inline void cmdPushConstants(const IPushConstantsLayout& layout, const void* const memory) const override {
            this->pushConstants(dynamic_cast<const push_constants_layout_type&>(layout), memory);
        }

        inline void cmdDispatchIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept override {
            this->dispatchIndirect(dynamic_cast<const buffer_type&>(batchBuffer), batchCount, offset);
        }

        inline void cmdDispatchMeshIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept override {
            this->dispatchMeshIndirect(dynamic_cast<const buffer_type&>(batchBuffer), batchCount, offset);
        }

        inline void cmdDispatchMeshIndirect(const IBuffer& batchBuffer, const IBuffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept override {
            this->dispatchMeshIndirect(dynamic_cast<const buffer_type&>(batchBuffer), dynamic_cast<const buffer_type&>(countBuffer), offset, countOffset, maxBatches);
        }
        
        inline void cmdDraw(const IVertexBuffer& vertexBuffer, UInt32 instances, UInt32 firstVertex, UInt32 firstInstance) const override {
            this->draw(dynamic_cast<const vertex_buffer_type&>(vertexBuffer), instances, firstVertex, firstInstance);
        }

        inline void cmdDrawIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept override {
            this->drawIndirect(dynamic_cast<const buffer_type&>(batchBuffer), batchCount, offset);
        }

        inline void cmdDrawIndirect(const IBuffer& batchBuffer, const IBuffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept override {
            this->drawIndirect(dynamic_cast<const buffer_type&>(batchBuffer), dynamic_cast<const buffer_type&>(countBuffer), offset, countOffset, maxBatches);
        }
        
        inline void cmdDrawIndexed(const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const override {
            this->drawIndexed(dynamic_cast<const index_buffer_type&>(indexBuffer), instances, firstIndex, vertexOffset, firstInstance);
        }
        
        inline void cmdDrawIndexed(const IVertexBuffer& vertexBuffer, const IIndexBuffer& indexBuffer, UInt32 instances, UInt32 firstIndex, Int32 vertexOffset, UInt32 firstInstance) const override {
            this->drawIndexed(dynamic_cast<const vertex_buffer_type&>(vertexBuffer), dynamic_cast<const index_buffer_type&>(indexBuffer), instances, firstIndex, vertexOffset, firstInstance);
        }

        inline void cmdDrawIndexedIndirect(const IBuffer& batchBuffer, UInt32 batchCount, UInt64 offset) const noexcept override {
            this->drawIndexedIndirect(dynamic_cast<const buffer_type&>(batchBuffer), batchCount, offset);
        }

        inline void cmdDrawIndexedIndirect(const IBuffer& batchBuffer, const IBuffer& countBuffer, UInt64 offset, UInt64 countOffset, UInt32 maxBatches) const noexcept override {
            this->drawIndexedIndirect(dynamic_cast<const buffer_type&>(batchBuffer), dynamic_cast<const buffer_type&>(countBuffer), offset, countOffset, maxBatches);
        }

        inline void cmdExecute(const SharedPtr<const ICommandBuffer>& commandBuffer) const override {
            this->execute(std::dynamic_pointer_cast<const command_buffer_type>(commandBuffer));
        }
        
        inline void cmdExecute(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers) const override {
            return this->execute(commandBuffers | std::views::transform([](const SharedPtr<const ICommandBuffer>& buffer) { return std::dynamic_pointer_cast<const command_buffer_type>(buffer); }));
        }

        void cmdBuildAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset) const override {
            this->buildAccelerationStructure(dynamic_cast<bottom_level_acceleration_structure_type&>(blas), std::dynamic_pointer_cast<const buffer_type>(scratchBuffer), dynamic_cast<const buffer_type&>(buffer), offset);
        }

        void cmdBuildAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset) const override {
            this->buildAccelerationStructure(dynamic_cast<top_level_acceleration_structure_type&>(tlas), std::dynamic_pointer_cast<const buffer_type>(scratchBuffer), dynamic_cast<const buffer_type&>(buffer), offset);
        }

        void cmdUpdateAccelerationStructure(IBottomLevelAccelerationStructure& blas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset) const override {
            this->updateAccelerationStructure(dynamic_cast<bottom_level_acceleration_structure_type&>(blas), std::dynamic_pointer_cast<const buffer_type>(scratchBuffer), dynamic_cast<const buffer_type&>(buffer), offset);
        }
        
        void cmdUpdateAccelerationStructure(ITopLevelAccelerationStructure& tlas, const SharedPtr<const IBuffer>& scratchBuffer, const IBuffer& buffer, UInt64 offset) const override {
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

    /// @brief Represents a graphics @ref Pipeline.
    ///
    /// @tparam TPipelineLayout The type of the render pipeline layout. Must implement @ref PipelineLayout.
    /// @tparam TShaderProgram The type of the shader program. Must implement @ref ShaderProgram.
    /// @tparam TInputAssembler The type of the input assembler state. Must implement @ref InputAssembler.
    /// @tparam TRasterizer The type of the rasterizer state. Must implement @ref Rasterizer.
    /// @see RenderPipelineBuilder
    template <typename TPipelineLayout, typename TShaderProgram, typename TInputAssembler, typename TRasterizer> requires
        meta::implements<TInputAssembler, InputAssembler<typename TInputAssembler::vertex_buffer_layout_type, typename TInputAssembler::index_buffer_layout_type>> &&
        meta::implements<TRasterizer, Rasterizer>
    class RenderPipeline : public IRenderPipeline, public virtual Pipeline<TPipelineLayout, TShaderProgram> {
    public:
        using input_assembler_type = TInputAssembler;
        using rasterizer_type = TRasterizer;

    protected:
        RenderPipeline() noexcept = default;
        RenderPipeline(RenderPipeline&&) noexcept = default;
        RenderPipeline(const RenderPipeline&) = default;
        RenderPipeline& operator=(RenderPipeline&&) noexcept = default;
        RenderPipeline& operator=(const RenderPipeline&) = default;

    public:
        ~RenderPipeline() noexcept override = default;

    public:
        /// @copydoc IRenderPipeline::inputAssembler()
        virtual SharedPtr<input_assembler_type> inputAssembler() const noexcept = 0;

        /// @copydoc IRenderPipeline::rasterizer()
        virtual SharedPtr<rasterizer_type> rasterizer() const noexcept = 0;

    private:
        inline SharedPtr<IInputAssembler> getInputAssembler() const noexcept override {
            return this->inputAssembler();
        }

        inline SharedPtr<IRasterizer> getRasterizer() const noexcept override {
            return this->rasterizer();
        }
    };

    /// @brief Represents a compute @ref Pipeline.
    ///
    /// @tparam TPipelineLayout The type of the render pipeline layout. Must implement @ref PipelineLayout.
    /// @tparam TShaderProgram The type of the shader program. Must implement @ref ShaderProgram.
    /// @see ComputePipelineBuilder
    template <typename TPipelineLayout, typename TShaderProgram>
    class ComputePipeline : public IComputePipeline, public virtual Pipeline<TPipelineLayout, TShaderProgram> {
    protected:
        ComputePipeline() noexcept = default;
        ComputePipeline(ComputePipeline&&) noexcept = default;
        ComputePipeline(const ComputePipeline&) = default;
        ComputePipeline& operator=(ComputePipeline&&) noexcept = default;
        ComputePipeline& operator=(const ComputePipeline&) = default;

    public:
        ~ComputePipeline() noexcept override = default;
    };

    /// @brief Represents a ray-tracing @ref Pipeline.
    ///
    /// @tparam TPipelineLayout The type of the render pipeline layout. Must implement @ref PipelineLayout.
    /// @tparam TShaderProgram The type of the shader program. Must implement @ref ShaderProgram.
    /// @see RayTracingPipelineBuilder
    template <typename TPipelineLayout, typename TShaderProgram>
    class RayTracingPipeline : public IRayTracingPipeline, public virtual Pipeline<TPipelineLayout, TShaderProgram> {
    public:
        using base_type = Pipeline<TPipelineLayout, TShaderProgram>;
        using descriptor_set_layout_type = base_type::pipeline_layout_type::descriptor_set_layout_type;
        using descriptor_set_type = descriptor_set_layout_type::descriptor_set_type;
        using descriptor_layout_type = descriptor_set_layout_type::descriptor_layout_type;
        using buffer_type = descriptor_set_type::buffer_type;
        using image_type = descriptor_set_type::image_type;
        using sampler_type = descriptor_set_type::sampler_type;

    protected:
        RayTracingPipeline() noexcept = default;
        RayTracingPipeline(RayTracingPipeline&&) noexcept = default;
        RayTracingPipeline(const RayTracingPipeline&) = default;
        RayTracingPipeline& operator=(RayTracingPipeline&&) noexcept = default;
        RayTracingPipeline& operator=(const RayTracingPipeline&) = default;

    public:
        ~RayTracingPipeline() noexcept override = default;

    public:
        /// @copydoc IRayTracingPipeline::allocateShaderBindingTable(ShaderBindingTableOffsets&, ShaderBindingGroup)
        virtual SharedPtr<buffer_type> allocateShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups = ShaderBindingGroup::All) const = 0;

    private:
        inline SharedPtr<IBuffer> getShaderBindingTable(ShaderBindingTableOffsets& offsets, ShaderBindingGroup groups) const override {
            return this->allocateShaderBindingTable(offsets, groups);
        }
    };

    /// @brief Stores the images used by a @ref RenderPass to either read from using input attachments or write to using render targets.
    ///
    /// @tparam TImage The type of the frame buffer images. Must be derived from @ref IImage.
    /// @see RenderTarget
    template <typename TImage> requires
        std::derived_from<TImage, IImage>
    class FrameBuffer : public virtual StateResource, public IFrameBuffer {
    public:
        using image_type = TImage;
        using allocation_callback_type = IFrameBuffer::allocation_callback_type<image_type>;

    public:
        using IFrameBuffer::addImage;

    protected:
        FrameBuffer() noexcept = default;
        FrameBuffer(FrameBuffer&&) noexcept = default;
        FrameBuffer(const FrameBuffer&) = default;
        FrameBuffer& operator=(FrameBuffer&&) noexcept = default;
        FrameBuffer& operator=(const FrameBuffer&) = default;

    public:
        ~FrameBuffer() noexcept override = default;

    public:
        /// @copydoc IFrameBuffer::images()
        virtual const Array<SharedPtr<const image_type>>& images() const = 0;

    private:
        inline Enumerable<const IImage&> getImages() const override {
            return this->images() | std::views::transform([](auto& image) -> const IImage& { return *image; });
        }
    };

    /// @brief Represents a command queue.
    ///
    /// @tparam TCommandBuffer The type of the command buffer for this queue. Must implement @ref CommandBuffer.
    template <typename TCommandBuffer> requires
        meta::implements<TCommandBuffer, CommandBuffer<typename TCommandBuffer::command_buffer_type, typename TCommandBuffer::buffer_type, typename TCommandBuffer::vertex_buffer_type, typename TCommandBuffer::index_buffer_type, typename TCommandBuffer::image_type, typename TCommandBuffer::barrier_type, typename TCommandBuffer::pipeline_type, typename TCommandBuffer::bottom_level_acceleration_structure_type, typename TCommandBuffer::top_level_acceleration_structure_type>>
    class CommandQueue : public ICommandQueue {
    public:
        using ICommandQueue::submit;

        using command_buffer_type = TCommandBuffer;

    protected:
        CommandQueue() noexcept = default;
        CommandQueue(CommandQueue&&) noexcept = default;
        CommandQueue(const CommandQueue&) = default;
        CommandQueue& operator=(CommandQueue&&) noexcept = default;
        CommandQueue& operator=(const CommandQueue&) = default;

    public:
        ~CommandQueue() override = default;

    public:
        /// @copydoc ICommandQueue::createCommandBuffer(bool, bool)
        virtual SharedPtr<command_buffer_type> createCommandBuffer(bool beginRecording = false, bool secondary = false) const = 0;

        /// @copydoc ICommandQueue::submit(const SharedPtr<ICommandBuffer>&)
        virtual inline UInt64 submit(const SharedPtr<command_buffer_type>& commandBuffer) const {
            return this->submit(std::static_pointer_cast<const command_buffer_type>(commandBuffer));
        }

        /// @copydoc ICommandQueue::submit(const SharedPtr<const ICommandBuffer>&)
        virtual UInt64 submit(const SharedPtr<const command_buffer_type>& commandBuffer) const = 0;

        /// @copydoc ICommandQueue::submit(Enumerable<SharedPtr<const ICommandBuffer>>)
        virtual UInt64 submit(Enumerable<SharedPtr<const command_buffer_type>> commandBuffers) const = 0;

    private:
        inline SharedPtr<ICommandBuffer> getCommandBuffer(bool beginRecording, bool secondary) const override {
            return this->createCommandBuffer(beginRecording, secondary);
        }

        inline UInt64 submitCommandBuffer(const SharedPtr<const ICommandBuffer>& commandBuffer) const override {
            return this->submit(std::dynamic_pointer_cast<const command_buffer_type>(commandBuffer));
        }

        inline UInt64 submitCommandBuffers(Enumerable<SharedPtr<const ICommandBuffer>> commandBuffers) const override {
            return this->submit(Enumerable<SharedPtr<const command_buffer_type>> { 
                commandBuffers | std::views::transform([](const SharedPtr<const ICommandBuffer>& buffer) { return std::dynamic_pointer_cast<const command_buffer_type>(buffer); }) 
            });
        }
    };

    /// @brief Represents a render pass.
    ///
    /// A render pass is a conceptual layer, that may not have any logical representation within the actual implementation. It is a high-level view on a specific workload on the GPU, that processes data using
    /// different @ref RenderPipelines and stores the outputs in the @ref IRenderTargets of a @ref FrameBuffer.
    ///
    /// @tparam TRenderPipeline The type of the render pipeline. Must implement @ref RenderPipeline.
    /// @tparam TCommandQueue The type of the command queue. Must implement @ref CommandQueue.
    /// @tparam TFrameBuffer The type of the frame buffer. Must implement @ref FrameBuffer.
    template <typename TCommandQueue, typename TFrameBuffer> requires
        meta::implements<TCommandQueue, CommandQueue<typename TCommandQueue::command_buffer_type>> &&
        meta::implements<TFrameBuffer, FrameBuffer<typename TFrameBuffer::image_type>>
    class RenderPass : public virtual StateResource, public IRenderPass {
    public:
        using command_queue_type = TCommandQueue;
        using command_buffer_type = TCommandQueue::command_buffer_type;
        using frame_buffer_type = TFrameBuffer;

    protected:
        RenderPass() noexcept = default;
        RenderPass(RenderPass&&) noexcept = default;
        RenderPass(const RenderPass&) = default;
        RenderPass& operator=(RenderPass&&) noexcept = default;
        RenderPass& operator=(const RenderPass&) = default;

    public:
        ~RenderPass() noexcept override = default;

    public:
        /// @copydoc IRenderPass::activeFrameBuffer()
        virtual SharedPtr<const frame_buffer_type> activeFrameBuffer() const noexcept = 0;

        /// @copydoc IRenderPass::commandBuffers()
        virtual Enumerable<SharedPtr<const command_buffer_type>> commandBuffers() const = 0;

        /// @copydoc IRenderPass::commandQueue()
        virtual const command_queue_type& commandQueue() const noexcept = 0;

        /// @copydoc IRenderPass::commandBuffer(UInt32)
        virtual SharedPtr<const command_buffer_type> commandBuffer(UInt32 index) const = 0;

        /// @copydoc IRenderPass::begin(const IFrameBuffer&)
        virtual void begin(const frame_buffer_type& frameBuffer) const = 0;

    private:
        inline SharedPtr<const IFrameBuffer> getActiveFrameBuffer() const noexcept override {
            return this->activeFrameBuffer();
        }

        inline SharedPtr<const ICommandBuffer> getCommandBuffer(UInt32 index) const noexcept override {
            return this->commandBuffer(index);
        }

        inline const ICommandQueue& getCommandQueue() const noexcept override {
            return this->commandQueue();
        }

        inline Enumerable<SharedPtr<const ICommandBuffer>> getCommandBuffers() const override {
            return this->commandBuffers();
        }

        inline void beginRenderPass(const IFrameBuffer& frameBuffer) const override {
            this->begin(dynamic_cast<const frame_buffer_type&>(frameBuffer));
        }
    };

    /// @brief Represents a swap chain, i.e. a chain of multiple @ref IImage instances, that can be presented to a @ref ISurface.
    ///
    /// @tparam TImageInterface The type of the image interface. Must inherit from @ref IImage.
    template <typename TImageInterface> requires
        std::derived_from<TImageInterface, IImage>
    class SwapChain : public ISwapChain {
    public:
        using image_interface_type = TImageInterface;

    protected:
        SwapChain() noexcept = default;
        SwapChain(SwapChain&&) noexcept = default;
        SwapChain(const SwapChain&) = default;
        SwapChain& operator=(SwapChain&&) noexcept = default;
        SwapChain& operator=(const SwapChain&) = default;

    public:
        ~SwapChain() noexcept override = default;

    public:
        /// @copydoc ISwapChain::images()
        virtual const Array<SharedPtr<image_interface_type>>& images() const noexcept = 0;

    private:
        inline Enumerable<IImage&> getImages() const override {
            return this->images() | std::views::transform([](auto& image) -> IImage& { return *image; });
        }
    };

    /// @brief Describes a factory that creates objects for a @ref GraphicsDevice.
    ///
    /// @tparam TDescriptorLayout The type of the descriptor layout. Must implement @ref IDescriptorLayout.
    /// @tparam TVertexBuffer The type of the vertex buffer. Must implement @ref VertexBuffer.
    /// @tparam TIndexBuffer The type of the index buffer. Must implement @ref IndexBuffer.
    /// @tparam TImage The type of the image. Must inherit from @ref IImage.
    /// @tparam TBuffer The type of the buffer. Must inherit from @ref IBuffer.
    /// @tparam TSampler The type of the sampler. Must inherit from @ref ISampler.
    /// @tparam TBLAS The type of the bottom-level acceleration structure. Must implement @ref IBottomLevelAccelerationStructure.
    /// @tparam TTLAS The type of the top-level acceleration structure. Must implement @ref ITopLevelAccelerationStructure.
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
        using IGraphicsFactory::tryCreateBuffer;
        using IGraphicsFactory::createVertexBuffer;
        using IGraphicsFactory::tryCreateVertexBuffer;
        using IGraphicsFactory::createIndexBuffer;
        using IGraphicsFactory::tryCreateIndexBuffer;
        using IGraphicsFactory::createTexture;
        using IGraphicsFactory::tryCreateTexture;
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

    protected:
        GraphicsFactory() noexcept = default;
        GraphicsFactory(GraphicsFactory&&) noexcept = default;
        GraphicsFactory(const GraphicsFactory&) = default;
        GraphicsFactory& operator=(GraphicsFactory&&) noexcept = default;
        GraphicsFactory& operator=(const GraphicsFactory&) = default;

    public:
        ~GraphicsFactory() noexcept override = default;

    public:
        /// @copydoc IGraphicsFactory::createBuffer(BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior)
        virtual SharedPtr<TBuffer> createBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::createBuffer(const String&, BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior)
        virtual SharedPtr<TBuffer> createBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::createVertexBuffer(const IVertexBufferLayout&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior)
        virtual SharedPtr<TVertexBuffer> createVertexBuffer(const vertex_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::createVertexBuffer(const String&, const IVertexBufferLayout&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior)
        virtual SharedPtr<TVertexBuffer> createVertexBuffer(const String& name, const vertex_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::createIndexBuffer(const IIndexBufferLayout&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior)
        virtual SharedPtr<TIndexBuffer> createIndexBuffer(const index_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::createIndexBuffer(const String&, const IIndexBufferLayout&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior)
        virtual SharedPtr<TIndexBuffer> createIndexBuffer(const String& name, const index_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::createTexture(Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior)
        virtual SharedPtr<TImage> createTexture(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::createTexture(const String&, Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior)
        virtual SharedPtr<TImage> createTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::tryCreateBuffer(SharedPtr<IBuffer>&, BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior)
        virtual bool tryCreateBuffer(SharedPtr<TBuffer>& buffer, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::tryCreateBuffer(SharedPtr<IBuffer>&, const String&, BufferType, ResourceHeap, size_t, UInt32, ResourceUsage, AllocationBehavior)
        virtual bool tryCreateBuffer(SharedPtr<TBuffer>& buffer, const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::tryCreateVertexBuffer(SharedPtr<IVertexBuffer>&, const IVertexBufferLayout&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior)
        virtual bool tryCreateVertexBuffer(SharedPtr<TVertexBuffer>& buffer, const vertex_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::tryCreateVertexBuffer(SharedPtr<IVertexBuffer>&, const String&, const IVertexBufferLayout&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior)
        virtual bool tryCreateVertexBuffer(SharedPtr<TVertexBuffer>& buffer, const String& name, const vertex_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements = 1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::tryCreateIndexBuffer(SharedPtr<IIndexBuffer>&, const IIndexBufferLayout&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior)
        virtual bool tryCreateIndexBuffer(SharedPtr<TIndexBuffer>& buffer, const index_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::tryCreateIndexBuffer(SharedPtr<IIndexBuffer>&, const String&, const IIndexBufferLayout&, ResourceHeap, UInt32, ResourceUsage, AllocationBehavior)
        virtual bool tryCreateIndexBuffer(SharedPtr<TIndexBuffer>& buffer, const String& name, const index_buffer_layout_type& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::tryCreateTexture(SharedPtr<IImage>&, Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior)
        virtual bool tryCreateTexture(SharedPtr<TImage>& image, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::tryCreateTexture(SharedPtr<IImage>&, const String&, Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior)
        virtual bool tryCreateTexture(SharedPtr<TImage>& image, const String& name, Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 levels = 1, UInt32 layers = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::createTextures(Format, const Size3d&, ImageDimensions, UInt32, UInt32, MultiSamplingLevel, ResourceUsage, AllocationBehavior)
        virtual Generator<SharedPtr<TImage>> createTextures(Format format, const Size3d& size, ImageDimensions dimension = ImageDimensions::DIM_2, UInt32 layers = 1, UInt32 levels = 1, MultiSamplingLevel samples = MultiSamplingLevel::x1, ResourceUsage usage = ResourceUsage::Default, AllocationBehavior allocationBehavior = AllocationBehavior::Default) const = 0;

        /// @copydoc IGraphicsFactory::createSampler(FilterMode, FilterMode, BorderMode, BorderMode, BorderMode, MipMapMode, Float, Float, Float, Float)
        virtual SharedPtr<TSampler> createSampler(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const = 0;

        /// @copydoc IGraphicsFactory::createSampler(const String&, FilterMode, FilterMode, BorderMode, BorderMode, BorderMode, MipMapMode, Float, Float, Float, Float)
        virtual SharedPtr<TSampler> createSampler(const String& name, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const = 0;

        /// @copydoc IGraphicsFactory::createSamplers(FilterMode, FilterMode, BorderMode, BorderMode, BorderMode, MipMapMode, Float, Float, Float, Float)
        virtual Generator<SharedPtr<TSampler>> createSamplers(FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float minLod = 0.f, Float anisotropy = 0.f) const = 0;

        /// @copydoc IGraphicsFactory::createBottomLevelAccelerationStructure(AccelerationStructureFlags)
        inline UniquePtr<TBLAS> createBottomLevelAccelerationStructure(AccelerationStructureFlags flags) const {
            return this->createBottomLevelAccelerationStructure("", flags);
        }

        /// @copydoc IGraphicsFactory::createBottomLevelAccelerationStructure(StringView, AccelerationStructureFlags)
        virtual UniquePtr<TBLAS> createBottomLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const = 0;

        /// @copydoc IGraphicsFactory::createTopLevelAccelerationStructure(AccelerationStructureFlags)
        inline UniquePtr<TTLAS> createTopLevelAccelerationStructure(AccelerationStructureFlags flags) const {
            return this->createTopLevelAccelerationStructure("", flags);
        }

        /// @copydoc IGraphicsFactory::createTopLevelAccelerationStructure(StringView, AccelerationStructureFlags)
        virtual UniquePtr<TTLAS> createTopLevelAccelerationStructure(StringView name, AccelerationStructureFlags flags) const = 0;

    private:
        inline SharedPtr<IBuffer> getBuffer(BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            return this->createBuffer(type, heap, elementSize, elements, usage, allocationBehavior);
        }

        inline SharedPtr<IBuffer> getBuffer(const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            return this->createBuffer(name, type, heap, elementSize, elements, usage, allocationBehavior);
        }

        inline SharedPtr<IVertexBuffer> getVertexBuffer(const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            return this->createVertexBuffer(dynamic_cast<const vertex_buffer_layout_type&>(layout), heap, elements, usage, allocationBehavior);
        }

        inline SharedPtr<IVertexBuffer> getVertexBuffer(const String& name, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            return this->createVertexBuffer(name, dynamic_cast<const vertex_buffer_layout_type&>(layout), heap, elements, usage, allocationBehavior);
        }
        
        inline SharedPtr<IIndexBuffer> getIndexBuffer(const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            return this->createIndexBuffer(dynamic_cast<const index_buffer_layout_type&>(layout), heap, elements, usage, allocationBehavior);
        }

        inline SharedPtr<IIndexBuffer> getIndexBuffer(const String& name, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            return this->createIndexBuffer(name, dynamic_cast<const index_buffer_layout_type&>(layout), heap, elements, usage, allocationBehavior);
        }
        
        inline SharedPtr<IImage> getTexture(Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            return this->createTexture(format, size, dimension, levels, layers, samples, usage, allocationBehavior);
        }

        inline SharedPtr<IImage> getTexture(const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            return this->createTexture(name, format, size, dimension, levels, layers, samples, usage, allocationBehavior);
        }

        inline bool tryGetBuffer(SharedPtr<IBuffer>& buffer, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            SharedPtr<buffer_type> actualBuffer;
            auto result = this->tryCreateBuffer(actualBuffer, type, heap, elementSize, elements, usage, allocationBehavior);
            buffer = actualBuffer;
            return result;
        }

        inline bool tryGetBuffer(SharedPtr<IBuffer>& buffer, const String& name, BufferType type, ResourceHeap heap, size_t elementSize, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            SharedPtr<buffer_type> actualBuffer;
            auto result = this->tryCreateBuffer(actualBuffer, name, type, heap, elementSize, elements, usage, allocationBehavior);
            buffer = actualBuffer;
            return result;
        }

        inline bool tryGetVertexBuffer(SharedPtr<IVertexBuffer>& buffer, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            SharedPtr<vertex_buffer_type> actualBuffer;
            auto result = this->tryCreateVertexBuffer(actualBuffer, dynamic_cast<const vertex_buffer_layout_type&>(layout), heap, elements, usage, allocationBehavior);
            buffer = actualBuffer;
            return result;
        }

        inline bool tryGetVertexBuffer(SharedPtr<IVertexBuffer>& buffer, const String& name, const IVertexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            SharedPtr<vertex_buffer_type> actualBuffer;
            auto result = this->tryCreateVertexBuffer(actualBuffer, name, dynamic_cast<const vertex_buffer_layout_type&>(layout), heap, elements, usage, allocationBehavior);
            buffer = actualBuffer;
            return result;
        }

        inline bool tryGetIndexBuffer(SharedPtr<IIndexBuffer>& buffer, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            SharedPtr<index_buffer_type> actualBuffer;
            auto result = this->tryCreateIndexBuffer(actualBuffer, dynamic_cast<const index_buffer_layout_type&>(layout), heap, elements, usage, allocationBehavior);
            buffer = actualBuffer;
            return result;
        }

        inline bool tryGetIndexBuffer(SharedPtr<IIndexBuffer>& buffer, const String& name, const IIndexBufferLayout& layout, ResourceHeap heap, UInt32 elements, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            SharedPtr<index_buffer_type> actualBuffer;
            auto result = this->tryCreateIndexBuffer(actualBuffer, name, dynamic_cast<const index_buffer_layout_type&>(layout), heap, elements, usage, allocationBehavior);
            buffer = actualBuffer;
            return result;
        }

        inline bool tryGetTexture(SharedPtr<IImage>& image, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            SharedPtr<image_type> actualImage;
            auto result = this->tryCreateTexture(actualImage, format, size, dimension, levels, layers, samples, usage, allocationBehavior);
            image = actualImage;
            return result;
        }

        inline bool tryGetTexture(SharedPtr<IImage>& image, const String& name, Format format, const Size3d& size, ImageDimensions dimension, UInt32 levels, UInt32 layers, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            SharedPtr<image_type> actualImage;
            auto result = this->tryCreateTexture(actualImage, name, format, size, dimension, levels, layers, samples, usage, allocationBehavior);
            image = actualImage;
            return result;
        }

        inline Generator<SharedPtr<IImage>> getTextures(Format format, const Size3d& size, ImageDimensions dimension, UInt32 layers, UInt32 levels, MultiSamplingLevel samples, ResourceUsage usage, AllocationBehavior allocationBehavior) const override {
            return [](Generator<SharedPtr<TImage>> gen) -> Generator<SharedPtr<IImage>> {
                for (auto texture : gen)
                    co_yield std::move(texture);
            }(this->createTextures(format, size, dimension, layers, levels, samples, usage, allocationBehavior));
        }
        
        inline SharedPtr<ISampler> getSampler(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const override {
            return this->createSampler(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }

        inline SharedPtr<ISampler> getSampler(const String& name, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const override {
            return this->createSampler(name, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
        }
        
        inline Generator<SharedPtr<ISampler>> getSamplers(FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float maxLod, Float minLod, Float anisotropy) const override {
            return [](Generator<SharedPtr<TSampler>> gen) -> Generator<SharedPtr<ISampler>> {
                for (auto sampler : gen)
                    co_yield std::move(sampler);
            }(this->createSamplers(magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy));
        }

        inline UniquePtr<IBottomLevelAccelerationStructure> getBlas(StringView name, AccelerationStructureFlags flags) const override {
            return this->createBottomLevelAccelerationStructure(name, flags);
        }

        inline UniquePtr<ITopLevelAccelerationStructure> getTlas(StringView name, AccelerationStructureFlags flags) const override {
            return this->createTopLevelAccelerationStructure(name, flags);
        }
    };

    /// @brief Represents the graphics device that a rendering back-end is doing work on.
    ///
    /// The graphics device is the central instance of a renderer. It has two major roles. First, it maintains the @ref GraphicsFactory instance, that is used to facilitate common objects. Second, it owns the
    /// device state, which contains objects required for communication between your application and the graphics driver. Most notably, those objects contain the @ref SwapChain instance and the @ref
    /// CommandQueue instances used for data and command transfer.
    ///
    /// @tparam TFactory The type of the graphics factory. Must implement @ref GraphicsFactory.
    /// @tparam TSurface The type of the surface. Must implement @ref ISurface.
    /// @tparam TGraphicsAdapter The type of the graphics adapter. Must implement @ref IGraphicsAdapter.
    /// @tparam TSwapChain The type of the swap chain. Must implement @ref SwapChain.
    /// @tparam TCommandQueue The type of the command queue. Must implement @ref CommandQueue.
    /// @tparam TRenderPass The type of the render pass. Must implement @ref RenderPass.
    /// @tparam TRenderPipeline The type of the render pipeline. Must implement @ref RenderPipeline.
    /// @tparam TComputePipeline The type of the compute pipeline. Must implement @ref ComputePipeline.
    /// @tparam TRayTracingPipeline The type of the ray-tracing pipeline. Must implement @ref RayTracingPipeline.
    /// @tparam TBarrier The type of the memory barrier. Must implement @ref Barrier.
    template <typename TFactory, typename TSurface, typename TGraphicsAdapter, typename TSwapChain, typename TCommandQueue, typename TRenderPass, typename TRenderPipeline, typename TComputePipeline, typename TRayTracingPipeline, typename TBarrier> requires
        meta::implements<TSurface, ISurface> &&
        meta::implements<TGraphicsAdapter, IGraphicsAdapter> &&
        meta::implements<TSwapChain, SwapChain<typename TFactory::image_type>> &&
        meta::implements<TCommandQueue, CommandQueue<typename TCommandQueue::command_buffer_type>> &&
        meta::implements<TFactory, GraphicsFactory<typename TFactory::descriptor_layout_type, typename TFactory::buffer_type, typename TFactory::vertex_buffer_type, typename TFactory::index_buffer_type, typename TFactory::image_type, typename TFactory::sampler_type, typename TFactory::bottom_level_acceleration_structure_type, typename TFactory::top_level_acceleration_structure_type>> &&
        meta::implements<TRenderPass, RenderPass<TCommandQueue, typename TRenderPass::frame_buffer_type>> &&
        meta::implements<TRenderPipeline, RenderPipeline<typename TRenderPipeline::pipeline_layout_type, typename TRenderPipeline::shader_program_type, typename TRenderPipeline::input_assembler_type, typename TRenderPipeline::rasterizer_type>> &&
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
        using descriptor_set_type = command_buffer_type::descriptor_set_type;
        using pipeline_type = command_buffer_type::pipeline_type;
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
        using render_pipeline_type = TRenderPipeline;
        using compute_pipeline_type = TComputePipeline;
        using ray_tracing_pipeline_type = TRayTracingPipeline;
        using pipeline_layout_type = render_pipeline_type::pipeline_layout_type;
        using shader_program_type = render_pipeline_type::shader_program_type;
        using input_assembler_type = render_pipeline_type::input_assembler_type;
        using rasterizer_type = render_pipeline_type::rasterizer_type;

    protected:
        GraphicsDevice() noexcept = default;
        GraphicsDevice(GraphicsDevice&&) noexcept = default;
        GraphicsDevice(const GraphicsDevice&) = default;
        GraphicsDevice& operator=(GraphicsDevice&&) noexcept = default;
        GraphicsDevice& operator=(const GraphicsDevice&) = default;

    public:
        ~GraphicsDevice() noexcept override = default;

    public:
        /// @copydoc IGraphicsDevice::surface
        const surface_type& surface() const noexcept override = 0;

        /// @copydoc IGraphicsDevice::adapter
        const adapter_type& adapter() const noexcept override = 0;

        /// @copydoc IGraphicsDevice::swapChain() const
        const swap_chain_type& swapChain() const noexcept override = 0;

        /// @copydoc IGraphicsDevice::swapChain()
        swap_chain_type& swapChain() noexcept override = 0;

        /// @copydoc IGraphicsDevice::factory
        const factory_type& factory() const noexcept override = 0;

        /// @copydoc IGraphicsDevice::defaultQueue(QueueType)
        virtual const command_queue_type& defaultQueue(QueueType type) const = 0;

        /// @copydoc IGraphicsDevice::createQueue(QueueType, QueuePriority)
        virtual SharedPtr<const command_queue_type> createQueue(QueueType type, QueuePriority priority = QueuePriority::Normal) = 0;

        /// @copydoc IGraphicsDevice::makeBarrier(PipelineStage, PipelineStage)
        [[nodiscard]] virtual UniquePtr<barrier_type> makeBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const = 0;

        /// @copydoc IGraphicsDevice::makeFrameBuffer(const Size2d&)
        [[nodiscard]] inline SharedPtr<frame_buffer_type> makeFrameBuffer(const Size2d& renderArea) const {
            return this->makeFrameBuffer("", renderArea);
        }

        /// @copydoc IGraphicsDevice::makeFrameBuffer(const Size2d&)
        ///
        /// @param allocationCallback A callback that gets invoked, when the frame buffer allocates a new image.
        [[nodiscard]] inline SharedPtr<frame_buffer_type> makeFrameBuffer(const Size2d& renderArea, frame_buffer_type::allocation_callback_type allocationCallback) const {
            return this->makeFrameBuffer("", renderArea, std::move(allocationCallback));
        }

        /// @copydoc IGraphicsDevice::makeFrameBuffer(StringView, const Size2d&)
        [[nodiscard]] virtual SharedPtr<frame_buffer_type> makeFrameBuffer(StringView name, const Size2d& renderArea) const = 0;

        /// @copydoc IGraphicsDevice::makeFrameBuffer(StringView, const Size2d&)
        ///
        /// @param allocationCallback A callback that gets invoked, when the frame buffer allocates a new image.
        [[nodiscard]] virtual SharedPtr<frame_buffer_type> makeFrameBuffer(StringView name, const Size2d& renderArea, frame_buffer_type::allocation_callback_type allocationCallback) const = 0;

    private:
        inline UniquePtr<IBarrier> getNewBarrier(PipelineStage syncBefore, PipelineStage syncAfter) const override {
            return this->makeBarrier(syncBefore, syncAfter);
        }

        inline SharedPtr<IFrameBuffer> getNewFrameBuffer(StringView name, const Size2d& renderArea) const override {
            return this->makeFrameBuffer(name, renderArea);
        }

        inline const ICommandQueue& getDefaultQueue(QueueType type) const override {
            return this->defaultQueue(type);
        }

        inline SharedPtr<const ICommandQueue> getNewQueue(QueueType type, QueuePriority priority) override {
            return std::static_pointer_cast<const ICommandQueue>(this->createQueue(type, priority));
        }

    public:
        /// @copydoc IGraphicsDevice::computeAccelerationStructureSizes(const IBottomLevelAccelerationStructure&, UInt64&, UInt64&, bool)
        virtual void computeAccelerationStructureSizes(const bottom_level_acceleration_structure_type& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate = false) const = 0;

        /// @copydoc IGraphicsDevice::computeAccelerationStructureSizes(const ITopLevelAccelerationStructure&, UInt64&, UInt64&, bool)
        virtual void computeAccelerationStructureSizes(const top_level_acceleration_structure_type& tlas, UInt64 & bufferSize, UInt64 & scratchSize, bool forUpdate = false) const = 0;

        /// @copydoc IGraphicsDevice::allocateGlobalDescriptors(const IDescriptorSet&, DescriptorHeapType)
        [[nodiscard]] virtual VirtualAllocator::Allocation allocateGlobalDescriptors(const descriptor_set_type& descriptorSet, DescriptorHeapType heapType) const = 0;

        /// @copydoc IGraphicsDevice::releaseGlobalDescriptors(const IDescriptorSet&)
        virtual void releaseGlobalDescriptors(const descriptor_set_type& descriptorSet) const = 0;

        /// @copydoc IGraphicsDevice::updateGlobalDescriptors(const IDescriptorSet&, UInt32, UInt32, UInt32)
        virtual void updateGlobalDescriptors(const descriptor_set_type& descriptorSet, UInt32 binding, UInt32 offset, UInt32 descriptors) const = 0;

        /// @copydoc IGraphicsDevice::bindDescriptorSet(const ICommandBuffer&, const IDescriptorSet&, const IPipeline&)
        virtual void bindDescriptorSet(const command_buffer_type& commandBuffer, const descriptor_set_type& descriptorSet, const pipeline_type& pipeline) const = 0;

        /// @copydoc IGraphicsDevice::bindGlobalDescriptorHeaps(const ICommandBuffer&)
        virtual void bindGlobalDescriptorHeaps(const command_buffer_type& commandBuffer) const noexcept = 0;

    private:
        inline void getAccelerationStructureSizes(const IBottomLevelAccelerationStructure& blas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const override {
            this->computeAccelerationStructureSizes(dynamic_cast<const bottom_level_acceleration_structure_type&>(blas), bufferSize, scratchSize, forUpdate);
        }

        inline void getAccelerationStructureSizes(const ITopLevelAccelerationStructure& tlas, UInt64& bufferSize, UInt64& scratchSize, bool forUpdate) const override {
            this->computeAccelerationStructureSizes(dynamic_cast<const top_level_acceleration_structure_type&>(tlas), bufferSize, scratchSize, forUpdate);
        }
        
        inline VirtualAllocator::Allocation doAllocateGlobalDescriptors(const IDescriptorSet& descriptorSet, DescriptorHeapType heapType) const override {
            return this->allocateGlobalDescriptors(dynamic_cast<const descriptor_set_type&>(descriptorSet), heapType);
        }

        inline void doReleaseGlobalDescriptors(const IDescriptorSet& descriptorSet) const override {
            this->releaseGlobalDescriptors(dynamic_cast<const descriptor_set_type&>(descriptorSet));
        }

        inline void doUpdateGlobalDescriptors(const IDescriptorSet& descriptorSet, UInt32 binding, UInt32 offset, UInt32 descriptors) const override {
            this->updateGlobalDescriptors(dynamic_cast<const descriptor_set_type&>(descriptorSet), binding, offset, descriptors);
        }

        inline void doBindDescriptorSet(const ICommandBuffer& commandBuffer, const IDescriptorSet& descriptorSet, const IPipeline& pipeline) const override {
            this->bindDescriptorSet(dynamic_cast<const command_buffer_type&>(commandBuffer), dynamic_cast<const descriptor_set_type&>(descriptorSet), dynamic_cast<const pipeline_type&>(pipeline));
        }

        inline void doBindGlobalDescriptorHeaps(const ICommandBuffer& commandBuffer) const noexcept override {
            this->bindGlobalDescriptorHeaps(dynamic_cast<const command_buffer_type&>(commandBuffer));
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

        /// @brief Returns a builder for a @ref RenderPass.
        ///
        /// @param commandBuffers The number of command buffers in each frame buffer.
        /// @return An instance of a builder that is used to create a new render pass.
        [[nodiscard]] virtual render_pass_builder_type buildRenderPass(UInt32 commandBuffers = 1) const = 0;

        /// @brief Returns a builder for a @ref RenderPass.
        ///
        /// @param name The name of the render pass.
        /// @param samples The number of samples, the render targets of the render pass should be sampled with.
        /// @return An instance of a builder that is used to create a new render pass.
        [[nodiscard]] virtual render_pass_builder_type buildRenderPass(const String& name, UInt32 commandBuffers = 1) const = 0;

        /// @brief Returns a builder for a @ref ComputePipeline.
        ///
        /// @param name The name of the compute pipeline.
        /// @return An instance of a builder that is used to create a new compute pipeline.
        [[nodiscard]] virtual compute_pipeline_builder_type buildComputePipeline(const String& name) const = 0;

        ///// @brief Returns a builder for a @see RenderPipeline.
        ///// 
        ///// @param name The name of the render pipeline.
        ///// @return An instance of a builder that is used to create a new render pipeline.
        //[[nodiscard]] virtual render_pipeline_builder_type buildRenderPipeline(const String& name) const = 0;

        /// @brief Returns a builder for a @ref RenderPipeline.
        ///
        /// @param renderPass The parent render pass of the pipeline.
        /// @param name The name of the render pipeline.
        /// @return An instance of a builder that is used to create a new render pipeline.
        [[nodiscard]] virtual render_pipeline_builder_type buildRenderPipeline(const render_pass_type& renderPass, const String& name) const = 0;

        /// @brief Returns a builder for a @ref RayTracingPipeline.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param shaderRecords The shader record collection that is used to build the shader binding table for the pipeline.
        /// @return An instance of a builder that is used to create a new ray-tracing pipeline.
        [[nodiscard]] virtual ray_tracing_pipeline_builder_type buildRayTracingPipeline(ShaderRecordCollection&& shaderRecords) const = 0;

        /// @brief Returns a builder for a @ref RayTracingPipeline.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param name The name of the ray-tracing pipeline.
        /// @param shaderRecords The shader record collection that is used to build the shader binding table for the pipeline.
        /// @return An instance of a builder that is used to create a new ray-tracing pipeline.
        [[nodiscard]] virtual ray_tracing_pipeline_builder_type buildRayTracingPipeline(const String& name, ShaderRecordCollection&& shaderRecords) const = 0;

        /// @brief Returns a builder for a @ref PipelineLayout.
        ///
        /// @return An instance of a builder that is used to create a new pipeline layout.
        [[nodiscard]] virtual pipeline_layout_builder_type buildPipelineLayout() const = 0;

        /// @brief Returns a builder for a @ref InputAssembler.
        ///
        /// @return An instance of a builder that is used to create a new input assembler.
        [[nodiscard]] virtual input_assembler_builder_type buildInputAssembler() const = 0;

        /// @brief Returns a builder for a @ref Rasterizer.
        ///
        /// @return An instance of a builder that is used to create a new rasterizer.
        [[nodiscard]] virtual rasterizer_builder_type buildRasterizer() const = 0;

        /// @brief Returns a builder for a @ref ShaderProgram.
        ///
        /// @return An instance of a builder that is used to create a new shader program.
        [[nodiscard]] virtual shader_program_builder_type buildShaderProgram() const = 0;

        /// @brief Returns a builder for a @ref Barrier.
        ///
        /// @return An instance of a builder that is used to create a new barrier.
        [[nodiscard]] virtual barrier_builder_Type buildBarrier() const = 0;
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)
    };

    /// @brief Defines a back-end, that provides a device instance for a certain surface and graphics adapter.
    ///
    /// @tparam TGraphicsDevice The type of the graphics device. Must implement @ref GraphicsDevice.
    template <typename TGraphicsDevice> requires
        meta::implements<TGraphicsDevice, GraphicsDevice<typename TGraphicsDevice::factory_type, typename TGraphicsDevice::surface_type, typename TGraphicsDevice::adapter_type, typename TGraphicsDevice::swap_chain_type, typename TGraphicsDevice::command_queue_type, typename TGraphicsDevice::render_pass_type, typename TGraphicsDevice::render_pipeline_type, typename TGraphicsDevice::compute_pipeline_type, typename TGraphicsDevice::ray_tracing_pipeline_type, typename TGraphicsDevice::barrier_type>>
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

    protected:
        RenderBackend() noexcept = default;
        RenderBackend(RenderBackend&&) noexcept = default;
        RenderBackend(const RenderBackend&) = default;
        RenderBackend& operator=(RenderBackend&&) noexcept = default;
        RenderBackend& operator=(const RenderBackend&) = default;

    public:
        ~RenderBackend() noexcept override = default;

    public:
        /// @copydoc IRenderBackend::listAdapters()
        virtual const Array<SharedPtr<const adapter_type>>& adapters() const = 0;

        /// @copydoc IRenderBackend::findAdapter(const Optional<UInt64>&) const
        const adapter_type* findAdapter(const Optional<UInt64>& adapterId = std::nullopt) const override = 0;

        /// @copydoc IRenderBackend::findAdapter(GpuPreference) const
        const adapter_type* findAdapter(GpuPreference preference) const override = 0;

        /// @brief Registers a new device on the backend.
        ///
        /// @param name The name of the device for lookups.
        /// @param device A pointer to the device.
        virtual void registerDevice(const String& name, SharedPtr<device_type>&& device) = 0;

        /// @brief Creates a new graphics device.
        ///
        /// @param name The name of the device for lookups.
        /// @param adapter The adapter from which to create the device.
        /// @param surface The surface that the device draws to.
        /// @param _args The arguments that are passed to the graphics device constructor.
        /// @return A pointer of the created graphics device instance.
        template <typename TSelf, typename ...TArgs>
        inline device_type& createDevice(this TSelf& self, const String& name, const adapter_type& adapter, UniquePtr<surface_type>&& surface, TArgs&&... _args) {
            auto devicePtr = device_type::create(self, adapter, std::move(surface), std::forward<TArgs>(_args)...);
            auto& device = *devicePtr;
            self.registerDevice(name, std::move(devicePtr));
            return device;
        }

        /// @brief Destroys and removes a device from the backend.
        ///
        /// @param name The name of the device.
        virtual void releaseDevice(const String& name) = 0;

        /// @copydoc IRenderBackend::device(const String&)
        device_type* device(const String& name) override = 0;

        /// @copydoc IRenderBackend::device(const String&) const
        const device_type* device(const String& name) const override = 0;

        /// @copydoc IRenderBackend::operator[](const String&) const
        inline const device_type* operator[](const String& name) const noexcept override {
            return this->device(name);
        };

        /// @copydoc IRenderBackend::operator[](const String&)
        inline device_type* operator[](const String& name) noexcept override {
            return this->device(name);
        };

        // IRenderBackend interface
    private:
        inline Enumerable<SharedPtr<const IGraphicsAdapter>> getAdapters() const override {
            return this->adapters();
        }
    };
}

// NOLINTEND(bugprone-derived-method-shadowing-base-method)