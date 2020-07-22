#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanIndexBufferLayout::VulkanIndexBufferLayoutImpl : public Implement<VulkanIndexBufferLayout> {
public:
    friend class VulkanIndexBufferLayout;

private:
    IndexType m_type;

public:
    VulkanIndexBufferLayoutImpl(VulkanIndexBufferLayout* parent, const IndexType& type) : base(parent), m_type(type) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanIndexBufferLayout::VulkanIndexBufferLayout(const VulkanInputAssembler& inputAssembler, const IndexType& type) :
    m_impl(makePimpl<VulkanIndexBufferLayoutImpl>(this, type)), VulkanRuntimeObject(inputAssembler.getDevice())
{
}

VulkanIndexBufferLayout::~VulkanIndexBufferLayout() noexcept = default;

size_t VulkanIndexBufferLayout::getElementSize() const noexcept
{
    return static_cast<UInt32>(m_impl->m_type) >> 3;
}

UInt32 VulkanIndexBufferLayout::getBinding() const noexcept
{
    return 0;
}

BufferType VulkanIndexBufferLayout::getType() const noexcept
{
    return BufferType::Index;
}

const IndexType& VulkanIndexBufferLayout::getIndexType() const noexcept
{
    return m_impl->m_type;
}

// ------------------------------------------------------------------------------------------------
// Index buffer implementation.
// ------------------------------------------------------------------------------------------------

VulkanIndexBuffer::VulkanIndexBuffer(VkBuffer buffer, const IIndexBufferLayout* layout, const UInt32& elements) :
    IndexBuffer(layout, elements), IResource(buffer)
{
}

VulkanIndexBuffer::~VulkanIndexBuffer() noexcept = default;