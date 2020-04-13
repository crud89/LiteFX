#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

//class RenderPipelineBuilder::RenderPipelineBuilderImpl {
//private:
//    Array<UniquePtr<IShaderModule>> m_modules;
//    const IGraphicsDevice* m_device;
//    String m_name;
//
//public:
//    RenderPipelineBuilderImpl(const IGraphicsDevice* device, const String& name) noexcept :
//        m_device(device), m_name(name) { }
//
//public:
//    const IGraphicsDevice* getDevice() const noexcept
//    {
//        return m_device;
//    }
//
//    const String& getName() const noexcept
//    {
//        return m_name;
//    }
//
//    Array<UniquePtr<IShaderModule>> shaderModules() noexcept
//    {
//        return std::move(m_modules);
//    }
//
//    void addShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint)
//    {
//        return;
//    }
//
//    void addVertexShaderModule(const String& fileName, const String& entryPoint)
//    {
//        return;
//    }
//
//    void addTessellationControlShaderModule(const String& fileName, const String& entryPoint)
//    {
//        return;
//    }
//
//    void addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
//    {
//        return;
//    }
//
//    void addGeometryShaderModule(const String& fileName, const String& entryPoint)
//    {
//        return;
//    }
//
//    void addFragmentShaderModule(const String& fileName, const String& entryPoint)
//    {
//        return;
//    }
//
//    void addComputeShaderModule(const String& fileName, const String& entryPoint)
//    {
//        return;
//    }
//};
//
//RenderPipelineBuilder::RenderPipelineBuilder(const IGraphicsDevice* device, const String& name) : 
//    m_impl(makePimpl<RenderPipelineBuilderImpl>(device, name))
//{
//    if (device == nullptr)
//        throw std::invalid_argument("The device must be initialized.");
//}
//
//RenderPipelineBuilder::~RenderPipelineBuilder() noexcept = default;
//
//const IGraphicsDevice* RenderPipelineBuilder::getDevice() const noexcept
//{
//    return m_impl->getDevice();
//}
//
//const String& RenderPipelineBuilder::getName() const noexcept
//{
//    return m_impl->getName();
//}
//
//Array<UniquePtr<IShaderModule>> RenderPipelineBuilder::shaderModules() noexcept
//{
//    return m_impl->shaderModules();
//}
//
//RenderPipelineBuilder& RenderPipelineBuilder::addShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint)
//{
//    return *this;
//}
//
//RenderPipelineBuilder& RenderPipelineBuilder::addVertexShaderModule(const String& fileName, const String& entryPoint)
//{
//    return *this;
//}
//
//RenderPipelineBuilder& RenderPipelineBuilder::addTessellationControlShaderModule(const String& fileName, const String& entryPoint)
//{
//    return *this;
//}
//
//RenderPipelineBuilder& RenderPipelineBuilder::addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
//{
//    return *this;
//}
//
//RenderPipelineBuilder& RenderPipelineBuilder::addGeometryShaderModule(const String& fileName, const String& entryPoint)
//{
//    return *this;
//}
//
//RenderPipelineBuilder& RenderPipelineBuilder::addFragmentShaderModule(const String& fileName, const String& entryPoint)
//{
//    return *this;
//}
//
//RenderPipelineBuilder& RenderPipelineBuilder::addComputeShaderModule(const String& fileName, const String& entryPoint)
//{
//    return *this;
//}