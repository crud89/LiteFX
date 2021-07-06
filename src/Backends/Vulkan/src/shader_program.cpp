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
    Array<UniquePtr<VulkanShaderModule>> m_modules;

public:
    VulkanShaderProgramImpl(VulkanShaderProgram* parent, Array<UniquePtr<VulkanShaderModule>>&& modules) :
        base(parent), m_modules(std::move(modules))
    {
    }

    VulkanShaderProgramImpl(VulkanShaderProgram* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgram::VulkanShaderProgram(const VulkanPipelineLayout& pipelineLayout, Array<UniquePtr<VulkanShaderModule>>&& modules) :
    m_impl(makePimpl<VulkanShaderProgramImpl>(this, std::move(modules))), VulkanRuntimeObject<VulkanPipelineLayout>(pipelineLayout, pipelineLayout.getDevice())
{
}

VulkanShaderProgram::VulkanShaderProgram(const VulkanPipelineLayout& pipelineLayout) noexcept :
    m_impl(makePimpl<VulkanShaderProgramImpl>(this)), VulkanRuntimeObject<VulkanPipelineLayout>(pipelineLayout, pipelineLayout.getDevice())
{
}

VulkanShaderProgram::~VulkanShaderProgram() noexcept = default;

Array<const VulkanShaderModule*> VulkanShaderProgram::modules() const noexcept
{
    return m_impl->m_modules |
        std::views::transform([](const UniquePtr<VulkanShaderModule>& shader) { return shader.get(); }) |
        ranges::to<Array<const VulkanShaderModule*>>();
}

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderProgramBuilder::VulkanShaderProgramBuilderImpl : public Implement<VulkanShaderProgramBuilder> {
public:
    friend class VulkanShaderProgramBuilder;

private:
    Array<UniquePtr<VulkanShaderModule>> m_modules;

public:
    VulkanShaderProgramBuilderImpl(VulkanShaderProgramBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgramBuilder::VulkanShaderProgramBuilder(VulkanPipelineLayoutBuilder& parent) :
    m_impl(makePimpl<VulkanShaderProgramBuilderImpl>(this)), ShaderProgramBuilder(parent, UniquePtr<VulkanShaderProgram>(new VulkanShaderProgram(*std::as_const(parent).instance())))
{
}

VulkanShaderProgramBuilder::~VulkanShaderProgramBuilder() noexcept = default;

VulkanPipelineLayoutBuilder& VulkanShaderProgramBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);

    return ShaderProgramBuilder::go();
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<VulkanShaderModule>(*this->instance()->getDevice(), type, fileName, entryPoint));
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