#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;
using TriangleMesh  = IBottomLevelAccelerationStructure::TriangleMesh;
using BoundingBoxes = IBottomLevelAccelerationStructure::BoundingBoxes;

extern PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructure;
extern PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructure;
extern PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresProperties;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBottomLevelAccelerationStructure::VulkanBottomLevelAccelerationStructureImpl : public Implement<VulkanBottomLevelAccelerationStructure> {
public:
    friend class VulkanBottomLevelAccelerationStructure;

private:
    Array<TriangleMesh>  m_triangleMeshes { };
    Array<BoundingBoxes> m_boundingBoxes  { };
    AccelerationStructureFlags m_flags;
    SharedPtr<const IVulkanBuffer> m_buffer;
    UInt64 m_offset { }, m_size { };
    const VulkanDevice* m_device { nullptr };
    VkQueryPool m_queryPool { VK_NULL_HANDLE };

public:
    VulkanBottomLevelAccelerationStructureImpl(VulkanBottomLevelAccelerationStructure* parent, AccelerationStructureFlags flags) :
        base(parent), m_flags(flags)
    {
        if (LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastBuild) && LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastTrace)) [[unlikely]]
            throw InvalidArgumentException("flags", "Cannot combine acceleration structure flags `PreferFastBuild` and `PreferFastTrace`.");
    }

public:
    Array<std::pair<UInt32, VkAccelerationStructureGeometryKHR>> build() const
    {
        return [this]() -> std::generator<std::pair<UInt32, VkAccelerationStructureGeometryKHR>> {
            // Build up mesh descriptions.
            for (auto& mesh : m_triangleMeshes)
            {
                // Find the position attribute.
                auto attributes = mesh.VertexBuffer->layout().attributes();
                auto positionAttribute = std::ranges::find_if(attributes, [](const BufferAttribute* attribute) { return attribute->semantic() == AttributeSemantic::Position; });

                if (positionAttribute == attributes.end()) [[unlikely]]
                    throw RuntimeException("A vertex buffer must contain a position attribute to be used in a bottom-level acceleration structure.");

                if ((*positionAttribute)->offset() != 0) [[unlikely]]
                    throw RuntimeException("The position attribute must not have a non-zero offset in the vertex buffer layout.");

                UInt32 primitiveCount = mesh.IndexBuffer == nullptr ? mesh.VertexBuffer->elements() / 3 : mesh.IndexBuffer->elements() / 3;

                co_yield std::make_pair(primitiveCount, VkAccelerationStructureGeometryKHR {
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                    .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
                    .geometry = {
                        .triangles = {
                            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                            .vertexFormat = Vk::getFormat((*positionAttribute)->format()),
                            .vertexData = {
                                .deviceAddress = mesh.VertexBuffer->virtualAddress(),
                            },
                            .vertexStride = mesh.VertexBuffer->alignedElementSize(),
                            .maxVertex = mesh.VertexBuffer->elements(),
                            .indexType = mesh.IndexBuffer == nullptr ? VK_INDEX_TYPE_NONE_KHR : (mesh.IndexBuffer->layout().indexType() == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32),
                            .indexData = {
                                .deviceAddress = mesh.IndexBuffer == nullptr ? 0 : mesh.IndexBuffer->virtualAddress(),
                            },
                            .transformData = {
                                .deviceAddress = mesh.TransformBuffer == nullptr ? 0 : mesh.TransformBuffer->virtualAddress()
                            },
                        }
                    },
                    .flags = std::bit_cast<VkGeometryFlagsKHR>(mesh.Flags)
                });
            }

            // Build up AABB descriptions.
            for (auto& bb : m_boundingBoxes)
            {
                if (bb.Buffer == nullptr) [[unlikely]]
                    throw RuntimeException("Cannot build bottom-level acceleration structure from uninitialized bounding boxes.");

                UInt32 primitiveCount = bb.Buffer->elements();

                co_yield std::make_pair(primitiveCount, VkAccelerationStructureGeometryKHR {
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                    .geometryType = VK_GEOMETRY_TYPE_AABBS_KHR,
                    .geometry = {
                        .aabbs = {
                            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
                            .data = {
                                .deviceAddress = bb.Buffer->virtualAddress(),
                            },
                            .stride = bb.Buffer->alignedElementSize()
                        }
                    },
                    .flags = std::bit_cast<VkGeometryFlagsKHR>(bb.Flags)
                });
            }
        }() | std::ranges::to<Array<std::pair<UInt32, VkAccelerationStructureGeometryKHR>>>();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBottomLevelAccelerationStructure::VulkanBottomLevelAccelerationStructure(AccelerationStructureFlags flags, StringView name) :
    m_impl(makePimpl<VulkanBottomLevelAccelerationStructureImpl>(this, flags)), Resource(VK_NULL_HANDLE), StateResource(name)
{
}

VulkanBottomLevelAccelerationStructure::~VulkanBottomLevelAccelerationStructure() noexcept
{
    if (this->handle() != VK_NULL_HANDLE)
        ::vkDestroyAccelerationStructure(m_impl->m_device->handle(), this->handle(), nullptr);

    if (m_impl->m_queryPool != VK_NULL_HANDLE)
        ::vkDestroyQueryPool(m_impl->m_device->handle(), m_impl->m_queryPool, nullptr);
}

AccelerationStructureFlags VulkanBottomLevelAccelerationStructure::flags() const noexcept
{
    return m_impl->m_flags;
}

SharedPtr<const IVulkanBuffer> VulkanBottomLevelAccelerationStructure::buffer() const noexcept
{
    return m_impl->m_buffer;
}

UInt64 VulkanBottomLevelAccelerationStructure::offset() const noexcept
{
    return m_impl->m_offset;
}

UInt64 VulkanBottomLevelAccelerationStructure::size() const noexcept
{
    UInt64 size = m_impl->m_size;
    VkResult res = VK_SUCCESS;

    // If compaction is enabled and the query pool is defined, check if there are query results.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction) && m_impl->m_queryPool != VK_NULL_HANDLE)
        res = ::vkGetQueryPoolResults(m_impl->m_device->handle(), m_impl->m_queryPool, 0, 1, sizeof(UInt64), &size, 0, VkQueryResultFlagBits::VK_QUERY_RESULT_64_BIT);

    return size;
}

void VulkanBottomLevelAccelerationStructure::build(const VulkanCommandBuffer& commandBuffer, SharedPtr<const IVulkanBuffer> scratchBuffer, SharedPtr<const IVulkanBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    // Validate the arguments.
    UInt64 requiredMemory, requiredScratchMemory;
    auto& device = static_cast<const VulkanQueue&>(commandBuffer.queue()).device();
    device.computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory);

    // NOTE: There appears to be no device limit for it, but rather this is a hard requirement.
    //       (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccelerationStructureCreateInfoKHR.html#VUID-VkAccelerationStructureCreateInfoKHR-offset-03734).
    if ((offset % 256) != 0) [[unlikely]]
        throw InvalidArgumentException("offset", "The offset must be aligned to {0} bytes", 256);

    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to build the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratchBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        buffer = m_impl->m_buffer && m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", std::make_pair(0uz, maxSize), requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0uz, buffer->size()), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // If the acceleration structure allows for compaction, create a query pool in order to query the compacted size later.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
    {
        if (m_impl->m_queryPool == VK_NULL_HANDLE)
        {
            VkQueryPoolCreateInfo queryPoolInfo = {
                .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                .queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
                .queryCount = 1
            };

            raiseIfFailed(::vkCreateQueryPool(device.handle(), &queryPoolInfo, nullptr, &m_impl->m_queryPool), "Unable to create query pool for compaction size queries.");
        }

        ::vkResetQueryPool(device.handle(), m_impl->m_queryPool, 0, 1);
    }

    // Perform the build.
    commandBuffer.buildAccelerationStructure(*this, scratchBuffer, *buffer, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = buffer;
    m_impl->m_size = requiredMemory;

    // Write out the acceleration structure properties to make the compacted size available.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
    {
        auto barrier = device.makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
        barrier->transition(*m_impl->m_buffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureRead);
        commandBuffer.barrier(*barrier);
        ::vkCmdWriteAccelerationStructuresProperties(commandBuffer.handle(), 1, &this->handle(), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, m_impl->m_queryPool, 0);
    }
}

void VulkanBottomLevelAccelerationStructure::update(const VulkanCommandBuffer& commandBuffer, SharedPtr<const IVulkanBuffer> scratchBuffer, SharedPtr<const IVulkanBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    // Validate the state.
    if (m_impl->m_buffer == nullptr) [[unlikely]]
        throw RuntimeException("The acceleration structure must have been built before it can be updated.");

    if (!LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowUpdate)) [[unlikely]]
        throw RuntimeException("The acceleration structure does not allow updates. Specify `AccelerationStructureFlags::AllowUpdate` during creation.");

    // Validate the arguments and create the buffers if required.
    UInt64 requiredMemory, requiredScratchMemory;
    auto& device = static_cast<const VulkanQueue&>(commandBuffer.queue()).device();
    device.computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory, true);

    // NOTE: There appears to be no device limit for it, but rather this is a hard requirement.
    //       (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccelerationStructureCreateInfoKHR.html#VUID-VkAccelerationStructureCreateInfoKHR-offset-03734).
    if ((offset % 256) != 0) [[unlikely]]
        throw InvalidArgumentException("offset", "The offset must be aligned to {0} bytes", 256);

    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to update the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratchBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        buffer = m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", std::make_pair(0uz, maxSize), requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0uz, buffer->size()), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // If the acceleration structure allows for compaction, reset the query pool.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
        ::vkResetQueryPool(device.handle(), m_impl->m_queryPool, 0, 1);

    // Perform the update.
    commandBuffer.updateAccelerationStructure(*this, scratchBuffer, *buffer, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = buffer;
    m_impl->m_size = requiredMemory;

    // Write out the acceleration structure properties to make the compacted size available.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
    {
        auto barrier = device.makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
        barrier->transition(*m_impl->m_buffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureRead);
        commandBuffer.barrier(*barrier);
        ::vkCmdWriteAccelerationStructuresProperties(commandBuffer.handle(), 1, &this->handle(), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, m_impl->m_queryPool, 0);
    }
}

void VulkanBottomLevelAccelerationStructure::copy(const VulkanCommandBuffer& commandBuffer, VulkanBottomLevelAccelerationStructure& destination, bool compress, SharedPtr<const IVulkanBuffer> buffer, UInt64 offset, bool copyBuildInfo) const
{
    // Validate the state.
    if (m_impl->m_buffer == nullptr) [[unlikely]]
        throw RuntimeException("The acceleration structure must have been built before it can be updated.");

    if (compress && !LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction)) [[unlikely]]
        throw RuntimeException("The acceleration structure does not allow compaction. Specify `AccelerationStructureFlags::AllowCompaction` during creation.");

    // NOTE: There appears to be no device limit for it, but rather this is a hard requirement.
    //       (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccelerationStructureCreateInfoKHR.html#VUID-VkAccelerationStructureCreateInfoKHR-offset-03734).
    if ((offset % 256) != 0) [[unlikely]]
        throw InvalidArgumentException("offset", "The offset must be aligned to {0} bytes", 256);

    // Query the compacted size, if compression is required, or use the device for requests as usual if not.
    std::array<UInt64, 2> requiredMemory;
    auto& device = static_cast<const VulkanQueue&>(commandBuffer.queue()).device();

    if (!LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
        device.computeAccelerationStructureSizes(*this, requiredMemory[0], requiredMemory[1], true);
    else
        raiseIfFailed(::vkGetQueryPoolResults(device.handle(), m_impl->m_queryPool, 0, 1, sizeof(UInt64), &requiredMemory, 0, VkQueryResultFlagBits::VK_QUERY_RESULT_64_BIT), "Unable to query for compressed acceleration structure size.");

    // Validate the input arguments.
    if (buffer == nullptr)
        buffer = destination.m_impl->m_buffer->size() >= requiredMemory[0] ? destination.m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory[0], 1, ResourceUsage::AllowWrite);
    else if (buffer->size() < offset + requiredMemory[0]) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0uz, buffer->size()), offset + requiredMemory[0], "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // Create or reset query pool on destination, if required. 
    if (LITEFX_FLAG_IS_SET(destination.flags(), AccelerationStructureFlags::AllowCompaction))
    {
        // TODO: This will cause issues when copying between devices, but this is currently not supported anyway.
        if (destination.m_impl->m_queryPool == VK_NULL_HANDLE)
        {
            VkQueryPoolCreateInfo queryPoolInfo = {
                .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                .queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
                .queryCount = 1
            };

            raiseIfFailed(::vkCreateQueryPool(device.handle(), &queryPoolInfo, nullptr, &destination.m_impl->m_queryPool), "Unable to create query pool for compaction size queries.");
        }

        ::vkResetQueryPool(device.handle(), destination.m_impl->m_queryPool, 0, 1);
    }

    // (Re-)create the destination handle, if the buffer changed.
    if (destination.handle() != VK_NULL_HANDLE)
        ::vkDestroyAccelerationStructure(device.handle(), destination.handle(), nullptr);

    VkAccelerationStructureCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .buffer = buffer->handle(),
        .offset = offset,
        .size = requiredMemory[0],
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
    };

    raiseIfFailed(::vkCreateAccelerationStructure(device.handle(), &info, nullptr, &destination.handle()), "Unable to update destination acceleration structure handle.");

    // Store the buffer and the offset.
    destination.m_impl->m_offset = offset;
    destination.m_impl->m_buffer = buffer;
    destination.m_impl->m_size = requiredMemory[0];
    destination.m_impl->m_device = m_impl->m_device;

    // Perform the update.
    commandBuffer.copyAccelerationStructure(*this, destination, compress);

    // Write out the acceleration structure properties to make the compacted size available.
    if (LITEFX_FLAG_IS_SET(destination.flags(), AccelerationStructureFlags::AllowCompaction))
    {
        auto barrier = device.makeBarrier(PipelineStage::AccelerationStructureCopy, PipelineStage::AccelerationStructureCopy);
        barrier->transition(*destination.m_impl->m_buffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureRead);
        commandBuffer.barrier(*barrier);
        ::vkCmdWriteAccelerationStructuresProperties(commandBuffer.handle(), 1, &destination.handle(), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, destination.m_impl->m_queryPool, 0);
    }

    // Copy build data, if requested.
    if (copyBuildInfo)
    {
        destination.m_impl->m_boundingBoxes = m_impl->m_boundingBoxes;
        destination.m_impl->m_triangleMeshes = m_impl->m_triangleMeshes;
    }
}

const Array<TriangleMesh>& VulkanBottomLevelAccelerationStructure::triangleMeshes() const noexcept
{
    return m_impl->m_triangleMeshes;
}

void VulkanBottomLevelAccelerationStructure::addTriangleMesh(const TriangleMesh& mesh)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("An acceleration structure cannot be modified after buffers for it have been created.");

    if (!m_impl->m_boundingBoxes.empty()) [[unlikely]]
        throw RuntimeException("A bottom-level acceleration structure can only contain either bounding boxes or triangle meshes, but not both at the same time.");

    m_impl->m_triangleMeshes.push_back(mesh);
}

const Array<BoundingBoxes>& VulkanBottomLevelAccelerationStructure::boundingBoxes() const noexcept
{
    return m_impl->m_boundingBoxes;
}

void VulkanBottomLevelAccelerationStructure::addBoundingBox(const BoundingBoxes& aabb)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("An acceleration structure cannot be modified after buffers for it have been created.");

    if (!m_impl->m_triangleMeshes.empty()) [[unlikely]]
        throw RuntimeException("A bottom-level acceleration structure can only contain either bounding boxes or triangle meshes, but not both at the same time.");

    m_impl->m_boundingBoxes.push_back(aabb);
}

void VulkanBottomLevelAccelerationStructure::clear() noexcept
{
    m_impl->m_boundingBoxes.clear();
    m_impl->m_triangleMeshes.clear();
}

bool VulkanBottomLevelAccelerationStructure::remove(const TriangleMesh& mesh) noexcept
{
    if (auto match = std::ranges::find_if(m_impl->m_triangleMeshes, [&mesh](const auto& e) { return std::addressof(e) == std::addressof(mesh); }); match != m_impl->m_triangleMeshes.end())
    {
        m_impl->m_triangleMeshes.erase(match);
        return true;
    }

    return false;
}

bool VulkanBottomLevelAccelerationStructure::remove(const BoundingBoxes& aabb) noexcept
{
    if (auto match = std::ranges::find_if(m_impl->m_boundingBoxes, [&aabb](const auto& e) { return std::addressof(e) == std::addressof(aabb); }); match != m_impl->m_boundingBoxes.end())
    {
        m_impl->m_boundingBoxes.erase(match);
        return true;
    }

    return false;
}

Array<std::pair<UInt32, VkAccelerationStructureGeometryKHR>> VulkanBottomLevelAccelerationStructure::buildInfo() const
{
    return m_impl->build();
}

void VulkanBottomLevelAccelerationStructure::updateState(const VulkanDevice* device, VkAccelerationStructureKHR handle) noexcept
{
    if (this->handle() != VK_NULL_HANDLE)
        ::vkDestroyAccelerationStructure(m_impl->m_device->handle(), handle, nullptr);

    m_impl->m_device = device;
    this->handle() = handle;
}

SharedPtr<const IBuffer> VulkanBottomLevelAccelerationStructure::getBuffer() const noexcept
{
    return std::static_pointer_cast<const IBuffer>(m_impl->m_buffer);
}

void VulkanBottomLevelAccelerationStructure::doBuild(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    this->build(dynamic_cast<const VulkanCommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(scratchBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(buffer), offset, maxSize);
}

void VulkanBottomLevelAccelerationStructure::doUpdate(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    this->update(dynamic_cast<const VulkanCommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(scratchBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(buffer), offset, maxSize);
}

void VulkanBottomLevelAccelerationStructure::doCopy(const ICommandBuffer& commandBuffer, IBottomLevelAccelerationStructure& destination, bool compress, SharedPtr<const IBuffer> buffer, UInt64 offset, bool copyBuildInfo) const
{
    this->copy(dynamic_cast<const VulkanCommandBuffer&>(commandBuffer), dynamic_cast<VulkanBottomLevelAccelerationStructure&>(destination), compress, std::dynamic_pointer_cast<const IVulkanBuffer>(buffer), offset, copyBuildInfo);
}