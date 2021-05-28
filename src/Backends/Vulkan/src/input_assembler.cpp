#include <litefx/backends/vulkan.hpp>

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
    void initialize(Array<UniquePtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<VulkanIndexBufferLayout>&& indexBufferLayout, const PrimitiveTopology& primitiveTopology)
    {
        m_primitiveTopology = primitiveTopology;

        if (indexBufferLayout == nullptr)
            throw ArgumentNotInitializedException("The index buffer layout must be initialized.");

        m_indexBufferLayout = std::move(indexBufferLayout);

        for (auto& vertexBufferLayout : vertexBufferLayouts)
        {
            if (vertexBufferLayout == nullptr)
                throw ArgumentNotInitializedException("One of the provided vertex buffer layouts is not initialized.");

            if (m_vertexBufferLayouts.contains(vertexBufferLayout->binding()))
                throw InvalidArgumentException("Multiple vertex buffer layouts use the binding point {0}, but only one layout per binding point is allowed.", vertexBufferLayout->binding());

            m_vertexBufferLayouts.emplace(vertexBufferLayout->binding(), std::move(vertexBufferLayout));
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssembler::VulkanInputAssembler(const VulkanDevice& device, Array<UniquePtr<VulkanVertexBufferLayout>>&& vertexBufferLayouts, UniquePtr<VulkanIndexBufferLayout>&& indexBufferLayout, const PrimitiveTopology& primitiveTopology) :
    m_impl(makePimpl<VulkanInputAssemblerImpl>(this)), VulkanRuntimeObject<VulkanDevice>(device, &device)
{
    m_impl->initialize(std::move(vertexBufferLayouts), std::move(indexBufferLayout), primitiveTopology);
}

VulkanInputAssembler::VulkanInputAssembler(const VulkanDevice& device) noexcept :
    m_impl(makePimpl<VulkanInputAssemblerImpl>(this)), VulkanRuntimeObject<VulkanDevice>(device, &device)
{
}

VulkanInputAssembler::~VulkanInputAssembler() noexcept = default;

Array<const VulkanVertexBufferLayout*> VulkanInputAssembler::vertexBufferLayouts() const noexcept
{
    return m_impl->m_vertexBufferLayouts |
        std::views::transform([](const auto& pair) { return pair.second.get(); }) |
        ranges::to<Array<const VulkanVertexBufferLayout*>>();
}

const VulkanVertexBufferLayout& VulkanInputAssembler::vertexBufferLayout(const UInt32& binding) const
{
    [[likely]] if (m_impl->m_vertexBufferLayouts.contains(binding))
        return *m_impl->m_vertexBufferLayouts[binding];

    throw ArgumentOutOfRangeException("No vertex buffer layout is bound to binding point {0}.", binding);
}

const VulkanIndexBufferLayout& VulkanInputAssembler::indexBufferLayout() const
{
    return *m_impl->m_indexBufferLayout;
}

const PrimitiveTopology& VulkanInputAssembler::topology() const noexcept
{
    return m_impl->m_primitiveTopology;
}

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

VulkanInputAssemblerBuilder::VulkanInputAssemblerBuilder(VulkanRenderPipelineBuilder& parent) noexcept :
    m_impl(makePimpl<VulkanInputAssemblerBuilderImpl>(this)), InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineBuilder>(parent, UniquePtr<VulkanInputAssembler>(new VulkanInputAssembler(*std::as_const(parent).instance()->getDevice())))
{
}

VulkanInputAssemblerBuilder::~VulkanInputAssemblerBuilder() noexcept = default;

VulkanVertexBufferLayoutBuilder VulkanInputAssemblerBuilder::addVertexBuffer(const size_t& elementSize, const UInt32& binding)
{
    return this->make<VulkanVertexBufferLayout>(elementSize, binding);
}

VulkanInputAssemblerBuilder& VulkanInputAssemblerBuilder::withIndexType(const IndexType& type)
{
    this->use(makeUnique<VulkanIndexBufferLayout>(*this->instance(), type));
    return *this;
}

VulkanInputAssemblerBuilder& VulkanInputAssemblerBuilder::withTopology(const PrimitiveTopology& topology)
{
    m_impl->m_primitiveTopology = topology;
    return *this;
}

void VulkanInputAssemblerBuilder::use(UniquePtr<VulkanVertexBufferLayout>&& layout)
{
    m_impl->m_vertexBufferLayouts.push_back(std::move(layout));
}

void VulkanInputAssemblerBuilder::use(UniquePtr<VulkanIndexBufferLayout>&& layout)
{
    m_impl->m_indexBufferLayout = std::move(layout);
}

VulkanRenderPipelineBuilder& VulkanInputAssemblerBuilder::go()
{
    this->instance()->m_impl->initialize(std::move(m_impl->m_vertexBufferLayouts), std::move(m_impl->m_indexBufferLayout), m_impl->m_primitiveTopology);
    return InputAssemblerBuilder::go();
}