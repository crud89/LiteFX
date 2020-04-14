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
    ////    VulkanBackendBuilder& withSurface(UniquePtr<ISurface>&& surface);
    ////    VulkanBackendBuilder& withSurface(VulkanSurface::surface_callback callback);
    ////    VulkanBackendBuilder& withAdapter(const UInt32& adapterId);
    ////    VulkanBackendBuilder& withAdapterOrDefault(const Optional<UInt32>& adapterId = std::nullopt);

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
    ////    template <typename TBackend, typename ...TArgs, std::enable_if_t<rtti::has_initializer_v<TBackend>, int> = 0, typename TBuilder = TBackend::initializer>
    ////    TBuilder useBackend(TArgs&&... _args) {
    ////        return TBuilder::makeFor<TBackend, TBuilder>(*this, *this->instance(), std::forward<TArgs>(_args)...);
    ////    }

    ////    template <typename TBackend, typename ...TArgs, std::enable_if_t<!rtti::has_initializer_v<TBackend>, int> = 0, typename TBuilder = Builder<TBackend, AppBuilder>>
    ////    TBuilder useBackend(TArgs&&... _args) {
    ////        return TBuilder::makeFor<TBackend, TBuilder>(*this, *this->instance(), std::forward<TArgs>(_args)...);
    ////    }

    //};


    class LITEFX_RENDERING_API IRenderBackend : public IBackend {
    public:
        virtual ~IRenderBackend() noexcept = default;

    public:
        virtual Array<const IGraphicsAdapter*> listAdapters() const = 0;
        virtual const IGraphicsAdapter* findAdapter(const Optional<uint32_t>& adapterId = std::nullopt) const = 0;
        //virtual const ISurface* getSurface() const noexcept;
        //virtual const IGraphicsAdapter* getAdapter() const noexcept;

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