#pragma once

#include <litefx/config.h>
#include <litefx/backends/dx12_export.h>

#if (!defined _WIN32 && !defined WINCE)
#  pragma message ("DirectX12: The DirectX backend requires Windows to be built on.")
#endif

#include <directx/d3d12.h>

#if defined(LITEFX_DEFINE_GLOBAL_EXPORTS)
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\d3d12\\"; }
#endif

#include <directx/dxcore.h>
#include <directx/d3dx12.h>
#include <dxguids/dxguids.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <comdef.h>

#include <wrl.h>
using namespace Microsoft::WRL;

#include <litefx/rendering.hpp>
#include "dx12_formatters.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Math;
    using namespace LiteFX::Rendering;

    constexpr StringView DIRECTX12_LOG = "Backend::DirectX12"sv;

    // Forward declarations.
    class DirectX12VertexBufferLayout;
    class DirectX12IndexBufferLayout;
    class DirectX12DescriptorLayout;
    class DirectX12DescriptorSetLayout;
    class DirectX12DescriptorSet;
    class DirectX12PushConstantsRange;
    class DirectX12PushConstantsLayout;
    class DirectX12PipelineLayout;
    class DirectX12ShaderModule;
    class DirectX12ShaderProgram;
    class DirectX12CommandBuffer;
    class DirectX12Barrier;
    class DirectX12InputAssembler;
    class DirectX12Rasterizer;
    class DirectX12PipelineState;
    class DirectX12RenderPipeline;
    class DirectX12ComputePipeline;
    class DirectX12RayTracingPipeline;
    class DirectX12FrameBuffer;
    class DirectX12RenderPass;
    class DirectX12SwapChain;
    class DirectX12Queue;
    class DirectX12GraphicsFactory;
    class DirectX12Device;
    class DirectX12Backend;

    // Interface declarations.
    class IDirectX12Buffer;
    class IDirectX12VertexBuffer;
    class IDirectX12IndexBuffer;
    class IDirectX12Image;
    class IDirectX12Sampler;
    class IDirectX12AccelerationStructure;
    class DirectX12BottomLevelAccelerationStructure;
    class DirectX12TopLevelAccelerationStructure;

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
    // Builder declarations.
    class DirectX12VertexBufferLayoutBuilder;
    class DirectX12DescriptorSetLayoutBuilder;
    class DirectX12PushConstantsLayoutBuilder;
    class DirectX12PipelineLayoutBuilder;
    class DirectX12ShaderProgramBuilder;
    class DirectX12InputAssemblerBuilder;
    class DirectX12RasterizerBuilder;
    class DirectX12RenderPipelineBuilder;
    class DirectX12ComputePipelineBuilder;
    class DirectX12RayTracingPipelineBuilder;
    class DirectX12RenderPassBuilder;
    class DirectX12BarrierBuilder;
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)

    /// @brief A resource that is hold by a `ComPtr`.
    ///
    /// @tparam THandle The type of the resource interface.
    template <class THandle>
    using ComResource = Resource<ComPtr<THandle>>;
    // TODO: We could overwrite the handle() methods here and return `.AsWeak` from the ComPtr and create an overload to get a reference-counted handle, if required.

    /// @brief Contains conversion helpers for DirectX 12.
    namespace DX12
    {
        Format LITEFX_DIRECTX12_API getFormat(const DXGI_FORMAT& format);

        DXGI_FORMAT LITEFX_DIRECTX12_API getFormat(Format format);

        DXGI_FORMAT LITEFX_DIRECTX12_API getFormat(BufferFormat format);

        bool LITEFX_DIRECTX12_API isSRGB(Format format);

        D3D12_RESOURCE_DIMENSION LITEFX_DIRECTX12_API getImageType(ImageDimensions dimensions);

        PolygonMode LITEFX_DIRECTX12_API getPolygonMode(const D3D12_FILL_MODE& mode);

        D3D12_FILL_MODE LITEFX_DIRECTX12_API getPolygonMode(PolygonMode mode);

        CullMode LITEFX_DIRECTX12_API getCullMode(const D3D12_CULL_MODE& mode);

        D3D12_CULL_MODE LITEFX_DIRECTX12_API getCullMode(CullMode mode);

        PrimitiveTopology LITEFX_DIRECTX12_API getPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY& topology);

        UInt32 LITEFX_DIRECTX12_API getPatchListControlPoints(const D3D12_PRIMITIVE_TOPOLOGY& topology) noexcept;

        D3D12_PRIMITIVE_TOPOLOGY LITEFX_DIRECTX12_API getPrimitiveTopology(PrimitiveTopology topology, UInt32 controlPoints = 0);

        D3D12_PRIMITIVE_TOPOLOGY_TYPE LITEFX_DIRECTX12_API getPrimitiveTopologyType(PrimitiveTopology topology);

        LPCTSTR LITEFX_DIRECTX12_API getSemanticName(AttributeSemantic semantic);

        /// @param vendorId
        /// @return
        String LITEFX_DIRECTX12_API getVendorName(UInt32 vendorId);

        D3D12_COMPARISON_FUNC LITEFX_DIRECTX12_API getCompareOp(CompareOperation compareOp);

        D3D12_STENCIL_OP LITEFX_DIRECTX12_API getStencilOp(StencilOperation stencilOp);

        D3D12_BLEND LITEFX_DIRECTX12_API getBlendFactor(BlendFactor blendFactor);

        D3D12_BLEND_OP LITEFX_DIRECTX12_API getBlendOperation(BlendOperation blendOperation);

        D3D12_BARRIER_SYNC LITEFX_DIRECTX12_API getPipelineStage(PipelineStage pipelineStage);

        D3D12_BARRIER_ACCESS LITEFX_DIRECTX12_API getResourceAccess(ResourceAccess resourceAccess);

        D3D12_BARRIER_LAYOUT LITEFX_DIRECTX12_API getImageLayout(ImageLayout imageLayout);
    }

    /// @brief Implements a DirectX12 @ref IGraphicsAdapter.
    class LITEFX_DIRECTX12_API DirectX12GraphicsAdapter final : public IGraphicsAdapter, public ComResource<IDXGIAdapter4> {
        LITEFX_IMPLEMENTATION(DirectX12GraphicsAdapterImpl);
        friend struct SharedObject::Allocator<DirectX12GraphicsAdapter>;

    private:
        /// @brief Initializes a new DirectX12 graphics adapter.
        ///
        /// @param adapter The DXGI adapter interface pointer.
        explicit DirectX12GraphicsAdapter(ComPtr<IDXGIAdapter4> adapter);

        /// @copydoc IGraphicsAdapter::IGraphicsAdapter(IGraphicsAdapter&&)
        DirectX12GraphicsAdapter(DirectX12GraphicsAdapter&&) noexcept = delete;

        /// @copydoc IGraphicsAdapter::IGraphicsAdapter(const IGraphicsAdapter&)
        DirectX12GraphicsAdapter(const DirectX12GraphicsAdapter&) = delete;

        /// @copydoc IGraphicsAdapter::operator=(IGraphicsAdapter&&)
        DirectX12GraphicsAdapter& operator=(DirectX12GraphicsAdapter&&) noexcept = delete;

        /// @copydoc IGraphicsAdapter::operator=(const IGraphicsAdapter&)
        DirectX12GraphicsAdapter& operator=(const DirectX12GraphicsAdapter&) = delete;

    public:
        /// @copydoc IGraphicsAdapter::~IGraphicsAdapter
        ~DirectX12GraphicsAdapter() noexcept override;

    public:
        /// @brief Creates a new DirectX12 graphics adapter.
        ///
        /// @param adapter The DXGI adapter interface pointer.
        /// @return A shared pointer to the graphics adapter instance.
        static inline auto create(ComPtr<IDXGIAdapter4> adapter) {
            return SharedObject::create<DirectX12GraphicsAdapter>(std::move(adapter));
        }

    public:
        /// @copydoc IGraphicsAdapter::name
        String name() const override;

        /// @copydoc IGraphicsAdapter::uniqueId
        UInt64 uniqueId() const noexcept override;

        /// @copydoc IGraphicsAdapter::vendorId
        UInt32 vendorId() const noexcept override;

        /// @copydoc IGraphicsAdapter::deviceId
        UInt32 deviceId() const noexcept override;

        /// @copydoc IGraphicsAdapter::type
        GraphicsAdapterType type() const noexcept override;

        /// @copydoc IGraphicsAdapter::driverVersion
        UInt64 driverVersion() const noexcept override;

        /// @copydoc IGraphicsAdapter::apiVersion
        UInt32 apiVersion() const noexcept override;

        /// @copydoc IGraphicsAdapter::dedicatedMemory
        UInt64 dedicatedMemory() const noexcept override;
    };

    /// @brief Implements a DirectX12 @ref ISurface.
    class LITEFX_DIRECTX12_API DirectX12Surface final : public ISurface, public Resource<HWND> {
    public:
        /// @brief Initializes a new DirectX 12 surface.
        ///
        /// @param hwnd The window handle.
        explicit DirectX12Surface(const HWND& hwnd) noexcept;

        /// @copydoc ISurface::ISurface(const ISurface&)
        DirectX12Surface(const DirectX12Surface&) = delete;
        
        /// @copydoc ISurface::ISurface(ISurface&&)
        DirectX12Surface(DirectX12Surface&&) noexcept = default;
        
        /// @copydoc ISurface::operator=(const ISurface&)
        DirectX12Surface& operator=(const DirectX12Surface&) = delete;
        
        /// @copydoc ISurface::operator=(ISurface&&)
        DirectX12Surface& operator=(DirectX12Surface&&) noexcept = default;

        /// @copydoc ISurface::~ISurface
        ~DirectX12Surface() noexcept override;
    };

    /// @brief An exception that is thrown, if a requested D3D12 operation could not be executed.
    class LITEFX_DIRECTX12_API DX12PlatformException : public RuntimeException {
    private:
        HRESULT m_code;
        _com_error m_error;

    public:
        /// @brief Initializes a new exception.
        ///
        /// @param result The error code returned by the operation.
        explicit DX12PlatformException(HRESULT result) :
            RuntimeException("{1} (HRESULT 0x{0:08X})", static_cast<unsigned>(result), _com_error(result).ErrorMessage()), m_code(result), m_error(result) { }

        /// @brief Initializes a new exception.
        ///
        /// @param result The error code returned by the operation.
        /// @param message The error message.
        explicit DX12PlatformException(HRESULT result, StringView message) :
            RuntimeException("{2} {1} (HRESULT 0x{0:08X})", static_cast<unsigned>(result), _com_error(result).ErrorMessage(), message), m_code(result), m_error(result) { }

        /// @brief Initializes a new exception.
        ///
        /// @param format The format string for the error message.
        /// @param result The error code returned by the operation.
        /// @param args The arguments passed to the error message format string.
        template <typename ...TArgs>
        explicit DX12PlatformException(HRESULT result, std::format_string<TArgs...> format, TArgs&&... args) :
            DX12PlatformException(result, std::format(format, std::forward<TArgs>(args)...)) { }

        /// @copydoc RuntimeException::RuntimeException(RuntimeException&&)
        DX12PlatformException(DX12PlatformException&&) noexcept = default;

        /// @copydoc RuntimeException::RuntimeException(const RuntimeException&)
        DX12PlatformException(const DX12PlatformException&) = default;

        /// @copydoc RuntimeException::operator=(RuntimeException&&)
        DX12PlatformException& operator=(DX12PlatformException&&) noexcept = default;
        
        /// @copydoc RuntimeException::operator=(const RuntimeException&)
        DX12PlatformException& operator=(const DX12PlatformException&) = default;
        
        /// @copydoc RuntimeException::~RuntimeException
        ~DX12PlatformException() noexcept override = default;

    public:
        /// @brief Returns the error object that contains details about the error.
        ///
        /// @return An error object that contains details about the error.
        const _com_error& error() const noexcept {
            return m_error;
        }

        /// @brief Returns the error code.
        ///
        /// @return The code of the error.
        HRESULT code() const noexcept {
            return m_code;
        }
    };

    /// @brief Raises a @ref DirectX12PlatformException, if @p hr does not equal `S_OK`.
    ///
    /// @param hr The error code returned by the operation.
    /// @param message The format string for the error message.
    /// @param args The arguments passed to the error message format string.
    template <typename ...TArgs>
    static inline void raiseIfFailed(HRESULT hr, std::format_string<TArgs...> message, TArgs&&... args) {
        if (SUCCEEDED(hr)) [[likely]]
            return;
        else
            throw DX12PlatformException(hr, message, std::forward<TArgs>(args)...);
    }
}