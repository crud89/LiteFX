#include "sample.h"

void SampleApp::run() 
{

}

//SampleApp::SampleApp() noexcept :
//	VulkanApp(Platform::Win32), m_window(nullptr)
//{
//}
//
//int SampleApp::start(const int argc, const char** argv)
//{
//
//}
//
//int SampleApp::start(const Array<String>& args)
//{
//}
//
//void SampleApp::stop()
//{
//	::glfwDestroyWindow(m_window.get());
//	::glfwTerminate();
//}
//
//void SampleApp::work()
//{
//	while (!::glfwWindowShouldClose(m_window.get())) 
//	{
//		::glfwPollEvents();
//	}
//
//	this->stop();
//}
//
//void SampleApp::initializeRenderer(const Array<String>& validationLayers)
//{
//	// Create a window.
//	this->createWindow();
//
//	// Get the required extensions from glfw.
//	uint32_t extensions = 0;
//	const char** extensionNames = ::glfwGetRequiredInstanceExtensions(&extensions);
//	Array<String> requiredExtensions;
//
//	for (uint32_t i(0); i < extensions; ++i)
//		requiredExtensions.push_back(String(extensionNames[i]));
//
//	// Create a rendering backend.
//	m_renderBackend = makeUnique<VulkanBackend>(*this, requiredExtensions, validationLayers);
//
//	// Create a surface.
//	m_surface = VulkanSurface::createSurface(*m_renderBackend, [&](const VkInstance& instance) {
//		VkSurfaceKHR surface;
//
//		if (::glfwCreateWindowSurface(instance, m_window.get(), nullptr, &surface) != VK_SUCCESS)
//			throw std::runtime_error("Unable to create GLFW window surface.");
//
//		return surface;
//	});
//}
//
//void SampleApp::createWindow()
//{
//}