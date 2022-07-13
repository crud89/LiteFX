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