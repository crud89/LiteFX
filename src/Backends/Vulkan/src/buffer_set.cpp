#include <litefx/backends/vulkan.hpp>
#include <array>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBufferSet::VulkanBufferSetImpl : public Implement<VulkanBufferSet> {
public:
    friend class VulkanBufferSet;

public:
    VulkanBufferSetImpl(VulkanBufferSet* parent) : base(parent) { }

    ~VulkanBufferSetImpl()
    {
        ::vkDestroyDescriptorSetLayout(m_parent->getDevice()->handle(), m_parent->handle(), nullptr);
    }

public:
    VkDescriptorSetLayout initialize()
    {
        // Vertex data does not get a descriptor set in Vulkan.
        if (m_parent->getType() == BufferSetType::VertexData)
            return nullptr;

        LITEFX_TRACE(VULKAN_LOG, "Defining buffer set {0} {{ {1} }}...", m_parent->getSetId(), m_parent->getType());

        // TODO: Also count storage and sampler bindings.
        Array<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0 },
            { VK_DESCRIPTOR_TYPE_SAMPLER, 0 }
        };

        // Parse descriptor set layouts.
        auto layouts = m_parent->getLayouts();
        Array<VkDescriptorSetLayoutBinding> bindings;

        std::for_each(std::begin(layouts), std::end(layouts), [&, i = 0](const IBufferLayout* layout) mutable {
            auto bindingPoint = layout->getBinding();
            auto type = layout->getType();
            auto layoutSet = m_parent->getSetId();

            LITEFX_TRACE(VULKAN_LOG, "\tWith buffer {0}/{1} {{ Type: {2}, Size: {3} bytes, Offset: {4}, Binding point: {5} }}...", ++i, layouts.size(), type, layout->getElementSize(), 0, bindingPoint);

            VkDescriptorSetLayoutBinding binding = {};
            binding.binding = bindingPoint;
            binding.descriptorCount = 1;		// TODO: Implement support for uniform buffer arrays.
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

            switch (type)
            {
            case BufferType::Uniform:
                binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                poolSizes[0].descriptorCount++;
                break;
            case BufferType::Storage:
            case BufferType::Vertex:
            case BufferType::Index:
            default:
                LITEFX_WARNING(VULKAN_LOG, "The buffer type is unsupported. Binding will be skipped.");
            }

            bindings.push_back(binding);
        });

        LITEFX_TRACE(VULKAN_LOG, "Creating buffer set {0} with {1} bindings {{ Uniform: {2}, Storage: {3}, Sampler: {4} }}...", m_parent->getSetId(), layouts.size(), 0, 0, 0);

        VkDescriptorSetLayoutCreateInfo uniformBufferLayoutInfo{};
        uniformBufferLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        uniformBufferLayoutInfo.bindingCount = bindings.size();
        uniformBufferLayoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout layout;

        if (::vkCreateDescriptorSetLayout(m_parent->getDevice()->handle(), &uniformBufferLayoutInfo, nullptr, &layout) != VK_SUCCESS)
            throw std::runtime_error("Unable to create uniform buffer descriptor set layout.");

        return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBufferSet::VulkanBufferSet(const VulkanInputAssembler& inputAssembler, const BufferSetType& type, const UInt32& id) :
    m_impl(makePimpl<VulkanBufferSetImpl>(this)), RuntimeObject(inputAssembler.getDevice()), BufferSet(type, id), IResource<VkDescriptorSetLayout>(nullptr)
{
}

VulkanBufferSet::~VulkanBufferSet() noexcept = default;

void VulkanBufferSet::create()
{
    auto& h = this->handle();

    if (h != nullptr)
        throw std::runtime_error("The buffer set can only created once.");

    this->handle() = m_impl->initialize();
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanInputAssemblerBuilder& VulkanBufferSetBuilder::go()
{
    this->instance()->create();

    return BufferSetBuilder::go();
}

VulkanBufferSetBuilder& VulkanBufferSetBuilder::addLayout(UniquePtr<IBufferLayout>&& layout)
{
    this->instance()->add(std::move(layout));
    return *this;
}