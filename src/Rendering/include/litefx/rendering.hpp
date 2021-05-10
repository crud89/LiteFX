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
	class LITEFX_RENDERING_API ICommandBuffer {
	public:
		virtual ~ICommandBuffer() noexcept = default;

	public:
		template <std::derived_from<ICommandQueue> TCommandQueue>
		const TCommandQueue* getQueue() const noexcept;

	public:
		virtual void begin() const = 0;
		virtual void end() const = 0;
		virtual void submit(const bool& waitForQueue = false) const = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API IRenderTarget {
	public:
		virtual ~IRenderTarget() noexcept = default;

	public:
		virtual RenderTargetType getType() const noexcept = 0;
		virtual void setType(const RenderTargetType& type) = 0;
		virtual MultiSamplingLevel getSamples() const noexcept = 0;
		virtual void setSamples(const MultiSamplingLevel& samples = MultiSamplingLevel::x1) = 0;
		virtual bool getClearBuffer() const noexcept = 0;
		virtual void setClearBuffer(const bool& clear = true) = 0;
		virtual bool getClearStencil() const noexcept = 0;
		virtual void setClearStencil(const bool& clear = true) = 0;
		virtual Format getFormat() const noexcept = 0;
		virtual void setFormat(const Format& format) = 0;

		/// <summary>
		/// Gets whether the target should be made persistent for access after the render pass has finished.
		/// </summary>
		/// <remarks>
		/// A render target can be marked as volatile if it does not need to be accessed after the render pass has finished. This can be used to optimize away unnecessary GPU/CPU 
		/// memory round-trips. For example a depth buffer may only be used as an input for the lighting stage of a deferred renderer, but is not required after this. So instead
		/// of reading it from the GPU after the lighting pass has finished and then discarding it anyway, it can be marked as volatile in order to prevent it from being read from
		/// the GPU memory again in the first place.
		/// </remarks>
		/// <seealso cref="IRenderTarget::setVolatile" />
		virtual bool getVolatile() const noexcept = 0;

		/// <summary>
		/// Sets whether the target should be made persistent for access after the render pass has finished.
		/// </summary>
		/// <seealso cref="IRenderTarget::getVolatile" />
		virtual void setVolatile(const bool& isVolatile = false) = 0;

		virtual const Vector4f& getClearValues() const noexcept = 0;
		virtual void setClearValues(const Vector4f& values) = 0;
	};

	class LITEFX_RENDERING_API RenderTarget : public IRenderTarget {
		LITEFX_IMPLEMENTATION(RenderTargetImpl);

	public:
		explicit RenderTarget();
		RenderTarget(const RenderTarget&) = delete;
		RenderTarget(RenderTarget&&) = delete;
		virtual ~RenderTarget() noexcept;

	public:
		/// <inheritdoc />
		virtual RenderTargetType getType() const noexcept override;

		/// <inheritdoc />
		virtual void setType(const RenderTargetType& type) override;

		/// <inheritdoc />
		virtual MultiSamplingLevel getSamples() const noexcept override;

		/// <inheritdoc />
		virtual void setSamples(const MultiSamplingLevel& samples = MultiSamplingLevel::x1) override;

		/// <inheritdoc />
		virtual bool getClearBuffer() const noexcept override;

		/// <inheritdoc />
		virtual void setClearBuffer(const bool& clear = true) override;

		/// <inheritdoc />
		virtual bool getClearStencil() const noexcept override;

		/// <inheritdoc />
		virtual void setClearStencil(const bool& clear = true) override;

		/// <inheritdoc />
		virtual Format getFormat() const noexcept override;

		/// <inheritdoc />
		virtual void setFormat(const Format& format) override;

		/// <inheritdoc />
		virtual bool getVolatile() const noexcept override;

		/// <inheritdoc />
		virtual void setVolatile(const bool& isVolatile = false) override;

		/// <inheritdoc />
		virtual const Vector4f& getClearValues() const noexcept override;

		/// <inheritdoc />
		virtual void setClearValues(const Vector4f& values) override;
	};

	/// <summary>
	/// 
	/// </summary>
	class LITEFX_RENDERING_API ISwapChain {
	public:
		virtual ~ISwapChain() noexcept = default;

	public:
		virtual const Size2d& getBufferSize() const noexcept = 0;
		virtual size_t getWidth() const noexcept = 0;
		virtual size_t getHeight() const noexcept = 0;
		virtual const Format& getFormat() const noexcept = 0;
		virtual UInt32 swapBackBuffer() const = 0;
		virtual void reset(const Size2d& frameBufferSize, const UInt32& frameBuffers) = 0;
		virtual UInt32 getBuffers() const noexcept = 0;
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
		/// <returns>The instance of the command buffer.</returns>
		virtual UniquePtr<TCommandBuffer> createCommandBuffer() const = 0;
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
	/// <typeparam name="TGraphicsAdapter">The type of the graphics adapter. Must implement <see cref="IGraphicsAdapter" />.</typeparam>
	/// <typeparam name="TSurface">The type of the surface. Must implement <see cref="ISurface" />.</typeparam>
	/// <typeparam name="TSwapChain">The type of the swap chain. Must implement <see cref="ISwapChain" />.</typeparam>
	/// <typeparam name="TCommandQueue">The type of the command queue. Must implement <see cref="ICommandQueue" />.</typeparam>
	/// <typeparam name="TFactory">The type of the graphics factory. Must implement <see cref="IGraphicsFactory" />.</typeparam>
	/// <typeparam name="TCommandBuffer">The type of the command buffer. Must implement <see cref="ICommandBuffer" />.</typeparam>
	/// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout" />.</typeparam>
	/// <typeparam name="TIndexBufferLayout">The type of the index buffer layout. Must implement <see cref="IIndexBufferLayout" />.</typeparam>
	/// <typeparam name="TDescriptorLayout">The type of the descriptor layout. Must implement <see cref="IDescriptorLayout" />.</typeparam>
	template <typename TSurface, typename TGraphicsAdapter, typename TSwapChain, typename TCommandQueue, typename TFactory, typename TCommandBuffer = TCommandQueue::command_buffer_type, typename TVertexBufferLayout = TFactory::vertex_buffer_layout_type, typename TIndexBufferLayout = TFactory::index_buffer_layout_type, typename TDescriptorLayout = TFactory::descriptor_layout_type> requires
		rtti::implements<TSurface, ISurface> &&
		rtti::implements<TGraphicsAdapter, IGraphicsAdapter> &&
		rtti::implements<TSwapChain, ISwapChain> &&
		rtti::implements<TCommandQueue, ICommandQueue<TCommandBuffer>> &&
		rtti::implements<TFactory, IGraphicsFactory<TVertexBufferLayout, TIndexBufferLayout, TDescriptorLayout>> &&
		rtti::implements<TCommandBuffer, ICommandBuffer> &&
		rtti::implements<TVertexBufferLayout, IVertexBufferLayout> &&
		rtti::implements<TIndexBufferLayout, IIndexBufferLayout> &&
		rtti::implements<TDescriptorLayout, IDescriptorLayout>
	class IGraphicsDevice {
	public:
		using surface_type = TSurface;
		using adapter_type = TGraphicsAdapter;
		using swap_chain_type = TSwapChain;
		using command_queue_type = TCommandQueue;
		using factory_type = TFactory;

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
		virtual void wait() = 0;

		// TODO: Move into IFrameBuffer instance ("present buffer"?) and move it to the swap chain. See issue #22.
		virtual void resize(int width, int height) = 0;
		virtual const TSwapChain& swapChain() const noexcept = 0;
		virtual size_t getBufferWidth() const noexcept = 0;
		virtual size_t getBufferHeight() const noexcept = 0;
		//virtual Color getBackColor() const noexcept = 0;
		//virtual void setBackColor(const Color& color) = 0;

		/// <summary>
		/// Returns an array of supported formats, that can be drawn to the surface.
		/// </summary>
		/// <returns>An array of supported formats, that can be drawn to the surface.</returns>
		/// <see cref="surface" />
		/// <seealso cref="ISurface" />
		virtual Array<Format> getSurfaceFormats() const = 0;

	protected:
		/// <summary>
		/// Initializes a new render pass of type <typeparamref name="T"/> and returns a builder instance for it.
		/// </summary>
		/// <param name="_args">The arguments which are passed to the constructor of the render pass.</param>
		/// <typeparam name="TRenderPass">The type of the render pass. The type must implement <see cref="IRenderPass" /> interface.</typeparam>
		/// <typeparam name="TDerived">The actual type of the parent graphics device that calls the build method.</typeparam>
		template <typename TRenderPass, typename TDerived, typename ...TArgs, typename TBuilder = TRenderPass::builder> requires
			rtti::implements<TRenderPass, IRenderPass> &&
			rtti::has_builder<TRenderPass> &&
			rtti::implements<TDerived, IGraphicsDevice<TSurface, TGraphicsAdapter, TSwapChain, TCommandQueue, TFactory>>
		[[nodiscard]] TBuilder build(TArgs&&... _args) const {
			// NOTE: If the cast raises an exception here, check the `TDerived` type - it must be equal to the parent graphics device class that calls this function.
			return TBuilder(makeUnique<TRenderPass>(dynamic_cast<const TDerived&>(*this), std::forward<TArgs>(_args)...));
		}
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
	template <typename TGraphicsDevice, typename TGraphicsAdapter = TGraphicsDevice::adapter_type, typename TSurface = TGraphicsDevice::surface_type, typename TSwapChain = TGraphicsDevice::swap_chain_type, typename TCommandQueue = TGraphicsDevice::command_queue_type, typename TFactory = TGraphicsDevice::factory_type> requires
		rtti::implements<TGraphicsAdapter, IGraphicsAdapter> &&
		rtti::implements<TSurface, ISurface> &&
		rtti::implements<TSwapChain, ISwapChain> &&
		rtti::implements<TGraphicsDevice, IGraphicsDevice<TSurface, TGraphicsAdapter, TSwapChain, TCommandQueue, TFactory>>
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