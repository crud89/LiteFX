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
#include <comdef.h>

#include <wrl.h>
using namespace Microsoft::WRL;

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Math;
    using namespace LiteFX::Rendering;

    constexpr char DIRECTX12_LOG[] = "Backend::DirectX12";

    // Forward declarations.
    class DirectX12VertexBufferLayout;
    class DirectX12IndexBufferLayout;
    class DirectX12DescriptorLayout;
    class DirectX12DescriptorSetLayout;
    class DirectX12DescriptorSet;
    class DirectX12PipelineLayout;
    class DirectX12ShaderModule;
    class DirectX12ShaderProgram;
    class DirectX12CommandBuffer;
    class DirectX12InputAssembler;
    class DirectX12Rasterizer;
    class DirectX12PipelineState;
    class DirectX12RenderPipeline;
    class DirectX12ComputePipeline;
    class DirectX12FrameBuffer;
    class DirectX12RenderPass;
    class DirectX12InputAttachmentMapping;
    class DirectX12SwapChain;
    class DirectX12Queue;
    class DirectX12GraphicsFactory;
    class DirectX12Device;
    class DirectX12Backend;

    // Interface declarations.
    class IDirectX12Buffer;
    class IDirectX12VertexBuffer;
    class IDirectX12IndexBuffer;
    class IDirectX12ConstantBuffer;
    class IDirectX12Image;
    class IDirectX12Texture;
    class IDirectX12Sampler;

    // Builder declarations.
    class DirectX12VertexBufferLayoutBuilder;
    class DirectX12RenderPipelineDescriptorSetLayoutBuilder;
    class DirectX12ComputePipelineDescriptorSetLayoutBuilder;
    class DirectX12RenderPipelineLayoutBuilder;
    class DirectX12ComputePipelineLayoutBuilder;
    class DirectX12GraphicsShaderProgramBuilder;
    class DirectX12ComputeShaderProgramBuilder;
    class DirectX12InputAssemblerBuilder;
    class DirectX12RasterizerBuilder;
    class DirectX12RenderPipelineBuilder;
    class DirectX12ComputePipelineBuilder;
    class DirectX12RenderPassBuilder;
    class DirectX12BackendBuilder;

    /// <summary>
    /// A resource that is hold by a <c>ComPtr</c>.
    /// </summary>
    /// <typeparam name="THandle">The type of the resource interface.</typeparam>
    template <class THandle>
    using ComResource = Resource<ComPtr<THandle>>;
    // TODO: We could overwrite the handle() methods here and return `.AsWeak` from the ComPtr and create an overload to get a reference-counted handle, if required.

    // Conversion helpers.
    /// <summary>
    /// 
    /// </summary>
    Format LITEFX_DIRECTX12_API getFormat(const DXGI_FORMAT& format);

    /// <summary>
    /// 
    /// </summary>
    DXGI_FORMAT LITEFX_DIRECTX12_API getFormat(const Format& format);

    /// <summary>
    /// 
    /// </summary>
    DXGI_FORMAT LITEFX_DIRECTX12_API getFormat(const BufferFormat& format);

    /// <summary>
    /// 
    /// </summary>
    PolygonMode LITEFX_DIRECTX12_API getPolygonMode(const D3D12_FILL_MODE& mode);

    /// <summary>
    /// 
    /// </summary>
    D3D12_FILL_MODE LITEFX_DIRECTX12_API getPolygonMode(const PolygonMode& mode);

    /// <summary>
    /// 
    /// </summary>
    CullMode LITEFX_DIRECTX12_API getCullMode(const D3D12_CULL_MODE& mode);

    /// <summary>
    /// 
    /// </summary>
    D3D12_CULL_MODE LITEFX_DIRECTX12_API getCullMode(const CullMode& mode);

    /// <summary>
    /// 
    /// </summary>
    PrimitiveTopology LITEFX_DIRECTX12_API getPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY& topology);

    /// <summary>
    /// 
    /// </summary>
    D3D12_PRIMITIVE_TOPOLOGY LITEFX_DIRECTX12_API getPrimitiveTopology(const PrimitiveTopology& topology);

    /// <summary>
    /// 
    /// </summary>
    D3D12_PRIMITIVE_TOPOLOGY_TYPE LITEFX_DIRECTX12_API getPrimitiveTopologyType(const PrimitiveTopology& topology);

    /// <summary>
    /// 
    /// </summary>
    LPCTSTR LITEFX_DIRECTX12_API getSemanticName(const AttributeSemantic& semantic);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="vendorId"></param>
    /// <returns></returns>
    String LITEFX_DIRECTX12_API getVendorName(const UInt32& vendorId);

    /// <summary>
    /// 
    /// </summary>
    D3D12_COMPARISON_FUNC LITEFX_DIRECTX12_API getCompareOp(const CompareOperation& compareOp);

    /// <summary>
    /// 
    /// </summary>
    D3D12_STENCIL_OP LITEFX_DIRECTX12_API getStencilOp(const StencilOperation& stencilOp);

    /// <summary>
    /// 
    /// </summary>
    D3D12_BLEND LITEFX_DIRECTX12_API getBlendFactor(const BlendFactor& blendFactor);

    /// <summary>
    /// 
    /// </summary>
    D3D12_BLEND_OP LITEFX_DIRECTX12_API getBlendOperation(const BlendOperation& blendOperation);

    /// <summary>
    /// Implements a DirectX12 <see cref="IGraphicsAdapter" />.
    /// </summary>
    class LITEFX_DIRECTX12_API DirectX12GraphicsAdapter : public IGraphicsAdapter, public ComResource<IDXGIAdapter4> {
        LITEFX_IMPLEMENTATION(DirectX12GraphicsAdapterImpl);

    public:
        /// <summary>
        /// Initializes a new DirectX12 graphics adapter.
        /// </summary>
        /// <param name="adapter">The DXGI adapter interface pointer.</param>
        explicit DirectX12GraphicsAdapter(ComPtr<IDXGIAdapter4> adapter);
        DirectX12GraphicsAdapter(const DirectX12GraphicsAdapter&) = delete;
        DirectX12GraphicsAdapter(DirectX12GraphicsAdapter&&) = delete;
        virtual ~DirectX12GraphicsAdapter() noexcept;

    public:
        /// <inheritdoc />
        virtual String getName() const noexcept override;

        /// <inheritdoc />
        virtual UInt32 getVendorId() const noexcept override;

        /// <inheritdoc />
        virtual UInt32 getDeviceId() const noexcept override;

        /// <inheritdoc />
        virtual GraphicsAdapterType getType() const noexcept override;

        /// <inheritdoc />
        /// <remarks>
        /// This property is not supported by DirectX 12. The method always returns `0`.
        /// </remarks>
        virtual UInt32 getDriverVersion() const noexcept override;

        /// <inheritdoc />
        /// <remarks>
        /// This property is not supported by DirectX 12. The method always returns `0`.
        /// </remarks>
        virtual UInt32 getApiVersion() const noexcept override;

        /// <inheritdoc />
        virtual UInt64 getDedicatedMemory() const noexcept override;
    };

    /// <summary>
    /// Implements a DirectX12 <see cref="ISurface" />.
    /// </summary>
    class LITEFX_DIRECTX12_API DirectX12Surface : public ISurface, public Resource<HWND> {
    public:
        /// <summary>
        /// Initializes a new DirectX 12 surface.
        /// </summary>
        /// <param name="hwnd">The window handle.</param>
        explicit DirectX12Surface(const HWND& hwnd) noexcept;
        DirectX12Surface(const DirectX12Surface&) = delete;
        DirectX12Surface(DirectX12Surface&&) = delete;
        virtual ~DirectX12Surface() noexcept;
    };

    template <typename TParent>
    class LITEFX_DIRECTX12_API DirectX12RuntimeObject {
    private:
        const TParent& m_parent;
        const DirectX12Device* m_device;

    public:
        explicit DirectX12RuntimeObject(const TParent& parent, const DirectX12Device* device) :
            m_parent(parent), m_device(device)
        {
            if (device == nullptr)
                throw ArgumentNotInitializedException("The device must be initialized.");
        }

        DirectX12RuntimeObject(DirectX12RuntimeObject&&) = delete;
        DirectX12RuntimeObject(const DirectX12RuntimeObject&) = delete;
        virtual ~DirectX12RuntimeObject() noexcept = default;

    public:
        virtual const TParent& parent() const noexcept { return m_parent; }
        virtual const DirectX12Device* getDevice() const noexcept { return m_device; };
    };

    DEFINE_EXCEPTION(DX12PlatformException, std::runtime_error);

    template <typename TException, typename ...TArgs>
    inline void raiseIfFailed(HRESULT hr, const std::string& message, TArgs&&... args) {
        if (SUCCEEDED(hr)) [[likely]]
            return;

        _com_error error(hr);

        if (message.empty())
            throw TException(DX12PlatformException("{1} (HRESULT 0x{0:08X})", static_cast<unsigned>(hr), error.ErrorMessage()));
        else
            throw TException(DX12PlatformException("{1} (HRESULT 0x{0:08X})", static_cast<unsigned>(hr), error.ErrorMessage()), fmt::format(message, std::forward<TArgs>(args)...));
    }
}