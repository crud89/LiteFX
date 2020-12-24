#pragma once

#if !defined (LITEFX_DIRECTX12_API)
#  if defined(LiteFX_Backends_DirectX12_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_DIRECTX12_API __declspec(dllexport)
#  elif (defined(LiteFX_Backends_DirectX12_EXPORTS) || defined(__APPLE__)) && defined __GNUC__ && __GNUC__ >= 4
#    define LITEFX_DIRECTX12_API __attribute__ ((visibility ("default")))
#  elif !defined(LiteFX_Backends_DirectX12_EXPORTS) && (defined _WIN32 || defined WINCE)
#    define LITEFX_DIRECTX12_API __declspec(dllimport)
#  endif
#endif

#ifndef LITEFX_DIRECTX12_API
#  define LITEFX_DIRECTX12_API
#endif

#if (!defined _WIN32 && !defined WINCE)
#  pragma message ("DirectX12: The DirectX backend requires Windows to be built on.")
#endif

#include <directx/d3d12.h>
#include <directx/dxcore.h>
#include <directx/d3dx12.h>
#include <dxguids/dxguids.h>
#include <dxgi1_6.h>

#include <wrl.h>
using namespace Microsoft::WRL;

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Math;
    using namespace LiteFX::Rendering;

    constexpr char DIRECTX12_LOG[] = "Backend::DirectX12";

    template <class THandle>
    using IComResource = IResource<ComPtr<THandle>>;

    // Forward declarations.
    //class DirectX12Texture;
    //class DirectX12SwapChain;
    //class DirectX12Queue;
    //class DirectX12Device;
    class DirectX12GraphicsAdapter;
    class DirectX12Backend;
    //class DirectX12RenderPipeline;
    //class DirectX12RenderPipelineLayout;
    //class DirectX12RenderPass;
    //class DirectX12Rasterizer;
    //class DirectX12Viewport;
    //class DirectX12InputAssembler;
    //class DirectX12ShaderModule;
    //class DirectX12ShaderProgram;
    //class DirectX12CommandBuffer;
    //class DirectX12DescriptorSetLayout;
    //class DirectX12VertexBufferLayout;
    //class DirectX12IndexBufferLayout;
    //class DirectX12DescriptorLayout;
    //class DirectX12VertexBuffer;
    //class DirectX12IndexBuffer;
    //class DirectX12ConstantBuffer;
    //class DirectX12Sampler;
    class DirectX12Surface;
}