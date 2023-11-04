#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12Backend::DirectX12BackendImpl : public Implement<DirectX12Backend> {
public:
    friend class DirectX12Backend;

private:
    Array<UniquePtr<DirectX12GraphicsAdapter>> m_adapters{ };
    Dictionary<String, UniquePtr<DirectX12Device>> m_devices;
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

    void loadAdapters(bool enableWarp = false)
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
                if (adapterDecriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
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

DirectX12Backend::DirectX12Backend(const App& app, bool useAdvancedSoftwareRasterizer) :
    m_impl(makePimpl<DirectX12BackendImpl>(this, app)), ComResource<IDXGIFactory7>(nullptr)
{
    this->handle() = m_impl->initialize();
    m_impl->loadAdapters(useAdvancedSoftwareRasterizer);
}

DirectX12Backend::~DirectX12Backend() noexcept = default;

BackendType DirectX12Backend::type() const noexcept
{
    return BackendType::Rendering;
}

String DirectX12Backend::name() const noexcept
{
    return "DirectX 12";
}

void DirectX12Backend::activate()
{
    this->state() = BackendState::Active;
}

void DirectX12Backend::deactivate()
{
    this->state() = BackendState::Inactive;
}

Enumerable<const DirectX12GraphicsAdapter*> DirectX12Backend::listAdapters() const
{
    return m_impl->m_adapters | std::views::transform([](const UniquePtr<DirectX12GraphicsAdapter>& adapter) { return adapter.get(); });
}

const DirectX12GraphicsAdapter* DirectX12Backend::findAdapter(const Optional<UInt64>& adapterId) const
{
    if (auto match = std::ranges::find_if(m_impl->m_adapters, [&adapterId](const auto& adapter) { return !adapterId.has_value() || adapter->uniqueId() == adapterId; }); match != m_impl->m_adapters.end()) [[likely]]
        return match->get();

    return nullptr;
}

void DirectX12Backend::registerDevice(String name, UniquePtr<DirectX12Device>&& device)
{
    if (m_impl->m_devices.contains(name)) [[unlikely]]
        throw InvalidArgumentException("The backend already contains a device with the name \"{0}\".", name);

#ifndef NDEBUG
    std::as_const(*device).handle()->SetName(Widen(name).c_str());
#endif

    m_impl->m_devices.insert(std::make_pair(name, std::move(device)));
}

void DirectX12Backend::releaseDevice(const String& name)
{
    if (!m_impl->m_devices.contains(name)) [[unlikely]]
        return;

    auto device = m_impl->m_devices[name].get();
    device->wait();

    m_impl->m_devices.erase(name);
}

DirectX12Device* DirectX12Backend::device(const String& name) noexcept
{
    if (!m_impl->m_devices.contains(name)) [[unlikely]]
        return nullptr;

    return m_impl->m_devices[name].get();
}

const DirectX12Device* DirectX12Backend::device(const String& name) const noexcept
{
    if (!m_impl->m_devices.contains(name)) [[unlikely]]
        return nullptr;

    return m_impl->m_devices[name].get();
}

UniquePtr<DirectX12Surface> DirectX12Backend::createSurface(const HWND& hwnd) const
{
    return makeUnique<DirectX12Surface>(hwnd);
}

void DirectX12Backend::enableAdvancedSoftwareRasterizer(bool enable)
{
    m_impl->loadAdapters(enable);
}