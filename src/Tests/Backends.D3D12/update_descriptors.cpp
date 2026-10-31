#include "common.h"
#include <filesystem>

#define FRAMEBUFFER_WIDTH  800
#define FRAMEBUFFER_HEIGHT 600

HWND _window{ nullptr };

SharedPtr<Viewport> _viewport;
SharedPtr<Scissor> _scissor;
SharedPtr<DirectX12Device> _device;

struct Vertex {
    Vector3f Position;
    Vector4f Color;
    Vector3f Normal;
    Vector2f TextureCoordinate0;
};

void TestApp::onInit()
{
    // Create a callback for backend startup and shutdown.
    auto startCallback = [](DirectX12Backend* backend) {
        // Create viewport and scissors.
        _viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));
        _scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(FRAMEBUFFER_WIDTH), static_cast<Float>(FRAMEBUFFER_HEIGHT)));

        // Find adapter and create surface.
        auto adapter = backend->findAdapter(std::nullopt);
        auto surface = backend->createSurface(_window);

        // Create the device.
        _device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, _viewport->getRectangle().extent(), 3, false).shared_from_this();

        // Returns `true`, if `callable` throws.
        auto throws = [](auto&& callable) -> bool {
            try { callable(); return false; }
            catch (const std::exception&) { return true; }
        };

        // Create a layout covering the different buffer types. Element sizes of 256 bytes satisfy every possible buffer alignment, so the elements are never padded.
        SharedPtr<DirectX12PipelineLayout> pipelineLayout = _device->buildPipelineLayout()
            .descriptorSet(0, ShaderStage::Compute)
            .withConstantBuffer(0, 256)
            .withByteAddressBuffer(1)
            .withStructuredBufferArray(2, 4)
            .withSampler(3)
            .add();

        auto& layout = pipelineLayout->descriptorSet(0);
        auto descriptorSet = layout.allocate();

        auto& factory = _device->factory();
        auto constantBuffer = factory.createBuffer(BufferType::Uniform, ResourceHeap::Dynamic, 256, 3);
        auto storageBuffer = factory.createBuffer(BufferType::Storage, ResourceHeap::Resource, 256, 4);
        auto sampler = factory.createSampler();

        // Single binding with an explicit binding point.
        if (throws([&] { descriptorSet->update(DescriptorBinding::constantBuffer(*constantBuffer, 1).at(0)); }))
            LITEFX_TEST_FAIL("Updating a single constant buffer failed.");

        // Collections use the index as the binding point.
        Array<DescriptorBinding> bindings {
            DescriptorBinding::constantBuffer(*constantBuffer),
            DescriptorBinding::byteAddressBuffer(*storageBuffer, 0, 1),
            DescriptorBinding::structuredBuffer(*storageBuffer, 0, 4),
            DescriptorBinding::sampler(*sampler)
        };

        if (throws([&] { descriptorSet->update(Span<const DescriptorBinding>{ bindings }); }))
            LITEFX_TEST_FAIL("Updating from a span failed.");

        if (throws([&] { descriptorSet->update({ DescriptorBinding::constantBuffer(*constantBuffer), DescriptorBinding::byteAddressBuffer(*storageBuffer, 0, 1) }); }))
            LITEFX_TEST_FAIL("Updating from an initializer list failed.");

        if (throws([&] { descriptorSet->update(Enumerable<const DescriptorBinding&>{ bindings }); }))
            LITEFX_TEST_FAIL("Updating from an enumerable failed.");

        if (throws([&] { descriptorSet->update([&]() -> Generator<const DescriptorBinding&> { for (const auto& binding : bindings) co_yield binding; }()); }))
            LITEFX_TEST_FAIL("Updating from a generator failed.");

        // Default bindings during allocation must also use the index.
        if (throws([&] { std::ignore = layout.allocate({ DescriptorBinding::constantBuffer(*constantBuffer), DescriptorBinding::byteAddressBuffer(*storageBuffer, 0, 1) }); }))
            LITEFX_TEST_FAIL("Allocating with default bindings failed.");

        // All elements in a single descriptor.
        if (throws([&] { descriptorSet->update(DescriptorBinding::byteAddressBuffer(*storageBuffer, 0, 4, 0).at(1)); }))
            LITEFX_TEST_FAIL("Binding all elements to a single descriptor failed.");

        // One descriptor per element exceeds the binding's capacity.
        if (!throws([&] { descriptorSet->update(DescriptorBinding::byteAddressBuffer(*storageBuffer, 0, 4, 1).at(1)); }))
            LITEFX_TEST_FAIL("Exceeding the descriptor capacity was not rejected.");

        // The bound elements must be divisible by the elements per descriptor.
        if (!throws([&] { descriptorSet->update(DescriptorBinding::structuredBuffer(*storageBuffer, 0, 3, 2).at(2)); }))
            LITEFX_TEST_FAIL("A non-divisible element count was not rejected.");

        // Buffer types must match the descriptor type.
        if (!throws([&] { descriptorSet->update(DescriptorBinding::constantBuffer(*storageBuffer).at(0)); }))
            LITEFX_TEST_FAIL("Binding a storage buffer to a constant buffer descriptor was not rejected.");

        // Unknown binding points must be rejected.
        if (!throws([&] { descriptorSet->update(DescriptorBinding::constantBuffer(*constantBuffer).at(42)); }))
            LITEFX_TEST_FAIL("An unknown binding point was not rejected.");

        // Views over multiple padded elements must be rejected. On D3D12, 4-byte storage elements are always padded to 16 bytes.
        auto paddedBuffer = factory.createBuffer(BufferType::Storage, ResourceHeap::Resource, 4, 4);

        if (paddedBuffer->alignedElementSize() != paddedBuffer->elementSize() && !throws([&] { descriptorSet->update(DescriptorBinding::byteAddressBuffer(*paddedBuffer, 0, 4, 0).at(1)); }))
            LITEFX_TEST_FAIL("A view over padded elements was not rejected.");

        // Descriptor ranges.
        auto range = _device->allocateGlobalDescriptors(DescriptorHeapType::Resource, 4);

        if (throws([&] { range->update(DescriptorType::StructuredBuffer, DescriptorBinding::structuredBuffer(*storageBuffer, 0, 4)); }))
            LITEFX_TEST_FAIL("Updating a descriptor range failed.");

        if (throws([&] { range->update(DescriptorType::ByteAddressBuffer, DescriptorBinding::byteAddressBuffer(*storageBuffer, 0, 4, 0).toDescriptor(3)); }))
            LITEFX_TEST_FAIL("Updating a single descriptor at an offset within a range failed.");

        if (!throws([&] { range->update(DescriptorType::StructuredBuffer, DescriptorBinding::structuredBuffer(*storageBuffer, 0, 4).toDescriptor(1)); }))
            LITEFX_TEST_FAIL("Exceeding the range was not rejected.");

        if (!throws([&] { range->update(DescriptorType::Sampler, DescriptorBinding::sampler(*sampler)); }))
            LITEFX_TEST_FAIL("Writing a sampler into a resource range was not rejected.");

        if (range->globalIndex(2) != range->globalIndex() + 2)
            LITEFX_TEST_FAIL("range->globalIndex(2) != range->globalIndex() + 2");

        return true;
    };

    auto stopCallback = [](DirectX12Backend* backend) {
        _device.reset();
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