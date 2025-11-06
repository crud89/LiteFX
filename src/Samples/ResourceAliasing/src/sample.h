#pragma once

#define LITEFX_AUTO_IMPORT_BACKEND_HEADERS
#include <litefx/litefx.h>
#include <litefx/graphics.hpp>

#if (defined _WIN32 || defined WINCE)
#  define GLFW_EXPOSE_NATIVE_WIN32
#else 
#  pragma message ("Resource Aliasing Sample: No supported surface platform detected.")
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <memory>

#include "config.h"

#ifdef LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER
#include <renderdoc_app.h>

extern RENDERDOC_API_1_5_0* renderDoc;
#endif

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;
using namespace LiteFX::Graphics;

struct GlfwWindowDeleter {
	void operator()(GLFWwindow* ptr) noexcept {
		::glfwDestroyWindow(ptr);
	}
};

typedef UniquePtr<GLFWwindow, GlfwWindowDeleter> GlfwWindowPtr;

class SampleApp : public LiteFX::App {
public:
	static StringView Name() noexcept { return "LiteFX Sample: Resource Aliasing"sv; }
	StringView name() const noexcept override { return Name(); }

	static AppVersion Version() noexcept { return AppVersion(1, 0, 0, 0); }
	AppVersion version() const noexcept override { return Version(); }

private:
	/// <summary>
	/// Stores the GLFW window pointer.
	/// </summary>
	GlfwWindowPtr m_window;

	/// <summary>
	/// Stores the preferred adapter ID (<c>std::nullopt</c>, if the default adapter is used).
	/// </summary>
	Optional<UInt32> m_adapterId;

	/// <summary>
	/// Stores a reference of the input assembler state.
	/// </summary>
	SharedPtr<IInputAssembler> m_inputAssembler;

	/// <summary>
	/// Stores the viewport.
	/// </summary>
	SharedPtr<IViewport> m_viewport;

	/// <summary>
	/// Stores the scissor.
	/// </summary>
	SharedPtr<IScissor> m_scissor;

	/// <summary>
	/// Stores a pointer to the currently active device.
	/// </summary>
	IGraphicsDevice* m_device{};

	/// <summary>
	/// Stores the fence created at application load time.
	/// </summary>
	UInt64 m_transferFence = 0;

public:
	SampleApp(GlfwWindowPtr&& window, Optional<UInt32> adapterId) : 
		App(), m_window(std::move(window)), m_adapterId(adapterId)
	{
		this->initializing += std::bind(&SampleApp::onInit, this);
		this->startup += std::bind(&SampleApp::onStartup, this);
		this->resized += std::bind(&SampleApp::onResize, this, std::placeholders::_1, std::placeholders::_2);
		this->shutdown += std::bind(&SampleApp::onShutdown, this);
	}

private:
	/// <summary>
	/// Initializes the buffers.
	/// </summary>
	/// <param name="backend">The render backend to use.</param>
	void initBuffers(IRenderBackend* backend);

	/// <summary>
	/// Updates the camera buffer. This needs to be done whenever the frame buffer changes, since we need to pass changes in the aspect ratio to the view/projection matrix.
	/// </summary>
	void updateCamera(const ICommandBuffer& commandBuffer, IBuffer& buffer) const;

private:
	void onInit();
	void onStartup();
	void onShutdown();
	void onResize(const void* sender, const ResizeEventArgs& e);

private:
	/// <summary>
	/// Stores temporary resource handles to render targets allocated during the frame buffer `resizing` event and returns them to the frame buffer in the allocation callback.
	/// </summary>
	SharedPtr<const IImage> m_depthBuffer, m_postColorBuffer;

public:
	/// <summary>
	/// Initializes the overlapping resources.
	/// </summary>
	/// <param name="renderArea">The render area that determines the size of the resources.</param>
	void initAliasingBuffers(const Size2d& renderArea) {
		// NOTE: Automatically mapping render targets to images relies on resource names, so it's important to use the same names here as for the render targets later.
		auto resourceInfos = std::array {
			ResourceAllocationInfo(ResourceAllocationInfo::ImageInfo { .Format = Format::D32_SFLOAT, .Size = renderArea }, ResourceUsage::FrameBufferImage, "Depth"),
			ResourceAllocationInfo(ResourceAllocationInfo::ImageInfo { .Format = Format::B8G8R8A8_UNORM, .Size = renderArea }, ResourceUsage::FrameBufferImage, "Post Color")
		};

		auto canAlias = m_device->factory().canAlias(resourceInfos);

		if (!canAlias)
			LITEFX_WARNING("SampleApp"sv, "Render targets can't be aliased and will be created as non-overlapping images.");
		
		auto resources = m_device->factory().allocate(resourceInfos, AllocationBehavior::Default, canAlias)
			| std::views::take(2)
			| std::ranges::to<std::vector>();
		m_depthBuffer = resources[0].image<const IImage>();
		m_postColorBuffer = resources[1].image<const IImage>();
	}

	void onFrameBufferResizing(const void* /*sender*/, const IFrameBuffer::ResizeEventArgs& e) {
		this->initAliasingBuffers(e.newSize());
	}
	
	template <typename TRenderBackend> requires
		meta::implements<TRenderBackend, IRenderBackend>
	SharedPtr<const typename TRenderBackend::image_type> frameBufferAllocationCallback(Optional<UInt64> renderTargetId, const Size2d& /*renderArea*/, ResourceUsage /*usage*/, Format /*format*/, MultiSamplingLevel /*samples*/, const String& name) const {
		switch (renderTargetId.value_or(hash(name)))
		{
		case hash("Depth"):
			return std::dynamic_pointer_cast<const typename TRenderBackend::image_type>(m_depthBuffer);
		case hash("Post Color"):
			return std::dynamic_pointer_cast<const typename TRenderBackend::image_type>(m_postColorBuffer);
		default:
			// Let the frame buffer perform the allocation using the default behavior.
			// Note that we could also call `device->factory().createTexture()` from here instead.
			return nullptr;
		}
	}

public:
	void keyDown(int key, int scancode, int action, int mods);
	void handleEvents();
	void drawFrame();
	void updateWindowTitle();
};