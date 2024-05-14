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

        // Create input assembler state.
        SharedPtr<VulkanInputAssembler> inputAssembler = _device->buildInputAssembler()
            .topology(PrimitiveTopology::TriangleList)
            .indexType(IndexType::UInt32)
            .vertexBuffer(sizeof(Vertex), 1)
                .withAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
                .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color), AttributeSemantic::Color)
                .add()
            .vertexBuffer(sizeof(Vertex), 4)
                .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Normal), AttributeSemantic::Normal)
                .withAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
                .add();

        // Validate input assembler.
        if (inputAssembler->topology() != PrimitiveTopology::TriangleList)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->topology() != PrimitiveTopology::TriangleList");

        if (inputAssembler->indexBufferLayout() == nullptr)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->indexBufferLayout() == nullptr");
        
        if (inputAssembler->indexBufferLayout()->indexType() != IndexType::UInt32)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->indexBufferLayout()->indexType() != IndexType::UInt32");

        if (inputAssembler->indexBufferLayout()->type() != BufferType::Index)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->indexBufferLayout()->type() != BufferType::Index");

        if (inputAssembler->vertexBufferLayouts().size() != 2)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayouts().size() != 2");

        try
        {
            inputAssembler->vertexBufferLayout(0);
        }
        catch (const InvalidArgumentException& ex)
        {
        }

        if (inputAssembler->vertexBufferLayout(1)->binding() != 1)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(1)->binding() != 1");

        if (inputAssembler->vertexBufferLayout(1)->elementSize() != sizeof(Vertex))
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(1)->elementSize() != sizeof(Vertex)");

        if (inputAssembler->vertexBufferLayout(1)->type() != BufferType::Vertex)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(1)->type() != BufferType::Vertex");

        if (inputAssembler->vertexBufferLayout(1)->attributes().size() != 2)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(1)->attributes().size() != 2");

        if (inputAssembler->vertexBufferLayout(1)->attributes().front()->format() != BufferFormat::XYZ32F)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(1)->attributes().front()->format() != BufferFormat::XYZ32F");

        if (inputAssembler->vertexBufferLayout(1)->attributes().front()->location() != 0)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(1)->attributes().front()->location() != 0");

        if (inputAssembler->vertexBufferLayout(1)->attributes().front()->offset() != 0)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(1)->attributes().front()->offset() != 0");

        if (inputAssembler->vertexBufferLayout(1)->attributes().front()->semantic() != AttributeSemantic::Position)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(1)->attributes().front()->semantic() != AttributeSemantic::Position");

        if (inputAssembler->vertexBufferLayout(1)->attributes().front()->semanticIndex() != 0)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(1)->attributes().front()->semanticIndex() != 0");

        if (inputAssembler->vertexBufferLayout(4)->binding() != 4)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(4)->binding() != 4");

        if (inputAssembler->vertexBufferLayout(4)->elementSize() != sizeof(Vertex))
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(4)->elementSize() != sizeof(Vertex)");

        if (inputAssembler->vertexBufferLayout(4)->type() != BufferType::Vertex)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(4)->type() != BufferType::Vertex");

        if (inputAssembler->vertexBufferLayout(4)->attributes().size() != 2)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(4)->attributes().size() != 2");

        if (inputAssembler->vertexBufferLayout(4)->attributes().front()->format() != BufferFormat::XYZW32F)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(4)->attributes().front()->format() != BufferFormat::XYZW32F");

        if (inputAssembler->vertexBufferLayout(4)->attributes().front()->location() != 1)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(4)->attributes().front()->location() != 1");

        if (inputAssembler->vertexBufferLayout(4)->attributes().front()->offset() != 28) // addressof(Vertex, Normal)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(4)->attributes().front()->offset() != 28");

        if (inputAssembler->vertexBufferLayout(4)->attributes().front()->semantic() != AttributeSemantic::Normal)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(4)->attributes().front()->semantic() != AttributeSemantic::Normal");

        if (inputAssembler->vertexBufferLayout(4)->attributes().front()->semanticIndex() != 0)
            LITEFX_ERROR(TEST_LOG, "inputAssembler->vertexBufferLayout(4)->attributes().front()->semanticIndex() != 0");

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