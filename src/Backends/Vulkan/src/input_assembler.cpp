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
    Array<UniquePtr<const VulkanVertexBufferLayout>> m_vertexBufferLayouts;
    Dictionary<UInt32, const VulkanVertexBufferLayout*> m_vertexBufferLayoutBindings;
    UniquePtr<const VulkanIndexBufferLayout> m_indexBufferLayout;
    PrimitiveTopology m_primitiveTopology;

public:
    VulkanInputAssemblerImpl(VulkanInputAssembler* parent) :
        base(parent)
    {
    }

public:
    void initialize(Array<UniquePtr<const VulkanVertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<const VulkanIndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology)
    {
        m_primitiveTopology = primitiveTopology;
        m_indexBufferLayout = std::move(indexBufferLayout);
        m_vertexBufferLayouts = std::move(vertexBufferLayouts);

        for (auto& vertexBufferLayout : m_vertexBufferLayouts)
        {
            if (vertexBufferLayout == nullptr) [[unlikely]]
                throw ArgumentNotInitializedException("vertexBufferLayouts", "One of the provided vertex buffer layouts is not initialized.");
            else if (m_vertexBufferLayoutBindings.contains(vertexBufferLayout->binding())) [[unlikely]]
                throw InvalidArgumentException("vertexBufferLayouts", "Multiple vertex buffer layouts use the binding point {0}, but only one layout per binding point is allowed.", vertexBufferLayout->binding());
            else
                m_vertexBufferLayoutBindings.emplace(vertexBufferLayout->binding(), vertexBufferLayout.get());
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssembler::VulkanInputAssembler(Enumerable<UniquePtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<VulkanIndexBufferLayout>&& indexBufferLayout, PrimitiveTopology primitiveTopology) :
    m_impl(makePimpl<VulkanInputAssemblerImpl>(this))
{
    Array<UniquePtr<const VulkanVertexBufferLayout>> layouts;
    layouts.assign_range(vertexBufferLayouts);

    m_impl->initialize(std::move(layouts), std::move(indexBufferLayout), primitiveTopology);
}

VulkanInputAssembler::VulkanInputAssembler() noexcept :
    m_impl(makePimpl<VulkanInputAssemblerImpl>(this))
{
}

VulkanInputAssembler::~VulkanInputAssembler() noexcept = default;

const Array<UniquePtr<const VulkanVertexBufferLayout>>& VulkanInputAssembler::vertexBufferLayouts() const noexcept
{
    return m_impl->m_vertexBufferLayouts;
}

const VulkanVertexBufferLayout& VulkanInputAssembler::vertexBufferLayout(UInt32 binding) const
{
    if (m_impl->m_vertexBufferLayoutBindings.contains(binding)) [[likely]]
        return *m_impl->m_vertexBufferLayouts[binding];

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

constexpr VulkanInputAssemblerBuilder::VulkanInputAssemblerBuilder() noexcept :
    m_impl(makePimpl<VulkanInputAssemblerBuilderImpl>(this)), InputAssemblerBuilder(SharedPtr<VulkanInputAssembler>(new VulkanInputAssembler()))
{
}

constexpr VulkanInputAssemblerBuilder::~VulkanInputAssemblerBuilder() noexcept = default;

void VulkanInputAssemblerBuilder::build()
{
    this->instance()->m_impl->initialize(std::move(m_state.vertexBufferLayouts), std::move(m_state.indexBufferLayout), m_state.topology);
}

constexpr VulkanVertexBufferLayoutBuilder VulkanInputAssemblerBuilder::vertexBuffer(size_t elementSize, UInt32 binding)
{
    return VulkanVertexBufferLayoutBuilder(*this, makeUnique<VulkanVertexBufferLayout>(elementSize, binding));
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)