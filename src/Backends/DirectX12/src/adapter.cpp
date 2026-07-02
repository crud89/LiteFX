#include <litefx/backends/dx12.hpp>
#include <tchar.h>
#include <dxcore.h>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12GraphicsAdapter::DirectX12GraphicsAdapterImpl {
public:
    friend class DirectX12GraphicsAdapter;

private:
	DXGI_ADAPTER_DESC1 m_properties{};
	UInt64 m_driverVersion{ 0 };
	UInt32 m_apiVersion{ D3D12_SDK_VERSION };
	GraphicsAdapterType m_type{};

public:
    DirectX12GraphicsAdapterImpl(const DirectX12GraphicsAdapter& adapter) noexcept
	{
		// Store adapter properties.
		HRESULT hr = adapter.handle()->GetDesc1(&m_properties);

		if (FAILED(hr)) [[unlikely]]
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to query adapter properties (HRESULT = {})", hr);

		// Attempt to resolve driver version.
		// NOTE: This returns the UMD driver version, i.e. not the user-facing version number of the GPU driver. This is different to Vulkan, but there's unfortunately 
		//       no native way to query this version without linking to vendor APIs.
		LARGE_INTEGER umdVersion; // [Product].[Version].[SubVersion].[Revision] (each 16 bits).
		hr = adapter.handle()->CheckInterfaceSupport(__uuidof(IDXGIDevice), &umdVersion);

		if (FAILED(hr)) [[unlikely]]
			LITEFX_WARNING(DIRECTX12_LOG, "Unable to query adapter driver version (HRESULT = {})", hr);
		else
			m_driverVersion = umdVersion.QuadPart;

		// Get the graphics adapter type.
		if (m_properties.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
			m_type = GraphicsAdapterType::Software;
		else
		{
			// DXGI is quite limited in the information it provides about an adapter. To determine the GPU type, query DXCore instead.
			ComPtr<IDXCoreAdapterFactory1> adapterFactory;
			ComPtr<IDXCoreAdapter1> coreAdapter;

			if (FAILED(hr = ::DXCoreCreateAdapterFactory(IID_PPV_ARGS(&adapterFactory))))
			{
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to acquire GPU type for adapter {}: core adapter factory creation returned {}.", adapter.uniqueId(), hr);
				return;
			}

			if (FAILED(hr = adapterFactory->GetAdapterByLuid(m_properties.AdapterLuid, IID_PPV_ARGS(&coreAdapter))))
			{
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to acquire GPU type for adapter {}: core adapter query returned {}.", adapter.uniqueId(), hr);
				return;
			}

			bool integrated{}, hardware{};

			if (FAILED(hr = coreAdapter->GetProperty(DXCoreAdapterProperty::IsIntegrated, &integrated)) ||
				FAILED(hr = coreAdapter->GetProperty(DXCoreAdapterProperty::IsHardware, &hardware)))
			{
				LITEFX_WARNING(DIRECTX12_LOG, "Unable to acquire GPU type for adapter {}: core adapter property query returned {}.", adapter.uniqueId(), hr);
				return;
			}

			// NOTE: We treat all non-integrated hardware adapters as dedicated GPU and all other devices as CPU (iGPU).
			m_type = hardware && !integrated ? GraphicsAdapterType::GPU : GraphicsAdapterType::CPU;
		}
	}
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12GraphicsAdapter::DirectX12GraphicsAdapter(ComPtr<IDXGIAdapter4> adapter) :
    ComResource<IDXGIAdapter4>(std::move(adapter)), m_impl(*this)
{
}

DirectX12GraphicsAdapter::~DirectX12GraphicsAdapter() noexcept = default;

String DirectX12GraphicsAdapter::name() const
{
    return Narrow(WString(static_cast<WCHAR*>(m_impl->m_properties.Description)));
}

UInt64 DirectX12GraphicsAdapter::uniqueId() const noexcept
{
    return (static_cast<UInt64>(m_impl->m_properties.AdapterLuid.HighPart) << 0x20) | static_cast<UInt64>(m_impl->m_properties.AdapterLuid.LowPart); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
}

UInt32 DirectX12GraphicsAdapter::vendorId() const noexcept
{
    return m_impl->m_properties.VendorId;
}

UInt32 DirectX12GraphicsAdapter::deviceId() const noexcept
{
    return m_impl->m_properties.DeviceId;
}

GraphicsAdapterType DirectX12GraphicsAdapter::type() const noexcept
{
	return m_impl->m_type;
}

UInt64 DirectX12GraphicsAdapter::driverVersion() const noexcept
{
	return m_impl->m_driverVersion;
}

UInt32 DirectX12GraphicsAdapter::apiVersion() const noexcept
{
    return m_impl->m_apiVersion;
}

UInt64 DirectX12GraphicsAdapter::dedicatedMemory() const noexcept
{
    return m_impl->m_properties.DedicatedVideoMemory;
}