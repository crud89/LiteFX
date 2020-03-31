#pragma once

#include <litefx/litefx.h>
#include <GLFW/glfw3.h>
#include <memory>

using namespace LiteFX;

class SampleApp : public LiteFX::CLiteFxApp 
{
private:
	struct GlfwWindowDeleter {
		void operator()(GLFWwindow* ptr) {
			::glfwDestroyWindow(ptr);
		}
	};

	typedef std::unique_ptr<GLFWwindow, GlfwWindowDeleter> GlfwWindowPtr;

private:
	GlfwWindowPtr m_window;

public:
	SampleApp();

public:
	virtual void start(const Array<String>& args) override;
	virtual void stop() override;
	virtual void work() override;

private:
	void initializeRenderer();
	void createWindow();
};