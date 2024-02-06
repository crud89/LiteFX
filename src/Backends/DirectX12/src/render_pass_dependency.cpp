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
    UInt32 m_location;

public:
    DirectX12RenderPassDependencyImpl(DirectX12RenderPassDependency* parent, const DirectX12RenderPass* renderPass, const RenderTarget& renderTarget, UInt32 location) :
        base(parent), m_renderPass(renderPass), m_location(location), m_renderTarget(renderTarget)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPassDependency::DirectX12RenderPassDependency() noexcept :
    m_impl(makePimpl<DirectX12RenderPassDependencyImpl>(this, nullptr, RenderTarget {}, 0))
{
}

DirectX12RenderPassDependency::DirectX12RenderPassDependency(const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget, UInt32 location) :
    m_impl(makePimpl<DirectX12RenderPassDependencyImpl>(this, &renderPass, renderTarget, location))
{
}

DirectX12RenderPassDependency::DirectX12RenderPassDependency(const DirectX12RenderPassDependency& _other) noexcept :
    m_impl(makePimpl<DirectX12RenderPassDependencyImpl>(this, _other.m_impl->m_renderPass, _other.m_impl->m_renderTarget, _other.m_impl->m_location))
{
}

DirectX12RenderPassDependency::DirectX12RenderPassDependency(DirectX12RenderPassDependency&& _other) noexcept :
    m_impl(makePimpl<DirectX12RenderPassDependencyImpl>(this, std::move(_other.m_impl->m_renderPass), std::move(_other.m_impl->m_renderTarget), std::move(_other.m_impl->m_location)))
{
}

DirectX12RenderPassDependency::~DirectX12RenderPassDependency() noexcept = default;

DirectX12RenderPassDependency& DirectX12RenderPassDependency::operator=(const DirectX12RenderPassDependency& _other) noexcept
{
    m_impl->m_renderPass = _other.m_impl->m_renderPass;
    m_impl->m_renderTarget = _other.m_impl->m_renderTarget;
    m_impl->m_location = _other.m_impl->m_location;

    return *this;
}

DirectX12RenderPassDependency& DirectX12RenderPassDependency::operator=(DirectX12RenderPassDependency&& _other) noexcept
{
    m_impl->m_renderPass = std::move(_other.m_impl->m_renderPass);
    m_impl->m_renderTarget = std::move(_other.m_impl->m_renderTarget);
    m_impl->m_location = std::move(_other.m_impl->m_location);

    return *this;
}

const DirectX12RenderPass* DirectX12RenderPassDependency::inputAttachmentSource() const noexcept
{
    return m_impl->m_renderPass;
}

UInt32 DirectX12RenderPassDependency::location() const noexcept
{
    return m_impl->m_location;
}

const RenderTarget& DirectX12RenderPassDependency::renderTarget() const noexcept
{
    return m_impl->m_renderTarget;
}