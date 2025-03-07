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

    /// <summary>
    /// Utility class that can be used to issue blit commands and generate mip maps.
    /// </summary>
    /// <remarks>
    /// This utility class can be used to generate mip maps for images. Note however, that it is more efficient to pre-compute mip maps if possible. Also note that if 
    /// you need a direct copy of a image, use a <see cref="ICommandBuffer::transfer" /> command instead.
    /// </remarks>
    /// <typeparam name="TBackend">The type of render backend that implements the blitter.</typeparam>
    template <render_backend TBackend>
    class LITEFX_GRAPHICS_API Blitter : public LiteFX::SharedObject {
        LITEFX_IMPLEMENTATION(BlitImpl);
        friend struct SharedObject::Allocator<Blitter>;

    private:
        /// <summary>
        /// Initializes a new blitter instance.
        /// </summary>
        /// <param name="device">The device to allocate resources from.</param>
        explicit Blitter(const TBackend::device_type& device);

        /// <inheritdoc />
        Blitter(const Blitter&) = delete;

        /// <inheritdoc />
        Blitter(Blitter&&) noexcept = delete;

        /// <inheritdoc />
        Blitter& operator=(const Blitter&) = delete;

        /// <inheritdoc />
        Blitter& operator=(Blitter&&) noexcept = delete;

    public:
        /// <inheritdoc />
        ~Blitter() noexcept override = default;

    public:
        /// <summary>
        /// Creates a new blitter instance.
        /// </summary>
        /// <param name="device">The device to allocate resources from.</param>
        /// <returns>A shared pointer to the newly created blitter instance.</returns>
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