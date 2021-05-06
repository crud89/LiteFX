#include <litefx/backends/dx12.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DirectX12RenderPass::DirectX12RenderPassImpl : public Implement<DirectX12RenderPass> {
public:
    friend class DirectX12RenderPassBuilder;
    friend class DirectX12RenderPass;

private:
    Array<UniquePtr<IRenderPipeline>> m_pipelines;
    const DirectX12Device* m_device;
    //const DirectX12SwapChain* m_swapChain{ nullptr };
    //const DirectX12Queue* m_queue{ nullptr };
    Array<UniquePtr<IRenderTarget>> m_targets;
    //Array<VkClearValue> m_clearValues;
    //Array<VkFramebuffer> m_frameBuffers;
    //Array<UniquePtr<DirectX12CommandBuffer>> m_commandBuffers;
    //UInt32 m_backBuffer{ 0 };
    //Array<VkSemaphore> m_semaphores;
    const DirectX12RenderPass* m_dependency{ nullptr };
    //bool m_present{ false };

    ///// <summary>
    ///// Stores the images for all attachments (except the present attachment, which is a swap-chain image) and maps them to the frame buffer index.
    ///// </summary>
    //Dictionary<UInt32, Array<UniquePtr<IImage>>> m_attachmentImages;

public:
    DirectX12RenderPassImpl(DirectX12RenderPass* parent, const DirectX12Device* device) : 
        base(parent), m_device(device)
    {
        if (device == nullptr)
            throw ArgumentNotInitializedException("The device is not a valid DirectX 12 device.");
    }

public:
    void initialize()
    {
    }

    void begin()
    {
    }

    void end(const bool present = false)
    {
    }

public:
    //const DirectX12CommandBuffer* getCurrentCommandBuffer() const noexcept
    //{
    //    return m_commandBuffers[m_backBuffer].get();
    //}

    //const VkSemaphore& getCurrentSemaphore() const noexcept
    //{
    //    return m_semaphores[m_backBuffer];
    //}

    void addTarget(UniquePtr<IRenderTarget>&& target)
    {
        m_targets.push_back(std::move(target));
    }

    Array<const IRenderTarget*> getTargets() const noexcept
    {
        Array<const IRenderTarget*> targets(m_targets.size());
        std::generate(std::begin(targets), std::end(targets), [&, i = 0]() mutable { return m_targets[i++].get(); });

        return targets;
    }

    UniquePtr<IRenderTarget> removeTarget(const IRenderTarget* target)
    {
        auto it = std::find_if(std::begin(m_targets), std::end(m_targets), [target](const UniquePtr<IRenderTarget>& t) { return t.get() == target; });

        if (it == m_targets.end())
            return UniquePtr<IRenderTarget>();
        else
        {
            auto result = std::move(*it);
            m_targets.erase(it);

            return std::move(result);
        }
    }
};

// ------------------------------------------------------------------------------------------------
// Interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPass::DirectX12RenderPass(const IGraphicsDevice* device) :
    DirectX12RenderPass(dynamic_cast<const DirectX12Device*>(device))
{
}

DirectX12RenderPass::DirectX12RenderPass(const DirectX12Device* device) :
    m_impl(makePimpl<DirectX12RenderPassImpl>(this, device)), DirectX12RuntimeObject(device)
{
}

DirectX12RenderPass::~DirectX12RenderPass() noexcept = default;

const DirectX12Device* DirectX12RenderPass::getDevice() const noexcept
{
    return m_impl->m_device;
}

const DirectX12CommandBuffer* DirectX12RenderPass::getDXCommandBuffer() const noexcept
{
    throw;
}

const ICommandBuffer* DirectX12RenderPass::getCommandBuffer() const noexcept
{
    throw;
}

const UInt32 DirectX12RenderPass::getCurrentBackBuffer() const
{
    throw;
}

void DirectX12RenderPass::addTarget(UniquePtr<IRenderTarget>&& target)
{
    m_impl->addTarget(std::move(target));
}

const Array<const IRenderTarget*> DirectX12RenderPass::getTargets() const noexcept
{
    return m_impl->getTargets();
}

UniquePtr<IRenderTarget> DirectX12RenderPass::removeTarget(const IRenderTarget* target)
{
    return m_impl->removeTarget(target);
}

Array<const IRenderPipeline*> DirectX12RenderPass::getPipelines() const noexcept
{
    Array<const IRenderPipeline*> pipelines(m_impl->m_pipelines.size());
    std::generate(std::begin(pipelines), std::end(pipelines), [&, i = 0]() mutable { return m_impl->m_pipelines[i++].get(); });

    return pipelines;
}

const IRenderPipeline* DirectX12RenderPass::getPipeline(const UInt32& id) const noexcept
{
    auto match = std::find_if(std::begin(m_impl->m_pipelines), std::end(m_impl->m_pipelines), [&id](const auto& pipeline) { return pipeline->id() == id; });

    return match == m_impl->m_pipelines.end() ? nullptr : match->get();
}

void DirectX12RenderPass::addPipeline(UniquePtr<IRenderPipeline>&& pipeline)
{
    if (pipeline == nullptr)
        throw ArgumentNotInitializedException("The pipeline must be initialized.");

    auto id = pipeline->id();
    auto match = std::find_if(std::begin(m_impl->m_pipelines), std::end(m_impl->m_pipelines), [&id](const auto& pipeline) { return pipeline->id() == id; });

    if (match != m_impl->m_pipelines.end())
        throw InvalidArgumentException("Another pipeline with the ID {0} already has been registered. Pipeline IDs must be unique within a render pass.", id);

    m_impl->m_pipelines.push_back(std::move(pipeline));
}

void DirectX12RenderPass::removePipeline(const UInt32& id)
{
    m_impl->m_pipelines.erase(std::remove_if(std::begin(m_impl->m_pipelines), std::end(m_impl->m_pipelines), [&id](const auto& pipeline) { return pipeline->id() == id; }), std::end(m_impl->m_pipelines));
}

void DirectX12RenderPass::setDependency(const IRenderPass* renderPass)
{
    auto dependency = dynamic_cast<const DirectX12RenderPass*>(renderPass);

    if (dependency == nullptr)
        throw std::invalid_argument("The render pass dependency must be a valid DirectX12 render pass.");

    m_impl->m_dependency = dependency;
}

const IRenderPass* DirectX12RenderPass::getDependency() const noexcept
{
    return m_impl->m_dependency;
}

void DirectX12RenderPass::begin() const
{
    m_impl->begin();
}

void DirectX12RenderPass::end(const bool& present)
{
    m_impl->end(present);
}

void DirectX12RenderPass::draw(const UInt32& vertices, const UInt32& instances, const UInt32& firstVertex, const UInt32& firstInstance) const
{
    throw;
}

void DirectX12RenderPass::drawIndexed(const UInt32& indices, const UInt32& instances, const UInt32& firstIndex, const Int32& vertexOffset, const UInt32& firstInstance) const
{
    throw;
}

const IImage* DirectX12RenderPass::getAttachment(const UInt32& attachmentId) const
{
    throw;
}

// ------------------------------------------------------------------------------------------------
// Builder interface.
// ------------------------------------------------------------------------------------------------

DirectX12RenderPassBuilder::DirectX12RenderPassBuilder(UniquePtr<DirectX12RenderPass>&& instance) :
    RenderPassBuilder(std::move(instance))
{
}

DirectX12RenderPassBuilder::~DirectX12RenderPassBuilder() noexcept = default;

UniquePtr<DirectX12RenderPass> DirectX12RenderPassBuilder::go()
{
    auto instance = this->instance();
    instance->m_impl->initialize();

    return RenderPassBuilder::go();
}

void DirectX12RenderPassBuilder::use(UniquePtr<IRenderTarget>&& target)
{
    this->instance()->addTarget(std::move(target));
}

void DirectX12RenderPassBuilder::use(UniquePtr<IRenderPipeline>&& pipeline)
{
    //if (pipeline == nullptr)
    //    throw std::invalid_argument("The pipeline must be initialized.");

    //this->instance()->m_impl->m_pipeline = std::move(pipeline);
    throw;
}

DirectX12RenderPipelineBuilder DirectX12RenderPassBuilder::addPipeline(const UInt32& id, const String& name)
{
    return this->make<DirectX12RenderPipeline>(id, name);
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::attachTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, const Vector4f& clearValues, bool clearColor, bool clearStencil, bool isVolatile)
{
    UniquePtr<IRenderTarget> target = makeUnique<RenderTarget>();
    target->setType(type);
    target->setFormat(format);
    target->setSamples(samples);
    target->setClearBuffer(clearColor);
    target->setClearStencil(clearStencil);
    target->setVolatile(isVolatile);
    target->setClearValues(clearValues);

    this->use(std::move(target));

    return *this;
}

DirectX12RenderPassBuilder& DirectX12RenderPassBuilder::dependsOn(const IRenderPass* renderPass)
{
    //if (renderPass == nullptr)
    //    throw std::invalid_argument("The render pass must be initialized.");

    //this->instance()->setDependency(renderPass);

    //return *this;
    throw;
}