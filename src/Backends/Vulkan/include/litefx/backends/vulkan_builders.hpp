#pragma once

#include <litefx/rendering.hpp>
#include "vulkan_api.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Rendering;

	class LITEFX_VULKAN_API VulkanBackendInitializer : public BackendInitializer<VulkanBackend> {
		LITEFX_IMPLEMENTATION(VulkanBackendInitializerImpl)

	public:
		VulkanBackendInitializer(builder_type& parent, UniquePtr<backend_type>&& instance) noexcept;
		virtual ~VulkanBackendInitializer() noexcept;

	public:
		virtual AppBuilder& go() override;

	public:
		VulkanBackendInitializer& withSurface(UniquePtr<ISurface>&& surface);
		VulkanBackendInitializer& withSurface(VulkanSurface::surface_callback callback);
		VulkanBackendInitializer& withAdapter(const UInt32& adapterId);
		VulkanBackendInitializer& withAdapterOrDefault(const Optional<UInt32>& adapterId = std::nullopt);
		VulkanBackendInitializer& useDeviceFormat(const Format& format);
	};

}