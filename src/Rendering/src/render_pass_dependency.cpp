#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderPassDependency::RenderPassDependencyImpl {
public:
    friend class RenderPassDependency;

private:
    RenderTarget m_renderTarget;
    DescriptorBindingPoint m_descriptorBinding;

public:
    RenderPassDependencyImpl(const RenderTarget& renderTarget, const DescriptorBindingPoint& descriptorBinding) :
        m_renderTarget(renderTarget), m_descriptorBinding(descriptorBinding)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderPassDependency::RenderPassDependency(const RenderTarget& renderTarget, const DescriptorBindingPoint& descriptorBinding) noexcept :
    m_impl(renderTarget, descriptorBinding)
{
}

RenderPassDependency::RenderPassDependency(const RenderTarget& renderTarget, UInt32 descriptorRegister, UInt32 space) noexcept :
    RenderPassDependency(renderTarget, DescriptorBindingPoint { .Register = descriptorRegister, .Space = space })
{
}

RenderPassDependency::RenderPassDependency(const RenderPassDependency& _other) noexcept = default;
RenderPassDependency::RenderPassDependency(RenderPassDependency&& _other) noexcept = default;
RenderPassDependency& RenderPassDependency::operator=(const RenderPassDependency& _other) noexcept = default;
RenderPassDependency& RenderPassDependency::operator=(RenderPassDependency&& _other) noexcept = default;
RenderPassDependency::~RenderPassDependency() noexcept = default;

const RenderTarget& RenderPassDependency::renderTarget() const noexcept 
{
    return m_impl->m_renderTarget;
}

const DescriptorBindingPoint& RenderPassDependency::binding() const noexcept
{
    return m_impl->m_descriptorBinding;
}