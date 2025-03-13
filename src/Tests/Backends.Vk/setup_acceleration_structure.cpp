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

const Array<Vertex> vertices {
    { { -0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, {  1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, {  1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, {  1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, {  1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } }
};

const Array<UInt16> indices {
    0, 1, 2, 1, 3, 2,       // Front
    4, 6, 5, 5, 6, 7,       // Back
    8, 9, 10, 9, 11, 10,    // Right
    12, 14, 13, 13, 14, 15, // Left
    16, 17, 18, 17, 19, 18, // Bottom
    20, 22, 21, 21, 22, 23  // Top
};

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
        _device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, _viewport->getRectangle().extent(), 3, false, GraphicsDeviceFeatures { .RayTracing = true }).shared_from_this();

        // Create input assembler state.
        SharedPtr<VulkanInputAssembler> inputAssembler = _device->buildInputAssembler()
            .topology(PrimitiveTopology::TriangleList)
            .indexType(IndexType::UInt16)
            .vertexBuffer(sizeof(Vertex), 0)
                .withAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
                .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color), AttributeSemantic::Color)
                .add();

        // Get a command buffer.
        auto commandBuffer = _device->defaultQueue(QueueType::Graphics).createCommandBuffer(true);

        // Create the vertex buffer.
        auto vertexBuffer = _device->factory().createVertexBuffer("Vertex Buffer", inputAssembler->vertexBufferLayout(0), 
            ResourceHeap::Resource, static_cast<UInt32>(vertices.size()), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
        commandBuffer->transfer(vertices.data(), static_cast<UInt32>(vertices.size() * sizeof(::Vertex)), *vertexBuffer, 0, static_cast<UInt32>(vertices.size()));

        // Create the index buffer.
        auto indexBuffer = _device->factory().createIndexBuffer("Index Buffer", *inputAssembler->indexBufferLayout(), 
            ResourceHeap::Resource, static_cast<UInt32>(indices.size()), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
        commandBuffer->transfer(indices.data(), static_cast<UInt32>(indices.size() * inputAssembler->indexBufferLayout()->elementSize()), *indexBuffer, 0, static_cast<UInt32>(indices.size()));

        // Wait for the transfer.
        auto barrier = _device->makeBarrier(PipelineStage::Transfer, PipelineStage::AccelerationStructureBuild);
        barrier->transition(*vertexBuffer, ResourceAccess::TransferWrite, ResourceAccess::Common);
        barrier->transition(*indexBuffer, ResourceAccess::TransferWrite, ResourceAccess::Common);
        commandBuffer->barrier(*barrier);

        // Create BLAS.
        auto blas = LiteFX::asShared(_device->factory().createBottomLevelAccelerationStructure("BLAS", AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory));
        blas->withTriangleMesh({ vertexBuffer, indexBuffer });

        // Allocate a buffer to built the BLAS into.
        UInt64 blasSize{}, scratchSize{};
        _device->computeAccelerationStructureSizes(*blas, blasSize, scratchSize);
        auto blasBuffer = _device->factory().createBuffer("BLAS Buffer", BufferType::AccelerationStructure, ResourceHeap::Resource, static_cast<size_t>(blasSize), 1u, ResourceUsage::AllowWrite);

        // Create TLAS.
        auto tlas = _device->factory().createTopLevelAccelerationStructure("TLAS", AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory);
        tlas->withInstance(blas, 0);

        // Allocate a buffer to built the TLAS into.
        UInt64 tlasSize{}, tlasScratchSize{};
        _device->computeAccelerationStructureSizes(*tlas, tlasSize, tlasScratchSize);
        auto scratchBufferSize = static_cast<size_t>(std::max(scratchSize, tlasScratchSize));
        auto scratchBuffer = _device->factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, scratchBufferSize, 1, ResourceUsage::AllowWrite);

        // Build the BLAS and the TLAS.
        blas->build(*commandBuffer, scratchBuffer, blasBuffer, 0, blasSize);
        barrier = _device->makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
        barrier->transition(*scratchBuffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureWrite);
        commandBuffer->barrier(*barrier);
        tlas->build(*commandBuffer, scratchBuffer);

        // Submit and wait.
        auto fence = commandBuffer->submit();
        _device->defaultQueue(QueueType::Graphics).waitFor(fence);

        if (tlas->instances().size() != 1)
            LITEFX_TEST_FAIL("tlas->instances().size() != 1");

        if (tlas->instances().front().BottomLevelAccelerationStructure != blas)
            LITEFX_TEST_FAIL("tlas->instances().front().BottomLevelAccelerationStructure != blas");

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