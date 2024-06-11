#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class RenderPassDependency::RenderPassDependencyImpl : public Implement<RenderPassDependency> {
public:
    friend class RenderPassDependency;

private:
    RenderTarget m_renderTarget;
    DescriptorBindingPoint m_descriptorBinding;

public:
    RenderPassDependencyImpl(RenderPassDependency* parent, const RenderTarget& renderTarget, const DescriptorBindingPoint& descriptorBinding) :
        base(parent), m_renderTarget(renderTarget), m_descriptorBinding(descriptorBinding)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

RenderPassDependency::RenderPassDependency(const RenderTarget& renderTarget, const DescriptorBindingPoint& descriptorBinding) noexcept :
    m_impl(makePimpl<RenderPassDependencyImpl>(this, renderTarget, descriptorBinding))
{
}

RenderPassDependency::RenderPassDependency(const RenderTarget& renderTarget, UInt32 descriptorRegister, UInt32 space) noexcept :
    RenderPassDependency(renderTarget, DescriptorBindingPoint { .Register = descriptorRegister, .Space = space })
{
}

RenderPassDependency::RenderPassDependency(const RenderPassDependency& _other) noexcept :
    m_impl(makePimpl<RenderPassDependencyImpl>(this, _other.renderTarget(), _other.binding()))
{
}

RenderPassDependency::RenderPassDependency(RenderPassDependency&& _other) noexcept :
    m_impl(makePimpl<RenderPassDependencyImpl>(this, std::move(_other.m_impl->m_renderTarget), std::move(_other.m_impl->m_descriptorBinding)))
{
}

RenderPassDependency::~RenderPassDependency() noexcept = default;

RenderPassDependency& RenderPassDependency::operator=(const RenderPassDependency& _other) noexcept
{
    m_impl->m_renderTarget = _other.renderTarget();
    m_impl->m_descriptorBinding = _other.binding();
    return *this;
}

RenderPassDependency& RenderPassDependency::operator=(RenderPassDependency&& _other) noexcept
{
    m_impl->m_renderTarget = std::move(_other.m_impl->m_renderTarget);
    m_impl->m_descriptorBinding = std::move(_other.m_impl->m_descriptorBinding);
    return *this;
}

const RenderTarget& RenderPassDependency::renderTarget() const noexcept 
{
    return m_impl->m_renderTarget;
}

const DescriptorBindingPoint& RenderPassDependency::binding() const noexcept
{
    return m_impl->m_descriptorBinding;
}