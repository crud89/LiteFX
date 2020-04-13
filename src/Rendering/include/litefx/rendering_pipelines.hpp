#pragma once

#include <litefx/rendering_api.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;

    class LITEFX_RENDERING_API IRenderPipeline {
    public:
        virtual ~IRenderPipeline() noexcept = default;
    };

    class LITEFX_RENDERING_API IRenderPipelineLayout {
    public:
        virtual ~IRenderPipelineLayout() noexcept = default;
    };

    class LITEFX_RENDERING_API IShaderModule {
    public:
        virtual ~IShaderModule() noexcept = default;

    public:
        virtual const IGraphicsDevice* getDevice() const noexcept = 0;
        virtual const ShaderType& getType() const noexcept = 0;
        virtual const String& getFileName() const noexcept = 0;
        virtual const String& getEntryPoint() const noexcept = 0;
    };

    class LITEFX_RENDERING_API IShaderProgram {
    public:
        virtual ~IShaderProgram() noexcept = default;
    };

    //class LITEFX_RENDERING_API RenderPipelineBuilder : public Builder<IRenderPipeline> {
    //    LITEFX_IMPLEMENTATION(RenderPipelineBuilderImpl)

    //public:
    //    explicit RenderPipelineBuilder(const IGraphicsDevice* device, const String& name);
    //    RenderPipelineBuilder(const RenderPipelineBuilder&&) = delete;
    //    RenderPipelineBuilder(RenderPipelineBuilder&) = delete;
    //    virtual ~RenderPipelineBuilder() noexcept;

    //public:
    //    virtual const IGraphicsDevice* getDevice() const noexcept;
    //    virtual const String& getName() const noexcept;
    //    virtual Array<UniquePtr<IShaderModule>> shaderModules() noexcept;

    //public:
    //    virtual RenderPipelineBuilder& addShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint = "main");
    //    virtual RenderPipelineBuilder& addVertexShaderModule(const String& fileName, const String& entryPoint = "main");
    //    virtual RenderPipelineBuilder& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main");
    //    virtual RenderPipelineBuilder& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main");
    //    virtual RenderPipelineBuilder& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main");
    //    virtual RenderPipelineBuilder& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main");
    //    virtual RenderPipelineBuilder& addComputeShaderModule(const String& fileName, const String& entryPoint = "main");
    //};

}