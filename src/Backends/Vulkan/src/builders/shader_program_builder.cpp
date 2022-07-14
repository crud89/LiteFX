#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

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

void VulkanGraphicsShaderProgramBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::withShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<VulkanShaderModule>(*this->instance()->getDevice(), type, fileName, entryPoint));
    return *this;
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::withVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::withTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::withTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::withGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

VulkanGraphicsShaderProgramBuilder& VulkanGraphicsShaderProgramBuilder::withFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Fragment, fileName, entryPoint);
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

void VulkanComputeShaderProgramBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);
}

VulkanComputeShaderProgramBuilder& VulkanComputeShaderProgramBuilder::withShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<VulkanShaderModule>(*this->instance()->getDevice(), type, fileName, entryPoint));
    return *this;
}

VulkanComputeShaderProgramBuilder& VulkanComputeShaderProgramBuilder::withComputeShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Compute, fileName, entryPoint);
}