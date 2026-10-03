# Dear ImGui

!!! abstract "Sample 15"

    [`Samples/ImGui`](https://github.com/crud89/LiteFX/tree/main/src/Samples/ImGui) · builds on
    [basic rendering](basic-rendering.md).

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
    initInfo.DescriptorPoolSize = 10u;
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
texture, and asks for them through the two callbacks above. The engine can hand out ranges from its global heaps:

```cpp
void SampleApp::allocImGuiD3D12DescriptorsCallback(ImGui_ImplDX12_InitInfo* context, D3D12_CPU_DESCRIPTOR_HANDLE* cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* gpu_handle)
{
    auto allocation = device.allocateGlobalDescriptors(1u, DescriptorHeapType::Resource);
    auto descriptorHandleIncrement = device.handle().Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE targetHandle(device.globalBufferHeap()->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(allocation.Offset), descriptorHandleIncrement);
    CD3DX12_GPU_DESCRIPTOR_HANDLE targetGpuHandle(device.globalBufferHeap()->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(allocation.Offset), descriptorHandleIncrement);

    app->m_d3dDescriptorAllocations.emplace(targetHandle.ptr, allocation);
    *cpu_handle = targetHandle;
    *gpu_handle = targetGpuHandle;
}
```

The allocation is kept in a map, so that the matching range can be released when ImGui frees the descriptor:

```cpp
auto match = app->m_d3dDescriptorAllocations.find(cpu_handle.ptr);

if (match != app->m_d3dDescriptorAllocations.end())
{
    device.releaseGlobalDescriptors(DescriptorHeapType::Resource, std::move(match->second));
    app->m_d3dDescriptorAllocations.erase(cpu_handle.ptr);
}
```

Before this feature, applications had to create a dummy descriptor set to get at the heap. With `allocateGlobalDescriptors` and
`releaseGlobalDescriptors`, any library that manages its own descriptors can be integrated this way.

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
