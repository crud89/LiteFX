#pragma once

#include <litefx/litefx.h>
#include <litefx/backends/vulkan.hpp>

#include <GLFW/glfw3.h>
#include <memory>

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

class SampleApp : public LiteFX::App {
private:
	struct GlfwWindowDeleter {
		void operator()(GLFWwindow* ptr) noexcept {
			::glfwDestroyWindow(ptr);
		}
	};

	typedef UniquePtr<GLFWwindow, GlfwWindowDeleter> GlfwWindowPtr;
	typedef UniquePtr<VulkanBackend> RenderBackendPtr;

private:
	GlfwWindowPtr m_window;
	RenderBackendPtr m_renderBackend;
	VkSurfaceKHR m_surface;

public:
	SampleApp() noexcept;

public:
	virtual String getName() const noexcept override { return "LiteFX Sample: Basic Rendering"; }
	virtual AppVersion getVersion() const noexcept override { return AppVersion(1, 0, 0, 0); }

public:
	virtual int start(const int argc, const char** argv) override;
	virtual int start(const Array<String>& args) override;
	virtual void stop() override;
	virtual void work() override;

private:
	void initializeRenderer(const Array<String>& validationLayers);
	void createWindow();
};