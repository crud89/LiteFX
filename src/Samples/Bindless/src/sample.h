#pragma once

#include <litefx/litefx.h>

#ifdef BUILD_VULKAN_BACKEND
#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>
#endif // BUILD_VULKAN_BACKEND

#ifdef BUILD_DIRECTX_12_BACKEND
#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>
#endif // BUILD_DIRECTX_12_BACKEND

#if (defined _WIN32 || defined WINCE)
#  define GLFW_EXPOSE_NATIVE_WIN32
#else 
#  pragma message ("Bindless Descriptors Sample: No supported surface platform detected.")
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <memory>

#include "config.h"

#ifdef BUILD_EXAMPLES_RENDERDOC_LOADER
#include <renderdoc_app.h>

extern RENDERDOC_API_1_5_0* renderDoc;
#endif

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

struct GlfwWindowDeleter {
	void operator()(GLFWwindow* ptr) noexcept {
		::glfwDestroyWindow(ptr);
	}
};

typedef UniquePtr<GLFWwindow, GlfwWindowDeleter> GlfwWindowPtr;

class SampleApp : public LiteFX::App {
public:
	static String Name() noexcept { return "LiteFX Sample: Bindless Descriptors"; }
	String name() const noexcept override { return Name(); }

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
	IGraphicsDevice* m_device;

public:
	SampleApp(GlfwWindowPtr&& window, Optional<UInt32> adapterId) : 
		App(), m_window(std::move(window)), m_adapterId(adapterId), m_device(nullptr)
	{
		this->initialize();
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
	void updateCamera(const ICommandBuffer& commandBuffer, IBuffer& stagingBuffer, const IBuffer& buffer) const;

public:
	virtual void run() override;
	virtual void initialize() override;
	virtual void resize(int width, int height) override;
	void keyDown(int key, int scancode, int action, int mods);
	void handleEvents();
	void drawFrame();
	void updateWindowTitle();
};