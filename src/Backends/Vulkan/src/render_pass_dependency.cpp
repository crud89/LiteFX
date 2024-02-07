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
    DescriptorBindingPoint m_bindingPoint;

public:
    VulkanRenderPassDependencyImpl(VulkanRenderPassDependency* parent, const VulkanRenderPass* renderPass, const RenderTarget& renderTarget, DescriptorBindingPoint binding) :
        base(parent), m_renderPass(renderPass), m_bindingPoint(binding), m_renderTarget(renderTarget)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPassDependency::VulkanRenderPassDependency(const VulkanRenderPass& renderPass, const RenderTarget& renderTarget, DescriptorBindingPoint binding) :
    m_impl(makePimpl<VulkanRenderPassDependencyImpl>(this, &renderPass, renderTarget, binding))
{
}

VulkanRenderPassDependency::VulkanRenderPassDependency(const VulkanRenderPass& renderPass, const RenderTarget& renderTarget, UInt32 bindingRegister, UInt32 spaces) :
    m_impl(makePimpl<VulkanRenderPassDependencyImpl>(this, &renderPass, renderTarget, DescriptorBindingPoint { .Register = bindingRegister, .Space = space }))
{
}

VulkanRenderPassDependency::VulkanRenderPassDependency(const VulkanRenderPassDependency& _other) noexcept :
    m_impl(makePimpl<VulkanRenderPassDependencyImpl>(this, _other.m_impl->m_renderPass, _other.m_impl->m_renderTarget, _other.m_impl->m_bindingPoint))
{
}

VulkanRenderPassDependency::VulkanRenderPassDependency(VulkanRenderPassDependency&& _other) noexcept :
    m_impl(makePimpl<VulkanRenderPassDependencyImpl>(this, std::move(_other.m_impl->m_renderPass), std::move(_other.m_impl->m_renderTarget), std::move(_other.m_impl->m_bindingPoint)))
{
}

VulkanRenderPassDependency::~VulkanRenderPassDependency() noexcept = default;

VulkanRenderPassDependency& VulkanRenderPassDependency::operator=(const VulkanRenderPassDependency& _other) noexcept
{
    m_impl->m_renderPass = _other.m_impl->m_renderPass;
    m_impl->m_renderTarget = _other.m_impl->m_renderTarget;
    m_impl->m_bindingPoint = _other.m_impl->m_bindingPoint;

    return *this;
}

VulkanRenderPassDependency& VulkanRenderPassDependency::operator=(VulkanRenderPassDependency&& _other) noexcept
{
    m_impl->m_renderPass = std::move(_other.m_impl->m_renderPass);
    m_impl->m_renderTarget = std::move(_other.m_impl->m_renderTarget);
    m_impl->m_bindingPoint = std::move(_other.m_impl->m_bindingPoint);

    return *this;
}

const VulkanRenderPass* VulkanRenderPassDependency::inputAttachmentSource() const noexcept
{
    return m_impl->m_renderPass;
}

const DescriptorBindingPoint& VulkanRenderPassDependency::binding() const noexcept
{
    return m_impl->m_bindingPoint;
}

const RenderTarget& VulkanRenderPassDependency::renderTarget() const noexcept
{
    return m_impl->m_renderTarget;
}