#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderProgram::VulkanShaderProgramImpl : public Implement<VulkanShaderProgram> {
public:
    friend class VulkanGraphicsShaderProgramBuilder;
    friend class VulkanComputeShaderProgramBuilder;
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
// Graphics shader program builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanGraphicsShaderProgramBuilder::VulkanGraphicsShaderProgramBuilderImpl : public Implement<VulkanGraphicsShaderProgramBuilder> {
public:
    friend class VulkanGraphicsShaderProgramBuilder;

private:
    Array<UniquePtr<VulkanShaderModule>> m_modules;

public:
    VulkanGraphicsShaderProgramBuilderImpl(VulkanGraphicsShaderProgramBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Graphics shader program builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanGraphicsShaderProgramBuilder::VulkanGraphicsShaderProgramBuilder(VulkanRenderPipelineLayoutBuilder& parent) :
    m_impl(makePimpl<VulkanGraphicsShaderProgramBuilderImpl>(this)), GraphicsShaderProgramBuilder(parent, UniquePtr<VulkanShaderProgram>(new VulkanShaderProgram(*std::as_const(parent).instance())))
{
}

VulkanGraphicsShaderProgramBuilder::~VulkanGraphicsShaderProgramBuilder() noexcept = default;

VulkanRenderPipelineLayoutBuilder& VulkanGraphicsShaderProgramBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);

    return GraphicsShaderProgramBuilder::go();
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<VulkanShaderModule>(*this->instance()->getDevice(), type, fileName, entryPoint));
    return *this;
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::addVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::addTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::addGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::addFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Fragment, fileName, entryPoint);
}

// ------------------------------------------------------------------------------------------------
// Compute shader program builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanComputeShaderProgramBuilder::VulkanComputeShaderProgramBuilderImpl : public Implement<VulkanComputeShaderProgramBuilder> {
public:
    friend class VulkanComputeShaderProgramBuilder;

private:
    Array<UniquePtr<VulkanShaderModule>> m_modules;

public:
    VulkanComputeShaderProgramBuilderImpl(VulkanComputeShaderProgramBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Compute shader program builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanComputeShaderProgramBuilder::VulkanComputeShaderProgramBuilder(VulkanComputePipelineLayoutBuilder& parent) :
    m_impl(makePimpl<VulkanComputeShaderProgramBuilderImpl>(this)), ComputeShaderProgramBuilder(parent, UniquePtr<VulkanShaderProgram>(new VulkanShaderProgram(*std::as_const(parent).instance())))
{
}

VulkanComputeShaderProgramBuilder::~VulkanComputeShaderProgramBuilder() noexcept = default;

VulkanComputePipelineLayoutBuilder& VulkanComputeShaderProgramBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);

    return ComputeShaderProgramBuilder::go();
}

VulkanComputeShaderProgramBuilder& VulkanComputeShaderProgramBuilder::addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<VulkanShaderModule>(*this->instance()->getDevice(), type, fileName, entryPoint));
    return *this;
}

VulkanComputeShaderProgramBuilder& VulkanComputeShaderProgramBuilder::addComputeShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderStage::Compute, fileName, entryPoint);
}