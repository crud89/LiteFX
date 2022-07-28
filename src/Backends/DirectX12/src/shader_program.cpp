#include <litefx/backends/dx12.hpp>
#include <litefx/backends/dx12_builders.hpp>

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
    DirectX12ShaderProgramImpl(DirectX12ShaderProgram* parent, Array<UniquePtr<DirectX12ShaderModule>>&& modules) :
        base(parent), m_modules(std::move(modules))
    {
    }

    DirectX12ShaderProgramImpl(DirectX12ShaderProgram* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgram::DirectX12ShaderProgram(Array<UniquePtr<DirectX12ShaderModule>>&& modules) noexcept :
    m_impl(makePimpl<DirectX12ShaderProgramImpl>(this, std::move(modules)))
{
}

DirectX12ShaderProgram::DirectX12ShaderProgram() noexcept :
    m_impl(makePimpl<DirectX12ShaderProgramImpl>(this))
{
}

DirectX12ShaderProgram::~DirectX12ShaderProgram() noexcept = default;

Array<const DirectX12ShaderModule*> DirectX12ShaderProgram::modules() const noexcept
{
    return m_impl->m_modules |
        std::views::transform([](const UniquePtr<DirectX12ShaderModule>& shader) { return shader.get(); }) |
        ranges::to<Array<const DirectX12ShaderModule*>>();
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Shader program builder implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12ShaderProgramBuilder::DirectX12ShaderProgramBuilderImpl : public Implement<DirectX12ShaderProgramBuilder> {
public:
    friend class DirectX12ShaderProgramBuilder;

private:
    Array<UniquePtr<DirectX12ShaderModule>> m_modules;
    const DirectX12Device& m_device;

public:
    DirectX12ShaderProgramBuilderImpl(DirectX12ShaderProgramBuilder* parent, const DirectX12Device& device) :
        base(parent), m_device(device)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shader program builder shared interface.
// ------------------------------------------------------------------------------------------------

DirectX12ShaderProgramBuilder::DirectX12ShaderProgramBuilder(const DirectX12Device& device) :
    m_impl(makePimpl<DirectX12ShaderProgramBuilderImpl>(this, device)), ShaderProgramBuilder(UniquePtr<DirectX12ShaderProgram>(new DirectX12ShaderProgram()))
{
}

DirectX12ShaderProgramBuilder::~DirectX12ShaderProgramBuilder() noexcept = default;

void DirectX12ShaderProgramBuilder::build()
{
    auto instance = this->instance();
    instance->m_impl->m_modules = std::move(m_impl->m_modules);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withShaderModule(const ShaderStage& type, const String& fileName, const String& entryPoint)
{
    m_impl->m_modules.push_back(makeUnique<DirectX12ShaderModule>(m_impl->m_device, type, fileName, entryPoint));
    return *this;
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withVertexShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Vertex, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withTessellationControlShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withGeometryShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Geometry, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withFragmentShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Fragment, fileName, entryPoint);
}

DirectX12ShaderProgramBuilder& DirectX12ShaderProgramBuilder::withComputeShaderModule(const String& fileName, const String& entryPoint)
{
    return this->withShaderModule(ShaderStage::Compute, fileName, entryPoint);
}
#endif // defined(BUILD_DEFINE_BUILDERS)