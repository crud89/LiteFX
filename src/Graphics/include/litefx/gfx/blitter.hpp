#pragma once

#include <litefx/graphics_api.hpp>
#include <litefx/rendering_api.hpp>

#ifdef LITEFX_BUILD_VULKAN_BACKEND
#include <litefx/backends/vulkan.hpp>
#endif // LITEFX_BUILD_VULKAN_BACKEND

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
#include <litefx/backends/dx12.hpp>
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

namespace LiteFX::Graphics {
    using namespace LiteFX;

    //class IBlitter {
    //    virtual blit(IImage, ICommandBuffer)
    //    virtual generateMipMaps(IImage, ICommandBuffer)
    //};

    /// @brief Utility class that can be used to issue blit commands and generate mip maps.
    ///
    /// This utility class can be used to generate mip maps for images. Note however, that it is more efficient to pre-compute mip maps if possible. Also note that if you need a direct copy of a image, use a
    /// @ref ICommandBuffer::transfer command instead.
    ///
    /// @tparam TBackend The type of render backend that implements the blitter.
    template <render_backend TBackend>
    class LITEFX_GRAPHICS_API Blitter : public LiteFX::SharedObject {
        LITEFX_IMPLEMENTATION(BlitImpl);
        friend struct SharedObject::Allocator<Blitter>;

    private:
        /// @brief Initializes a new blitter instance.
        ///
        /// @param device The device to allocate resources from.
        explicit Blitter(const TBackend::device_type& device);

        Blitter(const Blitter&) = delete;
        Blitter(Blitter&&) noexcept = delete;
        Blitter& operator=(const Blitter&) = delete;
        Blitter& operator=(Blitter&&) noexcept = delete;

    public:
        ~Blitter() noexcept override = default;

    public:
        /// @brief Creates a new blitter instance.
        ///
        /// @param device The device to allocate resources from.
        /// @return A shared pointer to the newly created blitter instance.
        static inline auto create(const TBackend::device_type& device) {
            return SharedObject::create<Blitter<TBackend>>(device);
        }

    public:
        //void blit(TBackend::image_type& image, TBackend::command_buffer_type& commandBuffer
        void generateMipMaps(TBackend::image_type& image, TBackend::command_buffer_type& commandBuffer) /*override*/;
    };

#ifdef LITEFX_LINK_SHARED
#ifdef LITEFX_BUILD_VULKAN_BACKEND
#ifndef LiteFX_Graphics_EXPORTS
    template class LITEFX_GRAPHICS_API Blitter<Backends::VulkanBackend>;
#endif // !LiteFX_Graphics_EXPORTS
#endif // LITEFX_BUILD_VULKAN_BACKEND

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
#ifndef LiteFX_Graphics_EXPORTS
    template class LITEFX_GRAPHICS_API Blitter<Backends::DirectX12Backend>;
#endif // !LiteFX_Graphics_EXPORTS
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND
#endif // LITEFX_LINK_SHARED

}