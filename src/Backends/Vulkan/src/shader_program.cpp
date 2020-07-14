#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderProgram::VulkanShaderProgramImpl : public Implement<VulkanShaderProgram> {
public:
    friend class VulkanShaderProgram;

private:
    Array<UniquePtr<IShaderModule>> m_modules;

public:
    VulkanShaderProgramImpl(VulkanShaderProgram* parent) : base(parent) { }

public:
    void addShader(UniquePtr<IShaderModule>&& module)
    {
        m_modules.push_back(std::move(module));
    }

    Array<const IShaderModule*> getShaders() const noexcept
    {
        Array<const IShaderModule*> modules(m_modules.size());
        std::generate(std::begin(modules), std::end(modules), [&, i = 0]() mutable { return m_modules[i++].get(); });

        return modules;
    }

    UniquePtr<IShaderModule> removeShader(const IShaderModule* module) 
    {
        auto it = std::find_if(std::begin(m_modules), std::end(m_modules), [module](const UniquePtr<IShaderModule>& m) { return m.get() == module; });

        if (it == m_modules.end())
            return UniquePtr<IShaderModule>();
        else
        {
            auto result = std::move(*it);
            m_modules.erase(it);

            return std::move(result);
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgram::VulkanShaderProgram(const VulkanRenderPipeline& pipeline) :
    m_impl(makePimpl<VulkanShaderProgramImpl>(this)), RuntimeObject(pipeline.getDevice())
{
}

VulkanShaderProgram::~VulkanShaderProgram() noexcept = default;

Array<const IShaderModule*> VulkanShaderProgram::getModules() const noexcept
{
    return m_impl->getShaders();
}

void VulkanShaderProgram::use(UniquePtr<IShaderModule>&& module)
{
    if (module == nullptr)
        throw std::invalid_argument("The shader module must be initialized.");

    m_impl->addShader(std::move(module));
}

UniquePtr<IShaderModule> VulkanShaderProgram::remove(const IShaderModule* module)
{
    return m_impl->removeShader(module);
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint)
{
    auto device = this->instance()->getDevice();
    this->instance()->use(makeUnique<VulkanShaderModule>(device, type, fileName, entryPoint));

    return *this;
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderType::Vertex, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderType::TessellationControl, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderType::TessellationEvaluation, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderType::Geometry, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderType::Fragment, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::addComputeShaderModule(const String& fileName, const String& entryPoint)
{
    return this->addShaderModule(ShaderType::Compute, fileName, entryPoint);
}