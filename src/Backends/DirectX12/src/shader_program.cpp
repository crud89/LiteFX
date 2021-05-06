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
    Array<UniquePtr<IShaderModule>> m_modules;

public:
    DirectX12ShaderProgramImpl(DirectX12ShaderProgram* parent) : 
        base(parent) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgram::DirectX12ShaderProgram(const DirectX12RenderPipelineLayout& pipelineLayout) :
    m_impl(makePimpl<DirectX12ShaderProgramImpl>(this)), DirectX12RuntimeObject(pipelineLayout.getDevice())
{
}

DirectX12ShaderProgram::~DirectX12ShaderProgram() noexcept = default;

Array<const IShaderModule*> DirectX12ShaderProgram::getModules() const noexcept
{
    Array<const IShaderModule*> modules(m_impl->m_modules.size());
    std::generate(std::begin(modules), std::end(modules), [&, i = 0]() mutable { return m_impl->m_modules[i++].get(); });

    return modules;
}

void DirectX12ShaderProgram::use(UniquePtr<IShaderModule>&& module)
{
    if (module == nullptr)
        throw std::invalid_argument("The shader module must be initialized.");

    m_impl->m_modules.push_back(std::move(module));
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    this->instance()->use(makeUnique<DirectX12ShaderModule>(type, fileName, entryPoint));

    return *this;
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Fragment, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::addComputeShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Compute, fileName, entryPoint);
}