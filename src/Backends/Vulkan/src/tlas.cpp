#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;
using Instance = ITopLevelAccelerationStructure::Instance;

extern PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructure;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructureImpl : public Implement<VulkanTopLevelAccelerationStructure> {
public:
    friend class VulkanTopLevelAccelerationStructure;

private:
    Array<Instance> m_instances { };
    AccelerationStructureFlags m_flags;
    SharedPtr<const IVulkanBuffer> m_buffer;
    UInt64 m_offset { };
    const VulkanDevice* m_device { nullptr };

public:
    VulkanTopLevelAccelerationStructureImpl(VulkanTopLevelAccelerationStructure* parent, AccelerationStructureFlags flags) :
        base(parent), m_flags(flags)
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

            std::memcpy(desc.transform.matrix, instance.Transform.elements(), sizeof(Float) * 12);
            return desc;
        }) | std::ranges::to<Array<VkAccelerationStructureInstanceKHR>>();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructure(AccelerationStructureFlags flags, StringView name) :
    m_impl(makePimpl<VulkanTopLevelAccelerationStructureImpl>(this, flags)), Resource(VK_NULL_HANDLE), StateResource(name)
{
}

VulkanTopLevelAccelerationStructure::~VulkanTopLevelAccelerationStructure() noexcept
{
    if (this->handle() != VK_NULL_HANDLE)
        ::vkDestroyAccelerationStructure(m_impl->m_device->handle(), this->handle(), nullptr);
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

void VulkanTopLevelAccelerationStructure::build(const VulkanCommandBuffer& commandBuffer, SharedPtr<const IVulkanBuffer> scratchBuffer, SharedPtr<const IVulkanBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    // Validate the arguments.
    UInt64 requiredMemory, requiredScratchMemory;
    auto& device = static_cast<const VulkanQueue&>(commandBuffer.queue()).device();
    device.computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory);
    
    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to build the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratchBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        buffer = m_impl->m_buffer != nullptr && m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", 0ull, maxSize, requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", 0ull, buffer->size(), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // Perform the build.
    commandBuffer.buildAccelerationStructure(*this, scratchBuffer, *buffer, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = buffer;
}

void VulkanTopLevelAccelerationStructure::update(const VulkanCommandBuffer& commandBuffer, SharedPtr<const IVulkanBuffer> scratchBuffer, SharedPtr<const IVulkanBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    // Validate the state.
    if (m_impl->m_buffer == nullptr) [[unlikely]]
        throw RuntimeException("The acceleration structure must have been built before it can be updated.");

    if (!LITEFX_FLAG_IS_SET(m_impl->m_flags, AccelerationStructureFlags::AllowUpdate)) [[unlikely]]
        throw RuntimeException("The acceleration structure does not allow updates. Specify `AccelerationStructureFlags::AllowUpdate` during creation.");

    // Validate the arguments and create the buffers if required.
    UInt64 requiredMemory, requiredScratchMemory;
    auto& device = static_cast<const VulkanQueue&>(commandBuffer.queue()).device();
    device.computeAccelerationStructureSizes(*this, requiredMemory, requiredScratchMemory);

    if (scratchBuffer != nullptr && scratchBuffer->size() < requiredScratchMemory)
        throw InvalidArgumentException("scratchBuffer", "The provided scratch buffer does not contain enough memory to update the acceleration structure (contained memory: {0} bytes, required memory: {1} bytes).", scratchBuffer->size(), requiredScratchMemory);
    else if (scratchBuffer == nullptr)
        scratchBuffer = device.factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, requiredScratchMemory, 1, ResourceUsage::AllowWrite);

    if (buffer == nullptr)
        buffer = m_impl->m_buffer->size() >= requiredMemory ? m_impl->m_buffer : device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, requiredMemory, 1, ResourceUsage::AllowWrite);
    else if (maxSize < requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("maxSize", 0ull, maxSize, requiredMemory, "The maximum available size is not sufficient to contain the acceleration structure.");
    else if (buffer->size() < offset + requiredMemory) [[unlikely]]
        throw ArgumentOutOfRangeException("buffer", 0ull, buffer->size(), offset + requiredMemory, "The buffer does not contain enough memory after offset {0} to fully contain the acceleration structure.", offset);

    // Perform the update.
    commandBuffer.updateAccelerationStructure(*this, scratchBuffer, *buffer, offset);

    // Store the buffer and the offset.
    m_impl->m_offset = offset;
    m_impl->m_buffer = buffer;
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

Array<VkAccelerationStructureInstanceKHR> VulkanTopLevelAccelerationStructure::buildInfo() const noexcept
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

void VulkanTopLevelAccelerationStructure::doBuild(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    this->build(dynamic_cast<const VulkanCommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(scratchBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(buffer), offset, maxSize);
}

void VulkanTopLevelAccelerationStructure::doUpdate(const ICommandBuffer& commandBuffer, SharedPtr<const IBuffer> scratchBuffer, SharedPtr<const IBuffer> buffer, UInt64 offset, UInt64 maxSize)
{
    this->update(dynamic_cast<const VulkanCommandBuffer&>(commandBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(scratchBuffer), std::dynamic_pointer_cast<const IVulkanBuffer>(buffer), offset, maxSize);
}