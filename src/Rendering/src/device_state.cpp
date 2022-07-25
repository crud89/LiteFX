#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class DeviceState::DeviceStateImpl : public Implement<DeviceState> {
public:
    friend class DeviceState;

private:
    Dictionary<String, UniquePtr<IRenderPass>> m_renderPasses;
    Dictionary<String, UniquePtr<IPipeline>> m_pipelines;

public:
    DeviceStateImpl(DeviceState* parent) :
        base(parent)
    {
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

DeviceState::DeviceState() noexcept :
    m_impl(makePimpl<DeviceStateImpl>(this))
{
}

DeviceState::~DeviceState() noexcept 
{
    this->clear();
}

void DeviceState::clear()
{
    // Make sure that everything is destroyed in order.

    // Clear pipelines.
    for (auto& pair : m_impl->m_pipelines)
        pair.second = nullptr;

    m_impl->m_pipelines.clear();

    // Clear render passes.
    for (auto& pair : m_impl->m_renderPasses)
        pair.second = nullptr;

    m_impl->m_renderPasses.clear();
}

void DeviceState::add(UniquePtr<IRenderPass>&& renderPass)
{
    this->add(renderPass->name(), std::move(renderPass));
}

void DeviceState::add(const String& id, UniquePtr<IRenderPass>&& renderPass)
{
    if (renderPass == nullptr) [[unlikely]]
        throw new InvalidArgumentException("The render pass must be initialized.");

    if (m_impl->m_renderPasses.contains(id)) [[unlikely]]
        throw InvalidArgumentException("Another render pass with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_renderPasses.insert(std::make_pair(id, std::move(renderPass)));
}

void DeviceState::add(UniquePtr<IPipeline>&& pipeline)
{
    this->add(pipeline->name(), std::move(pipeline));
}

void DeviceState::add(const String& id, UniquePtr<IPipeline>&& pipeline)
{
    if (pipeline == nullptr) [[unlikely]]
        throw new InvalidArgumentException("The pipeline must be initialized.");

    if (m_impl->m_pipelines.contains(id)) [[unlikely]]
        throw InvalidArgumentException("Another pipeline with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_pipelines.insert(std::make_pair(id, std::move(pipeline)));
}

IRenderPass& DeviceState::renderPass(const String& id) const
{
    if (!m_impl->m_renderPasses.contains(id)) [[unlikely]]
        throw InvalidArgumentException("No render pass with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_renderPasses[id];
}

IPipeline& DeviceState::pipeline(const String& id) const
{
    if (!m_impl->m_pipelines.contains(id)) [[unlikely]]
        throw InvalidArgumentException("No pipelines with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_pipelines[id];
}

bool DeviceState::release(const IRenderPass& renderPass)
{
    auto match = std::find_if(m_impl->m_renderPasses.begin(), m_impl->m_renderPasses.end(), [&renderPass](const auto& pair) { return pair.second.get() == &renderPass; });
    
    if (match == m_impl->m_renderPasses.end()) [[unlikely]]
        return false;

    match->second = nullptr;
    m_impl->m_renderPasses.erase(match->first);

    return true;
}

bool DeviceState::release(const IPipeline& pipeline)
{
    auto match = std::find_if(m_impl->m_pipelines.begin(), m_impl->m_pipelines.end(), [&pipeline](const auto& pair) { return pair.second.get() == &pipeline; });

    if (match == m_impl->m_pipelines.end()) [[unlikely]]
        return false;

    match->second = nullptr;
    m_impl->m_pipelines.erase(match->first);

    return true;
}