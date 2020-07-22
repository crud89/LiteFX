#include <litefx/rendering_pipelines.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class ConstantBuffer::ConstantBufferImpl : public Implement<ConstantBuffer> {
public:
    friend class ConstantBuffer;

private:
    const IDescriptorLayout* m_layout;

public:
    ConstantBufferImpl(ConstantBuffer* parent, const IDescriptorLayout* layout) :
        base(parent), m_layout(layout) { }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

ConstantBuffer::ConstantBuffer(const IDescriptorLayout* layout, const UInt32& elements) :
    m_impl(makePimpl<ConstantBufferImpl>(this, layout)), Buffer(layout->getType(), elements, layout->getElementSize() * elements)
{
    if (layout == nullptr)
        throw std::invalid_argument("The constant buffer descriptor layout must be initialized.");
}

ConstantBuffer::~ConstantBuffer() noexcept = default;

UInt32 ConstantBuffer::getBinding() const noexcept
{
    return m_impl->m_layout->getBinding();
}

const IDescriptorLayout* ConstantBuffer::getLayout() const noexcept
{
    return m_impl->m_layout;
}

BufferType ConstantBuffer::getType() const noexcept
{
    return this->getLayout()->getType();
}