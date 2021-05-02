#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPipelineLayout::DirectX12RenderPipelineLayoutImpl : public Implement<DirectX12RenderPipelineLayout> {
public:
    friend class DirectX12RenderPipelineLayoutBuilder;
    friend class DirectX12RenderPipelineLayout;

public:
    DirectX12RenderPipelineLayoutImpl(DirectX12RenderPipelineLayout* parent) : base(parent) { }

public:
    ComPtr<ID3D12RootSignature> initialize()
    {
        throw;

        //auto shaderProgram = m_parent->getProgram();

        //if (shaderProgram == nullptr)
        //    throw std::runtime_error("The shader program must be initialized.");

        //Array<VkDescriptorSetLayout> descriptorSetLayouts;
        //auto layouts = shaderProgram->getLayouts();

        //std::for_each(std::begin(layouts), std::end(layouts), [&](const IDescriptorSetLayout* layout) {
        //    auto descriptorSetLayout = dynamic_cast<const DirectX12DescriptorSetLayout*>(layout);

        //    if (descriptorSetLayout != nullptr && descriptorSetLayout->handle() != nullptr)
        //        descriptorSetLayouts.push_back(descriptorSetLayout->handle());
        //});

        //LITEFX_TRACE(VULKAN_LOG, "Creating render pipeline layout {0} {{ Sets: {1} }}...", fmt::ptr(m_parent), descriptorSetLayouts.size());

        //// Create the pipeline layout.
        //VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        //pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        //pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        //pipelineLayoutInfo.pushConstantRangeCount = 0;

        //VkPipelineLayout layout;

        //if (::vkCreatePipelineLayout(m_parent->getDevice()->handle(), &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
        //    throw std::runtime_error("Unable to create pipeline layout.");

        //return layout;
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipelineLayout::DirectX12RenderPipelineLayout(const DirectX12RenderPipeline& pipeline) :
    RenderPipelineLayout(), IComResource<ID3D12RootSignature>(nullptr), m_impl(makePimpl<DirectX12RenderPipelineLayoutImpl>(this))
{
    this->handle() = m_impl->initialize();
}

DirectX12RenderPipelineLayout::~DirectX12RenderPipelineLayout() noexcept
{
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPipelineBuilder& DirectX12RenderPipelineLayoutBuilder::go()
{
    auto instance = this->instance();
    instance->handle() = instance->m_impl->initialize();

    return RenderPipelineLayoutBuilder::go();
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IRasterizer>&& rasterizer)
{
    if (rasterizer == nullptr)
        throw std::invalid_argument("The rasterizer must be initialized.");

    this->instance()->use(std::move(rasterizer));
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IInputAssembler>&& inputAssembler)
{
    if (inputAssembler == nullptr)
        throw std::invalid_argument("The input assembler must be initialized.");

    this->instance()->use(std::move(inputAssembler));
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IViewport>&& viewport)
{
    if (viewport == nullptr)
        throw std::invalid_argument("The viewport must be initialized.");

    this->instance()->use(std::move(viewport));
}

void DirectX12RenderPipelineLayoutBuilder::use(UniquePtr<IShaderProgram>&& program)
{
    if (program == nullptr)
        throw std::invalid_argument("The program must be initialized.");

    this->instance()->use(std::move(program));
}

DirectX12RenderPipelineLayoutBuilder& DirectX12RenderPipelineLayoutBuilder::enableDepthTest(const bool& enable)
{
    this->instance()->setDepthTest(enable);
    return *this;
}

DirectX12RenderPipelineLayoutBuilder& DirectX12RenderPipelineLayoutBuilder::enableStencilTest(const bool& enable)
{
    this->instance()->setStencilTest(enable);
    return *this;
}

//DirectX12RasterizerBuilder DirectX12RenderPipelineLayoutBuilder::setRasterizer()
//{
//    return this->make<DirectX12Rasterizer>();
//}
//
//DirectX12InputAssemblerBuilder DirectX12RenderPipelineLayoutBuilder::setInputAssembler()
//{
//    return this->make<DirectX12InputAssembler>();
//}
//
//DirectX12ShaderProgramBuilder DirectX12RenderPipelineLayoutBuilder::setShaderProgram()
//{
//    return this->make<DirectX12ShaderProgram>();
//}

DirectX12ViewportBuilder DirectX12RenderPipelineLayoutBuilder::addViewport()
{
    return this->make<DirectX12Viewport>();
}