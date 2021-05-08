#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderProgram::VulkanShaderProgramImpl : public Implement<VulkanShaderProgram> {
public:
    friend class VulkanShaderProgramBuilder;
    friend class VulkanShaderProgram;

private:
    Array<UniquePtr<IShaderModule>> m_modules;

public:
    VulkanShaderProgramImpl(VulkanShaderProgram* parent) : 
        base(parent) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgram::VulkanShaderProgram(const VulkanRenderPipelineLayout& pipelineLayout) :
    m_impl(makePimpl<VulkanShaderProgramImpl>(this)), VulkanRuntimeObject<VulkanRenderPipelineLayout>(pipelineLayout, pipelineLayout.getDevice())
{
}

VulkanShaderProgram::~VulkanShaderProgram() noexcept = default;

Array<const IShaderModule*> VulkanShaderProgram::getModules() const noexcept
{
    Array<const IShaderModule*> modules(m_impl->m_modules.size());
    std::generate(std::begin(modules), std::end(modules), [&, i = 0]() mutable { return m_impl->m_modules[i++].get(); });

    return modules;
}

void VulkanShaderProgram::use(UniquePtr<IShaderModule>&& module)
{
    if (module == nullptr)
        throw std::invalid_argument("The shader module must be initialized.");

    m_impl->m_modules.push_back(std::move(module));
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    auto device = this->instance()->getDevice();
    this->instance()->use(makeUnique<VulkanShaderModule>(*device, type, fileName, entryPoint));

    return *this;
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Fragment, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addComputeShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Compute, fileName, entryPoint);
}