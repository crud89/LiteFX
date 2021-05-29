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

public:
    DirectX12BackendImpl(DirectX12Backend* parent) :
        base(parent) { }

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

public:
    Array<const IGraphicsAdapter*> listAdapters() const noexcept
    {
        Array<const IGraphicsAdapter*> results(m_adapters.size());
        std::generate(results.begin(), results.end(), [&, i = 0]() mutable { return m_adapters[i++].get(); });

        return results;
    }

    const IGraphicsAdapter* findAdapter(const Optional<uint32_t> adapterId) const noexcept
    {
        auto match = std::find_if(m_adapters.begin(), m_adapters.end(), [&adapterId](const UniquePtr<IGraphicsAdapter>& adapter) { return !adapterId.has_value() || adapter->getDeviceId() == adapterId; });

        if (match != m_adapters.end())
            return match->get();

        return nullptr;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12Backend::DirectX12Backend(const App& app, const bool& useAdvancedSoftwareRasterizer) :
    RenderBackend(app), m_impl(makePimpl<DirectX12BackendImpl>(this)), ComResource<IDXGIFactory7>(nullptr)
{
    this->handle() = m_impl->initialize();
    m_impl->loadAdapters(useAdvancedSoftwareRasterizer);
}

DirectX12Backend::~DirectX12Backend() noexcept
{
    m_impl.destroy();
}

Array<const IGraphicsAdapter*> DirectX12Backend::listAdapters() const
{
    return m_impl->listAdapters();
}

const IGraphicsAdapter* DirectX12Backend::findAdapter(const Optional<uint32_t>& adapterId) const
{
    return m_impl->findAdapter(adapterId);
}

const ISurface* DirectX12Backend::getSurface() const noexcept
{
    return m_impl->m_surface.get();
}

const IGraphicsAdapter* DirectX12Backend::getAdapter() const noexcept
{
    return m_impl->m_adapter;
}

void DirectX12Backend::use(const IGraphicsAdapter* adapter)
{
    if (adapter == nullptr)
        throw ArgumentNotInitializedException("The adapter must be initialized.");

    m_impl->m_adapter = adapter;
}

void DirectX12Backend::use(UniquePtr<ISurface>&& surface)
{
    if (surface == nullptr)
        throw ArgumentNotInitializedException("The surface must be initialized.");

    m_impl->m_surface = std::move(surface);
}

void DirectX12Backend::enableAdvancedSoftwareRasterizer(const bool& enable)
{
    m_impl->loadAdapters(enable);
}