#pragma once

#include <litefx/litefx.h>
#include <litefx/backends/vulkan.hpp>

#if (defined _WIN32 || defined WINCE)
#  define GLFW_EXPOSE_NATIVE_WIN32
#else 
#  pragma message ("Basic Rendering Sample: No supported surface platform detected.")
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <memory>

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
	static String name() noexcept { return "LiteFX Sample: Basic Rendering"; }
	String getName() const noexcept override { return name(); }

	static AppVersion version() noexcept { return AppVersion(1, 0, 0, 0); }
	AppVersion getVersion() const noexcept override { return version(); }

private:
	GlfwWindowPtr m_window;
	Optional<UInt32> m_adapterId;
	UniquePtr<VulkanSurface> m_surface;
	UniquePtr<VulkanDevice> m_device;
	UniquePtr<VulkanRenderPass> m_renderPass;
	SharedPtr<IViewport> m_viewport;
	SharedPtr<IScissor> m_scissor;
	UniquePtr<IVertexBuffer> m_vertexBuffer;
	UniquePtr<IIndexBuffer> m_indexBuffer;
	UniquePtr<IConstantBuffer> m_cameraBuffer, m_transformBuffer;
	UniquePtr<IDescriptorSet> m_perFrameBindings, m_perObjectBindings;

public:
	SampleApp(GlfwWindowPtr&& window, Optional<UInt32> adapterId) : 
		App(), m_window(std::move(window)), m_adapterId(adapterId)
	{
		::glfwSetWindowUserPointer(m_window.get(), this);
	}

private:
	void createRenderPasses();
	void initBuffers();

public:
	virtual void run() override;
	virtual void initialize() override;
	virtual void resize(int width, int height) override;
	void handleEvents();
	void drawFrame();
};