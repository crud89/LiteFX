#pragma once

#include <litefx/litefx.h>
#include <litefx/backends/vulkan.hpp>

#if (defined _WIN32 || defined WINCE)
#  define GLFW_EXPOSE_NATIVE_WIN32
#else 
#  pragma message ("Multithreading Sample: No supported surface platform detected.")
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

#define NUM_WORKERS 9

class SampleApp : public LiteFX::App {
public:
	static String Name() noexcept { return "LiteFX Sample: Vulkan Multithreading"; }
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
	/// Stores the main device instance.
	/// </summary>
	UniquePtr<VulkanDevice> m_device;

	/// <summary>
	/// Stores the only render pass used in this sample.
	/// </summary>
	UniquePtr<VulkanRenderPass> m_renderPass;

	/// <summary>
	/// Stores the only render pipeline used in this sample.
	/// </summary>
	UniquePtr<VulkanRenderPipeline> m_pipeline;

	/// <summary>
	/// Stores a reference of the input assembler state.
	/// </summary>
	SharedPtr<VulkanInputAssembler> m_inputAssembler;

	/// <summary>
	/// Stores the viewport.
	/// </summary>
	SharedPtr<IViewport> m_viewport;

	/// <summary>
	/// Stores the scissor.
	/// </summary>
	SharedPtr<IScissor> m_scissor;

	/// <summary>
	/// Stores the vertex buffer for the quad rendered in this sample.
	/// </summary>
	UniquePtr<IVulkanVertexBuffer> m_vertexBuffer;

	/// <summary>
	/// Stores the index buffer for the quad rendered in this sample.
	/// </summary>
	UniquePtr<IVulkanIndexBuffer> m_indexBuffer;

	/// <summary>
	/// Stores the buffer that contains the camera information. Since the camera is static, we only need one (immutable) buffer for it, so the buffer will only contain one element.
	/// </summary>
	UniquePtr<IVulkanBuffer> m_cameraBuffer, m_cameraStagingBuffer;

	/// <summary>
	/// Stores the buffer that holds the object transform. The buffer will contain three elements, since we have three frames in flight.
	/// </summary>
	UniquePtr<IVulkanBuffer> m_transformBuffer;

	/// <summary>
	/// Stores the bindings to the transform buffer.
	/// </summary>
	Array<UniquePtr<VulkanDescriptorSet>> m_perFrameBindings;

	/// <summary>
	/// Stores the binding for the camera buffer.
	/// </summary>
	UniquePtr<VulkanDescriptorSet> m_cameraBindings;

	/// <summary>
	/// Stores the array of worker threads.
	/// </summary>
	Array<std::thread> m_workers = Array<std::thread>(NUM_WORKERS);

public:
	SampleApp(GlfwWindowPtr&& window, Optional<UInt32> adapterId) :
		App(), m_window(std::move(window)), m_adapterId(adapterId)
	{
		this->initialize();
	}

private:
	/// <summary>
	/// Initializes the render pass.
	/// </summary>
	void initRenderGraph();

	/// <summary>
	/// Initializes the render pipelines.
	/// </summary>
	void initPipelines();

	/// <summary>
	/// Initializes the buffers.
	/// </summary>
	void initBuffers();

	/// <summary>
	/// Updates the camera buffer. This needs to be done whenever the frame buffer changes, since we need to pass changes in the aspect ratio to the view/projection matrix.
	/// </summary>
	void updateCamera(const VulkanCommandBuffer& commandBuffer);

public:
	virtual void run() override;
	virtual void initialize() override;
	virtual void resize(int width, int height) override;
	void handleEvents();
	void drawObject(int i, int backBuffer, float time);
	void drawFrame();
};