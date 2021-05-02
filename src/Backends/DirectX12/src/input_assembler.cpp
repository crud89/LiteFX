#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12InputAssembler::DirectX12InputAssembler(const DirectX12RenderPipelineLayout& layout) noexcept :
    InputAssembler()
{
}

DirectX12InputAssembler::~DirectX12InputAssembler() noexcept = default;

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12InputAssemblerBuilder& DirectX12InputAssemblerBuilder::withTopology(const PrimitiveTopology& topology)
{
    this->instance()->setTopology(topology);
    return *this;
}

void DirectX12InputAssemblerBuilder::use(UniquePtr<IVertexBufferLayout>&& layout)
{
    this->instance()->use(std::move(layout));
}

void DirectX12InputAssemblerBuilder::use(UniquePtr<IIndexBufferLayout>&& layout)
{
    this->instance()->use(std::move(layout));
}

DirectX12InputAssemblerBuilder& DirectX12InputAssemblerBuilder::withIndexType(const IndexType& type)
{
    this->instance()->use(makeUnique<DirectX12IndexBufferLayout>(*this->instance(), type));
    return *this;
}

DirectX12VertexBufferLayoutBuilder DirectX12InputAssemblerBuilder::addVertexBuffer(const size_t& elementSize, const UInt32& binding)
{
    return this->make<DirectX12VertexBufferLayout>(elementSize, binding);
}