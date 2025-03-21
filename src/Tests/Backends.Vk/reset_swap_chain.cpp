#include "common.h"

#define FRAMEBUFFER_WIDTH  800
#define FRAMEBUFFER_HEIGHT 600

HWND _window { nullptr };

SharedPtr<Viewport> _viewport;
SharedPtr<Scissor> _scissor;
SharedPtr<VulkanDevice> _device;

void TestApp::onInit()
{
    // Create a callback for backend startup and shutdown.
    auto startCallback = [this](VulkanBackend* backend) {
        // Create viewport and scissors.
        _viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));
        _scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));

        // Find adapter and create surface.
        auto adapter = backend->findAdapter(std::nullopt);
        auto surface = backend->createSurface(_window);

        // Create the device.
        _device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, _viewport->getRectangle().extent(), 3, false).shared_from_this();

        // Acquire and validate the swap chain.
        auto& swapChain = _device->swapChain();

        if (swapChain.surfaceFormat() != Format::B8G8R8A8_UNORM)
            LITEFX_TEST_FAIL("swapChain.surfaceFormat() != Format::B8G8R8A8_UNORM");

        if (swapChain.buffers() != 3)
            LITEFX_TEST_FAIL("swapChain.buffers() != 3");

        if (swapChain.verticalSynchronization() == true)
            LITEFX_TEST_FAIL("swapChain.verticalSynchronization() == true");

        // NOTE: Those checks may fail, if the underlying driver reports extents that are abbrevant.
        if (swapChain.renderArea().width() != _viewport->getRectangle().width())
            LITEFX_TEST_FAIL("swapChain.renderArea().width() != _viewport->getRectangle().width()");

        if (swapChain.renderArea().height() != _viewport->getRectangle().height())
            LITEFX_TEST_FAIL("swapChain.renderArea().height() != _viewport->getRectangle().height()");

        // Reset the swap chain.
        swapChain.reset(Format::R8G8B8A8_UNORM, { 400, 300 }, 4, true);

        // Validate the changes.
        if (swapChain.surfaceFormat() != Format::R8G8B8A8_UNORM)
            LITEFX_TEST_FAIL("swapChain.surfaceFormat() != Format::R8G8B8A8_UNORM");

        if (swapChain.buffers() != 4)
            LITEFX_TEST_FAIL("swapChain.buffers() != 4");

        if (swapChain.verticalSynchronization() == false)
            LITEFX_TEST_FAIL("swapChain.verticalSynchronization() == false");

        if (swapChain.renderArea().width() != 400)
            LITEFX_TEST_FAIL("swapChain.renderArea().width() != 400");

        if (swapChain.renderArea().height() != 300)
            LITEFX_TEST_FAIL("swapChain.renderArea().height() != 300");

        return true;
    };

    auto stopCallback = [](VulkanBackend* backend) {
        _device.reset();
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

void TestApp::onResize(const void* /*sender*/, ResizeEventArgs /*e*/)
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

int main(int /*argc*/, char* /*argv*/[])
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
    Array<String> layers { "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2" };

#if defined(WIN32)
    // Enable Windows-specific extensions.
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif // defined(WIN32)

	// Create the app.
	try
	{
		UniquePtr<App> app = App::build<TestApp>()
            .logTo<ConsoleSink>(LogLevel::Error)
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