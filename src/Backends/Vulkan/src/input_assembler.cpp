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

VulkanInputAssemblerBuilder& VulkanInputAssemblerBuilder::withBufferLayout(UniquePtr<BufferLayout>&& layout)
{
    this->instance()->use(std::move(layout));
    return *this;
}