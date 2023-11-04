#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12InputAttachmentMapping::DirectX12InputAttachmentMappingImpl : public Implement<DirectX12InputAttachmentMapping> {
public:
    friend class DirectX12InputAttachmentMapping;

private:
    const DirectX12RenderPass* m_renderPass;
    RenderTarget m_renderTarget;
    UInt32 m_location;

public:
    DirectX12InputAttachmentMappingImpl(DirectX12InputAttachmentMapping* parent, const DirectX12RenderPass* renderPass, const RenderTarget& renderTarget, UInt32 location) :
        base(parent), m_renderPass(renderPass), m_location(location), m_renderTarget(renderTarget)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12InputAttachmentMapping::DirectX12InputAttachmentMapping() noexcept :
    m_impl(makePimpl<DirectX12InputAttachmentMappingImpl>(this, nullptr, RenderTarget {}, 0))
{
}

DirectX12InputAttachmentMapping::DirectX12InputAttachmentMapping(const DirectX12RenderPass& renderPass, const RenderTarget& renderTarget, UInt32 location) :
    m_impl(makePimpl<DirectX12InputAttachmentMappingImpl>(this, &renderPass, renderTarget, location))
{
}

DirectX12InputAttachmentMapping::DirectX12InputAttachmentMapping(const DirectX12InputAttachmentMapping& _other) noexcept :
    m_impl(makePimpl<DirectX12InputAttachmentMappingImpl>(this, _other.m_impl->m_renderPass, _other.m_impl->m_renderTarget, _other.m_impl->m_location))
{
}

DirectX12InputAttachmentMapping::DirectX12InputAttachmentMapping(DirectX12InputAttachmentMapping&& _other) noexcept :
    m_impl(makePimpl<DirectX12InputAttachmentMappingImpl>(this, std::move(_other.m_impl->m_renderPass), std::move(_other.m_impl->m_renderTarget), std::move(_other.m_impl->m_location)))
{
}

DirectX12InputAttachmentMapping::~DirectX12InputAttachmentMapping() noexcept = default;

DirectX12InputAttachmentMapping& DirectX12InputAttachmentMapping::operator=(const DirectX12InputAttachmentMapping& _other) noexcept
{
    m_impl->m_renderPass = _other.m_impl->m_renderPass;
    m_impl->m_renderTarget = _other.m_impl->m_renderTarget;
    m_impl->m_location = _other.m_impl->m_location;

    return *this;
}

DirectX12InputAttachmentMapping& DirectX12InputAttachmentMapping::operator=(DirectX12InputAttachmentMapping&& _other) noexcept
{
    m_impl->m_renderPass = std::move(_other.m_impl->m_renderPass);
    m_impl->m_renderTarget = std::move(_other.m_impl->m_renderTarget);
    m_impl->m_location = std::move(_other.m_impl->m_location);

    return *this;
}

const DirectX12RenderPass* DirectX12InputAttachmentMapping::inputAttachmentSource() const noexcept
{
    return m_impl->m_renderPass;
}

UInt32 DirectX12InputAttachmentMapping::location() const noexcept
{
    return m_impl->m_location;
}

const RenderTarget& DirectX12InputAttachmentMapping::renderTarget() const noexcept
{
    return m_impl->m_renderTarget;
}