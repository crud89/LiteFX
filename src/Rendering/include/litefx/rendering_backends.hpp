#pragma once

#include <litefx/rendering_api.hpp>
#include <litefx/app.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;

    class LITEFX_RENDERING_API GraphicsDeviceBuilder : public IBuilder<IGraphicsDevice> {
        LITEFX_IMPLEMENTATION(GraphicsDeviceBuilderImpl)

    public:
        explicit GraphicsDeviceBuilder(const IRenderBackend* backend);
        GraphicsDeviceBuilder(const GraphicsDeviceBuilder&&) = delete;
        GraphicsDeviceBuilder(GraphicsDeviceBuilder&) = delete;
        virtual ~GraphicsDeviceBuilder() noexcept;

    public:
        virtual const IRenderBackend* getBackend() const noexcept;
        virtual const ISurface* getSurface() const noexcept;
        virtual const IGraphicsAdapter* getAdapter() const noexcept;

    public:
        virtual GraphicsDeviceBuilder& forSurface(const ISurface* surface);
        virtual GraphicsDeviceBuilder& useAdapter(const uint32_t& adapter);
        virtual GraphicsDeviceBuilder& useAdapterOrDefault(const Optional<uint32_t>& adapterId = std::nullopt);
    };

    class LITEFX_RENDERING_API IRenderBackend : public IBackend {
    public:
        virtual ~IRenderBackend() noexcept = default;

    public:
        virtual Array<const IGraphicsAdapter*> getAdapters() const = 0;
        virtual const IGraphicsAdapter* getAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const = 0;

    public:
        //virtual GraphicsDeviceBuilder& buildDevice() const = 0;
    };

    class LITEFX_RENDERING_API RenderBackend : public IRenderBackend {
        LITEFX_IMPLEMENTATION(RenderBackendImpl)

    public:
        explicit RenderBackend(const App& app) noexcept;
        RenderBackend(const RenderBackend&) noexcept = delete;
        RenderBackend(RenderBackend&&) noexcept = delete;
        virtual ~RenderBackend() noexcept;

    public:
        virtual BackendType getType() const noexcept override;
        const App& getApp() const noexcept;
    };

}