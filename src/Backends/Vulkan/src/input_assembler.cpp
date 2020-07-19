#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssembler::VulkanInputAssembler(const VulkanRenderPipelineLayout& layout) noexcept :
    InputAssembler(), VulkanRuntimeObject(layout.getDevice())
{
}

VulkanInputAssembler::~VulkanInputAssembler() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssemblerBuilder& VulkanInputAssemblerBuilder::withTopology(const PrimitiveTopology& topology)
{
    this->instance()->setTopology(topology);
    return *this;
}

void VulkanInputAssemblerBuilder::use(UniquePtr<IVertexBufferLayout>&& layout)
{
    this->instance()->use(std::move(layout));
}

void VulkanInputAssemblerBuilder::use(UniquePtr<IIndexBufferLayout>&& layout)
{
    this->instance()->use(std::move(layout));
}

VulkanInputAssemblerBuilder& VulkanInputAssemblerBuilder::withIndexType(const IndexType& type)
{
    this->instance()->use(makeUnique<VulkanIndexBufferLayout>(*this->instance(), type));
    return *this;
}