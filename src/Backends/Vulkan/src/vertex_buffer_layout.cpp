#include <litefx/backends/vulkan.hpp>
#include <litefx/backends/vulkan_builders.hpp>

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
    BufferType m_bufferType{ BufferType::Vertex };

public:
    VulkanVertexBufferLayoutImpl(VulkanVertexBufferLayout* parent, const size_t& vertexSize, const UInt32& binding) : 
        base(parent), m_vertexSize(vertexSize), m_binding(binding) 
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBufferLayout::VulkanVertexBufferLayout(const size_t& vertexSize, const UInt32& binding) :
    m_impl(makePimpl<VulkanVertexBufferLayoutImpl>(this, vertexSize, binding))
{
}

VulkanVertexBufferLayout::~VulkanVertexBufferLayout() noexcept = default;

size_t VulkanVertexBufferLayout::elementSize() const noexcept
{
    return m_impl->m_vertexSize;
}

const UInt32& VulkanVertexBufferLayout::binding() const noexcept
{
    return m_impl->m_binding;
}

const BufferType& VulkanVertexBufferLayout::type() const noexcept
{
    return m_impl->m_bufferType;
}

Array<const BufferAttribute*> VulkanVertexBufferLayout::attributes() const noexcept
{
    return m_impl->m_attributes |
        std::views::transform([](const UniquePtr<BufferAttribute>& attribute) { return attribute.get(); }) |
        std::ranges::to<Array<const BufferAttribute*>>();
}

#if defined(BUILD_DEFINE_BUILDERS)
// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

VulkanVertexBufferLayoutBuilder& VulkanVertexBufferLayoutBuilder::withAttribute(UniquePtr<BufferAttribute>&& attribute)
{
    this->instance()->m_impl->m_attributes.push_back(std::move(attribute));
    return *this;
}

VulkanVertexBufferLayoutBuilder& VulkanVertexBufferLayoutBuilder::withAttribute(const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic, const UInt32& semanticIndex)
{
    return this->withAttribute(std::move(makeUnique<BufferAttribute>(static_cast<UInt32>(this->instance()->attributes().size()), offset, format, semantic, semanticIndex)));
}

VulkanVertexBufferLayoutBuilder& VulkanVertexBufferLayoutBuilder::withAttribute(const UInt32& location, const BufferFormat& format, const UInt32& offset, const AttributeSemantic& semantic, const UInt32& semanticIndex)
{
    return this->withAttribute(std::move(makeUnique<BufferAttribute>(location, offset, format, semantic, semanticIndex)));
}
#endif // defined(BUILD_DEFINE_BUILDERS)