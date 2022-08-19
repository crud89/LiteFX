#include <litefx/backends/dx12.hpp>
#include <tchar.h>

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
	// There is no API support for this, so we need to query information directly from the registry. If something goes wrong, this would
	// indicate an invalid driver installation. In this case, we can simply return an invalid driver version.
	constexpr UInt32 INVALID_DRIVER_VERSION = std::numeric_limits<UInt32>::max();

	// Query the DX path.
	HKEY dxKey = nullptr;
	DWORD adapters = 0;
	LSTATUS status = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\DirectX"), 0, KEY_READ, &dxKey);

	if (status != ERROR_SUCCESS)
		return INVALID_DRIVER_VERSION;

	// Get the maximum sub-key length.
	DWORD subKeyLength = 0;
	status = ::RegQueryInfoKey(dxKey, nullptr, nullptr, nullptr, &adapters, &subKeyLength, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

	if (status != ERROR_SUCCESS)
		return INVALID_DRIVER_VERSION;

	// Allocate a string that contains the sub-key name.
	subKeyLength++;		// To also include terminating character.
	String subKeyName(subKeyLength, '\0');
	UInt64 driverVersion = 0;
	bool foundSubkey = false;
	LUID adapterId {};
	DWORD idSize = sizeof(UInt64);
	auto properties = m_impl->getProperties();

	// Parse each adapter individually until we found the current one.
	for (DWORD i(0); i < adapters; ++i)
	{
		subKeyLength = subKeyName.size();	// Reset, since it might have been overwritten by an earlier iteration of the loop.
		status = ::RegEnumKeyEx(dxKey, i, subKeyName.data(), &subKeyLength, nullptr, nullptr, nullptr, nullptr);

		if (status != ERROR_SUCCESS)
			continue;

		status = ::RegGetValue(dxKey, subKeyName.c_str(), _T("AdapterLuid"), RRF_RT_QWORD, nullptr, &adapterId, &idSize);

		if (status != ERROR_SUCCESS)
			continue;

		// Check if we've found the adapter.
		if (adapterId.HighPart == properties.AdapterLuid.HighPart && adapterId.LowPart == properties.AdapterLuid.LowPart)
		{
			status = ::RegGetValue(dxKey, subKeyName.c_str(), _T("DriverVersion"), RRF_RT_QWORD, nullptr, &driverVersion, &idSize);

			if (status == ERROR_SUCCESS)
				return static_cast<UInt32>(driverVersion >> 0x20);
		}
	}

	return INVALID_DRIVER_VERSION;
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