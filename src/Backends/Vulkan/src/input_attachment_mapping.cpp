#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanInputAttachmentMapping::VulkanInputAttachmentMappingImpl : public Implement<VulkanInputAttachmentMapping> {
public:
    friend class VulkanInputAttachmentMapping;

private:
    const VulkanRenderPass& m_renderPass;
    RenderTarget m_renderTarget;
    UInt32 m_location;

public:
    VulkanInputAttachmentMappingImpl(VulkanInputAttachmentMapping* parent, const VulkanRenderPass& renderPass, const RenderTarget& renderTarget, const UInt32& location) :
        base(parent), m_renderPass(renderPass), m_location(location), m_renderTarget(renderTarget)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAttachmentMapping::VulkanInputAttachmentMapping(const VulkanRenderPass& renderPass, const RenderTarget& renderTarget, const UInt32& location) :
    m_impl(makePimpl<VulkanInputAttachmentMappingImpl>(this, renderPass, renderTarget, location))
{
}

VulkanInputAttachmentMapping::VulkanInputAttachmentMapping(const VulkanInputAttachmentMapping& _other) noexcept :
    m_impl(makePimpl<VulkanInputAttachmentMappingImpl>(this, _other.m_impl->m_renderPass, _other.m_impl->m_renderTarget, _other.m_impl->m_location))
{
}

VulkanInputAttachmentMapping::VulkanInputAttachmentMapping(VulkanInputAttachmentMapping&& _other) noexcept :
    m_impl(makePimpl<VulkanInputAttachmentMappingImpl>(this, _other.m_impl->m_renderPass, std::move(_other.m_impl->m_renderTarget), std::move(_other.m_impl->m_location)))
{
}

VulkanInputAttachmentMapping::~VulkanInputAttachmentMapping() noexcept = default;

const VulkanRenderPass& VulkanInputAttachmentMapping::inputAttachmentSource() const noexcept
{
    return m_impl->m_renderPass;
}

const UInt32& VulkanInputAttachmentMapping::location() const noexcept
{
    return m_impl->m_location;
}

const RenderTarget& VulkanInputAttachmentMapping::renderTarget() const noexcept
{
    return m_impl->m_renderTarget;
}