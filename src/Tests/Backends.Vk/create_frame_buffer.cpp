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

        // Create a frame buffer and add targets to it.
        auto frameBuffer = _device->makeFrameBuffer("Frame Buffer", _viewport->getRectangle().extent());
        frameBuffer->addImage("Image 0", Format::B8G8R8A8_UNORM, MultiSamplingLevel::x4);
        frameBuffer->addImage("Image 1", Format::B8G8R8A8_UNORM, MultiSamplingLevel::x1, ResourceUsage::FrameBufferImage | ResourceUsage::AllowWrite);

        // Validate frame buffer.
        if (frameBuffer->getWidth() != FRAMEBUFFER_WIDTH)
            LITEFX_TEST_FAIL("frameBuffer->getWidth() != FRAMEBUFFER_WIDTH");

        if (frameBuffer->getHeight() != FRAMEBUFFER_HEIGHT)
            LITEFX_TEST_FAIL("frameBuffer->getHeight() != FRAMEBUFFER_HEIGHT");

        try
        {
            frameBuffer->image(2);
            LITEFX_TEST_FAIL("frameBuffer->image(2) was not expected to succeed.");
        }
        catch (const LiteFX::ArgumentOutOfRangeException& ex)
        {
        }

        auto& image0 = frameBuffer->image(0);
        auto& image1 = frameBuffer->image(1);
        
        if (image0.extent().width() != FRAMEBUFFER_WIDTH || image0.extent().height() != FRAMEBUFFER_HEIGHT)
            LITEFX_TEST_FAIL("image0.extent().width() != FRAMEBUFFER_WIDTH || image0.extent().height() != FRAMEBUFFER_HEIGHT");

        if (image1.extent().width() != FRAMEBUFFER_WIDTH || image1.extent().height() != FRAMEBUFFER_HEIGHT)
            LITEFX_TEST_FAIL("image1.extent().width() != FRAMEBUFFER_WIDTH || image1.extent().height() != FRAMEBUFFER_HEIGHT");

        if (image0.format() != Format::B8G8R8A8_UNORM)
            LITEFX_TEST_FAIL("image0.format() != Format::B8G8R8A8_UNORM");

        if (image1.format() != Format::B8G8R8A8_UNORM)
            LITEFX_TEST_FAIL("image1.format() != Format::B8G8R8A8_UNORM");

        if (image0.samples() != MultiSamplingLevel::x4)
            LITEFX_TEST_FAIL("image0.samples() != MultiSamplingLevel::x4");

        if (image1.samples() != MultiSamplingLevel::x1)
            LITEFX_TEST_FAIL("image1.samples() != MultiSamplingLevel::x1");

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