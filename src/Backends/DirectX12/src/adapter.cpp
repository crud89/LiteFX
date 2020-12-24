#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12GraphicsAdapter::DirectX12GraphicsAdapterImpl : public Implement<DirectX12GraphicsAdapter> {
public:
    friend class DirectX12GraphicsAdapter;

private:
    //Array<UniquePtr<DirectX12Queue>> m_queues;

public:
    DirectX12GraphicsAdapterImpl(DirectX12GraphicsAdapter* parent) : base(parent)
    {
        this->initialize();
    }

public:
    void initialize()
    {
    }

public:

public:
    DXGI_ADAPTER_DESC1 getProperties() const noexcept
    {
        DXGI_ADAPTER_DESC1 properties;
        m_parent->handle()->GetDesc1(&properties);

        return properties;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12GraphicsAdapter::DirectX12GraphicsAdapter(ComPtr<IDXGIAdapter4> adapter) :
    IComResource<IDXGIAdapter4>(adapter), m_impl(makePimpl<DirectX12GraphicsAdapterImpl>(this))
{
}

DirectX12GraphicsAdapter::~DirectX12GraphicsAdapter() noexcept = default;

String DirectX12GraphicsAdapter::getName() const noexcept
{
    auto properties = m_impl->getProperties();
    return Narrow(WString(properties.Description));
}

uint32_t DirectX12GraphicsAdapter::getVendorId() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.VendorId;
}

uint32_t DirectX12GraphicsAdapter::getDeviceId() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.DeviceId;
}

GraphicsAdapterType DirectX12GraphicsAdapter::getType() const noexcept
{
    auto properties = m_impl->getProperties();
    return LITEFX_FLAG_IS_SET(properties.Flags, DXGI_ADAPTER_FLAG3_SOFTWARE) ? GraphicsAdapterType::CPU : GraphicsAdapterType::GPU;
}

uint32_t DirectX12GraphicsAdapter::getDriverVersion() const noexcept
{
    return 0;
}

uint32_t DirectX12GraphicsAdapter::getApiVersion() const noexcept
{
    return 0;
}

uint32_t DirectX12GraphicsAdapter::getDedicatedMemory() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.DedicatedVideoMemory;
}

ICommandQueue* DirectX12GraphicsAdapter::findQueue(const QueueType & queueType) const
{
    //return m_impl->findQueue(queueType);
    throw;
}

ICommandQueue* DirectX12GraphicsAdapter::findQueue(const QueueType & queueType, const ISurface * surface) const
{
    //auto forSurface = dynamic_cast<const DirectX12Surface*>(surface);

    //if (forSurface == nullptr)
    //    throw std::invalid_argument("The provided surface is not a valid DirectX12 surface.");

    //return m_impl->findQueue(queueType, forSurface);
    throw;
}