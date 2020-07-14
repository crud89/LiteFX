#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBufferLayout::VulkanBufferLayoutImpl : public Implement<VulkanBufferLayout> {
public:
    friend class VulkanBufferLayout;

public:
    VulkanBufferLayoutImpl(VulkanBufferLayout* parent) : base(parent) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBufferLayout::VulkanBufferLayout(const VulkanBufferSet& bufferSet, const BufferType& type, const size_t& elementSize, const UInt32& binding) :
    m_impl(makePimpl<VulkanBufferLayoutImpl>(this)), BufferLayout(type, elementSize, binding)
{
}

VulkanBufferLayout::~VulkanBufferLayout() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanBufferLayoutBuilder& VulkanBufferLayoutBuilder::addAttribute(UniquePtr<BufferAttribute>&& attribute)
{
    this->instance()->add(std::move(attribute));
    return *this;
}

VulkanBufferLayoutBuilder& VulkanBufferLayoutBuilder::addAttribute(const BufferFormat& format, const UInt32& offset)
{
    return this->addAttribute(std::move(makeUnique<BufferAttribute>(static_cast<UInt32>(this->instance()->getAttributes().size()), offset, format)));
}

VulkanBufferLayoutBuilder& VulkanBufferLayoutBuilder::addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset)
{
    return this->addAttribute(std::move(makeUnique<BufferAttribute>(location, offset, format)));
}