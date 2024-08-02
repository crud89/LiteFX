#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanInputAssembler::VulkanInputAssemblerImpl : public Implement<VulkanInputAssembler> {
public:
    friend class VulkanInputAssembler;

private:
    Dictionary<UInt32, UniquePtr<VulkanVertexBufferLayout>> m_vertexBufferLayouts;
    UniquePtr<VulkanIndexBufferLayout> m_indexBufferLayout;
    PrimitiveTopology m_primitiveTopology;

public:
    VulkanInputAssemblerImpl(VulkanInputAssembler* parent) :
        base(parent)
    {
    }

public:
    void initialize(Enumerable<UniquePtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<VulkanIndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology)
    {
        m_primitiveTopology = primitiveTopology;
        m_indexBufferLayout = std::move(indexBufferLayout);

        for (auto& vertexBufferLayout : vertexBufferLayouts)
            if (vertexBufferLayout == nullptr) [[unlikely]]
                throw ArgumentNotInitializedException("vertexBufferLayouts", "One of the provided vertex buffer layouts is not initialized.");
            else if (m_vertexBufferLayouts.contains(vertexBufferLayout->binding())) [[unlikely]]
                throw InvalidArgumentException("vertexBufferLayouts", "Multiple vertex buffer layouts use the binding point {0}, but only one layout per binding point is allowed.", vertexBufferLayout->binding());
            else
                m_vertexBufferLayouts.emplace(vertexBufferLayout->binding(), std::move(vertexBufferLayout));
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssembler::VulkanInputAssembler(Enumerable<UniquePtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<VulkanIndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology) :
    m_impl(makePimpl<VulkanInputAssemblerImpl>(this))
{
    m_impl->initialize(std::move(vertexBufferLayouts), std::move(indexBufferLayout), primitiveTopology);
}

VulkanInputAssembler::VulkanInputAssembler() noexcept :
    m_impl(makePimpl<VulkanInputAssemblerImpl>(this))
{
}

VulkanInputAssembler::~VulkanInputAssembler() noexcept = default;

Enumerable<const VulkanVertexBufferLayout*> VulkanInputAssembler::vertexBufferLayouts() const noexcept
{
    return m_impl->m_vertexBufferLayouts | std::views::transform([](const auto& pair) { return pair.second.get(); });
}

const VulkanVertexBufferLayout* VulkanInputAssembler::vertexBufferLayout(UInt32 binding) const
{
    if (m_impl->m_vertexBufferLayouts.contains(binding)) [[likely]]
        return m_impl->m_vertexBufferLayouts[binding].get();

    throw InvalidArgumentException("binding", "No vertex buffer layout is bound to binding point {0}.", binding);
}

const VulkanIndexBufferLayout* VulkanInputAssembler::indexBufferLayout() const noexcept
{
    return m_impl->m_indexBufferLayout.get();
}

PrimitiveTopology VulkanInputAssembler::topology() const noexcept
{
    return m_impl->m_primitiveTopology;
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanInputAssemblerBuilder::VulkanInputAssemblerBuilderImpl : public Implement<VulkanInputAssemblerBuilder> {
public:
    friend class VulkanInputAssemblerBuilder;
    friend class VulkanInputAssembler;

private:
    Array<UniquePtr<VulkanVertexBufferLayout>> m_vertexBufferLayouts;
    UniquePtr<VulkanIndexBufferLayout> m_indexBufferLayout;
    PrimitiveTopology m_primitiveTopology;

public:
    VulkanInputAssemblerBuilderImpl(VulkanInputAssemblerBuilder* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Builder shared interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssemblerBuilder::VulkanInputAssemblerBuilder() noexcept :
    m_impl(makePimpl<VulkanInputAssemblerBuilderImpl>(this)), InputAssemblerBuilder(SharedPtr<VulkanInputAssembler>(new VulkanInputAssembler()))
{
}

VulkanInputAssemblerBuilder::~VulkanInputAssemblerBuilder() noexcept = default;

void VulkanInputAssemblerBuilder::build()
{
    this->instance()->m_impl->initialize(m_state.vertexBufferLayouts | std::views::as_rvalue, std::move(m_state.indexBufferLayout), m_state.topology);
}

VulkanVertexBufferLayoutBuilder VulkanInputAssemblerBuilder::vertexBuffer(size_t elementSize, UInt32 binding)
{
    return VulkanVertexBufferLayoutBuilder(*this, makeUnique<VulkanVertexBufferLayout>(elementSize, binding));
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)