#pragma once

#include <litefx/rendering_api.hpp>
#include <litefx/app.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;

    //class LITEFX_RENDERING_API GraphicsDeviceBuilder : public Builder<IGraphicsDevice> {
    //    LITEFX_IMPLEMENTATION(GraphicsDeviceBuilderImpl)
    //public:
    //    using Builder<App>::Builder;

    //public:
    //    virtual GraphicsDeviceBuilder& go() override;

    ////public:
    ////    VulkanBackendInitializer& withSurface(UniquePtr<ISurface>&& surface);
    ////    VulkanBackendInitializer& withSurface(VulkanSurface::surface_callback callback);
    ////    VulkanBackendInitializer& withAdapter(const UInt32& adapterId);
    ////    VulkanBackendInitializer& withAdapterOrDefault(const Optional<UInt32>& adapterId = std::nullopt);

    ////protected:
    ////    virtual const IBackend* findBackend(const BackendType& type) const noexcept { return this->instance()->operator[](type); }

    ////public:
    ////    void use(UniquePtr<IBackend>&& backend) {
    ////        this->instance()->useBackend(std::move(backend));
    ////    }

    ////    virtual UniquePtr<App> go() override {
    ////        this->instance()->run();
    ////        return Builder<App>::go();
    ////    }

    ////public:
    ////    template <typename TBackend, typename ...TArgs, std::enable_if_t<rtti::has_initializer_v<TBackend>, int> = 0, typename TInitializer = TBackend::initializer>
    ////    TInitializer useBackend(TArgs&&... _args) {
    ////        return TInitializer::makeFor<TBackend, TInitializer>(*this, *this->instance(), std::forward<TArgs>(_args)...);
    ////    }

    ////    template <typename TBackend, typename ...TArgs, std::enable_if_t<!rtti::has_initializer_v<TBackend>, int> = 0, typename TInitializer = Initializer<TBackend, AppBuilder>>
    ////    TInitializer useBackend(TArgs&&... _args) {
    ////        return TInitializer::makeFor<TBackend, TInitializer>(*this, *this->instance(), std::forward<TArgs>(_args)...);
    ////    }

    //};


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