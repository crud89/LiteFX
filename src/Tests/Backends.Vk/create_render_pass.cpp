#include "common.h"

#define FRAMEBUFFER_WIDTH  800
#define FRAMEBUFFER_HEIGHT 600

HWND _window { nullptr };

SharedPtr<Viewport> _viewport;
SharedPtr<Scissor> _scissor;
VulkanDevice* _device;

void TestApp::onInit()
{
    // Create a callback for backend startup and shutdown.
    auto startCallback = [this](VulkanBackend* backend) {
        // Create viewport and scissors.
        _viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));
        _scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));

        // As we've enabled WARP, it's the only one available.
        auto adapter = backend->findAdapter(std::nullopt);
        auto surface = backend->createSurface(_window);

        // Create the device.
        _device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, _viewport->getRectangle().extent(), 3, false);

        // Create a render pass.
        UniquePtr<VulkanRenderPass> renderPass = _device->buildRenderPass("Opaque", 5)
            .executeOn(_device->defaultQueue(QueueType::Graphics))
            .renderTarget("Color Target", 1, RenderTargetType::Color, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f })
            .renderTarget("Depth/Stencil Target", 3, RenderTargetType::DepthStencil, Format::D32_SFLOAT, RenderTargetFlags::Clear, { 1.f, 0.f, 0.f, 0.f });

        // Validate render pass.
        if (renderPass->commandBuffers().size() != 0) // No command buffers if not active.
            LITEFX_TEST_FAIL("renderPass->commandBuffers().size() != 0");

        if (renderPass->secondaryCommandBuffers() != 5)
            LITEFX_TEST_FAIL("renderPass->secondaryCommandBuffers() != 5");

        try
        {
            auto commandBuffer = renderPass->commandBuffer(5);
            LITEFX_TEST_FAIL("renderPass->commandBuffer(5) was not expected to succeed.");
        }
        catch (const LiteFX::RuntimeException& ex) // No active frame buffer
        {
        }
        catch (const LiteFX::InvalidArgumentException& ex)
        {
            LITEFX_TEST_FAIL("renderPass->commandBuffer(5): Expected LiteFX::RuntimeException but caught LiteFX::InvalidArgumentException.");
        }

        if (&renderPass->commandQueue() != &_device->defaultQueue(QueueType::Graphics))
            LITEFX_TEST_FAIL("&renderPass->commandQueue() != &_device->defaultQueue(QueueType::Graphics)");

        if (&renderPass->device() != _device)
            LITEFX_TEST_FAIL("&renderPass->device() != _device");

        if (renderPass->hasPresentTarget())
            LITEFX_TEST_FAIL("renderPass->hasPresentTarget()");

        // Validate render targets.
        if (renderPass->renderTargets().size() != 2)
            LITEFX_TEST_FAIL("renderPass->renderTargets().size() != 2");

        try
        {
            renderPass->renderTarget(2);
            LITEFX_TEST_FAIL("renderPass->renderTarget(2) was not expected to succeed.");
        }
        catch (const LiteFX::InvalidArgumentException& ex)
        {
        }

        auto& colorTarget = renderPass->renderTarget(1);
        auto& depthTarget = renderPass->renderTarget(3);

        if (colorTarget.location() != 1)
            LITEFX_TEST_FAIL("colorTarget.location() != 1");

        if (depthTarget.location() != 3)
            LITEFX_TEST_FAIL("colorTarget.location() != 3");

        if (colorTarget.type() != RenderTargetType::Color)
            LITEFX_TEST_FAIL("colorTarget.type() != RenderTargetType::Color");

        if (depthTarget.type() != RenderTargetType::DepthStencil)
            LITEFX_TEST_FAIL("colorTarget.type() != RenderTargetType::DepthStencil");

        if (colorTarget.format() != Format::B8G8R8A8_UNORM)
            LITEFX_TEST_FAIL("colorTarget.format() != Format::B8G8R8A8_UNORM");

        if (depthTarget.format() != Format::D32_SFLOAT)
            LITEFX_TEST_FAIL("colorTarget.format() != Format::D32_SFLOAT");

        if (colorTarget.flags() != RenderTargetFlags::Clear)
            LITEFX_TEST_FAIL("colorTarget.flags() != RenderTargetFlags::Clear");

        if (depthTarget.flags() != RenderTargetFlags::Clear)
            LITEFX_TEST_FAIL("colorTarget.flags() != RenderTargetFlags::Clear");

        // Create another render pass.
        UniquePtr<VulkanRenderPass> deferredPass = _device->buildRenderPass("Deferred", 1)
            .inputAttachmentSamplerBinding(DescriptorBindingPoint { .Register = 42, .Space = 5 })
            .inputAttachment(DescriptorBindingPoint { .Register = 1, .Space = 4 }, colorTarget)
            .inputAttachment(DescriptorBindingPoint { .Register = 2, .Space = 4 }, *renderPass, 3)
            .executeOn(_device->defaultQueue(QueueType::Graphics))
            .renderTarget("Output", 2, RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f });

        // Validate render pass.
        if (deferredPass->commandBuffers().size() != 0) // No command buffers if not active.
            LITEFX_TEST_FAIL("deferredPass->commandBuffers().size() != 0");

        if (deferredPass->secondaryCommandBuffers() != 1)
            LITEFX_TEST_FAIL("deferredPass->secondaryCommandBuffers() != 1");

        if (&deferredPass->commandQueue() != &_device->defaultQueue(QueueType::Graphics))
            LITEFX_TEST_FAIL("&deferredPass->commandQueue() != &_device->defaultQueue(QueueType::Graphics)");

        if (&deferredPass->device() != _device)
            LITEFX_TEST_FAIL("&deferredPass->device() != _device");

        if (!deferredPass->hasPresentTarget())
            LITEFX_TEST_FAIL("!deferredPass->hasPresentTarget()");

        // Validate render targets.
        if (deferredPass->renderTargets().size() != 1)
            LITEFX_TEST_FAIL("deferredPass->renderTargets().size() != 1");

        auto& presentTarget = deferredPass->renderTarget(2);

        if (presentTarget.location() != 2)
            LITEFX_TEST_FAIL("presentTarget.location() != 2");

        if (presentTarget.type() != RenderTargetType::Present)
            LITEFX_TEST_FAIL("presentTarget.type() != RenderTargetType::Present");

        if (presentTarget.format() != Format::B8G8R8A8_UNORM)
            LITEFX_TEST_FAIL("presentTarget.format() != Format::B8G8R8A8_UNORM");

        if (presentTarget.flags() != RenderTargetFlags::Clear)
            LITEFX_TEST_FAIL("presentTarget.flags() != RenderTargetFlags::Clear");

        // Validate input attachments.
        if (deferredPass->inputAttachments().size() != 2)
            LITEFX_TEST_FAIL("deferredPass->inputAttachments().size() != 2");

        try
        {
            renderPass->inputAttachment(3);
            LITEFX_TEST_FAIL("renderPass->inputAttachment(3) was not expected to succeed.");
        }
        catch (const LiteFX::ArgumentOutOfRangeException& ex)
        {
        }

        auto& colorAttachment = deferredPass->inputAttachment(0);
        auto& depthAttachment = deferredPass->inputAttachment(1);

        if (colorAttachment.binding().Register != 1 || colorAttachment.binding().Space != 4)
            LITEFX_TEST_FAIL("colorAttachment.binding().Register != 1 || colorAttachment.binding().Space != 4");

        if (depthAttachment.binding().Register != 2 || depthAttachment.binding().Space != 4)
            LITEFX_TEST_FAIL("depthAttachment.binding().Register != 2 || depthAttachment.binding().Space != 4");

        if (colorAttachment.renderTarget().identifier() != colorTarget.identifier())
            LITEFX_TEST_FAIL("colorAttachment.renderTarget().identifier() != colorTarget.identifier()");

        if (depthAttachment.renderTarget().identifier() != depthTarget.identifier())
            LITEFX_TEST_FAIL("depthAttachment.renderTarget().identifier() != depthTarget.identifier()");

        return true;
    };

    auto stopCallback = [](VulkanBackend* backend) {
        backend->releaseDevice("Default");
    };

    // Register the DirectX 12 backend de-/initializer.
    this->onBackendStart<VulkanBackend>(startCallback);
    this->onBackendStop<VulkanBackend>(stopCallback);
}

void TestApp::onStartup()
{
}

void TestApp::onShutdown()
{
}

void TestApp::onResize(const void* sender, ResizeEventArgs e)
{
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    // Register a window class.
    HINSTANCE instance = ::GetModuleHandle(nullptr);
    const auto windowClassName = "Test App Window Class";

    WNDCLASSEX windowClass {
        .cbSize = sizeof(WNDCLASSEX),
        .lpfnWndProc = ::WndProc,
        .hInstance = instance,
        .hIcon = ::LoadIcon(nullptr, IDI_APPLICATION),
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszClassName = windowClassName,
        .hIconSm = LoadIcon(NULL, IDI_APPLICATION)
    };

    if (!::RegisterClassEx(&windowClass))
    {
        std::cerr << "Unable to register window class." << std::endl;
        return EXIT_FAILURE;
    }

    // Create a window instance.
    _window = ::CreateWindowEx(WS_EX_CLIENTEDGE, windowClassName, "Test App",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT,
        nullptr, nullptr, instance, nullptr);

    if (_window == NULL)
    {
        std::cerr << "Unable to create test window." << std::endl;
        return EXIT_FAILURE;
    }

    // Show the window.
    ::ShowWindow(_window, SW_SHOWNORMAL);
    ::UpdateWindow(_window);

    // Setup instance extensions and validation layers.
    Array<String> extensions { VK_KHR_SURFACE_EXTENSION_NAME };
    Array<String> layers { "VK_LAYER_KHRONOS_validation" };

#if defined(WIN32)
    // Enable Windows-specific extensions.
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif // defined(WIN32)

	// Create the app.
	try
	{
		UniquePtr<App> app = App::build<TestApp>()
            .logTo<ConsoleSink>(LogLevel::Trace)
			.logTo<TerminationSink>(LogLevel::Error) // Exit on error.
			.useBackend<VulkanBackend>(extensions, layers);

		app->run();
	}
	catch (const LiteFX::Exception& ex)
	{
		std::cerr << "Unhandled exception: " << ex.what() << '\n' << "at: " << ex.trace() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}