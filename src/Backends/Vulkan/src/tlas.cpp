#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;
using Instance = ITopLevelAccelerationStructure::Instance;

extern PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructure;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructureImpl : public Implement<VulkanTopLevelAccelerationStructure> {
public:
    friend class VulkanTopLevelAccelerationStructure;

private:
    Array<Instance> m_instances { };
    AccelerationStructureFlags m_flags;
    UniquePtr<IVulkanBuffer> m_buffer;
    UInt64 m_scratchBufferSize { };

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
            if (instance.BottomLevelAccelerationStructure->buffer() == nullptr) [[unlikely]]
                throw RuntimeException("The bottom-level acceleration structure for at least one instance has not yet been built.");

            return VkAccelerationStructureInstanceKHR {
                .transform = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f },
                .instanceCustomIndex = instance.Id,
                .mask = instance.Mask,
                .instanceShaderBindingTableRecordOffset = instance.HitGroup,
                .flags = std::bit_cast<VkGeometryInstanceFlagsKHR>(instance.Flags),
                .accelerationStructureReference = instance.BottomLevelAccelerationStructure->buffer()->virtualAddress()
            };
        }) | std::ranges::to<Array<VkAccelerationStructureInstanceKHR>>();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanTopLevelAccelerationStructure::VulkanTopLevelAccelerationStructure(AccelerationStructureFlags flags) :
    m_impl(makePimpl<VulkanTopLevelAccelerationStructureImpl>(this, flags)), Resource(VK_NULL_HANDLE)
{
}

VulkanTopLevelAccelerationStructure::~VulkanTopLevelAccelerationStructure() noexcept = default;

AccelerationStructureFlags VulkanTopLevelAccelerationStructure::flags() const noexcept
{
    return m_impl->m_flags;
}

UInt64 VulkanTopLevelAccelerationStructure::requiredScratchMemory() const noexcept 
{
    return m_impl->m_scratchBufferSize;
}

const IVulkanBuffer* VulkanTopLevelAccelerationStructure::buffer() const noexcept
{
    return m_impl->m_buffer.get();
}

void VulkanTopLevelAccelerationStructure::allocateBuffer(const VulkanDevice& device)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("The buffer for this acceleration structure has already been allocated.");

    // Compute buffer sizes.
    UInt64 bufferSize{ };
    device.computeAccelerationStructureSizes(*this, bufferSize, m_impl->m_scratchBufferSize);

    // Allocate the buffer.
    m_impl->m_buffer = device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, bufferSize, 1, ResourceUsage::AllowWrite);

    // Create a handle for the acceleration structure.
    VkAccelerationStructureCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .buffer = std::as_const(*m_impl->m_buffer).handle(),
        .size = m_impl->m_buffer->alignedElementSize(),
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
    };

    ::vkCreateAccelerationStructure(device.handle(), &info, nullptr, &this->handle());
}

const Array<Instance>& VulkanTopLevelAccelerationStructure::instances() const noexcept
{
    return m_impl->m_instances;
}

void VulkanTopLevelAccelerationStructure::addInstance(const Instance& instance)
{
    m_impl->m_instances.push_back(instance);
}

Array<VkAccelerationStructureInstanceKHR> VulkanTopLevelAccelerationStructure::buildInfo() const noexcept
{
    return m_impl->buildInfo();
}

void VulkanTopLevelAccelerationStructure::makeBuffer(const IGraphicsDevice& device)
{
    this->allocateBuffer(dynamic_cast<const VulkanDevice&>(device));
}