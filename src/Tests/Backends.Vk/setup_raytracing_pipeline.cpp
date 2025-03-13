#include "common.h"
#include <filesystem>

#define FRAMEBUFFER_WIDTH  800
#define FRAMEBUFFER_HEIGHT 600

HWND _window { nullptr };

SharedPtr<Viewport> _viewport;
SharedPtr<Scissor> _scissor;
SharedPtr<VulkanDevice> _device;

struct Vertex {
    Vector3f Position;
    Vector4f Color;
    Vector3f Normal;
    Vector2f TextureCoordinate0;
};

struct alignas(8) GeometryData {
    UInt32 Index;
    UInt32 Reflective;
    UInt32 Padding[2];
};

void TestApp::onInit()
{
    // Create a callback for backend startup and shutdown.
    auto startCallback = [](VulkanBackend* backend) {
        // Create viewport and scissors.
        _viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));
        _scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));

        // Find adapter and create surface.
        auto adapter = backend->findAdapter(std::nullopt);
        auto surface = backend->createSurface(_window);

        // Create the device.
        _device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, _viewport->getRectangle().extent(), 3, false, GraphicsDeviceFeatures { .RayTracing = true }).shared_from_this();

        // Create input assembler state.
        SharedPtr<VulkanInputAssembler> inputAssembler = _device->buildInputAssembler()
            .topology(PrimitiveTopology::TriangleList)
            .indexType(IndexType::UInt16)
            .vertexBuffer(sizeof(Vertex), 0)
                .withAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
                .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color), AttributeSemantic::Color)
                .add();

        // Create the shader program.
        SharedPtr<VulkanShaderProgram> shaderProgram = _device->buildShaderProgram()
            .withRayGenerationShaderModule("shaders/raytracing_gen.spv")
            .withClosestHitShaderModule("shaders/raytracing_hit.spv", DescriptorBindingPoint{ .Register = 0, .Space = 3 })
            .withMissShaderModule("shaders/raytracing_miss.spv");

        // Build a shader record collection and create a ray-tracing pipeline.
        UniquePtr<VulkanRayTracingPipeline> rayTracingPipeline = _device->buildRayTracingPipeline("RayTracing",
            shaderProgram->buildShaderRecordCollection()
                .withShaderRecord("shaders/raytracing_gen.spv")
                .withShaderRecord("shaders/raytracing_miss.spv")
                .withMeshGeometryHitGroupRecord(std::nullopt, "shaders/raytracing_hit.spv", GeometryData{ .Index = 0, .Reflective = 0 })  // First geometry hit group for first BLAS.
                .withMeshGeometryHitGroupRecord(std::nullopt, "shaders/raytracing_hit.spv", GeometryData{ .Index = 1, .Reflective = 1 })) // Second geometry hit group for second BLAS.
            .maxBounces(16)                       // Important: If changed, the closest hit shader also needs to be updated!
            .maxPayloadSize(sizeof(Float) * 5)    // See HitInfo in raytracing_common.hlsli
            .maxAttributeSize(sizeof(Float) * 2)  // See Attributes in raytracing_common.hlsli
            .layout(shaderProgram->reflectPipelineLayout());

        // Validate the ray tracing pipeline.
        if (rayTracingPipeline->maxAttributeSize() != 8)
            LITEFX_TEST_FAIL("rayTracingPipeline->maxAttributeSize() != 8");

        if (rayTracingPipeline->maxPayloadSize() != 20)
            LITEFX_TEST_FAIL("rayTracingPipeline->maxPayloadSize() != 20");

        if (rayTracingPipeline->maxRecursionDepth() != 16)
            LITEFX_TEST_FAIL("rayTracingPipeline->maxRecursionDepth() != 16");

        if (rayTracingPipeline->shaderRecords().shaderRecords().size() != 4)
            LITEFX_TEST_FAIL("rayTracingPipeline->shaderRecords().shaderRecords().size() != 4");

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

int main(int /*argc*/, char* argv[])
{
    // Set the current path.
    auto binaryDir = std::filesystem::path(argv[0]);
    std::filesystem::current_path(binaryDir.remove_filename());

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