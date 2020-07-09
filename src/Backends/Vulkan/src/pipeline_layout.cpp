#include <litefx/backends/vulkan.hpp>
#include <array>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineLayout::VulkanRenderPipelineLayoutImpl : public Implement<VulkanRenderPipelineLayout> {
public:
    friend class VulkanRenderPipelineLayout;

private:
    const VulkanDevice* m_device;
    Array<VkDescriptorSetLayout> m_descriptorSetLayouts;
    VkDescriptorPool m_descriptorPool;

public:
    VulkanRenderPipelineLayoutImpl(VulkanRenderPipelineLayout* parent, const VulkanDevice* device) :
        base(parent), m_device(device) { }

    ~VulkanRenderPipelineLayoutImpl()
    {
        // Destroy the descriptor set layouts and the descriptor pool.
        std::for_each(std::begin(m_descriptorSetLayouts), std::end(m_descriptorSetLayouts), [&](VkDescriptorSetLayout& layout) { 
            ::vkDestroyDescriptorSetLayout(m_device->handle(), layout, nullptr); 
        });
        
        ::vkDestroyDescriptorPool(m_device->handle(), m_descriptorPool, nullptr);
    }

public:
    VkPipelineLayout initialize()
    {
        auto inputAssembler = m_parent->getInputAssembler();

        if (inputAssembler == nullptr)
            throw std::runtime_error("The input assembler must be initialized.");

        LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline layout {0}...", fmt::ptr(m_parent));

        // TODO: Also count storage and sampler bindings.
        Dictionary<UInt32, Array<VkDescriptorSetLayoutBinding>> descriptorSetBindings;
        std::array<VkDescriptorPoolSize, 1> poolSizes = {};

        // Parse uniform buffer descriptor sets.
        auto uniformBufferLayouts = inputAssembler->getLayouts(BufferType::Uniform);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = uniformBufferLayouts.size();

        std::for_each(std::begin(uniformBufferLayouts), std::end(uniformBufferLayouts), [&, i = 0](const BufferLayout* uniformBufferLayout) mutable {
            auto bindingPoint = uniformBufferLayout->getBinding();
            auto layoutSet = uniformBufferLayout->getSet();

            LITEFX_TRACE(VULKAN_LOG, "Defining uniform buffer layout {0}/{1} {{ Size: {2} bytes, Binding point: {3}, Descriptor set: {4} }}...", ++i, uniformBufferLayouts.size(), uniformBufferLayout->getElementSize(), bindingPoint, layoutSet);

            VkDescriptorSetLayoutBinding binding = {};
            binding.binding = bindingPoint;
            binding.descriptorCount = 1;		// TODO: Implement support for uniform buffer arrays.
            binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

            descriptorSetBindings[layoutSet].push_back(binding);
        });

        // TODO: Parse other buffer descriptor sets.

        // Create descriptor set layouts.
        std::for_each(std::begin(descriptorSetBindings), std::end(descriptorSetBindings), [&, i = 0](const auto& bindings) mutable {
            LITEFX_TRACE(VULKAN_LOG, "Creating descriptor set layout {0}/{1} {{ Bindings: {2} }}...", ++i, descriptorSetBindings.size(), bindings.second.size());

            VkDescriptorSetLayoutCreateInfo uniformBufferLayoutInfo{};
            uniformBufferLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            uniformBufferLayoutInfo.bindingCount = bindings.second.size();
            uniformBufferLayoutInfo.pBindings = bindings.second.data();

            VkDescriptorSetLayout layout;

            if (::vkCreateDescriptorSetLayout(m_device->handle(), &uniformBufferLayoutInfo, nullptr, &layout) != VK_SUCCESS)
                throw std::runtime_error("Unable to create uniform buffer descriptor set layout.");

            m_descriptorSetLayouts.push_back(layout);
        });

        // Create a descriptor pool.
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();

        // Currently there is only one descriptor set per pool, which means that for each draw call, the whole descriptor set needs to be re-written. This can be
        // inefficient. We should use an additional layout property (set) to determine the descriptor set a buffer belongs to. We can use multiple sets to group
        // writes by update frequency.
        // TODO: Use descriptor sets to group buffers by write frequency.
        poolInfo.maxSets = m_descriptorSetLayouts.size();

        LITEFX_TRACE(VULKAN_LOG, "Allocating descriptor pool {{ Uniforms: {0}, Storages: {1}, Samplers: {2} }}...", poolSizes[0].descriptorCount, 0, 0);

        if (::vkCreateDescriptorPool(m_device->handle(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("Unable to create descriptor pool.");

        // Allocate the descriptor sets.
        VkDescriptorSetAllocateInfo descriptorSetInfo = {};
        descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetInfo.descriptorPool = m_descriptorPool;
        descriptorSetInfo.descriptorSetCount = m_descriptorSetLayouts.size();
        descriptorSetInfo.pSetLayouts = m_descriptorSetLayouts.data();

        // Create the pipeline layout.
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = m_descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        VkPipelineLayout layout;

        if (::vkCreatePipelineLayout(m_device->handle(), &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
            throw std::runtime_error("Unable to create pipeline layout.");

        return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline) :
    RenderPipelineLayout(), IResource<VkPipelineLayout>(nullptr)
{
    auto device = dynamic_cast<const VulkanDevice*>(pipeline.getDevice());

    if (device == nullptr)
        throw std::invalid_argument("The pipeline is not bound to a valid Vulkan device.");

    m_impl = makePimpl<VulkanRenderPipelineLayoutImpl>(this, device);
}

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline, const VulkanBufferLayout& bufferLayout) :
    VulkanRenderPipelineLayout(pipeline)
{
    this->create();
}

VulkanRenderPipelineLayout::~VulkanRenderPipelineLayout() noexcept = default;

void VulkanRenderPipelineLayout::create() 
{
    this->handle() = m_impl->initialize();
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineBuilder& VulkanRenderPipelineLayoutBuilder::go()
{
    this->instance()->create();
    return RenderPipelineLayoutBuilder::go();
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<IRasterizer>&& rasterizer)
{
    if (rasterizer == nullptr)
        throw std::invalid_argument("The rasterizer must be initialized.");

    this->instance()->use(std::move(rasterizer));
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<IInputAssembler>&& inputAssembler)
{
    if (inputAssembler == nullptr)
        throw std::invalid_argument("The input assembler must be initialized.");

    this->instance()->use(std::move(inputAssembler));
}

void VulkanRenderPipelineLayoutBuilder::use(UniquePtr<IViewport>&& viewport)
{
    if (viewport == nullptr)
        throw std::invalid_argument("The viewport must be initialized.");

    this->instance()->use(std::move(viewport));
}