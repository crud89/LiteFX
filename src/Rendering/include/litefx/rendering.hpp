#pragma once

#include <litefx/app.hpp>
#include <litefx/math.hpp>
#include <litefx/rendering_api.hpp>
#include <litefx/rendering_formatters.hpp>
#include <litefx/rendering_pipelines.hpp>

namespace LiteFX::Rendering {
	using namespace LiteFX;
	using namespace LiteFX::Math;

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
	class IRenderPipelineLayout {
	public:
		virtual ~IRenderPipelineLayout() noexcept = default;

	public:
		virtual const IShaderProgram* getProgram() const noexcept = 0;
		virtual Array<const IDescriptorSetLayout*> getDescriptorSetLayouts() const noexcept = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	template <typename TRenderPipelineLayout> requires
		rtti::implements<TRenderPipelineLayout, IRenderPipelineLayout>
	class IRenderPipeline {
	public:
		using pipeline_layout_type = TRenderPipelineLayout;

	public:
		virtual ~IRenderPipeline() noexcept = default;

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		virtual const String& name() const noexcept = 0;

		/// <summary>
		/// Gets the ID of the pipeline.
		/// </summary>
		/// <remarks>
		/// The pipeline ID must be unique within the render pass.
		/// </remarks>
		/// <returns>The ID of the pipeline.</returns>
		virtual const UInt32& id() const noexcept = 0;

		virtual const IRenderPipelineLayout* layout() const noexcept = 0;
		virtual SharedPtr<IInputAssembler> inputAssembler() const noexcept = 0;
		virtual SharedPtr<IRasterizer> rasterizer() const noexcept = 0;
		virtual Array<const IViewport*> viewports() const noexcept = 0;
		virtual Array<const IScissor*> scissors() const noexcept = 0;

	public:
		virtual UniquePtr<IVertexBuffer> makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding = 0) const = 0;
		virtual UniquePtr<IIndexBuffer> makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const = 0;
		virtual UniquePtr<IDescriptorSet> makeBufferPool(const UInt32& bufferSet) const = 0;
		virtual void bind(const IVertexBuffer* buffer) const = 0;
		virtual void bind(const IIndexBuffer* buffer) const = 0;
		virtual void bind(IDescriptorSet* buffer) const = 0;
		virtual void use() const = 0;
		virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const = 0;
		virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const = 0;
	};

	/// <summary>
	/// Stores multiple <see cref="IRenderTarget" /> instances, as well as a <see cref="ICommandBuffer" /> instance, that records draw commands.
	/// </summary>
	/// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="ICommandBuffer"/>.</typeparam>
	/// <typeparam name="TImage">The type of the image interface. Must inherit from <see cref="IImage"/>.</typeparam>
	/// <seealso cref="RenderTarget" />
	template <typename TCommandBuffer, typename TImage> requires
		rtti::implements<TCommandBuffer, ICommandBuffer> &&
		std::derived_from<TImage, IImage>
	class IFrameBuffer {
	public:
		using command_buffer_type = TCommandBuffer;
		using image_type = TImage;

	public:
		virtual ~IFrameBuffer() noexcept = default;

	public:
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
		/// Returns <c>true</c>, if one of the render targets is used for presentation on a swap chain.
		/// </summary>
		/// <returns><c>true</c>, if one of the render targets is used for presentation on a swap chain.</returns>
		/// <seealso cref="renderTargets" />
		virtual bool hasPresentTarget() const noexcept = 0;

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
	/// <typeparam name="TImage">The type of the image interface. Must inherit from <see cref="IImage"/>.</typeparam>
	template <typename TFrameBuffer, typename TCommandBuffer = typename TFrameBuffer::command_buffer_type, typename TImage = typename TFrameBuffer::image_type> requires
		//rtti::implements<TFrameBuffer, IFrameBuffer<TCommandBuffer, TImage>>
		std::derived_from<TFrameBuffer, IFrameBuffer<TCommandBuffer, TImage>>
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
		virtual const TInputAttachmentMappingSource& inputAttachmentSource() const noexcept = 0;

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
	/// <typeparam name="TRenderPipelineLayout">The type of the render pipeline layout. Must implement <see cref="IRenderPipelineLayout" />.</typeparam>
	/// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="IFrameBuffer" />.</typeparam>
	/// <typeparam name="TInputAttachmentMapping">The type of the input attachment mapping. Must implement <see cref="IInputAttachmentMapping" />.</typeparam>
	/// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="ICommandBuffer"/>.</typeparam>
	/// <typeparam name="TImage">The type of the image interface. Must inherit from <see cref="IImage"/>.</typeparam>
	template <typename TRenderPipeline, typename TFrameBuffer, typename TInputAttachmentMapping, typename TRenderPipelineLayout = TRenderPipeline::pipeline_layout_type, typename TCommandBuffer = TFrameBuffer::command_buffer_type, typename TImage = TFrameBuffer::image_type> //requires
		//rtti::implements<TFrameBuffer, IFrameBuffer<TCommandBuffer, TImage>>
		//rtti::implements<TRenderPipeline, IRenderPipeline<TRenderPipelineLayout>>
		//rtti::implements<TInputAttachmentMapping, IInputAttachmentMapping<TDerived>>
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
		/// Returns the input attachment the render pass is consuming.
		/// </summary>
		/// <returns>An array of input attachment mappings, that are mapped to the render pass.</returns>
		virtual Span<const TInputAttachmentMapping> inputAttachments() const noexcept = 0;

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
		virtual void use(UniquePtr<TRenderPipeline>&& pipeline) = 0;

	public:
		virtual TDerived& renderTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;
		virtual TDerived& renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;
		virtual TDerived& inputAttachment(const UInt32& location, const RenderTarget& renderTarget, const TRenderPass& renderPass) = 0;
	};

	/// <summary>
	/// Represents a swap chain, i.e. a chain of multiple <see cref="IImage" /> instances, that can be presented to a <see cref="ISurface" />.
	/// </summary>
	/// <typeparam name="TImage">The type of the image interface. Must inherit from <see cref="IImage"/>.</typeparam>
	template <typename TImage> requires
		std::derived_from<TImage, IImage>
	class ISwapChain {
	public:
		using image_type = TImage;

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
		virtual Array<const TImage*> images() const noexcept = 0;

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
	template <typename TCommandBuffer> requires
		rtti::implements<TCommandBuffer, ICommandBuffer>
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
	/// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout" />.</typeparam>
	/// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout" />.</typeparam>
	/// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout" />.</typeparam>
	template <typename TVertexBufferLayout, typename TIndexBufferLayout, typename TDescriptorLayout> requires
		rtti::implements<TVertexBufferLayout, IVertexBufferLayout> &&
		rtti::implements<TIndexBufferLayout, IIndexBufferLayout> &&
		rtti::implements<TDescriptorLayout, IDescriptorLayout>
	class IGraphicsFactory {
	public:
		using vertex_buffer_layout_type = TVertexBufferLayout;
		using index_buffer_layout_type = TIndexBufferLayout;
		using descriptor_layout_type = TDescriptorLayout;

	public:
		virtual ~IGraphicsFactory() noexcept = default;

	public:
		/// <summary>
		/// Creates a image buffer.
		/// </summary>
		/// <param name="format">The format of the image.</param>
		/// <param name="size">The extent of the image.</param>
		/// <param name="levels">The number of mip map levels of the image.</param>
		/// <param name="samples">The number of samples, the image should be sampled with.</param>
		/// <returns>An instance of the image.</returns>
		virtual UniquePtr<IImage> createImage(const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;

		/// <summary>
		/// Creates an image that is used as render target attachment.
		/// </summary>
		/// <param name="format">The format of the image.</param>
		/// <param name="size">The extent of the image.</param>
		/// <param name="samples">The number of samples, the image should be sampled with.</param>
		/// <returns>The instance of the attachment image.</returns>
		virtual UniquePtr<IImage> createAttachment(const Format& format, const Size2d& size, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;

		/// <summary>
		/// Creates a buffer of type <paramref name="type" />.
		/// </summary>
		/// <remarks>
		/// Note that when allocating an array, the <paramref name="size" /> parameter must contain enough space for all elements to fit into the buffer.
		/// </remarks>
		/// <param name="type">The type of the buffer.</param>
		/// <param name="usage">The buffer usage.</param>
		/// <param name="size">The overall size of the buffer (in bytes).</param>
		/// <param name="elements">The number of elements in the buffer (in case the buffer is an array).</param>
		/// <returns>The instance of the buffer.</returns>
		virtual UniquePtr<IBuffer> createBuffer(const BufferType& type, const BufferUsage& usage, const size_t& size, const UInt32& elements = 1) const = 0;

		/// <summary>
		/// Creates a vertex buffer, based on the <paramref name="layout" />
		/// </summary>
		/// <remarks>
		/// The size of the buffer is computed from the element size vertex buffer layout, times the number of elements given by the <paramref name="elements" /> parameter.
		/// </remarks>
		/// <param name="layout">The layout of the vertex buffer.</param>
		/// <param name="usage">The buffer usage.</param>
		/// <param name="elements">The number of elements within the vertex buffer (i.e. the number of vertices).</param>
		/// <returns>The instance of the vertex buffer.</returns>
		virtual UniquePtr<IVertexBuffer> createVertexBuffer(const TVertexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements = 1) const = 0;

		/// <summary>
		/// Creates an index buffer, based on the <paramref name="layout" />.
		/// </summary>
		/// <remarks>
		/// The size of the buffer is computed from the element size index buffer layout, times the number of elements given by the <paramref name="elements" /> parameter.
		/// </remarks>
		/// <param name="layout">The layout of the index buffer.</param>
		/// <param name="usage">The buffer usage.</param>
		/// <param name="elements">The number of elements within the vertex buffer (i.e. the number of indices).</param>
		/// <returns>The instance of the index buffer.</returns>
		virtual UniquePtr<IIndexBuffer> createIndexBuffer(const TIndexBufferLayout& layout, const BufferUsage& usage, const UInt32& elements) const = 0;

		/// <summary>
		/// Creates a constant buffer, based on the <paramref name="layout" />.
		/// </summary>
		/// <param name="layout">The layout of the constant buffer.</param>
		/// <param name="usage">The buffer usage.</param>
		/// <param name="elements">The number of elements within the constant buffer (in case the buffer is an array).</param>
		/// <returns>The instance of the constant buffer.</returns>
		virtual UniquePtr<IConstantBuffer> createConstantBuffer(const TDescriptorLayout& layout, const BufferUsage& usage, const UInt32& elements) const = 0;

		/// <summary>
		/// Creates a texture, based on the <paramref name="layout" />.
		/// </summary>
		/// <param name="layout">The layout of the texture.</param>
		/// <param name="format">The format of the texture image.</param>
		/// <param name="size">The dimensions of the texture.</param>
		/// <param name="levels">The number of mip map levels of the texture.</param>
		/// <param name="samples">The number of samples, the texture should be sampled with.</param>
		/// <returns>The instance of the texture.</returns>
		virtual UniquePtr<ITexture> createTexture(const TDescriptorLayout& layout, const Format& format, const Size2d& size, const UInt32& levels = 1, const MultiSamplingLevel& samples = MultiSamplingLevel::x1) const = 0;
		
		/// <summary>
		/// Creates a texture sampler, based on the <paramref name="layout" />.
		/// </summary>
		/// <param name="layout">The layout of the sampler.</param>
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
		virtual UniquePtr<ISampler> createSampler(const TDescriptorLayout& layout, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& minLod = 0.f, const Float& anisotropy = 0.f) const = 0;
	};

	/// <summary>
	/// Represents the graphics device that a rendering backend is doing work on.
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
	/// <typeparam name="TImage">The type of the swap chain image interface. Must inherit from <see cref="IImage" />.</typeparam>
	/// <typeparam name="TFrameBuffer">The type of the frame buffer. Must implement <see cref="IFrameBuffer" />.</typeparam>
	/// <typeparam name="TInputAttachmentMapping">The type of the input attachment mapping. Must implement <see cref="IInputAttachmentMapping" />.</typeparam>
	/// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="ICommandBuffer" />.</typeparam>
	/// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout" />.</typeparam>
	/// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout" />.</typeparam>
	/// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout" />.</typeparam>
	template <typename TFactory, typename TSurface, typename TGraphicsAdapter, typename TSwapChain, typename TCommandQueue, typename TRenderPass, typename TFrameBuffer = TRenderPass::frame_buffer_type, typename TRenderPipeline = TRenderPass::render_pipeline_type, typename TInputAttachmentMapping = TRenderPass::input_attachment_mapping_type, typename TImage = TSwapChain::image_type, typename TCommandBuffer = TCommandQueue::command_buffer_type, typename TVertexBufferLayout = TFactory::vertex_buffer_layout_type, typename TIndexBufferLayout = TFactory::index_buffer_layout_type, typename TDescriptorLayout = TFactory::descriptor_layout_type > requires
		rtti::implements<TSurface, ISurface> &&
		rtti::implements<TGraphicsAdapter, IGraphicsAdapter> &&
		rtti::implements<TSwapChain, ISwapChain<TImage>> &&
		rtti::implements<TCommandQueue, ICommandQueue<TCommandBuffer>> &&
		rtti::implements<TFactory, IGraphicsFactory<TVertexBufferLayout, TIndexBufferLayout, TDescriptorLayout>> &&
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
	/// Defines a backend, that provides a device instance for a certain surface and graphics adapter.
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
		/// Note that the adapter ID is optional, which allows the backend to return a default adapter instance. Which adapter is used as <i>default</i> adapter, depends on
		/// the actual backend implementation. The interface does not make any constraints on the default adapter to choose. A naive implementation might simply return the 
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