#pragma once

#include <litefx/rendering_api.hpp>
#include <litefx/app.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;

    class LITEFX_RENDERING_API IRenderBackend : public IBackend {
    public:
        virtual ~IRenderBackend() noexcept = default;

    public:
        virtual Array<const IGraphicsAdapter*> getAdapters() const = 0;
        virtual const IGraphicsAdapter* getAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const = 0;
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