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
    Array<UniquePtr<IDescriptorSetLayout>> m_layouts;

public:
    VulkanShaderProgramImpl(VulkanShaderProgram* parent) : base(parent) { }

public:
    Array<const IShaderModule*> getShaders() const noexcept
    {
        Array<const IShaderModule*> modules(m_modules.size());
        std::generate(std::begin(modules), std::end(modules), [&, i = 0]() mutable { return m_modules[i++].get(); });

        return modules;
    }

    Array<const IDescriptorSetLayout*> getLayouts() const noexcept
    {
        Array<const IDescriptorSetLayout*> layous(m_layouts.size());
        std::generate(std::begin(layous), std::end(layous), [&, i = 0]() mutable { return m_layouts[i++].get(); });

        return layous;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgram::VulkanShaderProgram(const VulkanRenderPipeline& pipeline) :
    m_impl(makePimpl<VulkanShaderProgramImpl>(this)), VulkanRuntimeObject(pipeline.getDevice())
{
}

VulkanShaderProgram::~VulkanShaderProgram() noexcept = default;

Array<const IShaderModule*> VulkanShaderProgram::getModules() const noexcept
{
    return m_impl->getShaders();
}

Array<const IDescriptorSetLayout*> VulkanShaderProgram::getLayouts() const noexcept
{
    return m_impl->getLayouts();
}

void VulkanShaderProgram::use(UniquePtr<IShaderModule>&& module)
{
    if (module == nullptr)
        throw std::invalid_argument("The shader module must be initialized.");

    m_impl->m_modules.push_back(std::move(module));
}

void VulkanShaderProgram::use(UniquePtr<IDescriptorSetLayout>&& layout)
{
    if (layout == nullptr)
        throw std::invalid_argument("The descriptor set layout must be initialized.");

    m_impl->m_layouts.push_back(std::move(layout));
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    auto device = this->instance()->getDevice();
    this->instance()->use(makeUnique<VulkanShaderModule>(device, type, fileName, entryPoint));

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

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::use(UniquePtr<IDescriptorSetLayout>&& layout)
{
    this->instance()->use(std::move(layout));
    return *this;
}