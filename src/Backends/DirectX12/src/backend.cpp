#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Backend::DirectX12BackendImpl : public Implement<DirectX12Backend> {
public:
    friend class DirectX12Backend;

private:
    Array<UniquePtr<IGraphicsAdapter>> m_adapters{ };
    const IGraphicsAdapter* m_adapter{ nullptr };
    UniquePtr<ISurface> m_surface{ nullptr };
    ComPtr<ID3D12Debug> m_debugInterface;
    const App& m_app;

public:
    DirectX12BackendImpl(DirectX12Backend* parent, const App& app) :
        base(parent), m_app(app)
    { 
    }

public:
    [[nodiscard]]
    ComPtr<IDXGIFactory7> initialize()
    {
        ComPtr<IDXGIFactory7> factory;

#ifndef NDEBUG
        raiseIfFailed<RuntimeException>(::CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)), "Unable to create DirectX 12 factory instance.");
        
        if (SUCCEEDED(::D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugInterface))))
            m_debugInterface->EnableDebugLayer();
#else
        raiseIfFailed<RuntimeException>(::CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)), "Unable to create DirectX 12 factory instance.");
#endif

        return factory;
    }

    void loadAdapters(const bool& enableWarp = false)
    {
        ComPtr<IDXGIAdapter1> adapterInterface;
        ComPtr<IDXGIAdapter4> adapterInstance;

        // Clear the current adapter set.
        m_adapters.clear();

        if (enableWarp)
        {
            raiseIfFailed<RuntimeException>(m_parent->handle()->EnumWarpAdapter(IID_PPV_ARGS(&adapterInterface)), "Unable to iterate advanced software rasterizer adapters.");
            raiseIfFailed<RuntimeException>(adapterInterface.As(&adapterInstance), "The advanced software rasterizer adapter is not a valid IDXGIAdapter4 instance.");
            m_adapters.push_back(makeUnique<DirectX12GraphicsAdapter>(adapterInstance));
        }
        else
        {
            for (UInt32 i(0); m_parent->handle()->EnumAdapters1(i, &adapterInterface) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 adapterDecriptor;
                adapterInterface->GetDesc1(&adapterDecriptor);
                
                // Ignore software rasterizer adapters.
                if (LITEFX_FLAG_IS_SET(adapterDecriptor.Flags, DXGI_ADAPTER_FLAG_SOFTWARE))
                    continue;

                raiseIfFailed<RuntimeException>(adapterInterface.As(&adapterInstance), "The hardware adapter is not a valid IDXGIAdapter4 instance.");
                m_adapters.push_back(makeUnique<DirectX12GraphicsAdapter>(adapterInstance));
            }
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Backend::DirectX12Backend(const App& app, const bool& useAdvancedSoftwareRasterizer) :
    m_impl(makePimpl<DirectX12BackendImpl>(this, app)), ComResource<IDXGIFactory7>(nullptr)
{
    this->handle() = m_impl->initialize();
    m_impl->loadAdapters(useAdvancedSoftwareRasterizer);
}

DirectX12Backend::~DirectX12Backend() noexcept = default;

BackendType DirectX12Backend::getType() const noexcept
{
    return BackendType::Rendering;
}

Array<const DirectX12GraphicsAdapter*> DirectX12Backend::listAdapters() const
{
    return m_impl->m_adapters | std::views::transform([](const UniquePtr<DirectX12GraphicsAdapter>& adapter) { return adapter.get(); }) | ranges::to<Array<const DirectX12GraphicsAdapter*>>();
}

const DirectX12GraphicsAdapter* DirectX12Backend::findAdapter(const Optional<UInt32>& adapterId) const
{
    if (auto match = std::ranges::find_if(m_impl->m_adapters, [&adapterId](const auto& adapter) { return !adapterId.has_value() || adapter->getDeviceId() == adapterId; }); match != m_impl->m_adapters.end()) [[likely]]
        return match->get();

    return nullptr;
}

void DirectX12Backend::enableAdvancedSoftwareRasterizer(const bool& enable)
{
    m_impl->loadAdapters(enable);
}