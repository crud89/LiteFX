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

DirectX12ShaderProgram::DirectX12ShaderProgram(const DirectX12PipelineLayout& pipelineLayout) noexcept :
    m_impl(makePimpl<DirectX12ShaderProgramImpl>(this)), DirectX12RuntimeObject<DirectX12PipelineLayout>(pipelineLayout, pipelineLayout.getDevice())
{
}

DirectX12ShaderProgram::~DirectX12ShaderProgram() noexcept = default;

Array<const DirectX12ShaderModule*> DirectX12ShaderProgram::modules() const noexcept
{
    return m_impl->m_modules |
        std::views::transform([](const UniquePtr<DirectX12ShaderModule>& shader) { return shader.get(); }) |
        ranges::to<Array<const DirectX12ShaderModule*>>();
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ShaderProgramBuilder::DirectX12ShaderProgramBuilderImpl : public Implement<DirectX12ShaderProgramBuilder> {
public:
    friend class DirectX12ShaderProgramBuilder;

private:
    Array<UniquePtr<DirectX12ShaderModule>> m_modules;

public:
    DirectX12ShaderProgramBuilderImpl(DirectX12ShaderProgramBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgramBuilder::DirectX12ShaderProgramBuilder(DirectX12RenderPipelineLayoutBuilder& parent) :
    m_impl(makePimpl<DirectX12ShaderProgramBuilderImpl>(this)), ShaderProgramBuilder(parent, UniquePtr<DirectX12ShaderProgram>(new DirectX12ShaderProgram(*std::as_const(parent).instance())))
{
}

DirectX12ShaderProgramBuilder::~DirectX12ShaderProgramBuilder() noexcept = default;

DirectX12RenderPipelineLayoutBuilder& DirectX12ShaderProgramBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);

    return ShaderProgramBuilder::go();
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addShaderModule(const ShaderStage & type, const String & fileName, const String & entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<DirectX12ShaderModule>(*this->instance()->getDevice(), type, fileName, entryPoint));
    return *this;
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addVertexShaderModule(const String & fileName, const String & entryPoint)
{
    return this->addShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addTessellationControlShaderModule(const String & fileName, const String & entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addTessellationEvaluationShaderModule(const String & fileName, const String & entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addGeometryShaderModule(const String & fileName, const String & entryPoint)
{
    return this->addShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addFragmentShaderModule(const String & fileName, const String & entryPoint)
{
    return this->addShaderModule(ShaderStage::Fragment, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addComputeShaderModule(const String & fileName, const String & entryPoint)
{
    return this->addShaderModule(ShaderStage::Compute, fileName, entryPoint);
}