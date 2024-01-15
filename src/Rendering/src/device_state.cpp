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
    Dictionary<String, UniquePtr<IBuffer>> m_buffers;
    Dictionary<String, UniquePtr<IVertexBuffer>> m_vertexBuffers;
    Dictionary<String, UniquePtr<IIndexBuffer>> m_indexBuffers;
    Dictionary<String, UniquePtr<IImage>> m_images;
    Dictionary<String, UniquePtr<ISampler>> m_samplers;
    Dictionary<String, UniquePtr<IDescriptorSet>> m_descriptorSets;

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

    // Clear descriptor sets.
    for (auto& pair : m_impl->m_descriptorSets)
        pair.second = nullptr;

    m_impl->m_descriptorSets.clear();

    // Clear images, samplers and buffers.
    for (auto& pair : m_impl->m_buffers)
        pair.second = nullptr;

    m_impl->m_buffers.clear();

    for (auto& pair : m_impl->m_vertexBuffers)
        pair.second = nullptr;

    m_impl->m_vertexBuffers.clear();

    for (auto& pair : m_impl->m_indexBuffers)
        pair.second = nullptr;

    m_impl->m_indexBuffers.clear();

    for (auto& pair : m_impl->m_images)
        pair.second = nullptr;

    m_impl->m_images.clear();

    for (auto& pair : m_impl->m_samplers)
        pair.second = nullptr;

    m_impl->m_samplers.clear();

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
        throw InvalidArgumentException("renderPass", "The render pass must be initialized.");

    if (m_impl->m_renderPasses.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "Another render pass with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_renderPasses.insert(std::make_pair(id, std::move(renderPass)));
}

void DeviceState::add(UniquePtr<IPipeline>&& pipeline)
{
    this->add(pipeline->name(), std::move(pipeline));
}

void DeviceState::add(const String& id, UniquePtr<IPipeline>&& pipeline)
{
    if (pipeline == nullptr) [[unlikely]]
        throw InvalidArgumentException("pipeline", "The pipeline must be initialized.");

    if (m_impl->m_pipelines.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "Another pipeline with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_pipelines.insert(std::make_pair(id, std::move(pipeline)));
}

void DeviceState::add(UniquePtr<IBuffer>&& buffer)
{
    this->add(buffer->name(), std::move(buffer));
}

void DeviceState::add(const String& id, UniquePtr<IBuffer>&& buffer)
{
    if (buffer == nullptr) [[unlikely]]
        throw InvalidArgumentException("buffer", "The buffer must be initialized.");

    if (m_impl->m_buffers.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "Another buffer with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_buffers.insert(std::make_pair(id, std::move(buffer)));
}

void DeviceState::add(UniquePtr<IVertexBuffer>&& vertexBuffer)
{
    this->add(vertexBuffer->name(), std::move(vertexBuffer));
}

void DeviceState::add(const String& id, UniquePtr<IVertexBuffer>&& vertexBuffer)
{
    if (vertexBuffer == nullptr) [[unlikely]]
        throw InvalidArgumentException("vertexBuffer", "The vertex buffer must be initialized.");

    if (m_impl->m_vertexBuffers.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "Another vertex buffer with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_vertexBuffers.insert(std::make_pair(id, std::move(vertexBuffer)));
}

void DeviceState::add(UniquePtr<IIndexBuffer>&& indexBuffer)
{
    this->add(indexBuffer->name(), std::move(indexBuffer));
}

void DeviceState::add(const String& id, UniquePtr<IIndexBuffer>&& indexBuffer)
{
    if (indexBuffer == nullptr) [[unlikely]]
        throw InvalidArgumentException("indexBuffer", "The index buffer must be initialized.");

    if (m_impl->m_indexBuffers.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "Another index buffer with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_indexBuffers.insert(std::make_pair(id, std::move(indexBuffer)));
}

void DeviceState::add(UniquePtr<IImage>&& image)
{
    this->add(image->name(), std::move(image));
}

void DeviceState::add(const String& id, UniquePtr<IImage>&& image)
{
    if (image == nullptr) [[unlikely]]
        throw InvalidArgumentException("image", "The image must be initialized.");

    if (m_impl->m_images.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "Another image with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_images.insert(std::make_pair(id, std::move(image)));
}

void DeviceState::add(UniquePtr<ISampler>&& sampler)
{
    this->add(sampler->name(), std::move(sampler));
}

void DeviceState::add(const String& id, UniquePtr<ISampler>&& sampler)
{
    if (sampler == nullptr) [[unlikely]]
        throw InvalidArgumentException("sampler", "The sampler must be initialized.");

    if (m_impl->m_samplers.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "Another sampler with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_samplers.insert(std::make_pair(id, std::move(sampler)));
}

void DeviceState::add(const String& id, UniquePtr<IDescriptorSet>&& descriptorSet)
{
    if (descriptorSet == nullptr) [[unlikely]]
        throw InvalidArgumentException("descriptorSet", "The descriptor set must be initialized.");

    if (m_impl->m_descriptorSets.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "Another descriptor set with the identifier \"{0}\" has already been registered in the device state.", id);

    m_impl->m_descriptorSets.insert(std::make_pair(id, std::move(descriptorSet)));
}

IRenderPass& DeviceState::renderPass(const String& id) const
{
    if (!m_impl->m_renderPasses.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "No render pass with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_renderPasses[id];
}

IPipeline& DeviceState::pipeline(const String& id) const
{
    if (!m_impl->m_pipelines.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "No pipelines with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_pipelines[id];
}

IBuffer& DeviceState::buffer(const String& id) const
{
    if (!m_impl->m_buffers.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "No buffers with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_buffers[id];
}

IVertexBuffer& DeviceState::vertexBuffer(const String& id) const
{
    if (!m_impl->m_vertexBuffers.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "No vertex buffers with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_vertexBuffers[id];
}

IIndexBuffer& DeviceState::indexBuffer(const String& id) const
{
    if (!m_impl->m_indexBuffers.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "No index buffers with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_indexBuffers[id];
}

IImage& DeviceState::image(const String& id) const
{
    if (!m_impl->m_images.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "No images with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_images[id];
}

ISampler& DeviceState::sampler(const String& id) const
{
    if (!m_impl->m_samplers.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "No samplers with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_samplers[id];
}

IDescriptorSet& DeviceState::descriptorSet(const String& id) const
{
    if (!m_impl->m_descriptorSets.contains(id)) [[unlikely]]
        throw InvalidArgumentException("id", "No descriptor sets with the identifier \"{0}\" has been registered in the device state.", id);

    return *m_impl->m_descriptorSets[id];
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

bool DeviceState::release(const IBuffer& buffer)
{
    auto match = std::find_if(m_impl->m_buffers.begin(), m_impl->m_buffers.end(), [&buffer](const auto& pair) { return pair.second.get() == &buffer; });

    if (match == m_impl->m_buffers.end()) [[unlikely]]
        return false;

    match->second = nullptr;
    m_impl->m_buffers.erase(match->first);

    return true;
}

bool DeviceState::release(const IVertexBuffer& vertexBuffer)
{
    auto match = std::find_if(m_impl->m_vertexBuffers.begin(), m_impl->m_vertexBuffers.end(), [&vertexBuffer](const auto& pair) { return pair.second.get() == &vertexBuffer; });

    if (match == m_impl->m_vertexBuffers.end()) [[unlikely]]
        return false;

    match->second = nullptr;
    m_impl->m_vertexBuffers.erase(match->first);

    return true;
}

bool DeviceState::release(const IIndexBuffer& indexBuffer)
{
    auto match = std::find_if(m_impl->m_indexBuffers.begin(), m_impl->m_indexBuffers.end(), [&indexBuffer](const auto& pair) { return pair.second.get() == &indexBuffer; });

    if (match == m_impl->m_indexBuffers.end()) [[unlikely]]
        return false;

    match->second = nullptr;
    m_impl->m_indexBuffers.erase(match->first);

    return true;
}

bool DeviceState::release(const IImage& image)
{
    auto match = std::find_if(m_impl->m_images.begin(), m_impl->m_images.end(), [&image](const auto& pair) { return pair.second.get() == &image; });

    if (match == m_impl->m_images.end()) [[unlikely]]
        return false;

    match->second = nullptr;
    m_impl->m_images.erase(match->first);

    return true;
}

bool DeviceState::release(const ISampler& sampler)
{
    auto match = std::find_if(m_impl->m_samplers.begin(), m_impl->m_samplers.end(), [&sampler](const auto& pair) { return pair.second.get() == &sampler; });

    if (match == m_impl->m_samplers.end()) [[unlikely]]
        return false;

    match->second = nullptr;
    m_impl->m_samplers.erase(match->first);

    return true;
}

bool DeviceState::release(const IDescriptorSet& descriptorSet)
{
    auto match = std::find_if(m_impl->m_descriptorSets.begin(), m_impl->m_descriptorSets.end(), [&descriptorSet](const auto& pair) { return pair.second.get() == &descriptorSet; });

    if (match == m_impl->m_descriptorSets.end()) [[unlikely]]
        return false;

    match->second = nullptr;
    m_impl->m_descriptorSets.erase(match->first);

    return true;
}