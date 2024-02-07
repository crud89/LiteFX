#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPassDependency::DirectX12RenderPassDependencyImpl : public Implement<DirectX12RenderPassDependency> {
public:
    friend class DirectX12RenderPassDependency;

private:
    const DirectX12RenderPass* m_renderPass;
    RenderTarget m_renderTarget;
    DescriptorBindingPoint m_bindingPoint;

public:
    DirectX12RenderPassDependencyImpl(DirectX12RenderPassDependency* parent, const DirectX12RenderPass* renderPass, const RenderTarget& renderTarget, DescriptorBindingPoint bindingPoint) :
        base(parent), m_renderPass(renderPass), m_bindingPoint(bindingPoint), m_renderTarget(renderTarget)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPassDependency::DirectX12RenderPassDependency(const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget, DescriptorBindingPoint binding) :
    m_impl(makePimpl<DirectX12RenderPassDependencyImpl>(this, &renderPass, renderTarget, binding))
{
}

DirectX12RenderPassDependency::DirectX12RenderPassDependency(const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget, UInt32 bindingRegister, UInt32 space) :
    m_impl(makePimpl<DirectX12RenderPassDependencyImpl>(this, &renderPass, renderTarget, DescriptorBindingPoint { .Register = bindingRegister, .Space = space }))
{
}

DirectX12RenderPassDependency::DirectX12RenderPassDependency(const DirectX12RenderPassDependency& _other) noexcept :
    m_impl(makePimpl<DirectX12RenderPassDependencyImpl>(this, _other.m_impl->m_renderPass, _other.m_impl->m_renderTarget, _other.m_impl->m_bindingPoint))
{
}

DirectX12RenderPassDependency::DirectX12RenderPassDependency(DirectX12RenderPassDependency&& _other) noexcept :
    m_impl(makePimpl<DirectX12RenderPassDependencyImpl>(this, std::move(_other.m_impl->m_renderPass), std::move(_other.m_impl->m_renderTarget), std::move(_other.m_impl->m_bindingPoint)))
{
}

DirectX12RenderPassDependency::~DirectX12RenderPassDependency() noexcept = default;

DirectX12RenderPassDependency& DirectX12RenderPassDependency::operator=(const DirectX12RenderPassDependency& _other) noexcept
{
    m_impl->m_renderPass = _other.m_impl->m_renderPass;
    m_impl->m_renderTarget = _other.m_impl->m_renderTarget;
    m_impl->m_bindingPoint = _other.m_impl->m_bindingPoint;

    return *this;
}

DirectX12RenderPassDependency& DirectX12RenderPassDependency::operator=(DirectX12RenderPassDependency&& _other) noexcept
{
    m_impl->m_renderPass = std::move(_other.m_impl->m_renderPass);
    m_impl->m_renderTarget = std::move(_other.m_impl->m_renderTarget);
    m_impl->m_bindingPoint = std::move(_other.m_impl->m_bindingPoint);

    return *this;
}

const DirectX12RenderPass* DirectX12RenderPassDependency::inputAttachmentSource() const noexcept
{
    return m_impl->m_renderPass;
}

const DescriptorBindingPoint& DirectX12RenderPassDependency::binding() const noexcept
{
    return m_impl->m_bindingPoint;
}

const RenderTarget& DirectX12RenderPassDependency::renderTarget() const noexcept
{
    return m_impl->m_renderTarget;
}