#include <litefx/backends/vulkan.hpp>
#include <array>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBufferPool::VulkanBufferPoolImpl : public Implement<VulkanBufferPool> {
public:
    friend class VulkanBufferPool;

public:
    VulkanBufferPoolImpl(VulkanBufferPool* parent) : base(parent) { }

    ~VulkanBufferPoolImpl()
    {
    }

public:
    VkDescriptorPool initialize(const VulkanBufferSet& bufferSet)
    {
        throw;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBufferPool::VulkanBufferPool(const VulkanBufferSet& bufferSet) :
    m_impl(makePimpl<VulkanBufferPoolImpl>(this)), RuntimeObject(bufferSet.getDevice()), IResource<VkDescriptorSetLayout>(nullptr)
{
    this->handle() = m_impl->initialize(bufferSet);
}

VulkanBufferPool::~VulkanBufferPool() noexcept = default;

IBuffer* VulkanBufferPool::getBuffer(const UInt32& binding) const noexcept
{
    throw;
}