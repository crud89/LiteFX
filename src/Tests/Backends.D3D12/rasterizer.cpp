#include "common.h"
#include <filesystem>

#define FRAMEBUFFER_WIDTH  800
#define FRAMEBUFFER_HEIGHT 600

HWND _window{ nullptr };

SharedPtr<Viewport> _viewport;
SharedPtr<Scissor> _scissor;
DirectX12Device* _device;

void TestApp::onInit()
{
    // Create a callback for backend startup and shutdown.
    auto startCallback = [this](DirectX12Backend* backend) {
        // Create viewport and scissors.
        _viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));
        _scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));

        // As we've enabled WARP, it's the only one available.
        auto adapter = backend->findAdapter(std::nullopt);
        auto surface = backend->createSurface(_window);

        // Create the device.
        _device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, _viewport->getRectangle().extent(), 3, false);

        // Create a rasterizer.
        // TODO: Include depth/stencil state and bias.
        SharedPtr<DirectX12Rasterizer> rasterizer = _device->buildRasterizer()
            .polygonMode(PolygonMode::Solid)
            .cullMode(CullMode::BackFaces)
            .cullOrder(CullOrder::ClockWise)
            .lineWidth(1.f);

        // Validate rasterizer.
        if (std::as_const(*rasterizer).polygonMode() != PolygonMode::Solid)
            LITEFX_TEST_FAIL("rasterizer->polygonMode() != PolygonMode::Solid");

        if (std::as_const(*rasterizer).cullMode() != CullMode::BackFaces)
            LITEFX_TEST_FAIL("rasterizer->cullMode() != CullMode::BackFaces");

        if (std::as_const(*rasterizer).cullOrder() != CullOrder::ClockWise)
            LITEFX_TEST_FAIL("rasterizer->cullOrder() != CullOrder::ClockWise");

        if (std::as_const(*rasterizer).lineWidth() != 1.f)
            LITEFX_TEST_FAIL("rasterizer->lineWidth() != 1.f");

        return true;
    };

    auto stopCallback = [](DirectX12Backend* backend) {
        backend->releaseDevice("Default");
    };

    // Register the DirectX 12 backend de-/initializer.
    this->onBackendStart<DirectX12Backend>(startCallback);
    this->onBackendStop<DirectX12Backend>(stopCallback);
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
    // Set the current path.
    auto binaryDir = std::filesystem::path(argv[0]);
    std::filesystem::current_path(binaryDir.remove_filename());

    // Register a window class.
    HINSTANCE instance = ::GetModuleHandle(nullptr);
    const auto windowClassName = "Test App Window Class";

    WNDCLASSEX windowClass{
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

    // Create the app.
    try
    {
        UniquePtr<App> app = App::build<TestApp>()
            .logTo<ConsoleSink>(LogLevel::Error)
            .logTo<TerminationSink>(LogLevel::Error) // Exit on error.
            .useBackend<DirectX12Backend>(true); // Use WARP.

        app->run();
    }
    catch (const LiteFX::Exception& ex)
    {
        std::cerr << "Unhandled exception: " << ex.what() << '\n' << "at: " << ex.trace() << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}