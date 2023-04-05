#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

using GlobalBarrier = Tuple<ResourceAccess, ResourceAccess>;
using BufferBarrier = Tuple<ResourceAccess, ResourceAccess, IVulkanBuffer&, UInt32>;
using ImageBarrier = Tuple<ResourceAccess, ResourceAccess, IVulkanImage&, ImageLayout, UInt32, UInt32, UInt32, UInt32, UInt32>;

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
    VulkanBarrierImpl(VulkanBarrier* parent, const PipelineStage& syncBefore, const PipelineStage& syncAfter) :
        base(parent), m_syncBefore(syncBefore), m_syncAfter(syncAfter)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBarrier::VulkanBarrier(const PipelineStage& syncBefore, const PipelineStage& syncAfter) noexcept :
    m_impl(makePimpl<VulkanBarrierImpl>(this, syncBefore, syncAfter))
{
}

VulkanBarrier::~VulkanBarrier() noexcept = default;

const PipelineStage& VulkanBarrier::syncBefore() const noexcept
{
    return m_impl->m_syncBefore;
}

const PipelineStage& VulkanBarrier::syncAfter() const noexcept
{
    return m_impl->m_syncAfter;
}

void VulkanBarrier::wait(const ResourceAccess& accessBefore, const ResourceAccess& accessAfter) noexcept
{
    m_impl->m_globalBarriers.push_back({ accessBefore, accessAfter });
}

void VulkanBarrier::transition(IVulkanBuffer& buffer, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter)
{
    m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, std::numeric_limits<UInt32>::max() });
}

void VulkanBarrier::transition(IVulkanBuffer& buffer, const UInt32& element, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter)
{
    m_impl->m_bufferBarriers.push_back({ accessBefore, accessAfter, buffer, element });
}

void VulkanBarrier::transition(IVulkanImage& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, layout, 0, image.levels(), 0, image.layers(), 0 });
}

void VulkanBarrier::transition(IVulkanImage& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, layout, level, levels, layer, layers, plane });
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
	}) | ranges::to<Array<VkMemoryBarrier>>();

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
	}) | ranges::to<Array<VkBufferMemoryBarrier>>();

	// Image barriers.
	auto imageBarriers = m_impl->m_imageBarriers | std::views::transform([this](auto& barrier) {
		auto& image = std::get<2>(barrier);
		auto layout = image.layout(image.subresourceId(std::get<4>(barrier), std::get<6>(barrier), std::get<8>(barrier)));
		auto currentLayout = Vk::getImageLayout(layout);
		auto targetLayout = Vk::getImageLayout(std::get<3>(barrier));

		for (auto layer = std::get<6>(barrier); layer < std::get<7>(barrier); layer++)
		{
			for (auto level = std::get<4>(barrier); level < std::get<5>(barrier); level++)
			{
				auto subresource = image.subresourceId(level, layer, std::get<8>(barrier));

				if (image.layout(subresource) != layout) [[unlikely]]
					throw RuntimeException("All sub-resources in a sub-resource range need to have the same initial layout.");
				else
					image.layout(subresource) = std::get<3>(barrier);
			}
		}
        
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
                .aspectMask = image.aspectMask(std::get<8>(barrier)),
                .baseMipLevel = std::get<4>(barrier),
                .levelCount = std::get<5>(barrier),
                .baseArrayLayer = std::get<6>(barrier),
                .layerCount = std::get<7>(barrier)
            }
        };
	}) | ranges::to<Array<VkImageMemoryBarrier>>();

    // Execute the barriers.
    if (!globalBarriers.empty() || !bufferBarriers.empty() || !imageBarriers.empty())
        ::vkCmdPipelineBarrier(commandBuffer.handle(), Vk::getPipelineStage(m_impl->m_syncBefore), Vk::getPipelineStage(m_impl->m_syncAfter), 0,
            globalBarriers.size(), globalBarriers.data(), bufferBarriers.size(), bufferBarriers.data(), imageBarriers.size(), imageBarriers.data());
}