#pragma once

#include <litefx/rendering.hpp>
#include "vulkan_api.hpp"

namespace LiteFX::Rendering::Backends {
    using namespace LiteFX::Rendering;

    class LITEFX_VULKAN_API VulkanGraphicsDeviceBuilder : public GraphicsDeviceBuilder {
    public:
        explicit VulkanGraphicsDeviceBuilder(const IRenderBackend* backend);
        VulkanGraphicsDeviceBuilder(const VulkanGraphicsDeviceBuilder&&) = delete;
        VulkanGraphicsDeviceBuilder(VulkanGraphicsDeviceBuilder&) = delete;
        virtual ~VulkanGraphicsDeviceBuilder() noexcept = default;

    public:
        virtual UniquePtr<IGraphicsDevice> go() override;
    };

}