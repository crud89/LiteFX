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

constexpr VulkanBarrier::VulkanBarrier(PipelineStage syncBefore, PipelineStage syncAfter) noexcept :
    m_impl(makePimpl<VulkanBarrierImpl>(this, syncBefore, syncAfter))
{
}

constexpr VulkanBarrier::VulkanBarrier() noexcept :
    VulkanBarrier(PipelineStage::None, PipelineStage::None)
{
}

constexpr VulkanBarrier::~VulkanBarrier() noexcept = default;

constexpr PipelineStage VulkanBarrier::syncBefore() const noexcept
{
    return m_impl->m_syncBefore;
}

constexpr PipelineStage& VulkanBarrier::syncBefore() noexcept
{
    return m_impl->m_syncBefore;
}

constexpr PipelineStage VulkanBarrier::syncAfter() const noexcept
{
    return m_impl->m_syncAfter;
}

constexpr PipelineStage& VulkanBarrier::syncAfter() noexcept
{
    return m_impl->m_syncAfter;
}

constexpr void VulkanBarrier::wait(ResourceAccess accessBefore, ResourceAccess accessAfter) noexcept
{
    m_impl->m_globalBarriers.push_back({ accessBefore, accessAfter });
}

constexpr void VulkanBarrier::transition(const IVulkanBuffer& buffer, ResourceAccess accessBefore, ResourceAccess accessAfter)
{
    m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, std::numeric_limits<UInt32>::max() });
}

constexpr void VulkanBarrier::transition(const IVulkanBuffer& buffer, UInt32 element, ResourceAccess accessBefore, ResourceAccess accessAfter)
{
    m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, element });
}

constexpr void VulkanBarrier::transition(const IVulkanImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, 0, image.levels(), 0, image.layers(), 0 });
}

constexpr void VulkanBarrier::transition(const IVulkanImage& image, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, fromLayout, toLayout, 0, image.levels(), 0, image.layers(), 0 });
}

constexpr void VulkanBarrier::transition(const IVulkanImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout layout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, level, levels, layer, layers, plane });
}

constexpr void VulkanBarrier::transition(const IVulkanImage& image, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane, ResourceAccess accessBefore, ResourceAccess accessAfter, ImageLayout fromLayout, ImageLayout toLayout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, fromLayout, toLayout, level, levels, layer, layers, plane });
}

void VulkanBarrier::execute(const VulkanCommandBuffer& commandBuffer) const noexcept
{    
	// Global barriers.
	auto globalBarriers = m_impl->m_globalBarriers | std::views::transform([this](auto& barrier) { 
        return VkMemoryBarrier {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .srcAccessMask = Vk::getResourceAccess(std::get<0>(barrier)),
            .dstAccessMask = Vk::getResourceAccess(std::get<1>(barrier))
        };
	}) | std::ranges::to<Array<VkMemoryBarrier>>();

	// Buffer barriers.
	auto bufferBarriers = m_impl->m_bufferBarriers | std::views::transform([this](auto& barrier) {
        return VkBufferMemoryBarrier {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = Vk::getResourceAccess(std::get<0>(barrier)),
            .dstAccessMask = Vk::getResourceAccess(std::get<1>(barrier)),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = std::as_const(std::get<2>(barrier)).handle(),
            .offset = 0,
            .size = std::get<2>(barrier).size()
        };
	}) | std::ranges::to<Array<VkBufferMemoryBarrier>>();

	// Image barriers.
	auto imageBarriers = m_impl->m_imageBarriers | std::views::transform([this](auto& barrier) {
		auto& image = std::get<2>(barrier);
		auto currentLayout = Vk::getImageLayout(std::get<3>(barrier).value_or(ImageLayout::Undefined));
		auto targetLayout = Vk::getImageLayout(std::get<4>(barrier));

		for (auto layer = std::get<7>(barrier); layer < std::get<7>(barrier) + std::get<8>(barrier); layer++)
			for (auto level = std::get<5>(barrier); level < std::get<5>(barrier) + std::get<6>(barrier); level++)
				auto subresource = image.subresourceId(level, layer, std::get<9>(barrier));
        
        return VkImageMemoryBarrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = Vk::getResourceAccess(std::get<0>(barrier)),
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
	}) | std::ranges::to<Array<VkImageMemoryBarrier>>();

    // Execute the barriers.
    if (!globalBarriers.empty() || !bufferBarriers.empty() || !imageBarriers.empty())
        ::vkCmdPipelineBarrier(commandBuffer.handle(), Vk::getPipelineStage(m_impl->m_syncBefore), Vk::getPipelineStage(m_impl->m_syncAfter), 0,
            globalBarriers.size(), globalBarriers.data(), bufferBarriers.size(), bufferBarriers.data(), imageBarriers.size(), imageBarriers.data());
}

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

constexpr VulkanBarrierBuilder::VulkanBarrierBuilder() :
    BarrierBuilder(std::move(UniquePtr<VulkanBarrier>(new VulkanBarrier())))
{
}

constexpr VulkanBarrierBuilder::~VulkanBarrierBuilder() noexcept = default;

constexpr void VulkanBarrierBuilder::setupStages(PipelineStage waitFor, PipelineStage continueWith)
{
    this->instance()->syncBefore() = waitFor;
    this->instance()->syncAfter() = continueWith;
}

constexpr void VulkanBarrierBuilder::setupGlobalBarrier(ResourceAccess before, ResourceAccess after)
{
    this->instance()->wait(before, after);
}

constexpr void VulkanBarrierBuilder::setupBufferBarrier(IBuffer& buffer, ResourceAccess before, ResourceAccess after)
{
    this->instance()->transition(buffer, before, after);
}

constexpr void VulkanBarrierBuilder::setupImageBarrier(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane)
{
    auto numLevels = levels > 0 ? levels : image.levels() - level;
    auto numLayers = layers > 0 ? layers : image.layers() - layer;
    this->instance()->transition(image, level, numLevels, layer, numLayers, plane, before, after, layout);
}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)