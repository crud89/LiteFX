#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBufferLayoutBuilder& VulkanVertexBufferLayoutBuilder::addAttribute(UniquePtr<BufferAttribute>&& attribute)
{
    this->instance()->m_impl->m_attributes.push_back(std::move(attribute));
    return *this;
}

VulkanVertexBufferLayoutBuilder& VulkanVertexBufferLayoutBuilder::addAttribute(const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic, const UInt32& semanticIndex)
{
    return this->addAttribute(std::move(makeUnique<BufferAttribute>(static_cast<UInt32>(this->instance()->attributes().size()), offset, format, semantic, semanticIndex)));
}

VulkanVertexBufferLayoutBuilder& VulkanVertexBufferLayoutBuilder::addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic, const UInt32& semanticIndex)
{
    return this->addAttribute(std::move(makeUnique<BufferAttribute>(location, offset, format, semantic, semanticIndex)));
}