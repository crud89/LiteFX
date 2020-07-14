#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanRenderPipelineLayout::VulkanRenderPipelineLayoutImpl : public Implement<VulkanRenderPipelineLayout> {
public:
    friend class VulkanRenderPipelineLayout;

private:
    const VulkanDevice* m_device;

public:
    VulkanRenderPipelineLayoutImpl(VulkanRenderPipelineLayout* parent, const VulkanDevice* device) :
        base(parent), m_device(device) { }

public:
    VkPipelineLayout initialize()
    {
        auto inputAssembler = m_parent->getInputAssembler();

        if (inputAssembler == nullptr)
            throw std::runtime_error("The input assembler must be initialized.");

        LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline layout {0}...", fmt::ptr(m_parent));

        // TODO: Create a pool per "component" (i.e. camera or mesh) and pass the proper layout(s) to it.
        // NOTE: One object can contain multiple mesh components, that use different pipelines with different layouts.

        // I.E.: new method: `createDescriptorPool` ?

        //// Create a descriptor pool.
        //VkDescriptorPoolCreateInfo poolInfo = {};
        //poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        //poolInfo.poolSizeCount = poolSizes.size();
        //poolInfo.pPoolSizes = poolSizes.data();

        //// Currently there is only one descriptor set per pool, which means that for each draw call, the whole descriptor set needs to be re-written. This can be
        //// inefficient. We should use an additional layout property (set) to determine the descriptor set a buffer belongs to. We can use multiple sets to group
        //// writes by update frequency.
        //// TODO: Use descriptor sets to group buffers by write frequency.
        //poolInfo.maxSets = m_descriptorSetLayouts.size();

        //LITEFX_TRACE(VULKAN_LOG, "Allocating descriptor pool {{ Uniforms: {0}, Storages: {1}, Samplers: {2} }}...", poolSizes[0].descriptorCount, 0, 0);

        //if (::vkCreateDescriptorPool(m_device->handle(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
        //    throw std::runtime_error("Unable to create descriptor pool.");

        //// Allocate the descriptor sets.
        //VkDescriptorSetAllocateInfo descriptorSetInfo = {};
        //descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        //descriptorSetInfo.descriptorPool = m_descriptorPool;
        //descriptorSetInfo.descriptorSetCount = m_descriptorSetLayouts.size();
        //descriptorSetInfo.pSetLayouts = m_descriptorSetLayouts.data();

        //m_descriptorSets.resize(m_descriptorSetLayouts.size());
        //LITEFX_TRACE(VULKAN_LOG, "Allocating {0} descriptor sets...", m_descriptorSets.size());

        //if (::vkAllocateDescriptorSets(m_device->handle(), &descriptorSetInfo, m_descriptorSets.data()) != VK_SUCCESS)
        //    throw std::runtime_error("Unable to allocate descriptor sets.");


        throw;
        //// Create the pipeline layout.
        //VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //pipelineLayoutInfo.setLayoutCount = m_descriptorSetLayouts.size();
        //pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();
        //pipelineLayoutInfo.pushConstantRangeCount = 0;

        //VkPipelineLayout layout;

        //if (::vkCreatePipelineLayout(m_device->handle(), &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
        //    throw std::runtime_error("Unable to create pipeline layout.");

        //return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

VulkanRenderPipelineLayout::VulkanRenderPipelineLayout(const VulkanRenderPipeline& pipeline) :
    RenderPipelineLayout(), RuntimeObject(pipeline.getDevice()), IResource<VkPipelineLayout>(nullptr)
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