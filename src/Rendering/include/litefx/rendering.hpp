#pragma once

#include <litefx/app.hpp>
#include <litefx/math.hpp>
#include <litefx/rendering_api.hpp>
#include <litefx/rendering_formatters.hpp>
#include <litefx/rendering.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;
    using namespace LiteFX::Math;

    /// <summary>
    /// Describes a buffer layout.
    /// </summary>
    /// <seealso cref="IVertexBufferLayout" />
    /// <seealso cref="IIndexBufferLayout" />
    /// <seealso cref="IDescriptorLayout" />
    class IBufferLayout {
    public:
        virtual ~IBufferLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the size of a single element within the buffer.
        /// </summary>
        /// <returns>The size of a single element within the buffer.</returns>
        virtual size_t elementSize() const noexcept = 0;

        /// <summary>
        /// Returns the binding point, the buffer will be bound to.
        /// </summary>
        /// <remarks>
        /// In GLSL, the binding point is identified by the <c>binding</c> keyword, whilst in HLSL the binding maps to a register.
        /// </remarks>
        /// <returns>The binding point, the buffer will be bound to.</returns>
        virtual const UInt32& binding() const noexcept = 0;

        /// <summary>
        /// Returns the buffer type of the buffer.
        /// </summary>
        /// <returns>The buffer type of the buffer.</returns>
        virtual const BufferType& type() const noexcept = 0;
    };

    /// <summary>
    /// Describes a vertex buffer layout.
    /// </summary>
    /// <seealso cref="IVertexBufferBuffer" />
    class IVertexBufferLayout : public IBufferLayout {
    public:
        virtual ~IVertexBufferLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the vertex buffer attributes.
        /// </summary>
        /// <returns>The vertex buffer attributes.</returns>
        virtual Array<const BufferAttribute*> attributes() const noexcept = 0;
    };

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
    /// Describes a index buffer layout.
    /// </summary>
    /// <seealso cref="IIndexBuffer" />
    class IIndexBufferLayout : public IBufferLayout {
    public:
        virtual ~IIndexBufferLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the index type of the index buffer.
        /// </summary>
        /// <returns>The index type of the index buffer.</returns>
        virtual const IndexType& indexType() const noexcept = 0;
    };

    /// <summary>
    /// Describes a the layout of a single descriptor within a <see cref="IDescriptorSet" />.
    /// </summary>
    /// <seealso cref="IDescriptorSetLayout" />
    class IDescriptorLayout : public IBufferLayout {
    public:
        virtual ~IDescriptorLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the type of the descriptor.
        /// </summary>
        /// <returns>The type of the descriptor.</returns>
        virtual const DescriptorType& descriptorType() const noexcept = 0;

        /// <summary>
        /// Returns the number of descriptors in the descriptor array.
        /// </summary>
        /// <returns>The number of descriptors in the descriptor array.</returns>
        virtual const UInt32& descriptors() const noexcept = 0;
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
        /// <param name="element">The array element to map the data to.</param>
        virtual void map(const void* const data, const size_t& size, const UInt32& element = 0) = 0;

        /// <summary>
        /// Maps the memory blocks within <paramref name="data" /> to the internal memory of an array.
        /// </summary>
        /// <param name="data">The data blocks to map.</param>
        /// <param name="size">The size of each data block within <paramref name="data" />.</param>
        /// <param name="firsElement">The first element of the array to map.</param>
        virtual void map(Span<const void* const> data, const size_t& elementSize, const UInt32& firstElement = 0) = 0;
    };

    /// <summary>
    /// Describes a chunk of device memory.
    /// </summary>
    class IDeviceMemory {
    public:
        virtual ~IDeviceMemory() noexcept = default;

    public:
        /// <summary>
        /// Gets the number of sub-resources inside the memory chunk.
        /// </summary>
        /// <remarks>
        /// For buffers, this equals the number of array elements. For images, this equals the product of layers, levels and planes. This number represents the number of states, that 
        /// can be obtained by calling the <see cref="state" /> method.
        /// </remarks>
        /// <returns>The number of array elements inside the memory chunk.</returns>
        /// <seealso cref="state" />
        virtual const UInt32& elements() const noexcept = 0;

        /// <summary>
        /// Gets the size (in bytes) of the aligned memory chunk.
        /// </summary>
        /// <remarks>
        /// The size of the device memory block depends on different factors. The actual used memory of one element can be obtained by calling by the <see cref="elementSize" />. For 
        /// different reasons, though, elements may be required to be aligned to a certain size. The size of one aligned element is returned by <see cref="alignedElementSize" />.
        /// The size of the memory block, the elements get aligned to is returned by <see cref="elementAlignment" />.
        /// </remarks>
        /// <returns>The size (in bytes) of the memory chunk.</returns>
        /// <seealso cref="elements" />
        /// <seealso cref="elementSize" />
        /// <seealso cref="elementAlignment" />
        /// <seealso cref="alignedElementSize" />
        virtual size_t size() const noexcept = 0;

        /// <summary>
        /// Returns the size of a single element within the buffer. If there is only one element, this is equal to <see cref="size" />.
        /// </summary>
        /// <returns>The size of a single element within the buffer</returns>
        /// <seealso cref="elementAlignment" />
        /// <seealso cref="alignedElementSize" />
        virtual size_t elementSize() const noexcept = 0;

        /// <summary>
        /// Returns the alignment of a single element.
        /// </summary>
        /// <returns>The alignment of a single element.</returns>
        /// <seealso cref="elementSize" />
        /// <seealso cref="alignedElementSize" />
        virtual size_t elementAlignment() const noexcept = 0;

        /// <summary>
        /// Returns the actual size of the element in device memory.
        /// </summary>
        /// <returns>The actual size of the element in device memory.</returns>
        /// <seealso cref="elementAlignment" />
        /// <seealso cref="elementSize" />
        virtual size_t alignedElementSize() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if the resource can be bound to a read/write descriptor.
        /// </summary>
        /// <remarks>
        /// If the resource is not writable, attempting to bind it to a writable descriptor will result in an exception.
        /// </remarks>
        /// <returns><c>true</c>, if the resource can be bound to a read/write descriptor.</returns>
        virtual const bool& writable() const noexcept = 0;

        /// <summary>
        /// Returns the current state of the resource.
        /// </summary>
        /// <param name="subresource">The index of the sub-resource for which the state is requested.</param>
        /// <returns>The current state of the resource.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the specified sub-resource is not an element of the resource.</exception>
        virtual const ResourceState& state(const UInt32& subresource = 0) const = 0;

        /// <summary>
        /// Returns a reference of the current state of the resource.
        /// </summary>
        /// <remarks>
        /// This overload can be used to change the internal resource state. It exists, to support external resource transitions in certain scenarios, where 
        /// automatic resource state tracking is not supported. For example, there might be implicit state transitions in some scenarios. Usually those scenarios
        /// do not require you to transition the resource into another state, however if you have to, the internal state of the resource does not match the 
        /// actual state. In order for the barrier to be well-formed, you have to set the proper state first.
        /// 
        /// In most cases, however, use a <see cref="IBarrier" /> to transition between resource states.
        /// </remarks>
        /// <param name="subresource">The index of the sub-resource for which the state is requested.</param>
        /// <returns>A reference of the current state of the resource.</returns>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the specified sub-resource is not an element of the resource.</exception>
        virtual ResourceState& state(const UInt32& subresource = 0) = 0;
    };

    /// <summary>
    /// Base interface for buffer objects.
    /// </summary>
    class IBuffer : public virtual IDeviceMemory, public virtual IMappable {
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
    /// Describes a generic image.
    /// </summary>
    class IImage : public virtual IDeviceMemory {
    public:
        virtual ~IImage() noexcept = default;

    public:
        /// <summary>
        /// Returns the size (in bytes) of an image at a specified mip map level. If the image does not contain the provided mip map level, the method returns <c>0</c>.
        /// </summary>
        /// <remarks>
        /// Note that the size will only be returned for one layer. You have to multiply this value by the number of layers, if you want to receive the size of all layers
        /// of a certain mip-map level. This is especially important, if you use cube mapping, because this method will only return the size of one face.
        /// </remarks>
        /// <param name="level">The mip map level to return the size for.</param>
        /// <returns>The size (in bytes) of an image at a specified mip map level.</returns>
        virtual size_t size(const UInt32& level) const noexcept = 0;

        /// <summary>
        /// Gets the extent of the image at a certain mip-map level.
        /// </summary>
        /// <remarks>
        /// Not all components of the extent are actually used. Check the <see cref="dimensions" /> to see, which components are required. The extent will be 0 for invalid 
        /// mip-map levels and 1 or more for valid mip map levels.
        /// </remarks>
        /// <returns>The extent of the image at a certain mip-map level.</returns>
        /// <seealso cref="dimensions" />
        virtual Size3d extent(const UInt32& level = 0) const noexcept = 0;

        /// <summary>
        /// Gets the internal format of the image.
        /// </summary>
        /// <returns>The internal format of the image.</returns>
        virtual const Format& format() const noexcept = 0;

        /// <summary>
        /// Gets the images dimensionality.
        /// </summary>
        /// <remarks>
        /// The dimensions imply various things, most importantly, which components of the <see cref="extent" /> are used. Note that cube maps behave like 2D images when the 
        /// extent is used.
        /// </remarks>
        /// <returns>The images dimensionality.</returns>
        virtual const ImageDimensions& dimensions() const noexcept = 0;

        /// <summary>
        /// Gets the number of mip-map levels of the image.
        /// </summary>
        /// <returns>The number of mip-map levels of the image.</returns>
        virtual const UInt32& levels() const noexcept = 0;

        /// <summary>
        /// Gets the number of layers (slices) of the image.
        /// </summary>
        /// <returns>The number of layers (slices) of the image.</returns>
        virtual const UInt32& layers() const noexcept = 0;

        /// <summary>
        /// Returns the number of planes of the image resource.
        /// </summary>
        /// <remarks>
        /// The number of planes is dictated by the image format.
        /// </remarks>
        /// <returns>The number of planes of the image resource.</returns>
        /// <seealso cref="format" />
        virtual const UInt32& planes() const noexcept = 0;

        /// <summary>
        /// Gets the number of samples of the texture.
        /// </summary>
        /// <returns>The number of samples of the texture.</returns>
        virtual const MultiSamplingLevel& samples() const noexcept = 0;

        // TODO: getSampler() for combined samplers?
    };

    /// <summary>
    /// Describes a texture sampler.
    /// </summary>
    class ISampler {
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
    /// A barrier that transitions a set of resources backed by <see cref="IDeviceMemory" /> into different <see cref="ResourceState" />.
    /// </summary>
    /// <remarks>
    /// It is recommended to insert multiple transitions into one single barrier. This can be done by calling <see cref="transition" /> multiple times. 
    /// </remarks>
    template <typename TBuffer, typename TImage> requires
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TImage, IImage>
    class IBarrier {
    public:
        using buffer_type = TBuffer;
        using image_type = TImage;

    public:
        virtual ~IBarrier() noexcept = default;

    public:
        /// <summary>
        /// Inserts a transition for all sub-resources of <paramref name="buffer"/> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="buffer">The resource to transition.</param>
        /// <param name="targetState">The target state to transition the resource to.</param>
        virtual void transition(TBuffer& buffer, const ResourceState& targetState) = 0;

        /// <summary>
        /// Inserts a transition for the sub-resource <paramref name="element" /> of <paramref name="buffer" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="buffer">The resource to transition.</param>
        /// <param name="element">The element of the resource to transition.</param>
        /// <param name="targetState">The target state to transition the sub-resource to.</param>
        virtual void transition(TBuffer& buffer, const UInt32& element, const ResourceState& targetState) = 0;

        /// <summary>
        /// Inserts a transition for all sub-resources of <paramref name="buffer"/> from <paramref name="sourceState" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="buffer">The resource to transition.</param>
        /// <param name="sourceState">The source state to transition the resource from.</param>
        /// <param name="targetState">The target state to transition the resource to.</param>
        virtual void transition(TBuffer& buffer, const ResourceState& sourceState, const ResourceState& targetState) = 0;

        /// <summary>
        /// Inserts a transition for the sub-resource <paramref name="element" /> of <paramref name="buffer" /> from <paramref name="sourceState" /> into 
        /// <paramref name="targetState" />.
        /// </summary>
        /// <param name="buffer">The resource to transition.</param>
        /// <param name="sourceState">The source state to transition the sub-resource from.</param>
        /// <param name="element">The element of the resource to transition.</param>
        /// <param name="targetState">The target state to transition the sub-resource to.</param>
        virtual void transition(TBuffer& buffer, const ResourceState& sourceState, const UInt32& element, const ResourceState& targetState) = 0;

        /// <summary>
        /// Inserts a transition for all sub-resources of <paramref name="image"/> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="image">The resource to transition.</param>
        /// <param name="targetState">The target state to transition the resource to.</param>
        virtual void transition(TImage& image, const ResourceState& targetState) = 0;

        /// <summary>
        /// Inserts a transition for a sub-resource of <paramref name="image" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="image">The resource to transition.</param>
        /// <param name="level">The mip-map level of the sub-resource.</param>
        /// <param name="layer">The array layer of the sub-resource.</param>
        /// <param name="plane">The plane of the sub-resource.</param>
        /// <param name="targetState">The target state to transition the sub-resource to.</param>
        virtual void transition(TImage& image, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) = 0;

        /// <summary>
        /// Inserts a transition for all sub-resources of <paramref name="image"/> from <paramref name="sourceState" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="image">The resource to transition.</param>
        /// <param name="sourceState">The source state to transition the resource from.</param>
        /// <param name="targetState">The target state to transition the resource to.</param>
        virtual void transition(TImage& image, const ResourceState& sourceState, const ResourceState& targetState) = 0;

        /// <summary>
        /// Inserts a transition for a sub-resource of <paramref name="image" /> from <paramref name="sourceState" /> into <paramref name="targetState" />.
        /// </summary>
        /// <param name="image">The resource to transition.</param>
        /// <param name="sourceState">The source state to transition the sub-resource from.</param>
        /// <param name="level">The mip-map level of the sub-resource.</param>
        /// <param name="layer">The array layer of the sub-resource.</param>
        /// <param name="plane">The plane of the sub-resource.</param>
        /// <param name="targetState">The target state to transition the sub-resource to.</param>
        virtual void transition(TImage& image, const ResourceState& sourceState, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState) = 0;

        /// <summary>
        /// Inserts a barrier that waits for all read/write accesses to <paramref name="buffer" /> to be finished before continuing.
        /// </summary>
        /// <remarks>
        /// This translates to a UAV barrier in DirectX 12 and an execution + memory barrier with no layout transition in Vulkan.
        /// </remarks>
        /// <typeparam name="buffer">The buffer to wait for.</typeparam>
        virtual void waitFor(const TBuffer& buffer) = 0;

        /// <summary>
        /// Inserts a barrier that waits for all read/write accesses to <paramref name="image" /> to be finished before continuing.
        /// </summary>
        /// <remarks>
        /// This translates to a UAV barrier in DirectX 12 and an execution + memory barrier with no layout transition in Vulkan.
        /// </remarks>
        /// <typeparam name="image">The image to wait for.</typeparam>
        virtual void waitFor(const TImage& image) = 0;
    };

    /// <summary>
    /// Represents a command buffer, that buffers commands that should be submitted to a <see cref="ICommandQueue" />.
    /// </summary>
    template <typename TBuffer, typename TImage, typename TBarrier> requires
        rtti::implements<TBarrier, IBarrier<TBuffer, TImage>>
    class ICommandBuffer {
    public:
        using buffer_type = TBuffer;
        using image_type = TImage;
        using barrier_type = TBarrier;

    public:
        virtual ~ICommandBuffer() noexcept = default;

    public:
        /// <summary>
        /// Waits for the command buffer to be executed.
        /// </summary>
        /// <remarks>
        /// If the command buffer gets submitted, it does not necessarily get executed straight away. If you depend on a command buffer to be finished, you can call this method.
        /// </remarks>
        virtual void wait() const = 0;

        /// <summary>
        /// Sets the command buffer into recording state, so that it can receive command that should be submitted to the parent <see cref="ICommandQueue" />.
        /// </summary>
        /// <remarks>
        /// Note that, if a command buffer has been submitted before, this method waits for the earlier commands to be executed by calling <see cref="wait" />.
        /// </remarks>
        virtual void begin() const = 0;

        /// <summary>
        /// Ends recording commands on the command buffer.
        /// </summary>
        /// <param name="submit">If set to <c>true</c>, the command buffer is automatically submitted by calling the <see cref="submit" /> method.</param>
        /// <param name="wait">If <paramref name="submit" /> is set to <c>true</c>, this parameter gets passed to the <see cref="submit" /> method.</param>
        /// <seealso cref="submit" />
        virtual void end(const bool& submit = true, const bool& wait = false) const = 0;

        /// <summary>
        /// Submits the command buffer to the parent command queue.
        /// </summary>
        /// <remarks>
        /// </remarks>
        /// <param name="wait">If set to <c>true</c>, the command buffer blocks, until the submitted commands have been executed.</param>
        /// <seealso cref="wait" />
        virtual void submit(const bool& wait = false) const = 0;

    public:
        /// <summary>
        /// Executes the transitions that have been added to <paramref name="barrier" />.
        /// </summary>
        /// <remarks>
        /// Calling this method will also update the resource states of each resource within the barrier. However, the actual state of the resource does not change until the barrier
        /// is executed on the command queue. Keep this in mind when inserting multiple barriers from different threads or in different command buffers, which may not be executed in 
        /// order. You might have to manually synchronize barrier execution.
        /// </remarks>
        /// <param name="barrier">The barrier containing the transitions to perform.</param>
        /// <param name="invert">If set to <c>true</c>, the barrier will perform a transition back to the original resource states.</param>
        virtual void barrier(const TBarrier& barrier, const bool& invert = false) const noexcept = 0;

        /// <summary>
        /// Uses the image at level *0* to generate mip-maps for the remaining levels.
        /// </summary>
        /// <remarks>
        /// It is strongly advised, not to generate mip maps at runtime. Instead, prefer using a format that supports pre-computed mip maps. If you have to, prefer computing
        /// mip maps in a pre-process.
        /// 
        /// Note that not all texture formats and sizes are supported for mip map generation and the result might not be satisfactory. For example, it is not possible to compute 
        /// proper mip maps for pre-compressed formats. Textures should have power of two sizes in order to not appear under-sampled.
        /// 
        /// Note that generating mip maps might require the texture to be writable. You can transfer the texture into a non-writable resource afterwards to improve performance.
        /// </remarks>
        /// <param name="commandBuffer">The command buffer used to issue the transition and transfer operations.</param>
        virtual void generateMipMaps(TImage& image) noexcept = 0;
        
        /// <summary>
        /// Performs a buffer-to-buffer transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="targetElement">The index of the first element in the target buffer to copy to.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target buffer.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        virtual void transfer(const TBuffer& source, const TBuffer& target, const UInt32& sourceElement = 0, const UInt32& targetElement = 0, const UInt32& elements = 1) const = 0;

        /// <summary>
        /// Performs a buffer-to-image transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// The <paramref name="subresource" /> parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where 
        /// resources are counted in the following order:
        /// 
        /// <list type="bullet">
        ///     <item>
        ///         <term>Level</term>
        ///         <description>Contains the mip-map levels.</description>
        ///     </item>
        ///     <item>
        ///         <term>Layer</term>
        ///         <description>Contains the array slices.</description>
        ///     </item>
        ///     <item>
        ///         <term>Plane</term>
        ///         <description>Contains planes for multi-planar formats.</description>
        ///     </item>
        /// </list>
        /// 
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 
        /// contain the three mip-map levels of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure 
        /// would be repeated for each plane, however one buffer element only maps to one sub-resource.
        /// </remarks>
        /// <param name="source">The source buffer to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceElement">The index of the first element in the source buffer to copy.</param>
        /// <param name="firstSubresource">The index of the first sub-resource of the target image to receive data.</param>
        /// <param name="elements">The number of elements to copy from the source buffer into the target image sub-resources.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        virtual void transfer(const TBuffer& source, const TImage& target, const UInt32& sourceElement = 0, const UInt32& firstSubresource = 0, const UInt32& elements = 1) const = 0;

        /// <summary>
        /// Performs an image-to-image transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target image to transfer data to.</param>
        /// <param name="sourceSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetSubresource">The image of the first sub-resource in the target image to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy between the images.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        virtual void transfer(const TImage& source, const TImage& target, const UInt32& sourceSubresource = 0, const UInt32& targetSubresource = 0, const UInt32& subresources = 1) const = 0;

        /// <summary>
        /// Performs an image-to-buffer transfer from <paramref name="source" /> to <paramref name="target" />.
        /// </summary>
        /// <remarks>
        /// The <paramref name="firstSubresource" /> parameter describes the index of the first sub-resource to copy. Each element gets copied into the subsequent sub-resource, where 
        /// resources are counted in the following order:
        /// 
        /// <list type="bullet">
        ///     <item>
        ///         <term>Level</term>
        ///         <description>Contains the mip-map levels.</description>
        ///     </item>
        ///     <item>
        ///         <term>Layer</term>
        ///         <description>Contains the array slices.</description>
        ///     </item>
        ///     <item>
        ///         <term>Plane</term>
        ///         <description>Contains planes for multi-planar formats.</description>
        ///     </item>
        /// </list>
        /// 
        /// E.g., if 6 elements should be copied to an image with 3 mip-map levels and 3 layers, the elements 0-2 contain the mip-map levels of the first layer, while elements 3-5 
        /// contain the three mip-map levels of the second layer. The third layer would not receive any data in this example. If the image format has multiple planes, this procedure 
        /// would be repeated for each plane, however one buffer element only maps to one sub-resource.
        /// </remarks>
        /// <param name="source">The source image to transfer data from.</param>
        /// <param name="target">The target buffer to transfer data to.</param>
        /// <param name="firstSubresource">The index of the first sub-resource to copy from the source image.</param>
        /// <param name="targetElement">The index of the first target element to receive data.</param>
        /// <param name="subresources">The number of sub-resources to copy.</param>
        /// <exception cref="ArgumentOutOfRangeException">Thrown, if the number of either the source buffer or the target buffer has not enough elements for the specified <paramref name="elements" /> parameter.</exception>
        virtual void transfer(const TImage& source, const TBuffer& target, const UInt32& firstSubresource = 0, const UInt32& targetElement = 0, const UInt32& subresources = 1) const = 0;
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
    /// calling <see cref="IDescriptorSet::update" />. Note that this does not automatically ensure, that the buffer memory is visible for the GPU. Instead, a buffer may also 
    /// require a transfer into GPU visible memory, depending on the <see cref="BufferUsage" />. However, as long as a descriptor within a set is mapped to a buffer, modifying 
    /// this buffer also reflects the change to the shader, without requiring to update the descriptor, similarly to how modifying the object behind a pointer does not require 
    /// the pointer to change.
    /// 
    /// Note, that there might be multiple descriptor set instances of the same <see cref="IDescriptorSetLayout" />, pointing to different <see cref="IBuffer" /> instances, 
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
    /// containing different data into one giant buffer block. Calling <see cref="IRenderPipeline::bind" /> for a descriptor set would then receive an additional dynamic 
    /// offset for each descriptor within the descriptor set.
    /// </remarks>
    /// <typeparam name="TBuffer">The type of the buffer interface. Must inherit from <see cref="IBuffer"/>.</typeparam>
    /// <typeparam name="TImage">The type of the image interface. Must inherit from <see cref="IImage"/>.</typeparam>
    /// <typeparam name="TSampler">The type of the sampler interface. Must inherit from <see cref="ISampler"/>.</typeparam>
    /// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="ICommandBuffer"/>.</typeparam>
    /// <seealso cref="IDescriptorSetLayout" />
    template <typename TBuffer, typename TImage, typename TSampler, typename TCommandBuffer, typename TBarrier = TCommandBuffer::barrier_type> requires
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TSampler, ISampler> &&
        std::derived_from<TImage, IImage> &&
        rtti::implements<TCommandBuffer, ICommandBuffer<TBuffer, TImage, TBarrier>>
    class IDescriptorSet {
    public:
        using buffer_type = TBuffer;
        using sampler_type = TSampler;
        using image_type = TImage;
        using command_buffer_type = TCommandBuffer;

    public:
        virtual ~IDescriptorSet() noexcept = default;

    public:
        /// <summary>
        /// Updates a constant buffer within the current descriptor set.
        /// </summary>
        /// <param name="binding">The buffer binding point.</param>
        /// <param name="buffer">The constant buffer to write to the descriptor set.</param>
        /// <param name="bufferElement">The index of the first element in the buffer to bind to the descriptor set.</param>
        /// <param name="elements">The number of elements from the buffer to bind to the descriptor set.</param>
        /// <param name="firstDescriptor">The index of the first descriptor in the descriptor array to update.</param>
        virtual void update(const UInt32& binding, const TBuffer& buffer, const UInt32& bufferElement = 0, const UInt32& elements = 1, const UInt32& firstDescriptor = 0) const = 0;

        /// <summary>
        /// Updates a texture within the current descriptor set.
        /// </summary>
        /// <remarks>
        /// The exact representation of the level and layer parameters depends on the dimension of the provided texture, as well as the type of the descriptor identified by the 
        /// <paramref name="binding" /> parameter.
        /// 
        /// If the texture itself is not an array (i.e. the number of layers equals `1`), the parameters <paramref name="firstLayer" /> and <paramref name="layers" /> are ignored.
        /// 
        /// The descriptor type dictates, how mip-maps can be provided. If the descriptor type identifies a *writable texture*, the <paramref name="firstLevel" /> parameter specifies 
        /// the mip-map level to write to (or read from). Multiple levels are not allowed in this case, so the <paramref name="levels" /> parameter is ignored. Instead, you have to 
        /// bind them to separate descriptors. Furthermore, the <paramref name="firstLayer" /> and <paramref name="layers" /> parameter can be used to specify the number of depth
        /// or W-slices of a writable 3D texture or the side(s) of a cube map.
        /// </remarks>
        /// <param name="binding">The texture binding point.</param>
        /// <param name="texture">The texture to write to the descriptor set.</param>
        /// <param name="descriptor">The index of the descriptor in the descriptor array to bind the texture to.</param>
        /// <param name="firstLevel">The index of the first mip-map level to bind.</param>
        /// <param name="levels">The number of mip-map levels to bind. A value of `0` binds all available levels, starting at <paramref name="firstLevel" />.</param>
        /// <param name="firstLayer">The index of the first layer to bind.</param>
        /// <param name="layers">The number of layers to bind. A value of `0` binds all available layers, starting at <paramref name="firstLayer" />.</param>
        virtual void update(const UInt32& binding, const TImage& texture, const UInt32& descriptor = 0, const UInt32& firstLevel = 0, const UInt32& levels = 0, const UInt32& firstLayer = 0, const UInt32& layers = 0) const = 0;

        /// <summary>
        /// Updates a sampler within the current descriptor set.
        /// </summary>
        /// <param name="binding">The sampler binding point.</param>
        /// <param name="sampler">The sampler to write to the descriptor set.</param>
        /// <param name="descriptor">The index of the descriptor in the descriptor array to bind the sampler to.</param>
        virtual void update(const UInt32& binding, const TSampler& sampler, const UInt32& descriptor = 0) const = 0;

        /// <summary>
        /// Attaches an image as an input attachment to a descriptor bound at <paramref cref="binding" />.
        /// </summary>
        /// <param name="binding">The input attachment binding point.</param>
        /// <param name="image">The image to bind to the input attachment descriptor.</param>
        virtual void attach(const UInt32& binding, const TImage& image) const = 0;
    };

    /// <summary>
    /// Describes the layout of a descriptor set.
    /// </summary>
    /// <remarks>
    /// A descriptor set groups together multiple descriptors. This concept is identified by the <c>set</c> keyword in GLSL and <c>space</c> in HLSL.
    /// 
    /// For more information on buffer binding and resource management, refer to the remarks of the <see cref="IDescriptorSet" /> interface.
    /// </remarks>
    /// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout"/>.</typeparam>
    /// <typeparam name="TDescriptorSet">The type of the descriptor set. Must implement <see cref="IDescriptorSet"/>.</typeparam>
    /// <seealso cref="IDescriptorLayout" />
    /// <seealso cref="IDescriptorSet" />
    template <typename TDescriptorLayout, typename TDescriptorSet, typename TBuffer = TDescriptorSet::buffer_type, typename TSampler = TDescriptorSet::sampler_type, typename TImage = TDescriptorSet::image_type, typename TCommandBuffer = TDescriptorSet::command_buffer_type> requires
        rtti::implements<TDescriptorLayout, IDescriptorLayout> &&
        rtti::implements<TDescriptorSet, IDescriptorSet<TBuffer, TImage, TSampler, TCommandBuffer>>
    class IDescriptorSetLayout {
    public:
        using descriptor_layout_type = TDescriptorLayout;
        using descriptor_set_type = TDescriptorSet;

    public:
        virtual ~IDescriptorSetLayout() noexcept = default;

	public:
		/// <summary>
		/// Returns the layouts of the descriptors within the descriptor set.
		/// </summary>
		/// <returns>The layouts of the descriptors within the descriptor set.</returns>
		virtual Array<const TDescriptorLayout*> descriptors() const noexcept = 0;

		/// <summary>
		/// Returns the descriptor layout for the descriptor bound to the binding point provided with <paramref name="binding" />.
		/// </summary>
		/// <param name="binding">The binding point of the requested descriptor layout.</param>
		/// <returns>The descriptor layout for the descriptor bound to the binding point provided with <paramref name="binding" />.</returns>
		virtual const TDescriptorLayout& descriptor(const UInt32& binding) const = 0;

        /// <summary>
        /// Returns the space index of the descriptor set.
        /// </summary>
        /// <remarks>
        /// The descriptor set space maps to the space index in HLSL and the set index in GLSL.
        /// </remarks>
        /// <returns>The space index of the descriptor set.</returns>
        virtual const UInt32& space() const noexcept = 0;

        /// <summary>
        /// Returns the shader stages, the descriptor set is used in.
        /// </summary>
        /// <returns>The shader stages, the descriptor set is used in.</returns>
        virtual const ShaderStage& shaderStages() const noexcept = 0;

        /// <summary>
        /// Returns the number of uniform/constant buffer descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of uniform/constant buffer descriptors.</returns>
        virtual UInt32 uniforms() const noexcept = 0;

        /// <summary>
        /// Returns the number of shader storage buffer/unordered access view descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of shader storage buffer/unordered access view descriptors.</returns>
        virtual UInt32 storages() const noexcept = 0;

        /// <summary>
        /// Returns the number of image descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of image descriptors.</returns>
        virtual UInt32 images() const noexcept = 0;

        /// <summary>
        /// Returns the number of texel/structured buffer descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of texel/structured buffer descriptors.</returns>
        virtual UInt32 buffers() const noexcept = 0;

        /// <summary>
        /// Returns the number of sampler descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of sampler descriptors.</returns>
        virtual UInt32 samplers() const noexcept = 0;

        /// <summary>
        /// Returns the number of input attachment descriptors within the descriptor set.
        /// </summary>
        /// <returns>The number of input attachment descriptors.</returns>
        virtual UInt32 inputAttachments() const noexcept = 0;

    public:
        /// <summary>
        /// Allocates a new descriptor set or returns an instance of an unused descriptor set.
        /// </summary>
        /// <remarks>
        /// Allocating a new descriptor set may be an expensive operation. To improve performance, and prevent fragmentation, the descriptor set layout keeps track of
        /// created descriptor sets. It does this by never releasing them. Instead, when a <see cref="IDescriptorSet" /> instance gets destroyed, it should call 
        /// <see cref="free" /> in order to mark itself (i.e. its handle) as not being used any longer.
        /// 
        /// Before allocating a new descriptor set from a pool (which may even result in the creation of a new pool, if the existing pools are full), the layout tries 
        /// to hand out descriptor sets that marked as unused.
        /// 
        /// Descriptor sets are only deleted, if the whole layout instance and therefore the descriptor pools are deleted.
        /// </remarks>
        /// <returns>The instance of the descriptor set.</returns>
        virtual UniquePtr<TDescriptorSet> allocate() const noexcept = 0;

        /// <summary>
        /// Allocates an array of descriptor sets.
        /// </summary>
        /// <param name="descriptorSets">The number of descriptor sets to allocate.</param>
        /// <returns>The array of descriptor set instances.</returns>
        virtual Array<UniquePtr<TDescriptorSet>> allocate(const UInt32& descriptorSets) const noexcept = 0;

        /// <summary>
        /// Marks a descriptor set as unused, so that it can be handed out again instead of allocating a new one.
        /// </summary>
        virtual void free(const TDescriptorSet& descriptorSet) const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TDescriptorSetLayout, typename TParent, typename TDescriptorLayout = TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSet = TDescriptorSetLayout::descriptor_set_type> requires
        rtti::implements<TDescriptorSetLayout, IDescriptorSetLayout<TDescriptorLayout, TDescriptorSet>>
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
    /// Represents a shader program, consisting of multiple <see cref="IShaderModule" />s.
    /// </summary>
    /// <typeparam name="TShaderModule">The type of the shader module. Must implement <see cref="IShaderModule"/>.</typeparam>
    template <typename TShaderModule> requires
        rtti::implements<TShaderModule, IShaderModule>
    class IShaderProgram {
    public:
        using shader_module_type = TShaderModule;

    public:
        virtual ~IShaderProgram() noexcept = default;

    public:
        /// <summary>
        /// Returns the modules, the shader program is build from.
        /// </summary>
        /// <returns>The modules, the shader program is build from.</returns>
        virtual Array<const TShaderModule*> modules() const noexcept = 0;
    };

	/// <summary>
	/// 
	/// </summary>
	template <typename TDerived, typename TShaderProgram, typename TParent, typename TShaderModule = typename TShaderProgram::shader_module_type> requires
		rtti::implements<TShaderProgram, IShaderProgram<TShaderModule>>
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
	/// Represents a the layout of a <see cref="IRenderPipeline" />.
	/// </summary>
	/// <typeparam name="TDescriptorSetLayout">The type of the descriptor set layout. Must implement <see cref="IDescriptorSetLayout"/>.</typeparam>
	/// <typeparam name="TShaderProgram">The type of the shader program. Must implement <see cref="IShaderProgram"/>.</typeparam>
	template <typename TDescriptorSetLayout, typename TShaderProgram, typename TDescriptorLayout = TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSet = TDescriptorSetLayout::descriptor_set_type, typename TShaderModule = TShaderProgram::shader_module_type> requires
		rtti::implements<TDescriptorSetLayout, IDescriptorSetLayout<TDescriptorLayout, TDescriptorSet>> &&
		rtti::implements<TShaderProgram, IShaderProgram<TShaderModule>>
	class IPipelineLayout {
	public:
		using descriptor_set_layout_type = TDescriptorSetLayout;
		using shader_program_type = TShaderProgram;
		using descriptor_set_type = TDescriptorSet;
        using command_buffer_type = TDescriptorSet::command_buffer_type;

	public:
		virtual ~IPipelineLayout() noexcept = default;

    public:
        /// <summary>
        /// Returns the shader program, the pipeline uses for drawing.
        /// </summary>
        /// <returns>The shader program, the pipeline uses for drawing.</returns>
        virtual const TShaderProgram& program() const noexcept = 0;

		/// <summary>
		/// Returns the descriptor set layout for the descriptor set that is bound to the space provided by <paramref name="space" />.
		/// </summary>
		/// <param name="space">The space to request the descriptor set layout for.</param>
		/// <returns>The descriptor set layout for the descriptor set that is bound to the space provided by <paramref name="space" />.</returns>
		virtual const TDescriptorSetLayout& descriptorSet(const UInt32& space) const = 0;

		/// <summary>
		/// Returns all descriptor set layouts, the pipeline has been initialized with.
		/// </summary>
		/// <returns>All descriptor set layouts, the pipeline has been initialized with.</returns>
		virtual Array<const TDescriptorSetLayout*> descriptorSets() const noexcept = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	template <typename TDerived, typename TPipelineLayout, typename TParent, typename TDescriptorSetLayout = TPipelineLayout::descriptor_set_layout_type, typename TShaderProgram = TPipelineLayout::shader_program_type> requires
		rtti::implements<TPipelineLayout, IPipelineLayout<TDescriptorSetLayout, TShaderProgram>>
	class PipelineLayoutBuilder : public Builder<TDerived, TPipelineLayout, TParent> {
	public:
		using Builder<TDerived, TPipelineLayout, TParent>::Builder;

    public:
        virtual void use(UniquePtr<TShaderProgram>&& program) = 0;
        virtual void use(UniquePtr<TDescriptorSetLayout>&& layout) = 0;
    };

    /// <summary>
    /// Describes a vertex buffer.
    /// </summary>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout"/>.</typeparam>
    template <typename TVertexBufferLayout> requires
        rtti::implements<TVertexBufferLayout, IVertexBufferLayout>
    class IVertexBuffer : public virtual IBuffer {
    public:
        using vertex_buffer_layout_type = TVertexBufferLayout;

    public:
        virtual ~IVertexBuffer() noexcept = default;

    public:
        /// <summary>
        /// Gets the layout of the vertex buffer.
        /// </summary>
        /// <returns>The layout of the vertex buffer.</returns>
        virtual const TVertexBufferLayout& layout() const noexcept = 0;
    };

    /// <summary>
    /// Describes an index buffer.
    /// </summary>
    /// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout"/>.</typeparam>
    template <typename TIndexBufferLayout> requires
        rtti::implements<TIndexBufferLayout, IIndexBufferLayout>
    class IIndexBuffer : public virtual IBuffer {
    public:
        using index_buffer_layout_type = TIndexBufferLayout;

    public:
        virtual ~IIndexBuffer() noexcept = default;

    public:
        /// <summary>
        /// Gets the layout of the index buffer.
        /// </summary>
        /// <returns>The layout of the index buffer.</returns>
        virtual const TIndexBufferLayout& layout() const noexcept = 0;
    };

    /// <summary>
    /// Represents a the input assembler state of a <see cref="IRenderPipeline" />.
    /// </summary>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout"/>.</typeparam>
    /// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout"/>.</typeparam>
    template <typename TVertexBufferLayout, typename TIndexBufferLayout> requires
        rtti::implements<TVertexBufferLayout, IVertexBufferLayout> &&
        rtti::implements<TIndexBufferLayout, IIndexBufferLayout>
    class IInputAssembler {
    public:
        using vertex_buffer_layout_type = TVertexBufferLayout;
        using index_buffer_layout_type = TIndexBufferLayout;

    public:
        virtual ~IInputAssembler() noexcept = default;

    public:
        /// <summary>
        /// Returns all vertex buffer layouts of the input assembly.
        /// </summary>
        /// <returns>All vertex buffer layouts of the input assembly.</returns>
        virtual Array<const TVertexBufferLayout*> vertexBufferLayouts() const noexcept = 0;

        /// <summary>
        /// Returns the vertex buffer layout for binding provided with <paramref name="binding" />.
        /// </summary>
        /// <param name="binding">The binding point of the vertex buffer layout.</param>
        /// <returns>The vertex buffer layout for binding provided with <paramref name="binding" />.</returns>
        virtual const TVertexBufferLayout& vertexBufferLayout(const UInt32& binding) const = 0;

        /// <summary>
        /// Returns the index buffer layout.
        /// </summary>
        /// <returns>The index buffer layout.</returns>
        virtual const TIndexBufferLayout& indexBufferLayout() const = 0;

        /// <summary>
        /// Returns the primitive topology.
        /// </summary>
        /// <returns>The primitive topology.</returns>
        virtual const PrimitiveTopology& topology() const noexcept = 0;
    };

    /// <summary>
    /// Builds a <see cref="IInputAssembler" />.
    /// </summary>
    template <typename TDerived, typename TInputAssembler, typename TParent, typename TVertexBufferLayout = TInputAssembler::vertex_buffer_layout_type, typename TIndexBufferLayout = TInputAssembler::index_buffer_layout_type> requires
        rtti::implements<TInputAssembler, IInputAssembler<TVertexBufferLayout, TIndexBufferLayout>>
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
	/// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="IPipelineLayout"/>.</typeparam>
	/// <typeparam name="TShaderProgram">The type of the shader program. Must implement <see cref="IShaderProgram"/>.</typeparam>
	/// <typeparam name="TDescriptorSetLayout">The type of the descriptor set layout. Must implement <see cref="IDescriptorSetLayout"/>.</typeparam>
	/// <typeparam name="TDescriptorSet">The type of the descriptor set. Must implement <see cref="IDescriptorSet"/>.</typeparam>
	/// <seealso cref="IRenderPipeline" />
	/// <seealso cref="IComputePipeline" />
	template <typename TPipelineLayout, typename TDescriptorSetLayout = typename TPipelineLayout::descriptor_set_layout_type, typename TShaderProgram = typename TPipelineLayout::shader_program_type, typename TDescriptorSet = typename TDescriptorSetLayout::descriptor_set_type> requires 
		rtti::implements<TPipelineLayout, IPipelineLayout<TDescriptorSetLayout, TShaderProgram>>
	class IPipeline {
	public:
		using pipeline_layout_type = TPipelineLayout;

	public:
		virtual ~IPipeline() noexcept = default;

	public:
		/// <summary>
		/// Returns the name of the render pipeline.
		/// </summary>
		/// <returns>The name of the render pipeline.</returns>
		virtual const String& name() const noexcept = 0;

		/// <summary>
		/// Returns the layout of the render pipeline.
		/// </summary>
		/// <returns>The layout of the render pipeline.</returns>
		virtual const TPipelineLayout& layout() const noexcept = 0;

		/// <summary>
		/// Binds the provided descriptor set.
		/// </summary>
		/// <param name="descriptorSet">The descriptor set to bind.</param>
		virtual void bind(const TDescriptorSet& descriptorSet) const = 0;
	};

	/// <summary>
	/// Represents a graphics <see cref="IPipeline" />.
	/// </summary>
	/// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="IPipelineLayout"/>.</typeparam>
	/// <typeparam name="TInputAssembler">The type of the input assembler state. Must implement <see cref="IInputAssembler"/>.</typeparam>
	/// <typeparam name="TVertexBufferInterface">The type of the vertex buffer interface. Must inherit from <see cref="IVertexBuffer"/>.</typeparam>
	/// <typeparam name="TIndexBufferInterface">The type of the index buffer interface. Must inherit from <see cref="IIndexBuffer"/>.</typeparam>
	/// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout"/>.</typeparam>
	/// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout"/>.</typeparam>
	/// <seealso cref="IRenderPipelineBuilder" />
	template <typename TPipelineLayout, typename TInputAssembler, typename TVertexBufferInterface, typename TIndexBufferInterface, typename TVertexBufferLayout = TVertexBufferInterface::vertex_buffer_layout_type, typename TIndexBufferLayout = TIndexBufferInterface::index_buffer_layout_type> requires
		rtti::implements<TInputAssembler, IInputAssembler<TVertexBufferLayout, TIndexBufferLayout>> &&
		std::derived_from<TVertexBufferInterface, IVertexBuffer<TVertexBufferLayout>> &&
		std::derived_from<TIndexBufferInterface, IIndexBuffer<TIndexBufferLayout>>
	class IRenderPipeline : public IPipeline<TPipelineLayout> {
	public:
		using vertex_buffer_interface_type = TVertexBufferInterface;
		using index_buffer_interface_type = TIndexBufferInterface;
		using input_assembler_type = TInputAssembler;

    public:
        virtual ~IRenderPipeline() noexcept = default;

	public:
		/// <summary>
		/// Gets the ID of the pipeline.
		/// </summary>
		/// <remarks>
		/// The pipeline ID must be unique within the render pass.
		/// </remarks>
		/// <returns>The ID of the pipeline.</returns>
		virtual const UInt32& id() const noexcept = 0;

		/// <summary>
		/// Returns the input assembler state used by the render pipeline.
		/// </summary>
		/// <returns>The input assembler state used by the render pipeline.</returns>
		virtual SharedPtr<TInputAssembler> inputAssembler() const noexcept = 0;

        /// <summary>
        /// Returns the rasterizer state used by the render pipeline.
        /// </summary>
        /// <returns>The rasterizer state used by the render pipeline.</returns>
        virtual SharedPtr<IRasterizer> rasterizer() const noexcept = 0;

        /// <summary>
        /// Returns the viewports, the render pipeline can draw to.
        /// </summary>
        /// <returns>The viewports, the render pipeline can draw to.</returns>
        virtual Array<const IViewport*> viewports() const noexcept = 0;

        /// <summary>
        /// Returns the scissors of the render pipeline.
        /// </summary>
        /// <returns>The scissors of the render pipeline.</returns>
        virtual Array<const IScissor*> scissors() const noexcept = 0;

        /// <summary>
        /// Returns a reference to the stencil reference value.
        /// </summary>
        /// <remarks>
        /// The stencil reference value is used by the stencil test and is set with each call to <see cref="IRenderPipeline::use" />.
        /// </remarks>
        /// <returns>A reference to the stencil reference value.</returns>
        virtual UInt32& stencilRef() const noexcept = 0;

        /// <summary>
        /// Returns a reference of the constant blend factors for the pipeline.
        /// </summary>
        /// <remarks>
        /// You can change the values inside this vector reference to influence the constant blend factors. Blend factors are set for all render targets that use the
        /// blend factors <c>BlendFactor::ConstantColor</c>, <c>BlendFactor::OneMinusConstantColor</c>, <c>BlendFactor::ConstantAlpha</c> or 
        /// <c>BlendFactor::OneMinusConstantAlpha</c>. They are set on each call to <see cref="IRenderPipeline::use" />.
        /// </remarks>
        /// <returns>A reference of the constant blend factors for the pipeline.</returns>
        virtual Vector4f& blendFactors() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if the pipeline uses <i>Alpha-to-Coverage</i> multi-sampling.
        /// </summary>
        /// <remarks>
        /// Alpha-to-Coverage is a multi-sampling technique used for partially transparent sprites or textures (such as foliage) to prevent visible flickering 
        /// along edges. If enabled, the alpha-channel of the first (non-depth/stencil) render target is used to generate a temporary coverage mask that is combined
        /// with the fragment coverage mask using a logical <b>AND</b>.
        /// </remarks>
        /// <returns><c>true</c>, if the pipeline uses <i>Alpha-to-Coverage</i> multi-sampling.</returns>
        /// <seealso href="https://bgolus.medium.com/anti-aliased-alpha-test-the-esoteric-alpha-to-coverage-8b177335ae4f" />
        /// <seealso href="https://en.wikipedia.org/wiki/Alpha_to_coverage" />
        /// <seealso href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#fragops-covg" />
        /// <seealso href="https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-blend-state#alpha-to-coverage" />
        virtual const bool& alphaToCoverage() const noexcept = 0;

    public:
        /// <summary>
        /// Binds a vertex buffer to the pipeline.
        /// </summary>
        /// <remarks>
        /// After binding the vertex buffer, the next call to <see cref="draw" /> or <see cref="drawIndexed" /> will read from it, until another vertex buffer is bound. 
        /// </remarks>
        /// <param name="buffer">The vertex buffer to bind to the pipeline.</param>
        /// <seealso cref="IVertexBuffer" />
        /// <seealso cref="draw" />
        /// <seealso cref="drawIndexed" />
        virtual void bind(const TVertexBufferInterface& buffer) const = 0;

        /// <summary>
        /// Binds a index buffer to the pipeline.
        /// </summary>
        /// <remarks>
        /// After binding the index buffer, the next call to <see cref="drawIndexed" /> will read from it, until another index buffer is bound. 
        /// </remarks>
        /// <param name="buffer">The index buffer to bind to the pipeline.</param>
        /// <seealso cref="IIndexBuffer" />
        /// <seealso cref="drawIndexed" />
        virtual void bind(const TIndexBufferInterface& buffer) const = 0;

        /// <summary>
        /// Sets the active pipeline state.
        /// </summary>
        virtual void use() const = 0;

	public:
		/// <summary>
		/// Draws a number of vertices from the currently bound vertex buffer.
		/// </summary>
		/// <param name="vertices">The number of vertices to draw.</param>
		/// <param name="instances">The number of instances to draw.</param>
		/// <param name="firstVertex">The index of the first vertex to start drawing from.</param>
		/// <param name="firstInstance">The index of the first instance to draw.</param>
		virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const = 0;
		
		/// <summary>
		/// Draws the currently bound vertex buffer with a set of indices from the currently bound index buffer.
		/// </summary>
		/// <param name="indices">The number of indices to draw.</param>
		/// <param name="instances">The number of instances to draw.</param>
		/// <param name="firstIndex">The index of the first element of the index buffer to start drawing from.</param>
		/// <param name="vertexOffset">The offset added to each index to find the corresponding vertex.</param>
		/// <param name="firstInstance">The index of the first instance to draw.</param>
		virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const = 0;

        /// <summary>
        /// Draws all vertices from the vertex buffer provided in <paramref name="vertexBuffer" />.
        /// </summary>
        /// <remarks>
        /// This helper method binds the vertex buffer and issues a draw command for all vertices.
        /// </remarks>
        /// <param name="vertexBuffer">The vertex buffer to draw from.</param>
        /// <param name="instances">The number of instances to draw.</param>
        /// <param name="firstVertex">The index of the first vertex to start drawing from.</param>
        /// <param name="firstInstance">The index of the first instance to draw.</param>
        virtual void draw(const TVertexBufferInterface& vertexBuffer, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->draw(vertexBuffer.elements(), instances, firstVertex, firstInstance);
        }

        /// <summary>
        /// Draws the currently bound vertex buffer using the index buffer provided in <paramref name="indexBuffer" />.
        /// </summary>
        /// <remarks>
        /// This helper method binds the index buffer and issues a draw command for all indices.
        /// </remarks>
        /// <param name="indexBuffer">The index buffer to draw with.</param>
        /// <param name="instances">The number of instances to draw.</param>
        /// <param name="firstIndex">The index of the first element of the index buffer to start drawing from.</param>
        /// <param name="vertexOffset">The offset added to each index to find the corresponding vertex.</param>
        /// <param name="firstInstance">The index of the first instance to draw.</param>
        virtual void drawIndexed(const TIndexBufferInterface& indexBuffer, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const {
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }

        /// <summary>
        /// Draws the vertex buffer provided by <paramref name="vertexBuffer" /> using the index buffer, provided by <paramref name="indexBuffer" />.
        /// </summary>
        /// <remarks>
        /// This helper method binds the provided vertex and index buffers and issues a draw command for all indices.
        /// </remarks>
        /// <param name="vertexBuffer">The vertex buffer to draw from.</param>
        /// <param name="indexBuffer">The index buffer to draw with.</param>
        /// <param name="instances">The number of instances to draw.</param>
        /// <param name="firstIndex">The index of the first element of the index buffer to start drawing from.</param>
        /// <param name="vertexOffset">The offset added to each index to find the corresponding vertex.</param>
        /// <param name="firstInstance">The index of the first instance to draw.</param>
        virtual void drawIndexed(const TVertexBufferInterface& vertexBuffer, const TIndexBufferInterface& indexBuffer, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const {
            this->bind(vertexBuffer);
            this->bind(indexBuffer);
            this->drawIndexed(indexBuffer.elements(), instances, firstIndex, vertexOffset, firstInstance);
        }
    };

	/// <summary>
	/// Describes the interface of a render pipeline builder.
	/// </summary>
	/// <seealso cref="IRenderPipeline" />
	template <typename TDerived, typename TRenderPipeline, typename TInputAssembler = TRenderPipeline::input_assembler_type, typename TPipelineLayout = TRenderPipeline::pipeline_layout_type, typename TVertexBufferInterface = TRenderPipeline::vertex_buffer_interface_type, typename TIndexBufferInterface = TRenderPipeline::index_buffer_interface_type> requires
		rtti::implements<TRenderPipeline, IRenderPipeline<TPipelineLayout, TInputAssembler, TVertexBufferInterface, TIndexBufferInterface>>
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
	/// Represents a compute <see cref="IPipeline" />.
	/// </summary>
	/// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="IPipelineLayout"/>.</typeparam>
	/// <seealso cref="IComputePipelineBuilder" />
	template <typename TPipelineLayout, typename TCommandBuffer = TPipelineLayout::command_buffer_type>
	class IComputePipeline : public IPipeline<TPipelineLayout> {
	public:
		virtual ~IComputePipeline() noexcept = default;

    public:
        /// <summary>
        /// Returns the current command buffer.
        /// </summary>
        /// <returns>The current command buffer.</returns>
        /// <seealso cref="begin" />
        /// <seealso cref="end" />
        virtual const TCommandBuffer* commandBuffer() const noexcept = 0;

	public:
		/// <summary>
		/// Executes a compute shader.
		/// </summary>
		/// <remarks>
		/// The method records a dispatch command in a command buffer. This means, that the dispatch is not synchronously executed. In order to start the execution of one or 
		/// multiple recorded dispatches, call <see cref="submit" />.
		/// </remarks>
		/// <param name="threadCount">The number of thread groups per axis.</param>
		virtual void dispatch(const Vector3u& threadCount) const = 0;

        /// <summary>
        /// Sets the active pipeline state.
        /// </summary>
        /// <remarks>
        /// Calling this method causes the pipeline state and layout to be set on the <paramref name="commandBuffer" />. The command buffer instance will be stored until 
        /// <see cref="end" /> is called. A compute pipeline must only be bound to one command buffer at a time.
        /// </remarks>
        /// <param name="commandBuffer">The command buffer to use.</param>
        virtual void begin(const TCommandBuffer& commandBuffer) = 0;

        /// <summary>
        /// Sets the active pipeline state.
        /// </summary>
        virtual void end() = 0;
	};

	/// <summary>
	/// Describes the interface of a render pipeline builder.
	/// </summary>
	/// <seealso cref="IComputePipeline" />
	template <typename TDerived, typename TComputePipeline, typename TPipelineLayout = TComputePipeline::pipeline_layout_type> requires
		rtti::implements<TComputePipeline, IComputePipeline<TPipelineLayout>>
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
	/// Stores the images for the output attachments for a back buffer of a <see cref="IRenderPass" />, as well as a <see cref="ICommandBuffer" /> instance, that records draw commands.
	/// </summary>
	/// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="ICommandBuffer"/>.</typeparam>
	/// <seealso cref="RenderTarget" />
	template <typename TCommandBuffer, typename TBuffer = TCommandBuffer::buffer_type, typename TImage = TCommandBuffer::image_type, typename TBarrier = TCommandBuffer::barrier_type> requires
		rtti::implements<TCommandBuffer, ICommandBuffer<TBuffer, TImage, TBarrier>>
	class IFrameBuffer {
	public:
		using command_buffer_type = TCommandBuffer;

    public:
        virtual ~IFrameBuffer() noexcept = default;

    public:
        /// <summary>
        /// Returns the index of the buffer within the <see cref="IRenderPass" />.
        /// </summary>
        /// <remarks>
        /// A render pass stores multiple frame buffers, each with their own index. Calling <see cref="IRenderPass::frameBuffer" /> with this index on the frame buffers render
        /// pass returns the current frame buffer instance (i.e. the same instance, as the one, the index has been requested from).
        /// </remarks>
        /// <returns>the index of the buffer within the <see cref="IRenderPass" />.</returns>
        virtual const UInt32& bufferIndex() const noexcept = 0;

        /// <summary>
        /// Returns the current size of the frame buffer.
        /// </summary>
        /// <returns>The current size of the frame buffer.</returns>
        /// <seealso cref="height" />
        /// <seealso cref="width" />
        /// <seealso cref="resize" />
        virtual const Size2d& size() const noexcept = 0;

        /// <summary>
        /// Returns the current width of the frame buffer.
        /// </summary>
        /// <returns>The current width of the frame buffer.</returns>
        /// <seealso cref="height" />
        /// <seealso cref="size" />
        /// <seealso cref="resize" />
        virtual size_t getWidth() const noexcept = 0;

        /// <summary>
        /// Returns the current height of the frame buffer.
        /// </summary>
        /// <returns>The current height of the frame buffer.</returns
        /// <seealso cref="width" />
        /// <seealso cref="size" />
        /// <seealso cref="resize" />
        virtual size_t getHeight() const noexcept = 0;

        /// <summary>
        /// Returns the command buffer that records draw commands for the frame buffer.
        /// </summary>
        /// <returns>The command buffer that records draw commands for the frame buffer</returns>
        virtual const TCommandBuffer& commandBuffer() const noexcept = 0;

        /// <summary>
        /// Returns the images that store the output attachments for the render targets of the <see cref="IRenderPass" />.
        /// </summary>
        /// <returns>The images that store the output attachments for the render targets of the <see cref="IRenderPass" />.</returns>
        virtual Array<const TImage*> images() const noexcept = 0;

        /// <summary>
        /// Returns the image that stores the output attachment for the render target mapped the location passed with <paramref name="location" />.
        /// </summary>
        /// <returns>The image that stores the output attachment for the render target mapped the location passed with <paramref name="location" />.</returns>
        virtual const TImage& image(const UInt32& location) const = 0;

    public:
        /// <summary>
        /// Causes the frame buffer to be invalidated and recreated with a new size.
        /// </summary>
        /// <remarks>
        /// A frame buffer resize causes all render target resources (i.e. images) to be re-created. This is done by the implementation itself, except for present targets, which require
        /// a view of an image created on a <see cref="ISwapChain" />. If the frame buffer has a present target, it calls <see cref="ISwapChain::images" /> on the parent devices' swap 
        /// chain. Note that there should only be one render pass, that contains present targets, otherwise the images are written by different render passes, which may result in 
        /// undefined behavior.
        /// </remarks>
        /// <param name="renderArea">The new dimensions of the frame buffer.</param>
        virtual void resize(const Size2d& renderArea) = 0;
    };

    /// <summary>
    /// Represents the source for an input attachment mapping.
    /// </summary>
    /// <remarks>
    /// This interface is implemented by a <see cref="IRenderPass" /> to return the frame buffer for a given back buffer. It is called by a <see cref="IFrameBuffer" /> 
    /// during initialization or re-creation, in order to resolve input attachment dependencies.
    /// </remarks>
    /// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="IFrameBuffer" />.</typeparam>
    /// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="ICommandBuffer"/>.</typeparam>
    template <typename TFrameBuffer, typename TCommandBuffer = typename TFrameBuffer::command_buffer_type> requires
        rtti::implements<TFrameBuffer, IFrameBuffer<TCommandBuffer>>
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
    /// Represents a mapping between a set of <see cref="IRenderTarget" /> instances and the input attachments of a <see cref="IRenderPass" />.
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
	/// GPU, that processes data using different <see cref="IRenderPipeline" />s and stores the outputs in the <see cref="IRenderTarget" />s of a <see cref="IFrameBuffer" />.
	/// </remarks>
	/// <typeparam name="TRenderPipeline">The type of the render pipeline. Must implement <see cref="IRenderPipeline" />.</typeparam>
	/// <typeparam name="TPipelineLayout">The type of the render pipeline layout. Must implement <see cref="IPipelineLayout" />.</typeparam>
	/// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="IFrameBuffer" />.</typeparam>
	/// <typeparam name="TInputAttachmentMapping">The type of the input attachment mapping. Must implement <see cref="IInputAttachmentMapping" />.</typeparam>
	/// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="ICommandBuffer"/>.</typeparam>
	// TODO: Add concepts to constrain render pipeline and input attachments properly.
	template <typename TRenderPipeline, typename TFrameBuffer, typename TInputAttachmentMapping, typename TPipelineLayout = TRenderPipeline::pipeline_layout_type, typename TDescriptorSet = TPipelineLayout::descriptor_set_type, typename TCommandBuffer = TFrameBuffer::command_buffer_type> requires
		rtti::implements<TFrameBuffer, IFrameBuffer<TCommandBuffer>> /*&&
		rtti::implements<TRenderPipeline, IRenderPipeline<TPipelineLayout>> &&
		rtti::implements<TInputAttachmentMapping, IInputAttachmentMapping<TDerived>>*/
	class IRenderPass : public IInputAttachmentMappingSource<TFrameBuffer> {
	public:
		using frame_buffer_type = TFrameBuffer;
		using render_pipeline_type = TRenderPipeline;
		using input_attachment_mapping_type = TInputAttachmentMapping;

    public:
        virtual ~IRenderPass() noexcept = default;

    public:
        /// <summary>
        /// Returns the current frame buffer from of the render pass.
        /// </summary>
        /// <remarks>
        /// The frame buffer can only be obtained, if the render pass has been started by calling <see cref="begin" />. If the render pass has ended or not yet started, the
        /// method will instead raise an exception.
        /// </remarks>
        /// <param name="buffer">The index of the frame buffer.</param>
        /// <returns>A back buffer used by the render pass.</returns>
        virtual const TFrameBuffer& activeFrameBuffer() const = 0;

        /// <summary>
        /// Returns a list of all frame buffers.
        /// </summary>
        /// <returns>A list of all frame buffers. </returns>
        virtual Array<const TFrameBuffer*> frameBuffers() const noexcept = 0;

        /// <summary>
        /// Returns the render pipeline with the <paramref name="id" />, or <c>nullptr</c>, if the render pass does not contain a matching pipeline.
        /// </summary>
        /// <param name="id">The ID of the requested render pipeline.</param>
        /// <returns>The render pipeline with the <paramref name="id" />, or <c>nullptr</c>, if the render pass does not contain a matching pipeline.</returns>
        /// <seealso cref="IRenderPipeline" />
        virtual const TRenderPipeline& pipeline(const UInt32& id) const = 0;

        /// <summary>
        /// Returns an array of all render pipelines, owned by the render pass.
        /// </summary>
        /// <returns>An array of all render pipelines, owned by the render pass.</returns>
        /// <seealso cref="IRenderPipeline" />
        virtual Array<const TRenderPipeline*> pipelines() const noexcept = 0;

        /// <summary>
        /// Returns the render target mapped to the location provided by <paramref name="location" />.
        /// </summary>
        /// <param name="location">The location to return the render target for.</param>
        /// <returns>The render target mapped to the location provided by <paramref name="location" />.</returns>
        virtual const RenderTarget& renderTarget(const UInt32& location) const = 0;

        /// <summary>
        /// Returns the list of render targets, the render pass renders into.
        /// </summary>
        /// <remarks>
        /// Note that the actual render target image resources are stored within the individual <see cref="IFrameBuffer" />s of the render pass.
        /// </remarks>
        /// <returns>A list of render targets, the render pass renders into.</returns>
        /// <seealso cref="IFrameBuffer" />
        /// <seealso cref="frameBuffer" />
        virtual Span<const RenderTarget> renderTargets() const noexcept = 0;

        /// <summary>
        /// Returns <c>true</c>, if one of the render targets is used for presentation on a swap chain.
        /// </summary>
        /// <returns><c>true</c>, if one of the render targets is used for presentation on a swap chain.</returns>
        /// <seealso cref="renderTargets" />
        virtual bool hasPresentTarget() const noexcept = 0;

        /// <summary>
        /// Returns the input attachment the render pass is consuming.
        /// </summary>
        /// <returns>An array of input attachment mappings, that are mapped to the render pass.</returns>
        virtual Span<const TInputAttachmentMapping> inputAttachments() const noexcept = 0;

        /// <summary>
        /// Returns the number of samples, the render targets are sampled with.
        /// </summary>
        /// <returns>The number of samples, the render targets are sampled with.</returns>
        virtual const MultiSamplingLevel& multiSamplingLevel() const noexcept = 0;

    public:
        /// <summary>
        /// Begins the render pass.
        /// </summary>
        /// <param name="buffer">The back buffer to use. Typically this is the same as the value returned from <see cref="ISwapChain::swapBackBuffer" />.</param>
        virtual void begin(const UInt32& buffer) = 0;

        /// <summary>
        /// Ends the render pass.
        /// </summary>
        /// <remarks>
        /// If the frame buffer has a present render target, this causes the render pass to synchronize with the swap chain and issue a present command.
        /// </remarks>
        /// <param name="buffer">The back buffer to use. Typically this is the same as the value returned from <see cref="ISwapChain::swapBackBuffer" />.</param>
        virtual void end() const = 0;

        /// <summary>
        /// Resets the frame buffers of the render pass.
        /// </summary>
        /// <param name="renderArea">The size of the render area, the frame buffers will be resized to.</param>
        virtual void resizeFrameBuffers(const Size2d& renderArea) = 0;

        /// <summary>
        /// Changes the multi sampling level of the render pass.
        /// </summary>
        /// <remarks>
        /// The method causes the frame buffers to be re-created. It checks, if the <paramref name="samples" /> are supported by the device for each render target 
        /// format. If not, an exception will be thrown. To prevent this, call <see cref=IGraphicsDevice::maximumMultiSamplingLevel" /> for each render target format on 
        /// your own, in order to request the maximum number of samples supported.
        /// </remarks>
        /// <param name="samples">The number of samples per edge pixel.</param>
        /// <exception cref="InvalidArgumentException">Thrown, if one or more of the render targets have a format, that does not support the provided multi-sampling level.</exception>
        virtual void changeMultiSamplingLevel(const MultiSamplingLevel& samples) = 0;

        /// <summary>
        /// Resolves the input attachments mapped to the render pass and updates them on the descriptor set provided with <see cref="descriptorSet" />.
        /// </summary>
        /// <param name="descriptorSet">The descriptor set to update the input attachments on.</param>
        virtual void updateAttachments(const TDescriptorSet& descriptorSet) const = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TRenderPass, typename TRenderPipeline = TRenderPass::render_pipeline_type, typename TFrameBuffer = TRenderPass::frame_buffer_type, typename TInputAttachmentMapping = typename TRenderPass::input_attachment_mapping_type> requires
        rtti::implements<TRenderPass, IRenderPass<TRenderPipeline, TFrameBuffer, TInputAttachmentMapping>>
    class RenderPassBuilder : public Builder<TDerived, TRenderPass> {
    public:
        using Builder<TDerived, TRenderPass>::Builder;

    public:
        virtual void use(RenderTarget&& target) = 0;
        virtual void use(TInputAttachmentMapping&& inputAttachment) = 0;

    public:
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
    class ISwapChain {
    public:
        using image_interface_type = TImageInterface;

    public:
        virtual ~ISwapChain() noexcept = default;

    public:
        /// <summary>
        /// Returns the swap chain image format.
        /// </summary>
        /// <returns>The swap chain image format.</returns>
        virtual const Format& surfaceFormat() const noexcept = 0;

        /// <summary>
        /// Returns the number of images in the swap chain.
        /// </summary>
        /// <returns>The number of images in the swap chain.</returns>
        virtual const UInt32& buffers() const noexcept = 0;

        /// <summary>
        /// Returns the size of the render area.
        /// </summary>
        /// <returns>The size of the render area.</returns>
        virtual const Size2d& renderArea() const noexcept = 0;

        /// <summary>
        /// Returns an array of the swap chain present images.
        /// </summary>
        /// <returns>Returns an array of the swap chain present images.</returns>
        virtual Array<const TImageInterface*> images() const noexcept = 0;

    public:
        /// <summary>
        /// Returns an array of supported formats, that can be drawn to the surface.
        /// </summary>
        /// <returns>An array of supported formats, that can be drawn to the surface.</returns>
        /// <see cref="surface" />
        /// <seealso cref="ISurface" />
        virtual Array<Format> getSurfaceFormats() const noexcept = 0;

        /// <summary>
        /// Causes the swap chain to be re-created. All frame and command buffers will be invalidated and rebuilt.
        /// </summary>
        /// <remarks>
        /// There is no guarantee, that the swap chain images will end up in the exact format, as specified by <paramref name="surfaceFormat" />. If the format itself is not
        /// supported, a compatible format may be looked up. If the lookup fails, the method may raise an exception.
        /// 
        /// Similarly, it is not guaranteed, that the number of images returned by <see cref="images" /> matches the number specified in <paramref name="buffers" />. A swap chain may 
        /// require a minimum number of images or may constraint a maximum number of images. In both cases, <paramref name="buffers" /> will be clamped.
        /// </remarks>
        /// <param name="surfaceFormat">The swap chain image format.</param>
        /// <param name="renderArea">The dimensions of the frame buffers.</param>
        /// <param name="buffers">The number of buffers in the swap chain.</param>
        /// <seealso cref="multiSamplingLevel" />
        virtual void reset(const Format& surfaceFormat, const Size2d& renderArea, const UInt32& buffers) = 0;

        /// <summary>
        /// Swaps the front buffer with the next back buffer in order.
        /// </summary>
        /// <returns>A reference of the front buffer after the buffer swap.</returns>
        [[nodiscard]] virtual UInt32 swapBackBuffer() const = 0;
    };

    /// <summary>
    /// Represents a command queue.
    /// </summary>
    /// <typeparam name="TCommandBuffer">The type of the command buffer for this queue. Must implement <see cref="ICommandBuffer"/>.</typeparam>
    template <typename TCommandBuffer, typename TBuffer = TCommandBuffer::buffer_type, typename TImage = TCommandBuffer::image_type, typename TBarrier = TCommandBuffer::barrier_type> requires
        rtti::implements<TCommandBuffer, ICommandBuffer<TBuffer, TImage, TBarrier>>
    class ICommandQueue {
    public:
        using command_buffer_type = TCommandBuffer;

    public:
        virtual ~ICommandQueue() noexcept = default;

    public:
        /// <summary>
        /// Returns <c>true</c>, if the command queue is bound on the parent device.
        /// </summary>
        /// <remarks>
        /// Before a command queue can receive commands, it needs to be bound to a device. This ensures, that the queue is actually able to allocate commands. A 
        /// command queue starts in unbound state until <see cref="bind" /> gets called. Destroying the queue also releases it by calling <see cref="release" />.
        /// </remarks>
        /// <seealso cref="bind" />
        /// <seealso cref="release" />
        /// <returns><c>true</c>, if the command queue is bound on a device.</returns>
        virtual bool isBound() const noexcept = 0;

        /// <summary>
        /// Returns the priority of the queue.
        /// </summary>
        /// <returns>The priority of the queue.</returns>
        virtual const QueuePriority& priority() const noexcept = 0;

        /// <summary>
        /// Returns the type of the queue.
        /// </summary>
        /// <returns>The type of the queue.</returns>
        virtual const QueueType& type() const noexcept = 0;

    public:
        /// <summary>
        /// Binds the queue on the parent device.
        /// </summary>
        /// <seealso cref="isBound" />
        virtual void bind() = 0;

        /// <summary>
        /// Releases the queue from the parent device.
        /// </summary>
        virtual void release() = 0;

        /// <summary>
        /// Creates a command buffer that can be used to allocate commands on the queue.
        /// </summary>
        /// <param name="beginRecording">If set to <c>true</c>, the command buffer will be initialized in recording state and can receive commands straight away.</param>
        /// <returns>The instance of the command buffer.</returns>
        virtual UniquePtr<TCommandBuffer> createCommandBuffer(const bool& beginRecording = false) const = 0;
    };

    /// <summary>
    /// Describes a factory that creates objects for a <see cref="IGraphicsDevice" />.
    /// </summary>
    /// <remarks>
    /// Initial resource states depend on the provided <see cref="BufferUsage" />. *Staging* and *Dynamic* resources are always initialized in <see cref="ResourceState::GenericRead" /> 
    /// state, while *Resource* and *Readback* resources are initialized in <see cref="ResourceState::CopyDestination" /> state. Images (and attachments) can only be used as *Resource*,
    /// so they are always created as copy destination. Images require a transfer from a buffer, followed by an explicit <see cref="IBarrier" /> into the state required by the shader.
    /// Attachments are implicitly transitioned at the beginning and end of a render pass, so you typically do not need to create an explicit barrier for them.
    /// </remarks>
    /// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout" />.</typeparam>
    /// <typeparam name="TVertexBuffer">The type of the vertex buffer. Must implement <see cref="IVertexBuffer" />.</typeparam>
    /// <typeparam name="TIndexBuffer">The type of the index buffer. Must implement <see cref="IIndexBuffer" />.</typeparam>
    /// <typeparam name="TImage">The type of the image. Must inherit from <see cref="IImage"/>.</typeparam>
    /// <typeparam name="TBuffer">The type of the buffer. Must inherit from <see cref="IBuffer"/>.</typeparam>
    /// <typeparam name="TSampler">The type of the sampler. Must inherit from <see cref="ISampler"/>.</typeparam>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout"/>.</typeparam>
    /// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout"/>.</typeparam>
    template <typename TDescriptorLayout, typename TBuffer, typename TVertexBuffer, typename TIndexBuffer, typename TImage, typename TSampler, typename TVertexBufferLayout = TVertexBuffer::vertex_buffer_layout_type, typename TIndexBufferLayout = TIndexBuffer::index_buffer_layout_type> requires
        rtti::implements<TDescriptorLayout, IDescriptorLayout> &&
        std::derived_from<TVertexBuffer, IVertexBuffer<TVertexBufferLayout>> &&
        std::derived_from<TIndexBuffer, IIndexBuffer<TIndexBufferLayout>> &&
        std::derived_from<TImage, IImage> &&
        std::derived_from<TBuffer, IBuffer> &&
        std::derived_from<TSampler, ISampler>
    class IGraphicsFactory {
    public:
        using descriptor_layout_type = TDescriptorLayout;
        using vertex_buffer_type = TVertexBuffer;
        using index_buffer_type = TIndexBuffer;
        using buffer_type = TBuffer;
        using image_type = TImage;
        using sampler_type = TSampler;

    public:
        virtual ~IGraphicsFactory() noexcept = default;

    public:
        /// <summary>
        /// Creates a buffer of type <paramref name="type" />.
        /// </summary>
        /// <param name="type">The type of the buffer.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elementSize">The size of an element in the buffer (in bytes).</param>
        /// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the buffer.</returns>
        virtual UniquePtr<TBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& elementSize, const UInt32& elements = 1, const bool& allowWrite = false) const = 0;

        /// <summary>
        /// Creates a vertex buffer, based on the <paramref name="layout" />
        /// </summary>
        /// <remarks>
        /// A vertex buffer can be used by different <see cref="IRenderPipeline" />s, as long as they share a common input assembler state.
        /// 
        /// The size of the buffer is computed from the element size vertex buffer layout, times the number of elements given by the <paramref name="elements" /> parameter.
        /// </remarks>
        /// <param name="layout">The layout of the vertex buffer.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of vertices).</param>
        /// <returns>The instance of the vertex buffer.</returns>
        virtual UniquePtr<TVertexBuffer> createVertexBuffer(const TVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const = 0;

        /// <summary>
        /// Creates an index buffer, based on the <paramref name="layout" />.
        /// </summary>
        /// <remarks>
        /// An index buffer can be used by different <see cref="IRenderPipeline" />s, as long as they share a common input assembler state.
        /// 
        /// The size of the buffer is computed from the element size index buffer layout, times the number of elements given by the <paramref name="elements" /> parameter.
        /// </remarks>
        /// <param name="layout">The layout of the index buffer.</param>
        /// <param name="usage">The buffer usage.</param>
        /// <param name="elements">The number of elements within the vertex buffer (i.e. the number of indices).</param>
        /// <returns>The instance of the index buffer.</returns>
        virtual UniquePtr<TIndexBuffer> createIndexBuffer(const TIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const = 0;

        /// <summary>
        /// Creates an image that is used as render target attachment.
        /// </summary>
        /// <param name="format">The format of the image.</param>
        /// <param name="size">The extent of the image.</param>
        /// <param name="samples">The number of samples, the image should be sampled with.</param>
        /// <returns>The instance of the attachment image.</returns>
        virtual UniquePtr<TImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;

        /// <summary>
        /// Creates a texture, based on the <paramref name="layout" />.
        /// </summary>
        /// <remarks>
        /// A texture in LiteFX is always backed by GPU-only visible memory and thus can only be transferred to/from. Thus you typically have to create a buffer using 
        /// <see cref="createBuffer" /> first that holds the actual image bytes. You than can transfer/copy the contents into the texture.
        /// </remarks>
        /// <param name="format">The format of the texture image.</param>
        /// <param name="size">The dimensions of the texture.</param>
        /// <param name="dimension">The dimensionality of the texture.</param>
        /// <param name="layers">The number of layers (slices) in this texture.</param>
        /// <param name="levels">The number of mip map levels of the texture.</param>
        /// <param name="samples">The number of samples, the texture should be sampled with.</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>The instance of the texture.</returns>
        /// <seealso cref="createTextures" />
        virtual UniquePtr<TImage> createTexture(const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32& levels = 1, const UInt32& layers = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const = 0;

        /// <summary>
        /// Creates an array of textures, based on the <paramref name="layout" />.
        /// </summary>
        /// <param name="layout">The layout of the textures.</param>
        /// <param name="elements">The number of textures to create.</param>
        /// <param name="format">The format of the texture images.</param>
        /// <param name="size">The dimensions of the textures.</param>
        /// <param name="layers">The number of layers (slices) in this texture.</param>
        /// <param name="levels">The number of mip map levels of the textures.</param>
        /// <param name="samples">The number of samples, the textures should be sampled with.</param>
        /// <param name="allowWrite">Allows the resource to be bound to a read/write descriptor.</param>
        /// <returns>An array of texture instances.</returns>
        /// <seealso cref="createTexture" />
        virtual Array<UniquePtr<TImage>> createTextures(const UInt32& elements, const Format& format, const Size3d& size, const ImageDimensions& dimension = ImageDimensions::DIM_2, const UInt32 & layers = 1, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1, const bool& allowWrite = false) const = 0;

        /// <summary>
        /// Creates a texture sampler, based on the <paramref name="layout" />.
        /// </summary>
        /// <param name="magFilter">The filter operation used for magnifying.</param>
        /// <param name="minFilter">The filter operation used for minifying.</param>
        /// <param name="borderU">The border mode along the U-axis.</param>
        /// <param name="borderV">The border mode along the V-axis.</param>
        /// <param name="borderW">The border mode along the W-axis.</param>
        /// <param name="mipMapMode">The mip map mode.</param>
        /// <param name="mipMapBias">The mip map bias.</param>
        /// <param name="maxLod">The maximum level of detail value.</param>
        /// <param name="minLod">The minimum level of detail value.</param>
        /// <param name="anisotropy">The level of anisotropic filtering.</param>
        /// <returns>The instance of the sampler.</returns>
        /// <seealso cref="createSamplers" />
        virtual UniquePtr<TSampler> createSampler(const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const = 0;

        /// <summary>
        /// Creates an array of texture samplers, based on the <paramref name="layout" />.
        /// </summary>
        /// <param name="elements">The number of samplers to create.</param>
        /// <param name="magFilter">The filter operation used for magnifying.</param>
        /// <param name="minFilter">The filter operation used for minifying.</param>
        /// <param name="borderU">The border mode along the U-axis.</param>
        /// <param name="borderV">The border mode along the V-axis.</param>
        /// <param name="borderW">The border mode along the W-axis.</param>
        /// <param name="mipMapMode">The mip map mode.</param>
        /// <param name="mipMapBias">The mip map bias.</param>
        /// <param name="maxLod">The maximum level of detail value.</param>
        /// <param name="minLod">The minimum level of detail value.</param>
        /// <param name="anisotropy">The level of anisotropic filtering.</param>
        /// <returns>An array of sampler instances.</returns>
        /// <seealso cref="createSampler" />
        virtual Array<UniquePtr<TSampler>> createSamplers(const UInt32& elements, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const = 0;
    };

    /// <summary>
    /// Represents the graphics device that a rendering back-end is doing work on.
    /// </summary>
    /// <remarks>
    /// The graphics device is the central instance of a renderer. It has two major roles. First, it maintains the <see cref="IGraphicsFactory" /> instance, that is used to facilitate
    /// common objects. Second, it owns the device state, which contains objects required for communication between your application and the graphics driver. Most notably, those objects
    /// contain the <see cref="ISwapChain" /> instance and the <see cref="ICommandQueue" /> instances used for data and command transfer.
    /// </remarks>
    /// <typeparam name="TFactory">The type of the graphics factory. Must implement <see cref="IGraphicsFactory" />.</typeparam>
    /// <typeparam name="TSurface">The type of the surface. Must implement <see cref="ISurface" />.</typeparam>
    /// <typeparam name="TGraphicsAdapter">The type of the graphics adapter. Must implement <see cref="IGraphicsAdapter" />.</typeparam>
    /// <typeparam name="TSwapChain">The type of the swap chain. Must implement <see cref="ISwapChain" />.</typeparam>
    /// <typeparam name="TCommandQueue">The type of the command queue. Must implement <see cref="ICommandQueue" />.</typeparam>
    /// <typeparam name="TRenderPass">The type of the render pass. Must implement <see cref="IRenderPass" />.</typeparam>
    /// <typeparam name="TRenderPipeline">The type of the render pipeline. Must implement <see cref="IRenderPipeline" />.</typeparam>
    /// <typeparam name="TImage">The type of the swap chain image. Must inherit from <see cref="IImage" />.</typeparam>
    /// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="IFrameBuffer" />.</typeparam>
    /// <typeparam name="TInputAttachmentMapping">The type of the input attachment mapping. Must implement <see cref="IInputAttachmentMapping" />.</typeparam>
    /// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="ICommandBuffer" />.</typeparam>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout" />.</typeparam>
    /// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout" />.</typeparam>
    /// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout" />.</typeparam>
    template <typename TFactory, typename TSurface, typename TGraphicsAdapter, typename TSwapChain, typename TCommandQueue, typename TRenderPass, typename TFrameBuffer = TRenderPass::frame_buffer_type, typename TRenderPipeline = TRenderPass::render_pipeline_type, typename TInputAttachmentMapping = TRenderPass::input_attachment_mapping_type, typename TCommandBuffer = TCommandQueue::command_buffer_type, typename TImage = TFactory::image_type, typename TVertexBuffer = TFactory::vertex_buffer_type, typename TIndexBuffer = TFactory::index_buffer_type, typename TDescriptorLayout = TFactory::descriptor_layout_type, typename TBuffer = TFactory::buffer_type, typename TSampler = TFactory::sampler_type> requires
        rtti::implements<TSurface, ISurface> &&
        rtti::implements<TGraphicsAdapter, IGraphicsAdapter> &&
        rtti::implements<TSwapChain, ISwapChain<TImage>> &&
        rtti::implements<TCommandQueue, ICommandQueue<TCommandBuffer>> &&
        rtti::implements<TFactory, IGraphicsFactory<TDescriptorLayout, TBuffer, TVertexBuffer, TIndexBuffer, TImage, TSampler>> &&
        rtti::implements<TRenderPass, IRenderPass<TRenderPipeline, TFrameBuffer, TInputAttachmentMapping>>
    class IGraphicsDevice {
    public:
        using surface_type = TSurface;
        using adapter_type = TGraphicsAdapter;
        using swap_chain_type = TSwapChain;
        using command_queue_type = TCommandQueue;
        using factory_type = TFactory;
        using frame_buffer_type = TFrameBuffer;
        using render_pass_type = TRenderPass;

    public:
        virtual ~IGraphicsDevice() noexcept = default;

    public:
        /// <summary>
        /// Returns the surface, the device draws to.
        /// </summary>
        /// <returns>A reference of the surface, the device draws to.</returns>
        virtual const TSurface& surface() const noexcept = 0;

        /// <summary>
        /// Returns the graphics adapter, the device uses for drawing.
        /// </summary>
        /// <returns>A reference of the graphics adapter, the device uses for drawing.</returns>
        virtual const TGraphicsAdapter& adapter() const noexcept = 0;

        /// <summary>
        /// Returns the swap chain, that contains the back and front buffers used for presentation.
        /// </summary>
        /// <returns>The swap chain, that contains the back and front buffers used for presentation.</returns>
        virtual const TSwapChain& swapChain() const noexcept = 0;

        /// <summary>
        /// Returns the factory instance, used to create instances from the device.
        /// </summary>
        /// <returns>The factory instance, used to create instances from the device.</returns>
        virtual const TFactory& factory() const noexcept = 0;

        /// <summary>
        /// Returns the instance of the queue, used to process draw calls.
        /// </summary>
        /// <returns>The instance of the queue, used to process draw calls.</returns>
        virtual const TCommandQueue& graphicsQueue() const noexcept = 0;

        /// <summary>
        /// Returns the instance of the queue used for device-device transfers (e.g. between render-passes).
        /// </summary>
        /// <remarks>
        /// Note that this can be the same as <see cref="graphicsQueue" />, if no dedicated transfer queues are supported on the device.
        /// </remarks>
        /// <returns>The instance of the queue used for device-device transfers (e.g. between render-passes).</returns>
        virtual const TCommandQueue& transferQueue() const noexcept = 0;

        /// <summary>
        /// Returns the instance of the queue used for host-device transfers.
        /// </summary>
        /// <remarks>
        /// Note that this can be the same as <see cref="graphicsQueue" />, if no dedicated transfer queues are supported on the device.
        /// </remarks>
        /// <returns>The instance of the queue used for host-device transfers.</returns>
        virtual const TCommandQueue& bufferQueue() const noexcept = 0;

		/// <summary>
		/// Returns the instance of the queue used for compute calls.
		/// </summary>
		/// <remarks>
		/// Note that this can be the same as <see cref="graphicsQueue" />, if no dedicated compute queues are supported on the device.
		/// </remarks>
		/// <returns>The instance of the queue used for compute calls.</returns>
		virtual const TCommandQueue& computeQueue() const noexcept = 0;

		/// <summary>
		/// Queries the device for the maximum supported number of multi-sampling levels.
		/// </summary>
		/// <remarks>
		/// This method returns the maximum supported multi-sampling level for a certain format. Typically you want to pass a back-buffer format for your swap-chain here. All lower 
		/// multi-sampling levels are implicitly supported for this format.
		/// </remarks>
		/// <param name="format">The target (i.e. back-buffer) format.</param>
		/// <returns>The maximum multi-sampling level.</returns>
		virtual MultiSamplingLevel maximumMultiSamplingLevel(const Format& format) const noexcept = 0;

    public:
        /// <summary>
        /// Waits until the device is idle.
        /// </summary>
        /// <remarks>
        /// The complexity of this operation may depend on the graphics API that implements this method. Calling this method guarantees, that the device resources are in an unused state and 
        /// may safely be released.
        /// </remarks>
        virtual void wait() const = 0;
    };

    /// <summary>
    /// Defines a back-end, that provides a device instance for a certain surface and graphics adapter.
    /// </summary>
    /// <typeparam name="TGraphicsAdapter">The type of the graphics adapter. Must implement <see cref="IGraphicsAdapter" />.</typeparam>
    /// <typeparam name="TSurface">The type of the surface. Must implement <see cref="ISurface" />.</typeparam>
    /// <typeparam name="TSwapChain">The type of the swap chain. Must implement <see cref="ISwapChain" />.</typeparam>
    /// <typeparam name="TGraphicsDevice">The type of the graphics device. Must implement <see cref="IGraphicsDevice" />.</typeparam>
    /// <typeparam name="TCommandQueue">The type of the command queue. Must implement <see cref="ICommandQueue" />.</typeparam>
    /// <typeparam name="TFactory">The type of the graphics factory. Must implement <see cref="IGraphicsFactory" />.</typeparam>
    template <typename TGraphicsDevice, typename TGraphicsAdapter = TGraphicsDevice::adapter_type, typename TSurface = TGraphicsDevice::surface_type, typename TSwapChain = TGraphicsDevice::swap_chain_type, typename TFrameBuffer = TGraphicsDevice::frame_buffer_type, typename TCommandQueue = TGraphicsDevice::command_queue_type, typename TFactory = TGraphicsDevice::factory_type, typename TRenderPass = TGraphicsDevice::render_pass_type> requires
        rtti::implements<TGraphicsDevice, IGraphicsDevice<TFactory, TSurface, TGraphicsAdapter, TSwapChain, TCommandQueue, TRenderPass>>
    class IRenderBackend : public IBackend {
    public:
        virtual ~IRenderBackend() noexcept = default;

    public:
        /// <summary>
        /// Lists all available graphics adapters.
        /// </summary>
        /// <returns>An array of pointers to all available graphics adapters.</returns>
        virtual Array<const TGraphicsAdapter*> listAdapters() const = 0;

        /// <summary>
        /// Finds an adapter using its unique ID.
        /// </summary>
        /// <remarks>
        /// Note that the adapter ID is optional, which allows the back-end to return a default adapter instance. Which adapter is used as <i>default</i> adapter, depends on
        /// the actual back-end implementation. The interface does not make any constraints on the default adapter to choose. A naive implementation might simply return the 
        /// first available adapter.
        /// </remarks>
        /// <param name="adapterId">The unique ID of the adapter, or <c>std::nullopt</c> to find the default adapter.</param>
        /// <returns>A pointer to a graphics adapter, or <c>nullptr</c>, if no adapter could be found.</returns>
        /// <seealso cref="IGraphicsAdapter" />
        virtual const TGraphicsAdapter* findAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const = 0;

    public:
        /// <summary>
        /// Creates a new graphics device.
        /// </summary>
        /// <param name="..._args">The arguments that are passed to the graphics device constructor.</param>
        /// <returns>A pointer of the created graphics device instance.</returns>
        template <typename ...TArgs>
        [[nodiscard]] UniquePtr<TGraphicsDevice> createDevice(const TGraphicsAdapter& adapter, const TSurface& surface, TArgs&&... _args) const {
            return makeUnique<TGraphicsDevice>(adapter, surface, std::forward<TArgs>(_args)...);
        }
    };
}