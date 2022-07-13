#include <litefx/backends/vulkan_builder.cpp>

using namespace LiteFX::Rendering::Backends;

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
    m_impl(makePimpl<VulkanInputAssemblerBuilderImpl>(this)), InputAssemblerBuilder<VulkanInputAssemblerBuilder, VulkanInputAssembler, VulkanRenderPipelineBuilder>(parent, SharedPtr<VulkanInputAssembler>(new VulkanInputAssembler(*std::as_const(parent).instance()->getDevice())))
{
}

VulkanInputAssemblerBuilder::~VulkanInputAssemblerBuilder() noexcept = default;

VulkanVertexBufferLayoutBuilder VulkanInputAssemblerBuilder::addVertexBuffer(const size_t& elementSize, const UInt32& binding)
{
    return VulkanVertexBufferLayoutBuilder(*this, makeUnique<VulkanVertexBufferLayout>(*this->instance(), elementSize, binding));
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