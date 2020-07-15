#include <litefx/backends/vulkan.hpp>
#include <array>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBufferPool::VulkanBufferPoolImpl : public Implement<VulkanBufferPool> {
public:
    friend class VulkanBufferPool;

private:
    Array<UniquePtr<IBuffer>> m_buffers;
    VkDescriptorSet m_descriptorSet;
    BufferUsage m_usage;

public:
    VulkanBufferPoolImpl(VulkanBufferPool* parent, const BufferUsage& usage) : base(parent), m_usage(usage) { }

public:
    VkDescriptorPool initialize(const VulkanBufferSet& bufferSet)
    {
        auto poolSizes = bufferSet.getPoolSizes();
        auto layouts = bufferSet.getLayouts();

        LITEFX_TRACE(VULKAN_LOG, "Allocating buffer pool {{ Uniforms: {0}, Storages: {1}, Samplers: {2} }}...", poolSizes[0].descriptorCount, poolSizes[1].descriptorCount, poolSizes[2].descriptorCount);

        // Remove pool sizes with no descriptors to be compatible with the specs.
        poolSizes.erase(std::remove_if(std::begin(poolSizes), std::end(poolSizes), [](const VkDescriptorPoolSize& s) { return s.descriptorCount == 0; }), std::end(poolSizes));
        
        // Create a descriptor pool.
        // NOTE: Currently we only support one set to be created per pool. This makes managing allocation counts easier, since fragmentation is handled by the
        //       driver. However it can possibly be more efficient to create a pool for multiple buffer sets, and use this class as a part of a ring-buffer or
        //       something similar.
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;

        VkDescriptorPool descriptorPool;

        if (::vkCreateDescriptorPool(m_parent->getDevice()->handle(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("Unable to create buffer pool.");

        // Allocate the descriptor sets.
        VkDescriptorSetAllocateInfo descriptorSetInfo = {};
        descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetInfo.descriptorPool = descriptorPool;
        descriptorSetInfo.descriptorSetCount = 1;
        descriptorSetInfo.pSetLayouts = &bufferSet.handle();

        if (::vkAllocateDescriptorSets(m_parent->getDevice()->handle(), &descriptorSetInfo, &m_descriptorSet) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate descriptor sets.");

        // Allocate buffers.
        m_buffers.resize(layouts.size());
        Array<VkDescriptorBufferInfo> bufferInfos;
        Array<VkWriteDescriptorSet> descriptorWrites;

        std::generate(std::begin(m_buffers), std::end(m_buffers), [&, i = 0]() mutable {
            auto layout = layouts[i++];

            // Create the buffer.
            // TODO: Support object arrays (i.e. buffers with more than 1 element).
            auto deviceBuffer = m_parent->getDevice()->createBuffer(layout, m_usage, 1);
            auto buffer = dynamic_cast<const VulkanBuffer*>(deviceBuffer.get());

            if (buffer == nullptr)
                throw std::runtime_error("The created buffer is not a valid Vulkan buffer.");

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = buffer->handle();
            bufferInfo.range = buffer->getSize();
            bufferInfo.offset = 0;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_descriptorSet;
            descriptorWrite.dstBinding = layout->getBinding();
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            switch (layout->getType())
            {
            case BufferType::Uniform:
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            default:
                throw std::runtime_error("Unsupported buffer type.");
            }

            bufferInfos.push_back(bufferInfo);
            descriptorWrites.push_back(descriptorWrite);

            return deviceBuffer;
        });

        // Update the descriptor set accordingly.
        ::vkUpdateDescriptorSets(m_parent->getDevice()->handle(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

        return descriptorPool;
    }

    IBuffer* getBuffer(const UInt32& binding) const
    {
        auto match = std::find_if(std::begin(m_buffers), std::end(m_buffers), [&](const UniquePtr<IBuffer>& buffer) { return buffer->getBinding() == binding; });
        return match == m_buffers.end() ? nullptr : match->get();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBufferPool::VulkanBufferPool(const VulkanBufferSet& bufferSet, const BufferUsage& usage) :
    m_impl(makePimpl<VulkanBufferPoolImpl>(this, usage)), VulkanRuntimeObject(bufferSet.getDevice()), IResource<VkDescriptorPool>(nullptr)
{
    this->handle() = m_impl->initialize(bufferSet);
}

VulkanBufferPool::~VulkanBufferPool() noexcept
{
    ::vkDestroyDescriptorPool(this->getDevice()->handle(), this->handle(), nullptr);
}

IBuffer* VulkanBufferPool::getBuffer(const UInt32& binding) const noexcept
{
    return m_impl->getBuffer(binding);
}

const BufferUsage& VulkanBufferPool::getUsage() const noexcept
{
    return m_impl->m_usage;
}

const VkDescriptorSet VulkanBufferPool::getDescriptorSet() const noexcept
{
    return m_impl->m_descriptorSet;
}