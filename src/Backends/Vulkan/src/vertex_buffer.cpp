#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanVertexBufferLayout::VulkanVertexBufferLayoutImpl : public Implement<VulkanVertexBufferLayout> {
public:
    friend class VulkanVertexBufferLayoutBuilder;
    friend class VulkanVertexBufferLayout;

private:
    Array<UniquePtr<BufferAttribute>> m_attributes;
    size_t m_vertexSize;
    UInt32 m_binding;

public:
    VulkanVertexBufferLayoutImpl(VulkanVertexBufferLayout* parent, const size_t& vertexSize, const UInt32& binding) : 
        base(parent), m_vertexSize(vertexSize), m_binding(binding) { }

public:
    Array<const BufferAttribute*> getAttributes() const noexcept
    {
        Array<const BufferAttribute*> attributes(m_attributes.size());
        std::generate(std::begin(attributes), std::end(attributes), [&, i = 0]() mutable { return m_attributes[i++].get(); });
        
        return attributes;
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBufferLayout::VulkanVertexBufferLayout(const VulkanInputAssembler& inputAssembler, const size_t& vertexSize, const UInt32& binding) :
    m_impl(makePimpl<VulkanVertexBufferLayoutImpl>(this, vertexSize, binding)), VulkanRuntimeObject(inputAssembler.getDevice())
{
}

VulkanVertexBufferLayout::~VulkanVertexBufferLayout() noexcept = default;

size_t VulkanVertexBufferLayout::getElementSize() const noexcept
{
    return m_impl->m_vertexSize;
}

UInt32 VulkanVertexBufferLayout::getBinding() const noexcept
{
    return m_impl->m_binding;
}

BufferType VulkanVertexBufferLayout::getType() const noexcept
{
    return BufferType::Vertex;
}

Array<const BufferAttribute*> VulkanVertexBufferLayout::getAttributes() const noexcept
{
    return m_impl->getAttributes();
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBufferLayoutBuilder& VulkanVertexBufferLayoutBuilder::addAttribute(UniquePtr<BufferAttribute>&& attribute)
{
    this->instance()->m_impl->m_attributes.push_back(std::move(attribute));
    return *this;
}

VulkanVertexBufferLayoutBuilder& VulkanVertexBufferLayoutBuilder::addAttribute(const BufferFormat& format, const UInt32& offset)
{
    return this->addAttribute(std::move(makeUnique<BufferAttribute>(static_cast<UInt32>(this->instance()->getAttributes().size()), offset, format)));
}

VulkanVertexBufferLayoutBuilder& VulkanVertexBufferLayoutBuilder::addAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset)
{
    return this->addAttribute(std::move(makeUnique<BufferAttribute>(location, offset, format)));
}