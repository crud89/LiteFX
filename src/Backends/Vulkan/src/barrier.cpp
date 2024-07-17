#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

using GlobalBarrier = Tuple<ResourceAccess, ResourceAccess>;
using BufferBarrier = Tuple<ResourceAccess, ResourceAccess, const IVulkanBuffer&, UInt32>;
using ImageBarrier = Tuple<ResourceAccess, ResourceAccess, const IVulkanImage&, Optional<ImageLayout>, ImageLayout, UInt32, UInt32, UInt32, UInt32, UInt32>;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBarrier::VulkanBarrierImpl : public Implement<VulkanBarrier> {
public:
    friend class VulkanBarrier;

private:
    PipelineStage m_syncBefore, m_syncAfter;
    Array<GlobalBarrier> m_globalBarriers;
    Array<BufferBarrier> m_bufferBarriers;
    Array<ImageBarrier> m_imageBarriers;

public:
    VulkanBarrierImpl(VulkanBarrier* parent, PipelineStage syncBefore, PipelineStage syncAfter) :
        base(parent), m_syncBefore(syncBefore), m_syncAfter(syncAfter)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBarrier::VulkanBarrier(PipelineStage syncBefore, PipelineStage syncAfter) noexcept :
    m_impl(makePimpl<VulkanBarrierImpl>(this, syncBefore, syncAfter))
{
}

VulkanBarrier::VulkanBarrier() noexcept :
    VulkanBarrier(PipelineStage::None, PipelineStage::None)
{
}

VulkanBarrier::~VulkanBarrier() noexcept = default;

PipelineStage VulkanBarrier::syncBefore() const noexcept
{
    return m_impl->m_syncBefore;
}

PipelineStage& VulkanBarrier::syncBefore() noexcept
{
    return m_impl->m_syncBefore;
}

PipelineStage VulkanBarrier::syncAfter() const noexcept
{
    return m_impl->m_syncAfter;
}

PipelineStage& VulkanBarrier::syncAfter() noexcept
{
    return m_impl->m_syncAfter;
}

void VulkanBarrier::wait(ResourceAccess accessBefore, ResourceAccess accessAfter) noexcept
{
    m_impl->m_globalBarriers.push_back({ accessBefore, accessAfter });
}

void VulkanBarrier::transition(const IVulkanBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter)
{
    m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, std::numeric_limits<UInt32>::max() });
}

void VulkanBarrier::transition(const IVulkanBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter)
{
    m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, element });
}

void VulkanBarrier::transition(const IVulkanImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, 0, image.levels(), 0, image.layers(), 0 });
}

void VulkanBarrier::transition(const IVulkanImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, fromLayout, toLayout, 0, image.levels(), 0, image.layers(), 0 });
}

void VulkanBarrier::transition(const IVulkanImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, level, levels, layer, layers, plane });
}

void VulkanBarrier::transition(const IVulkanImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, fromLayout, toLayout, level, levels, layer, layers, plane });
}

void VulkanBarrier::execute(const VulkanCommandBuffer& commandBuffer) const noexcept
{
    auto syncBefore = Vk::getPipelineStage(m_impl->m_syncBefore);
    auto syncAfter = Vk::getPipelineStage(m_impl->m_syncAfter);

	// Global barriers.
	auto globalBarriers = m_impl->m_globalBarriers | std::views::transform([syncBefore, syncAfter](auto& barrier) { 
        return VkMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = syncBefore,
            .srcAccessMask = Vk::getResourceAccess(std::get<0>(barrier)),
            .dstStageMask = syncAfter,
            .dstAccessMask = Vk::getResourceAccess(std::get<1>(barrier))
        };
	}) | std::ranges::to<Array<VkMemoryBarrier2>>();

	// Buffer barriers.
	auto bufferBarriers = m_impl->m_bufferBarriers | std::views::transform([syncBefore, syncAfter](auto& barrier) {
        return VkBufferMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .srcStageMask = syncBefore,
            .srcAccessMask = Vk::getResourceAccess(std::get<0>(barrier)),
            .dstStageMask = syncAfter,
            .dstAccessMask = Vk::getResourceAccess(std::get<1>(barrier)),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = std::as_const(std::get<2>(barrier)).handle(),
            .size = std::get<2>(barrier).size()
        };
	}) | std::ranges::to<Array<VkBufferMemoryBarrier2>>();

	// Image barriers.
	auto imageBarriers = m_impl->m_imageBarriers | std::views::transform([syncBefore, syncAfter](auto& barrier) {
		auto& image = std::get<2>(barrier);
		auto currentLayout = Vk::getImageLayout(std::get<3>(barrier).value_or(ImageLayout::Undefined));
		auto targetLayout = Vk::getImageLayout(std::get<4>(barrier));
        
        return VkImageMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = syncBefore,
            .srcAccessMask = Vk::getResourceAccess(std::get<0>(barrier)),
            .dstStageMask = syncAfter,
            .dstAccessMask = Vk::getResourceAccess(std::get<1>(barrier)),
            .oldLayout = currentLayout,
            .newLayout = targetLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = std::as_const(image).handle(),
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = image.aspectMask(std::get<9>(barrier)),
                .baseMipLevel = std::get<5>(barrier),
                .levelCount = std::get<6>(barrier),
                .baseArrayLayer = std::get<7>(barrier),
                .layerCount = std::get<8>(barrier)
            }
        };
	}) | std::ranges::to<Array<VkImageMemoryBarrier2>>();

    // Execute the barriers.
    if (!globalBarriers.empty() || !bufferBarriers.empty() || !imageBarriers.empty())
    {
        VkDependencyInfo barriers = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .memoryBarrierCount = static_cast<UInt32>(globalBarriers.size()),
            .pMemoryBarriers = globalBarriers.data(),
            .bufferMemoryBarrierCount = static_cast<UInt32>(bufferBarriers.size()),
            .pBufferMemoryBarriers = bufferBarriers.data(),
            .imageMemoryBarrierCount = static_cast<UInt32>(imageBarriers.size()),
            .pImageMemoryBarriers = imageBarriers.data()
        };

        ::vkCmdPipelineBarrier2(commandBuffer.handle(), &barriers);
    }
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanBarrierBuilder::VulkanBarrierBuilder() :
    BarrierBuilder(std::move(UniquePtr<VulkanBarrier>(new VulkanBarrier())))
{
}

VulkanBarrierBuilder::~VulkanBarrierBuilder() noexcept = default;

void VulkanBarrierBuilder::setupStages(PipelineStage waitFor, PipelineStage continueWith)
{
    this->instance()->syncBefore() = waitFor;
    this->instance()->syncAfter() = continueWith;
}

void VulkanBarrierBuilder::setupGlobalBarrier(ResourceAccess before, ResourceAccess after)
{
    this->instance()->wait(before, after);
}

void VulkanBarrierBuilder::setupBufferBarrier(IBuffer& buffer, ResourceAccess before, ResourceAccess after)
{
    this->instance()->transition(buffer, before, after);
}

void VulkanBarrierBuilder::setupImageBarrier(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane)
{
    auto numLevels = levels > 0 ? levels : image.levels() - level;
    auto numLayers = layers > 0 ? layers : image.layers() - layer;
    this->instance()->transition(image, level, numLevels, layer, numLayers, plane, before, after, layout);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)