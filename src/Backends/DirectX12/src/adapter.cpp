#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12GraphicsAdapter::DirectX12GraphicsAdapterImpl : public Implement<DirectX12GraphicsAdapter> {
public:
    friend class DirectX12GraphicsAdapter;

public:
    DirectX12GraphicsAdapterImpl(DirectX12GraphicsAdapter* parent) : 
        base(parent) { }

public:
    [[nodiscard]]
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
    ComResource<IDXGIAdapter4>(adapter), m_impl(makePimpl<DirectX12GraphicsAdapterImpl>(this))
{
}

DirectX12GraphicsAdapter::~DirectX12GraphicsAdapter() noexcept = default;

String DirectX12GraphicsAdapter::name() const noexcept
{
    auto properties = m_impl->getProperties();
    return Narrow(WString(properties.Description));
}

UInt64 DirectX12GraphicsAdapter::uniqueId() const noexcept
{
    auto properties = m_impl->getProperties();
    return (static_cast<UInt64>(properties.AdapterLuid.HighPart) << 0x20) | static_cast<UInt64>(properties.AdapterLuid.LowPart);
}

UInt32 DirectX12GraphicsAdapter::vendorId() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.VendorId;
}

UInt32 DirectX12GraphicsAdapter::deviceId() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.DeviceId;
}

GraphicsAdapterType DirectX12GraphicsAdapter::type() const noexcept
{
    auto properties = m_impl->getProperties();
    return LITEFX_FLAG_IS_SET(properties.Flags, DXGI_ADAPTER_FLAG3_SOFTWARE) ? GraphicsAdapterType::CPU : GraphicsAdapterType::GPU;
}

UInt32 DirectX12GraphicsAdapter::driverVersion() const noexcept
{
    return 0;
}

UInt32 DirectX12GraphicsAdapter::apiVersion() const noexcept
{
    return D3D12_SDK_VERSION;
}

UInt64 DirectX12GraphicsAdapter::dedicatedMemory() const noexcept
{
    auto properties = m_impl->getProperties();
    return properties.DedicatedVideoMemory;
}