#include <litefx/backends/vulkan.hpp>
#include <array>
#include "image.h"

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDescriptorSet::VulkanDescriptorSetImpl : public Implement<VulkanDescriptorSet> {
public:
    friend class VulkanDescriptorSet;

private:
    Array<VkDescriptorSet> m_descriptorSets;
    UInt32 m_currentSet = 0;

public:
    VulkanDescriptorSetImpl(VulkanDescriptorSet* parent) : base(parent) { }

public:
    VkDescriptorPool initialize()
    {
        Array<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,   m_parent->parent().uniforms() },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,   m_parent->parent().storages() },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,    m_parent->parent().images() },
            { VK_DESCRIPTOR_TYPE_SAMPLER,          m_parent->parent().samplers() },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, m_parent->parent().inputAttachments() }
        };

        auto descriptorLayouts = m_parent->parent().getLayouts();
        auto frames = m_parent->getDevice()->swapChain().getBuffers();

        LITEFX_TRACE(VULKAN_LOG, "Allocating descriptor pool with {5} sets {{ Uniforms: {0}, Storages: {1}, Images: {2}, Samplers: {3}, Input attachments: {4} }}...", poolSizes[0].descriptorCount, poolSizes[1].descriptorCount, poolSizes[2].descriptorCount, poolSizes[3].descriptorCount, poolSizes[4].descriptorCount, frames);

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
        poolInfo.maxSets = frames;

        VkDescriptorPool descriptorPool;

        if (::vkCreateDescriptorPool(m_parent->getDevice()->handle(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("Unable to create buffer pool.");

        // Allocate one descriptor set for each frame.
        m_descriptorSets.resize(poolInfo.maxSets);
        std::generate(std::begin(m_descriptorSets), std::end(m_descriptorSets), [&]() mutable {
            VkDescriptorSetAllocateInfo descriptorSetInfo = {};
            descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorSetInfo.descriptorPool = descriptorPool;
            descriptorSetInfo.descriptorSetCount = 1;
            descriptorSetInfo.pSetLayouts = &m_parent->parent().handle();

            VkDescriptorSet descriptorSet;

            if (::vkAllocateDescriptorSets(m_parent->getDevice()->handle(), &descriptorSetInfo, &descriptorSet) != VK_SUCCESS)
                throw std::runtime_error("Unable to allocate descriptor sets.");

            return descriptorSet;
        });

        return descriptorPool;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanDescriptorSetLayout& bufferSet) :
    m_impl(makePimpl<VulkanDescriptorSetImpl>(this)), VulkanRuntimeObject<VulkanDescriptorSetLayout>(bufferSet, bufferSet.getDevice()), IResource<VkDescriptorPool>(nullptr)
{
    this->handle() = m_impl->initialize();
}

VulkanDescriptorSet::~VulkanDescriptorSet() noexcept
{
    ::vkDestroyDescriptorPool(this->getDevice()->handle(), this->handle(), nullptr);
}

const VkDescriptorSet VulkanDescriptorSet::swapBuffer()
{
    auto descriptorSet = m_impl->m_descriptorSets[m_impl->m_currentSet];
    m_impl->m_currentSet = (m_impl->m_currentSet + 1) % static_cast<UInt32>(m_impl->m_descriptorSets.size());
    return descriptorSet;
}

UniquePtr<IConstantBuffer> VulkanDescriptorSet::makeBuffer(const UInt32& binding, const BufferUsage& usage, const UInt32& elements) const noexcept
{
    auto layout = this->parent().getLayout(binding);
    return this->getDevice()->factory().createConstantBuffer(dynamic_cast<const VulkanDescriptorLayout&>(*layout), usage, elements);
}

UniquePtr<ITexture> VulkanDescriptorSet::makeTexture(const UInt32& binding, const Format& format, const Size2d& size, const UInt32& levels, const MultiSamplingLevel& samples) const noexcept
{
    auto layout = this->parent().getLayout(binding);
    return this->getDevice()->factory().createTexture(dynamic_cast<const VulkanDescriptorLayout&>(*layout), format, size, levels, samples);
}

UniquePtr<ISampler> VulkanDescriptorSet::makeSampler(const UInt32& binding, const FilterMode& magFilter, const FilterMode& minFilter, const BorderMode& borderU, const BorderMode& borderV, const BorderMode& borderW, const MipMapMode& mipMapMode, const Float& mipMapBias, const Float& maxLod, const Float& minLod, const Float& anisotropy) const noexcept
{
    auto layout = this->parent().getLayout(binding);
    return this->getDevice()->factory().createSampler(dynamic_cast<const VulkanDescriptorLayout&>(*layout), magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, maxLod, minLod, anisotropy);
}

void VulkanDescriptorSet::update(const IConstantBuffer* buffer) const
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
    descriptorWrite.dstSet = m_impl->m_descriptorSets[m_impl->m_currentSet];
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

void VulkanDescriptorSet::update(const ITexture* texture) const
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
    descriptorWrite.dstSet = m_impl->m_descriptorSets[m_impl->m_currentSet];
    descriptorWrite.dstBinding = texture->getBinding();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::update(const ISampler* sampler) const
{
    auto resource = dynamic_cast<const IResource<VkSampler>*>(sampler);

    if (resource == nullptr)
        throw std::invalid_argument("The sampler is not a valid Vulkan sampler.");

    VkDescriptorImageInfo imageInfo{ };
    imageInfo.sampler = resource->handle();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorWrite.dstSet = m_impl->m_descriptorSets[m_impl->m_currentSet];
    descriptorWrite.dstBinding = sampler->getBinding();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanDescriptorSet::updateAll(const IConstantBuffer* buffer) const
{
    auto resource = dynamic_cast<const IResource<VkBuffer>*>(buffer);

    if (resource == nullptr)
        throw std::invalid_argument("The buffer is not a valid Vulkan buffer.");

    VkDescriptorBufferInfo bufferInfo{ };
    bufferInfo.buffer = resource->handle();
    bufferInfo.range = buffer->getSize();
    bufferInfo.offset = 0;

    Array<VkWriteDescriptorSet> descriptorWrites(m_impl->m_descriptorSets.size());

    std::generate(std::begin(descriptorWrites), std::end(descriptorWrites), [&, i = 0]() mutable {
        VkWriteDescriptorSet descriptorWrite = {};

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_impl->m_descriptorSets[i++];
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

        return descriptorWrite;
    });

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void VulkanDescriptorSet::updateAll(const ITexture* texture) const
{
    auto image = dynamic_cast<const IVulkanImage*>(texture);

    if (image == nullptr)
        throw std::invalid_argument("The texture is not a valid Vulkan texture.");

    VkDescriptorImageInfo imageInfo{ };
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = image->getImageView();

    Array<VkWriteDescriptorSet> descriptorWrites(m_impl->m_descriptorSets.size());

    std::generate(std::begin(descriptorWrites), std::end(descriptorWrites), [&, i = 0]() mutable {
        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrite.dstSet = m_impl->m_descriptorSets[i++];
        descriptorWrite.dstBinding = texture->getBinding();
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        return descriptorWrite;
    });

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void VulkanDescriptorSet::updateAll(const ISampler* sampler) const
{
    auto resource = dynamic_cast<const IResource<VkSampler>*>(sampler);

    if (resource == nullptr)
        throw std::invalid_argument("The sampler is not a valid Vulkan sampler.");

    VkDescriptorImageInfo imageInfo{ };
    imageInfo.sampler = resource->handle();

    Array<VkWriteDescriptorSet> descriptorWrites(m_impl->m_descriptorSets.size());

    std::generate(std::begin(descriptorWrites), std::end(descriptorWrites), [&, i = 0]() mutable {
        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorWrite.dstSet = m_impl->m_descriptorSets[i++];
        descriptorWrite.dstBinding = sampler->getBinding();
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        return descriptorWrite;
    });

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void VulkanDescriptorSet::attach(const UInt32& binding, const IRenderPass* renderPass, const UInt32& attachmentId) const
{
    if (renderPass == nullptr)
        throw std::invalid_argument("The render pass must be initialized.");
    
    this->attach(binding, renderPass->getAttachment(attachmentId));
}

void VulkanDescriptorSet::attach(const UInt32& binding, const IImage* image) const
{
    auto resource = dynamic_cast<const IVulkanImage*>(image);

    if (resource == nullptr)
        throw std::invalid_argument("The input attachment image is not a valid Vulkan resource.");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = resource->getImageView();

    VkWriteDescriptorSet descriptorWrite{ };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptorWrite.dstSet = m_impl->m_descriptorSets[m_impl->m_currentSet];
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    ::vkUpdateDescriptorSets(this->getDevice()->handle(), 1, &descriptorWrite, 0, nullptr);
}