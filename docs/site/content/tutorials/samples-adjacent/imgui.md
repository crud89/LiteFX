# Dear ImGui

!!! abstract "Sample 15"

    [`Samples/ImGui`](https://github.com/crud89/LiteFX/tree/main/src/Samples/ImGui) · builds on
    [basic rendering](../samples-beginner/basic-rendering.md).

[Dear ImGui](https://github.com/ocornut/imgui) is a widely used library for debug and tool interfaces. It brings its own backends for Vulkan
and DirectX 12, which need a few handles from the engine. The sample shows how to provide them, and draws its interface on top of the
geometry.

## Initializing the backends

ImGui is set up when a backend starts, and shut down when it stops. Both backends get the objects they need from the device:

=== "Vulkan"

    ```cpp
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = backend->handle();
    initInfo.PhysicalDevice = device->adapter().handle();
    initInfo.Device = device->handle();
    initInfo.Queue = device->defaultQueue(QueueType::Graphics).handle();
    initInfo.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE;
    initInfo.MinImageCount = device->swapChain().buffers();
    initInfo.ImageCount = device->swapChain().buffers();
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = { /* color attachment format of the render pass */ };

    ImGui_ImplVulkan_Init(&initInfo);
    ```

    The engine uses dynamic rendering, so ImGui is told the formats of the render targets instead of a render pass handle.

=== "DirectX 12"

    ```cpp
    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device = device->handle().Get();
    initInfo.CommandQueue = device->defaultQueue(QueueType::Graphics).handle().Get();
    initInfo.NumFramesInFlight = static_cast<int>(device->swapChain().buffers());
    initInfo.SrvDescriptorHeap = device->globalBufferHeap();
    initInfo.SrvDescriptorAllocFn = SampleApp::allocImGuiD3D12DescriptorsCallback;
    initInfo.SrvDescriptorFreeFn = SampleApp::releaseImGuiD3D12DescriptorsCallback;

    ImGui_ImplDX12_Init(&initInfo);
    ```

In both cases, the platform backend is initialized as well: `ImGui_ImplGlfw_InitForOther(window, true)`.

## Descriptors in DirectX 12

DirectX 12 keeps all descriptors in global heaps, which the device manages. ImGui needs descriptors of its own, for example for the font
texture, and asks for them through the two callbacks above. The engine hands out *descriptor ranges* from its global heaps, which already
provide the CPU and GPU handles ImGui expects:

```cpp
void SampleApp::allocImGuiD3D12DescriptorsCallback(ImGui_ImplDX12_InitInfo* context, D3D12_CPU_DESCRIPTOR_HANDLE* cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* gpu_handle)
{
    auto app = static_cast<SampleApp*>(context->UserData);
    auto& device = dynamic_cast<const DirectX12Device&>(*app->m_device);

    auto descriptorRange = device.allocateGlobalDescriptors(DescriptorHeapType::Resource, 1u);
    *cpu_handle = descriptorRange->cpuHandle();
    *gpu_handle = descriptorRange->gpuHandle();

    // Store the descriptor range, so we can properly release it later.
    app->m_d3dDescriptorAllocations.emplace(cpu_handle->ptr, std::move(descriptorRange));
}
```

A descriptor range owns its descriptors and releases them when it is destroyed. The sample keeps the ranges in a map, keyed by their CPU
handle, so freeing a descriptor only means removing its range:

```cpp
void SampleApp::releaseImGuiD3D12DescriptorsCallback(ImGui_ImplDX12_InitInfo* context, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE /*gpu_handle*/)
{
    auto app = static_cast<SampleApp*>(context->UserData);
    app->m_d3dDescriptorAllocations.erase(cpu_handle.ptr);
}
```

When the backend stops, the sample clears the map after shutting ImGui down, which releases all remaining ranges before the device goes
away. The same `allocateGlobalDescriptors` call is used by the [dynamic descriptors](../samples-advanced/dynamic-descriptors.md) sample, so
any library that manages its own descriptors can be integrated this way.

## Drawing the interface

The interface is built and drawn inside the render pass, after the geometry, so that it appears on top:

```cpp
renderPass.begin(frameBuffer);
auto commandBuffer = renderPass.commandBuffer(0);
// ... draw the geometry as before ...

ImGui_ImplDX12_NewFrame();          // or ImGui_ImplVulkan_NewFrame()
ImGui_ImplGlfw_NewFrame();
ImGui::NewFrame();

ImGui::Begin("Debug");
// ... widgets ...
ImGui::End();

ImGui::Render();
ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dynamic_cast<const DirectX12CommandBuffer&>(commandBuffer).handle().Get());
renderPass.end();
```

ImGui records into the engine's command buffer, which it reaches through the backend-specific handle. The Vulkan backend works the same way,
with `ImGui_ImplVulkan_RenderDrawData` and the Vulkan command buffer handle.

!!! warning "Draw the interface last"

    ImGui binds its own descriptors: descriptor heaps under DirectX 12, classic descriptor sets and push constants under Vulkan. This replaces
    the engine's global descriptor heaps for the rest of the command buffer. Since the interface is the last thing the sample draws, that is
    harmless. If you draw anything after it, bind the global heaps again with `m_device->bindGlobalDescriptorHeaps(commandBuffer)` first.
    That call is expensive, so prefer drawing everything else before the interface.

## Switching backends

The [basic rendering](../samples-beginner/basic-rendering.md) sample switches backends directly from the key callback. With ImGui, that is
not safe: ImGui installs its own GLFW callbacks, and stopping a backend shuts ImGui down, which removes those callbacks while one of them is
still running. The result is a use-after-free.

The sample therefore only records the switch in the key callback:

```cpp
std::function<void()> switchBackendHandler{ nullptr };

void SampleApp::keyDown(int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
        switchBackendHandler = [this]() { this->startBackend<VulkanBackend>(); };
    else if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
        switchBackendHandler = [this]() { this->startBackend<DirectX12Backend>(); };
}
```

The application loop then performs it after the events have been handled, outside of any callback:

```cpp
while (!::glfwWindowShouldClose(m_window.get()))
{
    this->handleEvents();

    // If a backend switch was requested, execute it here, reset it and continue.
    if (switchBackendHandler)
    {
        std::exchange(switchBackendHandler, nullptr)();
        continue;
    }

    this->drawFrame();
    this->updateWindowTitle();
}
```

`std::exchange` resets the handler before calling it, so the switch runs exactly once. The loop then starts over, so the next frame is drawn
with the new backend. The same pattern applies to any library that registers callbacks with the window and is torn down with the backend.
