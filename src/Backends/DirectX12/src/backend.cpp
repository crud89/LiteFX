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

public:
    DirectX12BackendImpl(DirectX12Backend* parent) :
        base(parent) { }

public:
    ComPtr<IDXGIFactory7> initialize()
    {
        ComPtr<IDXGIFactory7> factory;

#ifndef NDEBUG
        HRESULT hr = ::CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
#else
        HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
#endif

        if (FAILED(hr))
            throw std::runtime_error("Unable to create DirectX 12 factory instance.");

        return factory;
    }

    void loadAdapters(const bool& enableWarp = false)
    {
        ComPtr<IDXGIAdapter1> adapterInterface;
        ComPtr<IDXGIAdapter4> adapterInstance;
        HRESULT hr;

        // Clear the current adapter set.
        m_adapters.clear();

        if (enableWarp)
        {
            if (FAILED(hr = m_parent->handle()->EnumWarpAdapter(IID_PPV_ARGS(&adapterInterface))) ||
                FAILED(hr = adapterInterface.As(&adapterInstance)))
                throw std::runtime_error("The advanced software rasterizer adapter is not a valid IDXGIAdapter4 instance.");

            // TODO: Create adapter instance and add it to the list.
        }
        else
        {
            for (UInt32 i(0); m_parent->handle()->EnumAdapters1(i, &adapterInterface) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 adapterDecriptor;
                adapterInterface->GetDesc1(&adapterDecriptor);

                // Ignore software rasterizer adapters.
                if (!LITEFX_FLAG_IS_SET(adapterDecriptor.Flags, DXGI_ADAPTER_FLAG_SOFTWARE) &&
                    FAILED(hr = adapterInterface.As(&adapterInstance)))
                    throw std::runtime_error("The hardware adapter is not a valid IDXGIAdapter4 instance.");

                // TODO: Create adapter instance and add it to the list.
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
    RenderBackend(app), m_impl(makePimpl<DirectX12BackendImpl>(this)), IComResource<IDXGIFactory7>(nullptr)
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
        throw std::invalid_argument("The adapter must be initialized.");

    m_impl->m_adapter = adapter;
}

void DirectX12Backend::use(UniquePtr<ISurface>&& surface)
{
    if (surface == nullptr)
        throw std::invalid_argument("The surface must be initialized.");

    m_impl->m_surface = std::move(surface);
}

void DirectX12Backend::enableAdvancedSoftwareRasterizer(const bool& enable)
{
    m_impl->loadAdapters(enable);
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

AppBuilder& DirectX12BackendBuilder::go()
{
    auto adapter = this->instance()->getAdapter();
    auto surface = this->instance()->getSurface();

    if (adapter == nullptr)
        throw std::runtime_error("No adapter has been defined to use for this backend.");

    if (surface == nullptr)
        throw std::runtime_error("No surface has been defined to use for this backend.");

    Logger::get(DIRECTX12_LOG).info("Creating DirectX12 rendering backend for adapter {0} ({1}).", adapter->getName(), adapter->getDeviceId());

    Logger::get(DIRECTX12_LOG).debug("--------------------------------------------------------------------------");
    Logger::get(DIRECTX12_LOG).debug("Vendor: {0:#0x}", adapter->getVendorId());
    Logger::get(DIRECTX12_LOG).debug("Driver Version: {0:#0x}", adapter->getDriverVersion());
    Logger::get(DIRECTX12_LOG).debug("API Version: {0:#0x}", adapter->getApiVersion());
    Logger::get(DIRECTX12_LOG).debug("--------------------------------------------------------------------------");

    return builder_type::go();
}

DirectX12BackendBuilder& DirectX12BackendBuilder::useAdvancedSoftwareRasterizer(const bool& enable)
{
    this->instance()->enableAdvancedSoftwareRasterizer(enable);
    return *this;
}

DirectX12BackendBuilder& DirectX12BackendBuilder::withSurface(UniquePtr<ISurface>&& surface)
{
    Logger::get(DIRECTX12_LOG).trace("Setting surface...");
    this->instance()->use(std::move(surface));
    return *this;
}

DirectX12BackendBuilder& DirectX12BackendBuilder::withAdapter(const UInt32& adapterId)
{
    auto adapter = this->instance()->findAdapter(adapterId);

    if (adapter == nullptr)
        throw std::invalid_argument("The argument `adapterId` is invalid.");

    Logger::get(DIRECTX12_LOG).trace("Using adapter id: {0}...", adapterId);
    this->instance()->use(adapter);
    return *this;
}

DirectX12BackendBuilder& DirectX12BackendBuilder::withAdapterOrDefault(const Optional<UInt32>& adapterId)
{
    auto adapter = this->instance()->findAdapter(adapterId);

    if (adapter == nullptr)
        adapter = this->instance()->findAdapter(std::nullopt);

    Logger::get(DIRECTX12_LOG).trace("Using adapter id: {0}...", adapter->getDeviceId());
    this->instance()->use(adapter);
    return *this;
}