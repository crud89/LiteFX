#pragma once

#define LITEFX_AUTO_IMPORT_BACKEND_HEADERS
#include <litefx/litefx.h>
#include <litefx/graphics.hpp>
#include <litefx/backends/dx12_api.hpp>
#include <litefx/backends/vulkan_api.hpp>

#if (defined _WIN32 || defined WINCE)
#  define GLFW_EXPOSE_NATIVE_WIN32
#else 
#  pragma message ("ImGui Integration Sample: No supported surface platform detected.")
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <memory>

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

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
	static StringView Name() noexcept { return "LiteFX Sample: ImGui Integration"sv; }
	StringView name() const noexcept override { return Name(); }

	static AppVersion Version() { return AppVersion(1, 0, 0, 0); }
	AppVersion version() const noexcept override { return Version(); }

private:
	/// @brief Stores the GLFW window pointer.
	GlfwWindowPtr m_window;

	/// @brief Stores the preferred adapter ID (`std::nullopt`, if the default adapter is used).
	Optional<UInt32> m_adapterId;

	/// @brief Stores a reference of the input assembler state.
	SharedPtr<IInputAssembler> m_inputAssembler;

	/// @brief Stores the viewport.
	SharedPtr<IViewport> m_viewport;

	/// @brief Stores the scissor.
	SharedPtr<IScissor> m_scissor;

	/// @brief Stores a pointer to the currently active device.
	IGraphicsDevice* m_device{ nullptr };

	/// @brief Stores the fence created at application load time.
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
	/// @brief Initializes the buffers.
	///
	/// @param backend The render backend to use.
	void initBuffers(IRenderBackend* backend);

	/// @brief Updates the camera buffer. This needs to be done whenever the frame buffer changes, since we need to pass changes in the aspect ratio to the view/projection matrix.
	void updateCamera(const ICommandBuffer& commandBuffer, IBuffer& buffer) const;

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
	Dictionary<SIZE_T, VirtualAllocator::Allocation> m_d3dDescriptorAllocations;
	static void allocImGuiD3D12DescriptorsCallback(ImGui_ImplDX12_InitInfo* context, D3D12_CPU_DESCRIPTOR_HANDLE* cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* gpu_handle);
	static void releaseImGuiD3D12DescriptorsCallback(ImGui_ImplDX12_InitInfo* context, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle);
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

	std::function<void()> m_beginUiCallback;
	std::function<void(const ICommandBuffer&)> m_endUiCallback;

private:
	void onInit();
	void onStartup();
	void onShutdown();
	void onResize(const void* sender, const ResizeEventArgs& e);

public:
	void keyDown(int key, int scancode, int action, int mods);
	void handleEvents();
	void drawFrame();
	void updateWindowTitle();
};