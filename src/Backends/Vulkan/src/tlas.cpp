#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;
using Instance = ITopLevelAccelerationStructure::Instance;

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
extern PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructure;
extern PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructure;
extern PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresProperties;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructureImpl {
public:
    friend class VulkanTopLevelAccelerationStructure;

private:
    Array<Instance> m_instances { };
    AccelerationStructureFlags m_flags;
    SharedPtr<const IVulkanBuffer> m_buffer;
    UInt64 m_offset { }, m_size { };
    const VulkanDevice* m_device { nullptr };
    VkQueryPool m_queryPool { VK_NULL_HANDLE };

public:
    VulkanTopLevelAccelerationStructureImpl(AccelerationStructureFlags flags) :
        m_flags(flags)
    {
        if (LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastBuild) && LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastTrace)) [[unlikely]]
            throw InvalidArgumentException("flags", "Cannot combine acceleration structure flags `PreferFastBuild` and `PreferFastTrace`.");
    }

public:
    Array<VkAccelerationStructureInstanceKHR> buildInfo() const
    {
        return m_instances | std::views::transform([](const Instance& instance) { 
            const auto& blasBuffer = instance.BottomLevelAccelerationStructure->buffer();

            auto desc = VkAccelerationStructureInstanceKHR {
                .instanceCustomIndex = instance.Id,
                .mask = instance.Mask,
                .instanceShaderBindingTableRecordOffset = instance.HitGroupOffset,
                .flags = std::bit_cast<VkGeometryInstanceFlagsKHR>(instance.Flags),
                .accelerationStructureReference = blasBuffer == nullptr ? 0ull : blasBuffer->virtualAddress() + instance.BottomLevelAccelerationStructure->offset()
            };

            std::memcpy(desc.transform.matrix, std::addressof(instance.Transform), sizeof(instance.Transform)); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            return desc;
        }) | std::ranges::to<Array<VkAccelerationStructureInstanceKHR>>();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructure(AccelerationStructureFlags flags, StringView name) :
    StateResource(name), Resource(VK_NULL_HANDLE), m_impl(flags)
{
}

VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructure(VulkanTopLevelAccelerationStructure&&) noexcept = default;
VulkanTopLevelAccelerationStructure& VulkanTopLevelAccelerationStructure::operator=(VulkanTopLevelAccelerationStructure&&) noexcept = default;

VulkanTopLevelAccelerationStructure::~VulkanTopLevelAccelerationStructure() noexcept
{
    if (this->handle() != VK_NULL_HANDLE)
        ::vkDestroyAccelerationStructure(m_impl->m_device->handle(), this->handle(), nullptr);

    if (m_impl->m_queryPool != VK_NULL_HANDLE)
        ::vkDestroyQueryPool(m_impl->m_device->handle(), m_impl->m_queryPool, nullptr);
}

AccelerationStructureFlags VulkanTopLevelAccelerationStructure::flags() const noexcept
{
    return m_impl->m_flags;
}

SharedPtr<const IVulkanBuffer> VulkanTopLevelAccelerationStructure::buffer() const noexcept
{
    return m_impl->m_buffer;
}

UInt64 VulkanTopLevelAccelerationStructure::offset() const noexcept
{
    return m_impl->m_offset;
}

UInt64 VulkanTopLevelAccelerationStructure::size() const noexcept
{
    UInt64 size = m_impl->m_size;

    // If compaction is enabled and the query pool is defined, check if there are query results.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction) && m_impl->m_queryPool != VK_NULL_HANDLE)
        ::vkGetQueryPoolResults(m_impl->m_device->handle(), m_impl->m_queryPool, 0, 1, sizeof(UInt64), &size, 0, VkQueryResultFlagBits::VK_QUERY_RESULT_64_BIT);

    return size;
}

void VulkanTopLevelAccelerationStructure::build(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const SharedPtr<const IVulkanBuffer>& buffer, UInt64 offset, UInt64 maxSize)
{
    // Validate the arguments.
    UInt64 requiredMemory{}, requiredScratchMemory{};
    auto device = commandBuffer.queue()->device();
    device->computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory);

    auto scratch = scratchBuffer;
    auto memory = buffer;

    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to build the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratch = device->factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        memory = m_impl->m_buffer != nullptr && m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device->factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", std::make_pair(0_ui64, maxSize), requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
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

            raiseIfFailed(::vkCreateQueryPool(device->handle(), &queryPoolInfo, nullptr, &m_impl->m_queryPool), "Unable to create query pool for compaction size queries.");
        }

        ::vkResetQueryPool(device->handle(), m_impl->m_queryPool, 0, 1);
    }

    // Perform the build.
    commandBuffer.buildAccelerationStructure(*this, scratch, *memory, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = memory;
    m_impl->m_size = requiredMemory;

    // Write out the acceleration structure properties to make the compacted size available.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
    {
        auto barrier = device->makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
        barrier->transition(*m_impl->m_buffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureRead);
        commandBuffer.barrier(*barrier);
        ::vkCmdWriteAccelerationStructuresProperties(commandBuffer.handle(), 1, &this->handle(), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, m_impl->m_queryPool, 0);
    }
}

void VulkanTopLevelAccelerationStructure::update(const VulkanCommandBuffer& commandBuffer, const SharedPtr<const IVulkanBuffer>& scratchBuffer, const SharedPtr<const IVulkanBuffer>& buffer, UInt64 offset, UInt64 maxSize)
{
    // Validate the state.
    if (m_impl->m_buffer == nullptr) [[unlikely]]
        throw RuntimeException("The acceleration structure must have been built before it can be updated.");

    if (!LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowUpdate)) [[unlikely]]
        throw RuntimeException("The acceleration structure does not allow updates. Specify `AccelerationStructureFlags::AllowUpdate` during creation.");

    // Validate the arguments and create the buffers if required.
    UInt64 requiredMemory{}, requiredScratchMemory{};
    auto device = commandBuffer.queue()->device();
    device->computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory, true);
    auto scratch = scratchBuffer;
    auto memory = buffer;

    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to update the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratch = device->factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        memory = m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device->factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", std::make_pair(0_ui64, maxSize), requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", std::make_pair(0uz, buffer->size()), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // If the acceleration structure allows for compaction, reset the query pool.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
        ::vkResetQueryPool(device->handle(), m_impl->m_queryPool, 0, 1);

    // Perform the update.
    commandBuffer.updateAccelerationStructure(*this, scratch, *memory, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = memory;
    m_impl->m_size = requiredMemory;

    // Write out the acceleration structure properties to make the compacted size available.
    if (LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
    {
        auto barrier = device->makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
        barrier->transition(*m_impl->m_buffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureRead);
        commandBuffer.barrier(*barrier);
        ::vkCmdWriteAccelerationStructuresProperties(commandBuffer.handle(), 1, &this->handle(), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, m_impl->m_queryPool, 0);
    }
}

void VulkanTopLevelAccelerationStructure::copy(const VulkanCommandBuffer& commandBuffer, VulkanTopLevelAccelerationStructure& destination, bool compress, const SharedPtr<const IVulkanBuffer>& buffer, UInt64 offset, bool copyBuildInfo) const
{
    // Validate the state.
    if (m_impl->m_buffer == nullptr) [[unlikely]]
        throw RuntimeException("The acceleration structure must have been built before it can be updated.");

    if (compress && !LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction)) [[unlikely]]
        throw RuntimeException("The acceleration structure does not allow compaction. Specify `AccelerationStructureFlags::AllowCompaction` during creation.");

    // Query the compacted size, if compression is required, or use the device for requests as usual if not.
    std::array<UInt64, 2> requiredMemory{};
    auto device = commandBuffer.queue()->device();

    if (!LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowCompaction))
        device->computeAccelerationStructureSizes(*this, requiredMemory[0], requiredMemory[1], true);
    else
        raiseIfFailed(::vkGetQueryPoolResults(device->handle(), m_impl->m_queryPool, 0, 1, sizeof(UInt64), &requiredMemory, 0, VkQueryResultFlagBits::VK_QUERY_RESULT_64_BIT), "Unable to query for compressed acceleration structure size.");

    // Validate the input arguments.
    auto memory = buffer;

    if (buffer == nullptr)
        memory = destination.m_impl->m_buffer->size() >= requiredMemory[0] ? destination.m_impl->m_buffer : device->factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory[0], 1, ResourceUsage::AllowWrite);
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

            raiseIfFailed(::vkCreateQueryPool(device->handle(), &queryPoolInfo, nullptr, &destination.m_impl->m_queryPool), "Unable to create query pool for compaction size queries.");
        }

        ::vkResetQueryPool(device->handle(), destination.m_impl->m_queryPool, 0, 1);
    }

    // (Re-)create the destination handle, if the buffer changed.
    if (destination.handle() != VK_NULL_HANDLE)
        ::vkDestroyAccelerationStructure(device->handle(), destination.handle(), nullptr);

    VkAccelerationStructureCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .buffer = memory->handle(),
        .offset = offset,
        .size = requiredMemory[0],
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
    };

    raiseIfFailed(::vkCreateAccelerationStructure(device->handle(), &info, nullptr, &destination.handle()), "Unable to update destination acceleration structure handle.");

    // Store the buffer and the offset.
    destination.m_impl->m_offset = offset;
    destination.m_impl->m_buffer = memory;
    destination.m_impl->m_size = requiredMemory[0];
    destination.m_impl->m_device = m_impl->m_device;

    // Perform the update.
    commandBuffer.copyAccelerationStructure(*this, destination, compress);

    // Write out the acceleration structure properties to make the compacted size available.
    if (LITEFX_FLAG_IS_SET(destination.flags(), AccelerationStructureFlags::AllowCompaction))
    {
        auto barrier = device->makeBarrier(PipelineStage::AccelerationStructureCopy, PipelineStage::AccelerationStructureCopy);
        barrier->transition(*destination.m_impl->m_buffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureRead);
        commandBuffer.barrier(*barrier);
        ::vkCmdWriteAccelerationStructuresProperties(commandBuffer.handle(), 1, &destination.handle(), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, destination.m_impl->m_queryPool, 0);
    }

    // Copy build data, if requested.
    if (copyBuildInfo)
        destination.m_impl->m_instances = m_impl->m_instances;
}

const Array<Instance>& VulkanTopLevelAccelerationStructure::instances() const noexcept
{
    return m_impl->m_instances;
}

void VulkanTopLevelAccelerationStructure::addInstance(const Instance& instance)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("An acceleration structure cannot be modified after buffers for it have been created.");

    m_impl->m_instances.push_back(instance);
}

void VulkanTopLevelAccelerationStructure::clear() noexcept
{
    m_impl->m_instances.clear();
}

bool VulkanTopLevelAccelerationStructure::remove(const Instance& instance) noexcept
{
    if (auto match = std::ranges::find_if(m_impl->m_instances, [&instance](const auto& e) { return std::addressof(e) == std::addressof(instance); }); match != m_impl->m_instances.end())
    {
        m_impl->m_instances.erase(match);
        return true;
    }

    return false;
}

Array<VkAccelerationStructureInstanceKHR> VulkanTopLevelAccelerationStructure::buildInfo() const
{
    return m_impl->buildInfo();
}

void VulkanTopLevelAccelerationStructure::updateState(const VulkanDevice* device, VkAccelerationStructureKHR handle) noexcept
{
    if (this->handle() != VK_NULL_HANDLE)
        ::vkDestroyAccelerationStructure(m_impl->m_device->handle(), handle, nullptr);

    m_impl->m_device = device;
    this->handle() = handle;
}

SharedPtr<const IBuffer> VulkanTopLevelAccelerationStructure::getBuffer() const noexcept
{
    return std::static_pointer_cast<const IBuffer>(m_impl->m_buffer);
}

void VulkanTopLevelAccelerationStructure::doBuild(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize)
{
    this->build(dynamic_cast<const VulkanCommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(scratchBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(buffer), offset, maxSize);
}

void VulkanTopLevelAccelerationStructure::doUpdate(const ICommandBuffer& commandBuffer, const SharedPtr<const IBuffer>& scratchBuffer, const SharedPtr<const IBuffer>& buffer, UInt64 offset, UInt64 maxSize)
{
    this->update(dynamic_cast<const VulkanCommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(scratchBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(buffer), offset, maxSize);
}

void VulkanTopLevelAccelerationStructure::doCopy(const ICommandBuffer& commandBuffer, ITopLevelAccelerationStructure& destination, bool compress, const SharedPtr<const IBuffer>& buffer, UInt64 offset, bool copyBuildInfo) const
{
    this->copy(dynamic_cast<const VulkanCommandBuffer&>(commandBuffer), dynamic_cast<VulkanTopLevelAccelerationStructure&>(destination), compress, std::dynamic_pointer_cast<const IVulkanBuffer>(buffer), offset, copyBuildInfo);
}