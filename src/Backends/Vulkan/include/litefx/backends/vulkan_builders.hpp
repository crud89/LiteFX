#pragma once

#include <litefx/rendering.hpp>
#include "vulkan_api.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Rendering;

	class LITEFX_VULKAN_API VulkanDeviceBuilder : public GraphicsDeviceBuilder<VulkanDeviceBuilder, VulkanDevice> {
		LITEFX_IMPLEMENTATION(VulkanDeviceBuilderImpl)

	public:
		typedef std::function<VkSurfaceKHR(const VkInstance&)> surface_callback;

	public:
		VulkanDeviceBuilder(UniquePtr<VulkanDevice>&& instance) noexcept;
		virtual ~VulkanDeviceBuilder() noexcept;

	public:
		virtual UniquePtr<VulkanDevice> go() override;

	public:
		virtual VulkanDeviceBuilder& withFormat(const Format& format);
		virtual VulkanDeviceBuilder& withQueue(const QueueType& queueType);
	};

	class LITEFX_VULKAN_API VulkanBackendBuilder : public Builder<VulkanBackendBuilder, VulkanBackend, AppBuilder> {
	public:
		using builder_type::Builder;

	public:
		virtual AppBuilder& go() override;

	public:
		VulkanBackendBuilder& withSurface(UniquePtr<ISurface>&& surface);
		VulkanBackendBuilder& withSurface(VulkanSurface::surface_callback callback);
		VulkanBackendBuilder& withAdapter(const UInt32& adapterId);
		VulkanBackendBuilder& withAdapterOrDefault(const Optional<UInt32>& adapterId = std::nullopt);
	};

}