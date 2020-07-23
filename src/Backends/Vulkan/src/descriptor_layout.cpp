#include <litefx/backends/vulkan.hpp>
#include <array>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanDescriptorSetLayout::VulkanDescriptorSetLayoutImpl : public Implement<VulkanDescriptorSetLayout> {
public:
    friend class VulkanDescriptorSetLayoutBuilder;
    friend class VulkanDescriptorSetLayout;

private:
    UInt32 m_setId;
    ShaderStage m_stages;
    Dictionary<DescriptorType, UInt32> m_poolSizes = {
        { DescriptorType::Uniform, 0 },
        { DescriptorType::Storage, 0 },
        { DescriptorType::Image, 0 },
        { DescriptorType::Sampler, 0 },
        { DescriptorType::InputAttachment, 0 }
    };
    Array<UniquePtr<IDescriptorLayout>> m_layouts;

public:
    VulkanDescriptorSetLayoutImpl(VulkanDescriptorSetLayout* parent, const UInt32& id, const ShaderStage& stages) : 
        base(parent), m_setId(id), m_stages(stages) { }

public:
    VkDescriptorSetLayout initialize()
    {
        LITEFX_TRACE(VULKAN_LOG, "Defining layout for descriptor set {0} {{ Stages: {1} }}...", m_setId, m_stages);

        // Parse the shader stage descriptor.
        VkShaderStageFlags shaderStages = {};

        if ((m_stages & ShaderStage::Vertex) == ShaderStage::Vertex)
            shaderStages |= VK_SHADER_STAGE_VERTEX_BIT;
        if ((m_stages & ShaderStage::Geometry) == ShaderStage::Geometry)
            shaderStages |= VK_SHADER_STAGE_GEOMETRY_BIT;
        if ((m_stages & ShaderStage::Fragment) == ShaderStage::Fragment)
            shaderStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if ((m_stages & ShaderStage::TessellationEvaluation) == ShaderStage::TessellationEvaluation)
            shaderStages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        if ((m_stages & ShaderStage::TessellationControl) == ShaderStage::TessellationControl)
            shaderStages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if ((m_stages & ShaderStage::Compute) == ShaderStage::Compute)
            shaderStages |= VK_SHADER_STAGE_COMPUTE_BIT;

        // Parse descriptor set layouts.
        Array<VkDescriptorSetLayoutBinding> bindings;

        std::for_each(std::begin(m_layouts), std::end(m_layouts), [&, i = 0](const UniquePtr<IDescriptorLayout>& layout) mutable {
            auto bindingPoint = layout->getBinding();
            auto type = layout->getDescriptorType();

            LITEFX_TRACE(VULKAN_LOG, "\tWith descriptor {0}/{1} {{ Type: {2}, Element size: {3} bytes, Offset: {4}, Binding point: {5} }}...", ++i, m_layouts.size(), type, layout->getElementSize(), 0, bindingPoint);

            VkDescriptorSetLayoutBinding binding = {};
            binding.binding = bindingPoint;
            binding.descriptorCount = 1;		// TODO: Implement support for uniform buffer arrays.
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = shaderStages;

            switch (type)
            {
            case DescriptorType::Uniform:         binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;   break;
            case DescriptorType::Storage:         binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;   break;
            case DescriptorType::Image:           binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;    break;
            case DescriptorType::Sampler:         binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;          break;
            case DescriptorType::InputAttachment: binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; break;
            default: LITEFX_WARNING(VULKAN_LOG, "The descriptor type is unsupported. Binding will be skipped.");
            }

            m_poolSizes[type]++;
            bindings.push_back(binding);
        });

        LITEFX_TRACE(VULKAN_LOG, "Creating descriptor set {0} layout with {1} bindings {{ Uniform: {2}, Storage: {3}, Images: {4}, Sampler: {5}, Input attachments: {6} }}...", m_setId, m_layouts.size(), m_poolSizes[DescriptorType::Uniform], m_poolSizes[DescriptorType::Storage], m_poolSizes[DescriptorType::Image], m_poolSizes[DescriptorType::Sampler], m_poolSizes[DescriptorType::InputAttachment]);

        VkDescriptorSetLayoutCreateInfo uniformBufferLayoutInfo{};
        uniformBufferLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        uniformBufferLayoutInfo.bindingCount = bindings.size();
        uniformBufferLayoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout layout;

        if (::vkCreateDescriptorSetLayout(m_parent->getDevice()->handle(), &uniformBufferLayoutInfo, nullptr, &layout) != VK_SUCCESS)
            throw std::runtime_error("Unable to create descriptor set layout.");

        return layout;
    }

public:
    Array<const IDescriptorLayout*> getLayouts() const noexcept
    {
        Array<const IDescriptorLayout*> layouts(m_layouts.size());
        std::generate(std::begin(layouts), std::end(layouts), [&, i = 0]() mutable { return m_layouts[i++].get(); });

        return layouts;
    }

    const IDescriptorLayout* getLayout(const UInt32& binding) const noexcept
    {
        auto layout = std::find_if(std::begin(m_layouts), std::end(m_layouts), [&](const UniquePtr<IDescriptorLayout>& layout) { return layout->getBinding() == binding; });

        return layout == m_layouts.end() ? nullptr : layout->get();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const VulkanShaderProgram& shaderProgram, const UInt32& id, const ShaderStage& stages) :
    m_impl(makePimpl<VulkanDescriptorSetLayoutImpl>(this, id, stages)), VulkanRuntimeObject(shaderProgram.getDevice()), IResource<VkDescriptorSetLayout>(nullptr)
{
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() noexcept
{
    ::vkDestroyDescriptorSetLayout(this->getDevice()->handle(), this->handle(), nullptr);
}

Array<const IDescriptorLayout*> VulkanDescriptorSetLayout::getLayouts() const noexcept
{
    return m_impl->getLayouts();
}

const IDescriptorLayout* VulkanDescriptorSetLayout::getLayout(const UInt32& binding) const noexcept
{
    return m_impl->getLayout(binding);
}

const UInt32& VulkanDescriptorSetLayout::getSetId() const noexcept
{
    return m_impl->m_setId;
}

const ShaderStage& VulkanDescriptorSetLayout::getShaderStages() const noexcept
{
    return m_impl->m_stages;
}

UniquePtr<IDescriptorSet> VulkanDescriptorSetLayout::createBufferPool() const noexcept
{
    return makeUnique<VulkanDescriptorSet>(*this);
}

UInt32 VulkanDescriptorSetLayout::uniforms() const noexcept
{
    return m_impl->m_poolSizes[DescriptorType::Uniform];
}

UInt32 VulkanDescriptorSetLayout::storages() const noexcept
{
    return m_impl->m_poolSizes[DescriptorType::Storage];
}

UInt32 VulkanDescriptorSetLayout::images() const noexcept
{
    return m_impl->m_poolSizes[DescriptorType::Image];
}

UInt32 VulkanDescriptorSetLayout::samplers() const noexcept
{
    return m_impl->m_poolSizes[DescriptorType::Sampler];
}

UInt32 VulkanDescriptorSetLayout::inputAttachments() const noexcept
{
    return m_impl->m_poolSizes[DescriptorType::InputAttachment];
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanShaderProgramBuilder& VulkanDescriptorSetLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->handle() = instance->m_impl->initialize();

    return DescriptorSetLayoutBuilder::go();
}


VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::addDescriptor(UniquePtr<IDescriptorLayout>&& layout)
{
    this->instance()->m_impl->m_layouts.push_back(std::move(layout));
    return *this;
}

VulkanDescriptorSetLayoutBuilder& VulkanDescriptorSetLayoutBuilder::addDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize)
{
    UniquePtr<IDescriptorLayout> layout = makeUnique<VulkanDescriptorLayout>(*(this->instance()), type, binding, descriptorSize);
    return this->addDescriptor(std::move(layout));
}