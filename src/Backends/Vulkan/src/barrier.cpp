#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

using GlobalBarrier = Tuple<ResourceAccess, ResourceAccess>;
using BufferBarrier = Tuple<ResourceAccess, ResourceAccess, IVulkanBuffer&, UInt32>;
using ImageBarrier = Tuple<ResourceAccess, ResourceAccess, IVulkanImage&, Optional<ImageLayout>, ImageLayout, UInt32, UInt32, UInt32, UInt32, UInt32>;

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

VulkanBarrier::VulkanBarrier() noexcept :
    VulkanBarrier(PipelineStage::None, PipelineStage::None)
{
}

VulkanBarrier::~VulkanBarrier() noexcept = default;

const PipelineStage& VulkanBarrier::syncBefore() const noexcept
{
    return m_impl->m_syncBefore;
}

PipelineStage& VulkanBarrier::syncBefore() noexcept
{
    return m_impl->m_syncBefore;
}

const PipelineStage& VulkanBarrier::syncAfter() const noexcept
{
    return m_impl->m_syncAfter;
}

PipelineStage& VulkanBarrier::syncAfter() noexcept
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
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, 0, image.levels(), 0, image.layers(), 0 });
}

void VulkanBarrier::transition(IVulkanImage& image, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, fromLayout, toLayout, 0, image.levels(), 0, image.layers(), 0 });
}

void VulkanBarrier::transition(IVulkanImage& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& layout)
{
    m_impl->m_imageBarriers.push_back({ accessBefore, accessAfter, image, std::nullopt, layout, level, levels, layer, layers, plane });
}

void VulkanBarrier::transition(IVulkanImage& image, const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane, const ResourceAccess& accessBefore, const ResourceAccess& accessAfter, const ImageLayout& fromLayout, const ImageLayout& toLayout)
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
		auto layout = image.layout(image.subresourceId(std::get<5>(barrier), std::get<7>(barrier), std::get<9>(barrier)));
		auto currentLayout = Vk::getImageLayout(std::get<3>(barrier).value_or(layout));
		auto targetLayout = Vk::getImageLayout(std::get<4>(barrier));

		for (auto layer = std::get<7>(barrier); layer < std::get<7>(barrier) + std::get<8>(barrier); layer++)
		{
			for (auto level = std::get<5>(barrier); level < std::get<5>(barrier) + std::get<6>(barrier); level++)
			{
				auto subresource = image.subresourceId(level, layer, std::get<9>(barrier));

				if (image.layout(subresource) != layout && currentLayout != VK_IMAGE_LAYOUT_UNDEFINED) [[unlikely]]
					throw RuntimeException("All sub-resources in a sub-resource range need to have the same initial layout.");
				else
					image.layout(subresource) = std::get<4>(barrier);
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
                .aspectMask = image.aspectMask(std::get<9>(barrier)),
                .baseMipLevel = std::get<5>(barrier),
                .levelCount = std::get<6>(barrier),
                .baseArrayLayer = std::get<7>(barrier),
                .layerCount = std::get<8>(barrier)
            }
        };
	}) | ranges::to<Array<VkImageMemoryBarrier>>();

    // Execute the barriers.
    if (!globalBarriers.empty() || !bufferBarriers.empty() || !imageBarriers.empty())
        ::vkCmdPipelineBarrier(commandBuffer.handle(), Vk::getPipelineStage(m_impl->m_syncBefore), Vk::getPipelineStage(m_impl->m_syncAfter), 0,
            globalBarriers.size(), globalBarriers.data(), bufferBarriers.size(), bufferBarriers.data(), imageBarriers.size(), imageBarriers.data());
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Sub-builder definitions.
// ------------------------------------------------------------------------------------------------

class VulkanBarrierBuilder::VulkanSecondStageBarrierBuilder : public VulkanBarrierBuilder::SecondStageBuilder {
private:
	VulkanBarrierBuilder& m_parent;

public:
	VulkanSecondStageBarrierBuilder(VulkanBarrierBuilder& parent) :
		m_parent(parent)
	{
	}

public:
	virtual VulkanBarrierBuilder& toContinueWith(const PipelineStage& stage) override;
};

class VulkanBarrierBuilder::VulkanGlobalBarrierBuilder : public VulkanBarrierBuilder::GlobalBarrierBuilder {
private:
	VulkanBarrierBuilder& m_parent;

public:
	ResourceAccess m_accessAfter;

public:
	VulkanGlobalBarrierBuilder(VulkanBarrierBuilder& parent) :
		m_parent(parent)
	{
	}

public:
	virtual VulkanBarrierBuilder& untilFinishedWith(const ResourceAccess& access) override;
};

class VulkanBarrierBuilder::VulkanBufferBarrierBuilder : public VulkanBarrierBuilder::BufferBarrierBuilder {
private:
	VulkanBarrierBuilder& m_parent;

public:
	IBuffer* m_buffer;
	UInt32 m_subresource;
	ResourceAccess m_accessAfter;

public:
	VulkanBufferBarrierBuilder(VulkanBarrierBuilder& parent) :
		m_parent(parent)
	{
	}

public:
	virtual VulkanBarrierBuilder& untilFinishedWith(const ResourceAccess& access) override;
};

class VulkanBarrierBuilder::VulkanImageBarrierBuilder : public VulkanBarrierBuilder::ImageBarrierBuilder {
private:
	VulkanBarrierBuilder& m_parent;

public:
	VulkanImageBarrierBuilder(VulkanBarrierBuilder& parent) :
		m_parent(parent)
	{
	}

public:
	virtual ImageLayoutBarrierBuilder& transitionLayout(const ImageLayout& layout) override;
	virtual VulkanImageBarrierBuilder& subresource(const UInt32& level, const UInt32& levels, const UInt32& layer = 0, const UInt32& layers = 1, const UInt32& plane = 0) override;
};

class VulkanBarrierBuilder::VulkanImageLayoutBarrierBuilder : public VulkanBarrierBuilder::ImageLayoutBarrierBuilder {
public:
	friend class VulkanBarrierBuilder;
	friend class VulkanBarrierBuilder::VulkanImageBarrierBuilder;

private:
	VulkanBarrierBuilder& m_parent;
	UInt32 m_level, m_levels;
	UInt32 m_layer, m_layers;
	UInt32 m_plane;
	ImageLayout m_layout;
	IImage* m_image;
	ResourceAccess m_accessAfter;

public:
	VulkanImageLayoutBarrierBuilder(VulkanBarrierBuilder& parent) :
		m_parent(parent), m_level(0), m_levels(0), m_layer(0), m_layers(0), m_plane(0), m_layout(ImageLayout::Common)
	{
	}

private:
	void setSubresource(const UInt32& level = 0, const UInt32& levels = 0, const UInt32& layer = 0, const UInt32& layers = 0, const UInt32& plane = 0) noexcept {
		m_level = level;
		m_levels = levels;
		m_layer = layer;
		m_layers = layers;
		m_plane = plane;
	}

public:
	virtual VulkanBarrierBuilder& whenFinishedWith(const ResourceAccess& access) override;
};

// ------------------------------------------------------------------------------------------------
// Builder implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBarrierBuilder::VulkanBarrierBuilderImpl : public Implement<VulkanBarrierBuilder> {
public:
	friend class VulkanBarrierBuilder;
	friend class VulkanBarrierBuilder::VulkanSecondStageBarrierBuilder;
	friend class VulkanBarrierBuilder::VulkanGlobalBarrierBuilder;
	friend class VulkanBarrierBuilder::VulkanBufferBarrierBuilder;
	friend class VulkanBarrierBuilder::VulkanImageBarrierBuilder;
	friend class VulkanBarrierBuilder::VulkanImageLayoutBarrierBuilder;

private:
	Optional<PipelineStage> m_syncBefore, m_syncAfter;
	VulkanBarrierBuilder::VulkanSecondStageBarrierBuilder m_secondStageBuilder;
	VulkanBarrierBuilder::VulkanGlobalBarrierBuilder      m_globalBuilder;
	VulkanBarrierBuilder::VulkanBufferBarrierBuilder      m_bufferBuilder;
	VulkanBarrierBuilder::VulkanImageBarrierBuilder       m_imageBuilder;
	VulkanBarrierBuilder::VulkanImageLayoutBarrierBuilder m_imageLayoutBuilder;

public:
	VulkanBarrierBuilderImpl(VulkanBarrierBuilder* parent) :
		base(parent), m_secondStageBuilder(*parent), m_globalBuilder(*parent), m_bufferBuilder(*parent), m_imageBuilder(*parent), m_imageLayoutBuilder(*parent)
	{
	}
};

// ------------------------------------------------------------------------------------------------
// Sub-builder implementations.
// ------------------------------------------------------------------------------------------------

VulkanBarrierBuilder& VulkanBarrierBuilder::VulkanSecondStageBarrierBuilder::toContinueWith(const PipelineStage& stage)
{
	m_parent.m_impl->m_syncAfter = stage;
	return m_parent;
}

VulkanBarrierBuilder& VulkanBarrierBuilder::VulkanGlobalBarrierBuilder::untilFinishedWith(const ResourceAccess& access)
{
	m_parent.instance()->wait(access, m_accessAfter);
	return m_parent;
}

VulkanBarrierBuilder& VulkanBarrierBuilder::VulkanBufferBarrierBuilder::untilFinishedWith(const ResourceAccess& access)
{
	m_parent.instance()->transition(*m_buffer, m_subresource, access, m_accessAfter);
	return m_parent;
}

VulkanBarrierBuilder::ImageLayoutBarrierBuilder& VulkanBarrierBuilder::VulkanImageBarrierBuilder::transitionLayout(const ImageLayout& layout)
{
	m_parent.m_impl->m_imageLayoutBuilder.m_layout = layout;
	return m_parent.m_impl->m_imageLayoutBuilder;
}

VulkanBarrierBuilder::VulkanImageBarrierBuilder& VulkanBarrierBuilder::VulkanImageBarrierBuilder::subresource(const UInt32& level, const UInt32& levels, const UInt32& layer, const UInt32& layers, const UInt32& plane)
{
	m_parent.m_impl->m_imageLayoutBuilder.setSubresource(level, levels, layer, layers, plane);
	return *this;
}

VulkanBarrierBuilder& VulkanBarrierBuilder::VulkanImageLayoutBarrierBuilder::whenFinishedWith(const ResourceAccess& access)
{
	auto levels = m_levels > 0 ? m_levels : m_image->levels() - m_level;
	auto layers = m_layers > 0 ? m_layers : m_image->layers() - m_layer;
	m_parent.instance()->transition(*m_image, m_level, levels, m_layer, layers, m_plane, access, m_accessAfter, m_layout);
	return m_parent;
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanBarrierBuilder::VulkanBarrierBuilder() :
	m_impl(makePimpl<VulkanBarrierBuilderImpl>(this)), BarrierBuilder(std::move(UniquePtr<VulkanBarrier>(new VulkanBarrier())))
{
}

VulkanBarrierBuilder::~VulkanBarrierBuilder() noexcept = default;

void VulkanBarrierBuilder::build()
{
	if (!m_impl->m_syncBefore.has_value() || !m_impl->m_syncAfter.has_value())
		throw RuntimeException("A pipeline requires a synchronization scope. Make sure to call `waitFor` to define it.");

	this->instance()->syncBefore() = m_impl->m_syncBefore.value();
	this->instance()->syncAfter() = m_impl->m_syncAfter.value();
}

VulkanBarrierBuilder::SecondStageBuilder& VulkanBarrierBuilder::waitFor(const PipelineStage& stage)
{
	m_impl->m_syncBefore = stage;
	return m_impl->m_secondStageBuilder;
}

VulkanBarrierBuilder::GlobalBarrierBuilder& VulkanBarrierBuilder::blockAccessTo(const ResourceAccess& access)
{
	m_impl->m_globalBuilder.m_accessAfter = access;
	return m_impl->m_globalBuilder;
}

VulkanBarrierBuilder::BufferBarrierBuilder& VulkanBarrierBuilder::blockAccessTo(IBuffer& buffer, const ResourceAccess& access)
{
	m_impl->m_bufferBuilder.m_buffer = &buffer;
	m_impl->m_bufferBuilder.m_accessAfter = access;
	m_impl->m_bufferBuilder.m_subresource = 0;
	return m_impl->m_bufferBuilder;
}

VulkanBarrierBuilder::BufferBarrierBuilder& VulkanBarrierBuilder::blockAccessTo(IBuffer& buffer, const UInt32 subresource, const ResourceAccess& access)
{
	m_impl->m_bufferBuilder.m_buffer = &buffer;
	m_impl->m_bufferBuilder.m_accessAfter = access;
	m_impl->m_bufferBuilder.m_subresource = subresource;
	return m_impl->m_bufferBuilder;
}

VulkanBarrierBuilder::ImageBarrierBuilder& VulkanBarrierBuilder::blockAccessTo(IImage& image, const ResourceAccess& access)
{
	m_impl->m_imageLayoutBuilder.m_image = &image;
	m_impl->m_imageLayoutBuilder.m_accessAfter = access;
	m_impl->m_imageLayoutBuilder.setSubresource();
	return m_impl->m_imageBuilder;
}
#endif // defined(BUILD_DEFINE_BUILDERS)