#include <litefx/backends/vulkan.hpp>
#include <array>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBufferPool::VulkanBufferPoolImpl : public Implement<VulkanBufferPool> {
public:
    friend class VulkanBufferPool;

private:
    const IDescriptorSetLayout* m_layout{ nullptr };
    VkDescriptorSet m_descriptorSet;

public:
    VulkanBufferPoolImpl(VulkanBufferPool* parent) : base(parent) { }

public:
    VkDescriptorPool initialize(const VulkanDescriptorSetLayout& descriptorSetLayout)
    {
        Array<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,   descriptorSetLayout.uniforms() },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,   descriptorSetLayout.storages() },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,    descriptorSetLayout.images() },
            { VK_DESCRIPTOR_TYPE_SAMPLER,          descriptorSetLayout.samplers() },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, descriptorSetLayout.inputAttachments() }
        };

        auto descriptorLayouts = descriptorSetLayout.getLayouts();

        LITEFX_TRACE(VULKAN_LOG, "Allocating buffer pool {{ Uniforms: {0}, Storages: {1}, Images: {2}, Samplers: {3}, Input attachments: {4} }}...", poolSizes[0].descriptorCount, poolSizes[1].descriptorCount, poolSizes[2].descriptorCount, poolSizes[3].descriptorCount, poolSizes[4].descriptorCount);

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
        descriptorSetInfo.pSetLayouts = &descriptorSetLayout.handle();

        if (::vkAllocateDescriptorSets(m_parent->getDevice()->handle(), &descriptorSetInfo, &m_descriptorSet) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate descriptor sets.");

        m_layout = &descriptorSetLayout;
        return descriptorPool;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBufferPool::VulkanBufferPool(const VulkanDescriptorSetLayout& bufferSet) :
    m_impl(makePimpl<VulkanBufferPoolImpl>(this)), VulkanRuntimeObject(bufferSet.getDevice()), IResource<VkDescriptorPool>(nullptr)
{
    this->handle() = m_impl->initialize(bufferSet);
}

VulkanBufferPool::~VulkanBufferPool() noexcept
{
    ::vkDestroyDescriptorPool(this->getDevice()->handle(), this->handle(), nullptr);
}

const IDescriptorSetLayout* VulkanBufferPool::getDescriptorSetLayout() const noexcept
{
    return m_impl->m_layout;
}

const VkDescriptorSet VulkanBufferPool::getDescriptorSet() const noexcept
{
    return m_impl->m_descriptorSet;
}

UniquePtr<IConstantBuffer> VulkanBufferPool::makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements) const noexcept
{
    auto layout = this->getDescriptorSetLayout()->getLayout(binding);
    return this->getDevice()->createConstantBuffer(layout, usage, elements);
}

UniquePtr<ITexture> VulkanBufferPool::makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const noexcept
{
    auto layout = this->getDescriptorSetLayout()->getLayout(binding);
    return this->getDevice()->createTexture(layout, format, size, levels, samples);
}

void VulkanBufferPool::update(const IConstantBuffer* buffer) const
{
    auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);

    if (resource == nullptr)
        throw std::invalid_argument("The buffer is not a valid Vulkan buffer.");

    VkDescriptorBufferInfo bufferInfo{ };
    bufferInfo.buffer = resource->handle();
    bufferInfo.range = buffer->getSize();
    bufferInfo.offset = 0;

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_impl->m_descriptorSet;
    descriptorWrite.dstBinding = buffer->getLayout()->getBinding();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    switch (buffer->getLayout()->getDescriptorType())
    {
    case DescriptorType::Uniform: descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
    case DescriptorType::Storage: descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
    default: throw std::runtime_error("Unsupported buffer type.");
    }

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanBufferPool::update(const ITexture* texture) const
{
    auto image = dynamic_cast<const IVulkanImage*>(texture);

    if (image == nullptr)
        throw std::invalid_argument("The texture is not a valid Vulkan texture.");

    VkDescriptorImageInfo imageInfo{ };
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = image->getImageView();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptorWrite.dstSet = m_impl->m_descriptorSet;
    descriptorWrite.dstBinding = texture->getBinding();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanBufferPool::update(const ISampler* sampler) const
{
    auto resource = dynamic_cast<const IResource<VkSampler>*>(sampler);

    if (resource == nullptr)
        throw std::invalid_argument("The sampler is not a valid Vulkan sampler.");

    VkDescriptorImageInfo imageInfo{ };
    imageInfo.sampler = resource->handle();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorWrite.dstSet = m_impl->m_descriptorSet;
    descriptorWrite.dstBinding = sampler->getBinding();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}