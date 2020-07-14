#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssembler::VulkanInputAssembler(const VulkanRenderPipelineLayout& layout) noexcept :
    InputAssembler(), RuntimeObject(layout.getDevice())
{
}

VulkanInputAssembler::~VulkanInputAssembler() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssemblerBuilder& VulkanInputAssemblerBuilder::addBufferSet(UniquePtr<IBufferSet>&& set)
{
    this->instance()->use(std::move(set));
    return *this;
}

VulkanInputAssemblerBuilder& VulkanInputAssemblerBuilder::withTopology(const PrimitiveTopology& topology)
{
    this->instance()->setTopology(topology);
    return *this;
}