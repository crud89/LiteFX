#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ShaderProgram::DirectX12ShaderProgramImpl : public Implement<DirectX12ShaderProgram> {
public:
    friend class DirectX12ShaderProgramBuilder;
    friend class DirectX12ShaderProgram;

private:
    Array<UniquePtr<DirectX12ShaderModule>> m_modules;

public:
    DirectX12ShaderProgramImpl(DirectX12ShaderProgram* parent) : 
        base(parent) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgram::DirectX12ShaderProgram() noexcept :
    m_impl(makePimpl<DirectX12ShaderProgramImpl>(this))
{
}

DirectX12ShaderProgram::~DirectX12ShaderProgram() noexcept = default;

Array<const DirectX12ShaderModule*> DirectX12ShaderProgram::modules() const noexcept
{
    return m_impl->m_modules |
        std::views::transform([](const UniquePtr<DirectX12ShaderModule>& shader) { return shader.get(); }) |
        ranges::to<Array<const DirectX12ShaderModule*>>();
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Graphics shader program builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12GraphicsShaderProgramBuilder::DirectX12GraphicsShaderProgramBuilderImpl : public Implement<DirectX12GraphicsShaderProgramBuilder> {
public:
    friend class DirectX12GraphicsShaderProgramBuilder;

private:
    Array<UniquePtr<DirectX12ShaderModule>> m_modules;

public:
    DirectX12GraphicsShaderProgramBuilderImpl(DirectX12GraphicsShaderProgramBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Graphics shader program builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12GraphicsShaderProgramBuilder::DirectX12GraphicsShaderProgramBuilder(DirectX12RenderPipelineLayoutBuilder& parent) :
    m_impl(makePimpl<DirectX12GraphicsShaderProgramBuilderImpl>(this)), GraphicsShaderProgramBuilder(parent, UniquePtr<DirectX12ShaderProgram>(new DirectX12ShaderProgram(*std::as_const(parent).instance())))
{
}

DirectX12GraphicsShaderProgramBuilder::~DirectX12GraphicsShaderProgramBuilder() noexcept = default;

DirectX12RenderPipelineLayoutBuilder& DirectX12GraphicsShaderProgramBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);

    return GraphicsShaderProgramBuilder::go();
}

DirectX12GraphicsShaderProgramBuilder& DirectX12GraphicsShaderProgramBuilder::addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<DirectX12ShaderModule>(*this->instance()->getDevice(), type, fileName, entryPoint));
    return *this;
}

DirectX12GraphicsShaderProgramBuilder& DirectX12GraphicsShaderProgramBuilder::addVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

DirectX12GraphicsShaderProgramBuilder& DirectX12GraphicsShaderProgramBuilder::addTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

DirectX12GraphicsShaderProgramBuilder& DirectX12GraphicsShaderProgramBuilder::addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

DirectX12GraphicsShaderProgramBuilder& DirectX12GraphicsShaderProgramBuilder::addGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

DirectX12GraphicsShaderProgramBuilder& DirectX12GraphicsShaderProgramBuilder::addFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Fragment, fileName, entryPoint);
}
#endif // defined(BUILD_DEFINE_BUILDERS)