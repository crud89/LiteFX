#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPassDependency::VulkanRenderPassDependencyImpl : public Implement<VulkanRenderPassDependency> {
public:
    friend class VulkanRenderPassDependency;

private:
    const VulkanRenderPass* m_renderPass;
    RenderTarget m_renderTarget;
    UInt32 m_location;

public:
    VulkanRenderPassDependencyImpl(VulkanRenderPassDependency* parent, const VulkanRenderPass* renderPass, const RenderTarget& renderTarget, UInt32 location) :
        base(parent), m_renderPass(renderPass), m_location(location), m_renderTarget(renderTarget)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPassDependency::VulkanRenderPassDependency() noexcept :
    m_impl(makePimpl<VulkanRenderPassDependencyImpl>(this, nullptr, RenderTarget {}, 0))
{
}

VulkanRenderPassDependency::VulkanRenderPassDependency(const VulkanRenderPass& renderPass, const RenderTarget& renderTarget, UInt32 location) :
    m_impl(makePimpl<VulkanRenderPassDependencyImpl>(this, &renderPass, renderTarget, location))
{
}

VulkanRenderPassDependency::VulkanRenderPassDependency(const VulkanRenderPassDependency& _other) noexcept :
    m_impl(makePimpl<VulkanRenderPassDependencyImpl>(this, _other.m_impl->m_renderPass, _other.m_impl->m_renderTarget, _other.m_impl->m_location))
{
}

VulkanRenderPassDependency::VulkanRenderPassDependency(VulkanRenderPassDependency&& _other) noexcept :
    m_impl(makePimpl<VulkanRenderPassDependencyImpl>(this, std::move(_other.m_impl->m_renderPass), std::move(_other.m_impl->m_renderTarget), std::move(_other.m_impl->m_location)))
{
}

VulkanRenderPassDependency::~VulkanRenderPassDependency() noexcept = default;

VulkanRenderPassDependency& VulkanRenderPassDependency::operator=(const VulkanRenderPassDependency& _other) noexcept
{
    m_impl->m_renderPass = _other.m_impl->m_renderPass;
    m_impl->m_renderTarget = _other.m_impl->m_renderTarget;
    m_impl->m_location = _other.m_impl->m_location;

    return *this;
}

VulkanRenderPassDependency& VulkanRenderPassDependency::operator=(VulkanRenderPassDependency&& _other) noexcept
{
    m_impl->m_renderPass = std::move(_other.m_impl->m_renderPass);
    m_impl->m_renderTarget = std::move(_other.m_impl->m_renderTarget);
    m_impl->m_location = std::move(_other.m_impl->m_location);

    return *this;
}

const VulkanRenderPass* VulkanRenderPassDependency::inputAttachmentSource() const noexcept
{
    return m_impl->m_renderPass;
}

UInt32 VulkanRenderPassDependency::location() const noexcept
{
    return m_impl->m_location;
}

const RenderTarget& VulkanRenderPassDependency::renderTarget() const noexcept
{
    return m_impl->m_renderTarget;
}