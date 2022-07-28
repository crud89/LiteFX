#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

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

VulkanShaderProgram::VulkanShaderProgram(Array<UniquePtr<VulkanShaderModule>>&& modules) :
    m_impl(makePimpl<VulkanShaderProgramImpl>(this, std::move(modules)))
{
}

VulkanShaderProgram::VulkanShaderProgram() noexcept :
    m_impl(makePimpl<VulkanShaderProgramImpl>(this))
{
}

VulkanShaderProgram::~VulkanShaderProgram() noexcept = default;

Array<const VulkanShaderModule*> VulkanShaderProgram::modules() const noexcept
{
    return m_impl->m_modules |
        std::views::transform([](const UniquePtr<VulkanShaderModule>& shader) { return shader.get(); }) |
        ranges::to<Array<const VulkanShaderModule*>>();
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Graphics shader program builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanShaderProgramBuilder::VulkanShaderProgramBuilderImpl : public Implement<VulkanShaderProgramBuilder> {
public:
    friend class VulkanShaderProgramBuilder;

private:
    Array<UniquePtr<VulkanShaderModule>> m_modules;
    const VulkanDevice& m_device;

public:
    VulkanShaderProgramBuilderImpl(VulkanShaderProgramBuilder* parent, const VulkanDevice& device) :
        base(parent), m_device(device)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Graphics shader program builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgramBuilder::VulkanShaderProgramBuilder(const VulkanDevice& device) :
    m_impl(makePimpl<VulkanShaderProgramBuilderImpl>(this, device)), ShaderProgramBuilder(SharedPtr<VulkanShaderProgram>(new VulkanShaderProgram()))
{
}

VulkanShaderProgramBuilder::~VulkanShaderProgramBuilder() noexcept = default;

void VulkanShaderProgramBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<VulkanShaderModule>(m_impl->m_device, type, fileName, entryPoint));
    return *this;
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Fragment, fileName, entryPoint);
}

VulkanShaderProgramBuilder& VulkanShaderProgramBuilder::withComputeShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Compute, fileName, entryPoint);
}
#endif // defined(BUILD_DEFINE_BUILDERS)