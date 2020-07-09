#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssembler::VulkanInputAssembler(const VulkanRenderPipelineLayout& layout) noexcept :
    InputAssembler()
{
}

VulkanInputAssembler::~VulkanInputAssembler() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssemblerBuilder& VulkanInputAssemblerBuilder::addBufferLayout(UniquePtr<BufferLayout>&& layout)
{
    this->instance()->use(std::move(layout));
    return *this;
}

VulkanInputAssemblerBuilder& VulkanInputAssemblerBuilder::withTopology(const PrimitiveTopology& topology)
{
    this->instance()->setTopology(topology);
    return *this;
}