#pragma once

#include <litefx/litefx.h>
#include <litefx/backends/vulkan.hpp>

#include <GLFW/glfw3.h>
#include <memory>

using namespace LiteFX;
using namespace LiteFX::Rendering;
using namespace LiteFX::Rendering::Backends;

class SampleApp : public LiteFX::App 
{
private:
	struct GlfwWindowDeleter {
		void operator()(GLFWwindow* ptr) {
			::glfwDestroyWindow(ptr);
		}
	};

	typedef UniquePtr<GLFWwindow, GlfwWindowDeleter> GlfwWindowPtr;
	typedef UniquePtr<VulkanBackend> RenderBackendPtr;

private:
	GlfwWindowPtr m_window;
	RenderBackendPtr m_renderBackend;

public:
	SampleApp();

public:
	virtual String getName() const override { return "LiteFX Sample: Basic Rendering"; }
	virtual AppVersion getVersion() const override { return AppVersion(1, 0, 0, 0); }

public:
	virtual int start(int argc, char** argv) override;
	virtual void stop() override;
	virtual void work() override;

private:
	void initializeRenderer(const Array<String>& validationLayers);
	void createWindow();
};