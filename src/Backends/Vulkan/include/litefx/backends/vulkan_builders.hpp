#pragma once

#include <litefx/rendering.hpp>
#include "vulkan_api.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Rendering;

	class LITEFX_VULKAN_API VulkanBackendBuilder : public BackendBuilder<VulkanBackend> {
		LITEFX_IMPLEMENTATION(VulkanBackendBuilderImpl)

	public:
		VulkanBackendBuilder(builder_type& parent, UniquePtr<backend_type>&& instance) noexcept;
		VulkanBackendBuilder(builder_type& parent, UniquePtr<backend_type>&& instance, const Optional<UInt32>& adapterId, UniquePtr<ISurface>&& surface = nullptr);
		virtual ~VulkanBackendBuilder() noexcept;

	public:
		virtual AppBuilder& go() override;

	public:
		VulkanBackendBuilder& withSurface(UniquePtr<ISurface>&& surface);
		VulkanBackendBuilder& withSurface(VulkanSurface::surface_callback callback);
		VulkanBackendBuilder& withAdapter(const UInt32& adapterId);
		VulkanBackendBuilder& withAdapterOrDefault(const Optional<UInt32>& adapterId = std::nullopt);
	};

}