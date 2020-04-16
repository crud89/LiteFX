#pragma once

#include <litefx/rendering_api.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;
    using namespace LiteFX::Math;

    class LITEFX_RENDERING_API BufferAttribute {
        LITEFX_IMPLEMENTATION(BufferAttributeImpl)

    public:
        BufferAttribute() noexcept;
        BufferAttribute(const UInt32& location, const UInt32& binding, const UInt32& offset, const BufferFormat& format) noexcept;
        BufferAttribute(BufferAttribute&&) noexcept;
        BufferAttribute(const BufferAttribute&) noexcept;
        virtual ~BufferAttribute() noexcept;

    public:
        virtual const UInt32& getLocation() const noexcept;
        virtual const UInt32& getBinding() const noexcept;
        virtual const BufferFormat& getFormat() const noexcept;
        virtual const UInt32& getOffset() const noexcept;
    };

    class LITEFX_RENDERING_API BufferLayout {
        LITEFX_IMPLEMENTATION(BufferLayoutImpl)

    public:
        BufferLayout() noexcept;
        BufferLayout(BufferLayout&&) = delete;
        BufferLayout(const BufferLayout&) = delete;
        virtual ~BufferLayout() noexcept;

    public:
        virtual void add(UniquePtr<BufferAttribute>&& attribute);
        virtual void remove(const BufferAttribute* attribute);
        virtual Array<const BufferAttribute*> getAttributes() const noexcept;
    };

    class LITEFX_RENDERING_API IBuffer {
    public:
        virtual ~IBuffer() noexcept = default;

    public:
        virtual const BufferLayout* getLayout() const noexcept = 0;
    };

    template <typename T>
    class Buffer : public IBuffer {
    private:
        Array<T> m_elements;

    public:
        Buffer(Array<T>&& elements) noexcept : m_elements(std::move(_other.m_elements)) {}
        Buffer(const Buffer& _other) noexcept : m_elements(std::copy(_other.m_elements)) {}
        Buffer(Buffer&& _other) noexcept : m_elements(std::move(_other.m_elements)) {}
        virtual ~Buffer() noexcept = default;

    public:
        virtual const Array<T>& getElements() const noexcept { return m_elements; }
        virtual Array<T>& elements() noexcept = 0 { return m_elements; }
    };

    class LITEFX_RENDERING_API IRenderPipeline {
    public:
        virtual ~IRenderPipeline() noexcept = default;
    
    public:
        virtual const IRenderPipelineLayout* getLayout() const noexcept = 0;
        virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) = 0;
    };

    class LITEFX_RENDERING_API IRenderPipelineLayout {
    public:
        virtual ~IRenderPipelineLayout() noexcept = default;

    public:
        virtual Array<const IViewport*> getViewports() const noexcept = 0;
        virtual void use(UniquePtr<IViewport>&& viewport) = 0;
        virtual void remove(const IViewport* viewport) const noexcept = 0;
        virtual const IRasterizer* getRasterizer() const noexcept = 0;
        virtual void use(UniquePtr<IRasterizer>&& rasterizer) = 0;
        virtual const IShaderProgram* getProgram() const noexcept = 0;
        virtual void use(UniquePtr<IShaderProgram>&& program) = 0;
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
    
    public:
        virtual Array<const IShaderModule*> getModules() const noexcept = 0;
        virtual void use(UniquePtr<IShaderModule>&& module) = 0;
        virtual void remove(const IShaderModule* module) const noexcept = 0;
    };

    // TODO: Rename to ShaderProgramBuilder
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