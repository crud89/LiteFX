#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

using BufferElement = std::tuple<IVulkanBuffer&, UInt32, ResourceState, ResourceState>;
using ImageElement = std::tuple<IVulkanImage&, UInt32, ResourceState, ResourceState>;
#define VK_RESOURCE_BARRIER_ALL_SUBRESOURCES std::numeric_limits<UInt32>::max()

// ------------------------------------------------------------------------------------------------
// Helper functions.
// ------------------------------------------------------------------------------------------------

static Dictionary<VkPipelineStageFlagBits, UInt32> VK_PIPELINE_STAGE_ORDER_MAP {
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,						0  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,						1  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,						2  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,						3  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,		4  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,	5  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,					6  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,					7  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,				8  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,				9  },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,	    	10 },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,		    		11 },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,				            12 },
    { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			    	13 },
};

constexpr const VkPipelineStageFlagBits& earliest(const VkPipelineStageFlagBits& a, const VkPipelineStageFlagBits& b) noexcept
{
    return VK_PIPELINE_STAGE_ORDER_MAP[a] < VK_PIPELINE_STAGE_ORDER_MAP[b] ? a : b;
}

constexpr const VkPipelineStageFlagBits& latest(const VkPipelineStageFlagBits& a, const VkPipelineStageFlagBits& b) noexcept
{
    return VK_PIPELINE_STAGE_ORDER_MAP[a] > VK_PIPELINE_STAGE_ORDER_MAP[b] ? a : b;
}

constexpr VkPipelineStageFlagBits getEarliestPossibleAccess(const ResourceState& state) noexcept
{
    switch (state)
    {
        case ResourceState::Common:             return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        case ResourceState::UniformBuffer:      return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
        case ResourceState::VertexBuffer:       return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        case ResourceState::IndexBuffer:        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        case ResourceState::GenericRead:        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        case ResourceState::ReadOnly:           return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        case ResourceState::ReadWrite:          return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        case ResourceState::CopySource:         return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
        case ResourceState::CopyDestination:    return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
        case ResourceState::RenderTarget:       return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        case ResourceState::DepthRead:          return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        case ResourceState::DepthWrite:         return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        case ResourceState::Present:            return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case ResourceState::ResolveSource:      return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case ResourceState::ResolveDestination: return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
}

constexpr VkPipelineStageFlagBits getLatestPossibleAccess(const ResourceState& state) noexcept
{
    switch (state)
    {
    case ResourceState::Common:             return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    case ResourceState::UniformBuffer:      return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case ResourceState::VertexBuffer:       return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case ResourceState::IndexBuffer:        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    case ResourceState::GenericRead:        return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case ResourceState::ReadOnly:           return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case ResourceState::ReadWrite:          return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case ResourceState::CopySource:         return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
    case ResourceState::CopyDestination:    return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
    case ResourceState::RenderTarget:       return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case ResourceState::DepthRead:          return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case ResourceState::DepthWrite:         return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case ResourceState::Present:            return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case ResourceState::ResolveSource:      return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case ResourceState::ResolveDestination: return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
}

inline VkPipelineStageFlagBits getEarliestStage(const VkPipelineStageFlagBits& currentStage, const ResourceState& state) noexcept
{
    return earliest(currentStage, getEarliestPossibleAccess(state));
}

inline VkPipelineStageFlagBits getLatestStage(const VkPipelineStageFlagBits& currentStage, const ResourceState& state) noexcept
{
    return latest(currentStage, getLatestPossibleAccess(state));
}

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBarrier::VulkanBarrierImpl : public Implement<VulkanBarrier> {
public:
    friend class VulkanBarrier;

private:
    Array<BufferElement> m_buffers;
    Array<ImageElement> m_images;
    Array<VkMemoryBarrier> m_waitBarriers;

public:
    VulkanBarrierImpl(VulkanBarrier* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBarrier::VulkanBarrier() noexcept :
    m_impl(makePimpl<VulkanBarrierImpl>(this))
{
}

VulkanBarrier::~VulkanBarrier() noexcept = default;

void VulkanBarrier::transition(IVulkanBuffer& buffer, const ResourceState& targetState)
{
    this->transition(buffer, buffer.state(0), targetState);
}

void VulkanBarrier::transition(IVulkanBuffer& buffer, const UInt32& element, const ResourceState& targetState)
{
    this->transition(buffer, buffer.state(element), element, targetState);
}

void VulkanBarrier::transition(IVulkanBuffer& buffer, const ResourceState& sourceState, const ResourceState& targetState)
{
    m_impl->m_buffers.push_back(BufferElement(buffer, VK_RESOURCE_BARRIER_ALL_SUBRESOURCES, sourceState, targetState));
}

void VulkanBarrier::transition(IVulkanBuffer& buffer, const ResourceState& sourceState, const UInt32& element, const ResourceState& targetState)
{
    m_impl->m_buffers.push_back(BufferElement(buffer, element, sourceState, targetState));
}

void VulkanBarrier::transition(IVulkanImage& image, const ResourceState& targetState)
{
    this->transition(image, image.state(0), targetState);
}

void VulkanBarrier::transition(IVulkanImage& image, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState)
{
    this->transition(image, image.state(image.subresourceId(level, layer, plane)), level, layer, plane, targetState);
}

void VulkanBarrier::transition(IVulkanImage& image, const ResourceState& sourceState, const ResourceState& targetState)
{
    m_impl->m_images.push_back(ImageElement(image, VK_RESOURCE_BARRIER_ALL_SUBRESOURCES, sourceState, targetState));
}

void VulkanBarrier::transition(IVulkanImage& image, const ResourceState& sourceState, const UInt32& level, const UInt32& layer, const UInt32& plane, const ResourceState& targetState)
{
    m_impl->m_images.push_back(ImageElement(image, image.subresourceId(level, layer, plane), sourceState, targetState));
}

void VulkanBarrier::waitFor(const IVulkanBuffer& buffer)
{
    // TODO: We could possible use a VkBufferMemoryBarrier with no layout transition for this instead of a global barrier. This should
    //       be equal to a default barrier, but with matching target and source states.
    m_impl->m_waitBarriers.push_back(VkMemoryBarrier {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = ::getAccessFlags(buffer.state(0)),
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
    });
}

void VulkanBarrier::waitFor(const IVulkanImage& image)
{
    // TODO: We could possible use a VkImageMemoryBarrier with no layout transition for this instead of a global barrier. This should
    //       be equal to a default barrier, but with matching target and source states.
    m_impl->m_waitBarriers.push_back(VkMemoryBarrier {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = ::getAccessFlags(image.state(0)),
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
    });
}

void VulkanBarrier::execute(const VulkanCommandBuffer& commandBuffer) const noexcept
{
    // Compute the pipeline stages along
    VkPipelineStageFlagBits lastStageToProduce = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, firstStageToConsume = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    // If there are wait-barriers, we need to assume the proper stages. The last stage to write to a resource might be the compute shader, whilst the first to read will be the vertex shader.
    // TODO: We might be able to improve this, by providing a way to explicitly specify the pipeline stages.
    if (!m_impl->m_waitBarriers.empty())
    {
        lastStageToProduce = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        firstStageToConsume = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    // Create the buffer barriers.
    Array<VkBufferMemoryBarrier> bufferBarriers(m_impl->m_buffers.size());
    std::ranges::generate(bufferBarriers, [this, &lastStageToProduce, &firstStageToConsume, i = 0]() mutable {
        auto& bufferElement = m_impl->m_buffers[i++];
        auto& buffer = std::get<0>(bufferElement);
        auto& element = std::get<1>(bufferElement);
        auto& sourceState = std::get<2>(bufferElement);
        lastStageToProduce = getLatestStage(lastStageToProduce, sourceState);
        auto& targetState = std::get<3>(bufferElement);
        firstStageToConsume = getEarliestStage(firstStageToConsume, targetState);
        UInt32 offset = 0, size = buffer.size();

        if (element == VK_RESOURCE_BARRIER_ALL_SUBRESOURCES)
        {
            for (UInt32 e(0); e < buffer.elements(); ++e)
                buffer.state(e) = targetState;
        }
        else
        {
            offset = buffer.alignedElementSize() * element;
            size = buffer.elementAlignment();
            buffer.state(element) = targetState;
        }

        return VkBufferMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = ::getAccessFlags(sourceState),
            .dstAccessMask = ::getAccessFlags(targetState),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = std::as_const(buffer).handle(),
            .offset = offset,
            .size = size
        };
    });

    // Create the image barriers.
    Array<VkImageMemoryBarrier> imageBarriers(m_impl->m_buffers.size());
    std::ranges::generate(imageBarriers, [this, &lastStageToProduce, &firstStageToConsume, i = 0]() mutable {
        auto& imageElement = m_impl->m_images[i++];
        auto& image = std::get<0>(imageElement);
        auto& subresource = std::get<1>(imageElement);
        auto& sourceState = std::get<2>(imageElement);
        lastStageToProduce = getLatestStage(lastStageToProduce, sourceState);
        auto& targetState = std::get<3>(imageElement);
        firstStageToConsume = getEarliestStage(firstStageToConsume, targetState);

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        UInt32 layer = 0, level = 0, plane = 0;

        if (subresource == VK_RESOURCE_BARRIER_ALL_SUBRESOURCES)
        {
            // Get the image aspect mask for all planes.
            aspectMask = image.aspectMask();

            // Update the image target states.
            for (UInt32 e(0); e < image.elements(); ++e)
                image.state(e) = targetState;
        }
        else
        {
            // Resolve the sub-resource.
            image.resolveSubresource(subresource, plane, layer, level);

            // Get the image aspect mask for the plane.
            aspectMask = image.aspectMask(plane);

            // Update the sub-resource target state.
            image.state(subresource) = targetState;
        }

        return VkImageMemoryBarrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = ::getAccessFlags(sourceState),
            .dstAccessMask = ::getAccessFlags(targetState),
            .oldLayout = ::getImageLayout(sourceState),
            .newLayout = ::getImageLayout(targetState),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = std::as_const(image).handle(),
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = aspectMask,
                .baseMipLevel = level,
                .levelCount = subresource == VK_RESOURCE_BARRIER_ALL_SUBRESOURCES ? image.levels() : 1,
                .baseArrayLayer = layer,
                .layerCount = subresource == VK_RESOURCE_BARRIER_ALL_SUBRESOURCES ? image.layers() : 1
            }
        };
    });

    // Execute the barriers.
    ::vkCmdPipelineBarrier(commandBuffer.handle(), lastStageToProduce, firstStageToConsume, 0, static_cast<UInt32>(m_impl->m_waitBarriers.size()), m_impl->m_waitBarriers.data(), static_cast<UInt32>(bufferBarriers.size()), bufferBarriers.data(), static_cast<UInt32>(imageBarriers.size()), imageBarriers.data());
}

void VulkanBarrier::executeInverse(const VulkanCommandBuffer& commandBuffer) const noexcept
{
    // Compute the pipeline stages along
    VkPipelineStageFlagBits lastStageToProduce = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, firstStageToConsume = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    // If there are wait-barriers, we need to assume the proper stages. The last stage to write to a resource might be the compute shader, whilst the first to read will be the vertex shader.
    // TODO: We might be able to improve this, by providing a way to explicitly specify the pipeline stages.
    if (!m_impl->m_waitBarriers.empty())
    {
        lastStageToProduce = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        firstStageToConsume = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    // Create the buffer barriers.
    Array<VkBufferMemoryBarrier> bufferBarriers(m_impl->m_buffers.size());
    std::ranges::generate(bufferBarriers, [this, &lastStageToProduce, &firstStageToConsume, i = 0]() mutable {
        auto& bufferElement = m_impl->m_buffers[i++];
        auto& buffer = std::get<0>(bufferElement);
        auto& element = std::get<1>(bufferElement);
        auto& sourceState = std::get<3>(bufferElement);
        lastStageToProduce = getLatestStage(lastStageToProduce, sourceState);
        auto& targetState = std::get<2>(bufferElement);
        firstStageToConsume = getEarliestStage(firstStageToConsume, targetState);
        UInt32 offset = 0, size = buffer.size();

        if (element == VK_RESOURCE_BARRIER_ALL_SUBRESOURCES)
        {
            for (UInt32 e(0); e < buffer.elements(); ++e)
                buffer.state(e) = targetState;
        }
        else
        {
            offset = buffer.alignedElementSize() * element;
            size = buffer.elementAlignment();
            buffer.state(element) = targetState;
        }

        return VkBufferMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = ::getAccessFlags(sourceState),
            .dstAccessMask = ::getAccessFlags(targetState),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = std::as_const(buffer).handle(),
            .offset = offset,
            .size = size
        };
    });

    // Create the image barriers.
    Array<VkImageMemoryBarrier> imageBarriers(m_impl->m_buffers.size());
    std::ranges::generate(imageBarriers, [this, &lastStageToProduce, &firstStageToConsume, i = 0]() mutable {
        auto& imageElement = m_impl->m_images[i++];
        auto& image = std::get<0>(imageElement);
        auto& subresource = std::get<1>(imageElement);
        auto& sourceState = std::get<3>(imageElement);
        lastStageToProduce = getLatestStage(lastStageToProduce, sourceState);
        auto& targetState = std::get<2>(imageElement);
        firstStageToConsume = getEarliestStage(firstStageToConsume, targetState);

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        UInt32 layer = 0, level = 0, plane = 0;

        if (subresource == VK_RESOURCE_BARRIER_ALL_SUBRESOURCES)
        {
            // Get the image aspect mask for all planes.
            aspectMask = image.aspectMask();

            // Update the image target states.
            for (UInt32 e(0); e < image.elements(); ++e)
                image.state(e) = targetState;
        }
        else
        {
            // Resolve the sub-resource.
            image.resolveSubresource(subresource, plane, layer, level);

            // Get the image aspect mask for the plane.
            aspectMask = image.aspectMask(plane);

            // Update the sub-resource target state.
            image.state(subresource) = targetState;
        }

        return VkImageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = ::getAccessFlags(sourceState),
            .dstAccessMask = ::getAccessFlags(targetState),
            .oldLayout = ::getImageLayout(sourceState),
            .newLayout = ::getImageLayout(targetState),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = std::as_const(image).handle(),
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = aspectMask,
                .baseMipLevel = level,
                .levelCount = subresource == VK_RESOURCE_BARRIER_ALL_SUBRESOURCES ? image.levels() : 1,
                .baseArrayLayer = layer,
                .layerCount = subresource == VK_RESOURCE_BARRIER_ALL_SUBRESOURCES ? image.layers() : 1
            }
        };
    });

    // Execute the barriers.
    ::vkCmdPipelineBarrier(commandBuffer.handle(), lastStageToProduce, firstStageToConsume, 0, static_cast<UInt32>(m_impl->m_waitBarriers.size()), m_impl->m_waitBarriers.data(), static_cast<UInt32>(bufferBarriers.size()), bufferBarriers.data(), static_cast<UInt32>(imageBarriers.size()), imageBarriers.data());
}