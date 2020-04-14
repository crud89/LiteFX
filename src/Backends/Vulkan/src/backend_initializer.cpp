#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanBackendBuilder::VulkanBackendBuilderImpl {
private:
    UniquePtr<ISurface> m_surface = nullptr;
    const IGraphicsAdapter* m_adapter = nullptr;

public:
    VulkanBackendBuilderImpl() = default;

public:
    const ISurface* getSurface() const noexcept { return m_surface.get(); }
    void setSurface(UniquePtr<ISurface>&& surface) noexcept { m_surface = std::move(surface); }
    const IGraphicsAdapter* getAdapter() const noexcept { return m_adapter; }
    void setAdapter(const IGraphicsAdapter* adapter) noexcept { m_adapter = adapter; }
};

VulkanBackendBuilder::VulkanBackendBuilder(builder_type& parent, UniquePtr<backend_type>&& instance) noexcept :
    BackendBuilder(parent, std::move(instance)), m_impl(makePimpl<VulkanBackendBuilderImpl>())
{
}

VulkanBackendBuilder::VulkanBackendBuilder(builder_type& parent, UniquePtr<backend_type>&& instance, const Optional<UInt32>& adapterId, UniquePtr<ISurface>&& surface) :
    BackendBuilder(parent, std::move(instance)), m_impl(makePimpl<VulkanBackendBuilderImpl>())
{
    this->withAdapterOrDefault(adapterId);

    if (surface != nullptr)
        this->withSurface(std::move(surface));
}

VulkanBackendBuilder::~VulkanBackendBuilder() noexcept = default;

AppBuilder& VulkanBackendBuilder::go()
{
    auto adapter = m_impl->getAdapter();
    auto surface = m_impl->getSurface();

    if (adapter == nullptr)
        throw std::runtime_error("No adapter has been defined to use for this backend.");

    if (surface == nullptr)
        throw std::runtime_error("No surface has been defined to use for this backend.");

    // TODO:
    //// Find a graphics queue.
    //auto queue = adapter->findQueue(QueueType::Graphics, surface);
    //
    //if (queue == nullptr)
    //    throw std::runtime_error("Unable to find a fitting command queue to present the specified surface.");
    //
    //this->instance()->useDevice(makeUnique<VulkanDevice>(adapter, surface, queue, m_impl->getFormat(), this->instance()->getExtensions()));

    return BackendBuilder<VulkanBackend>::go();
}

VulkanBackendBuilder& VulkanBackendBuilder::withSurface(UniquePtr<ISurface>&& surface)
{
    m_impl->setSurface(std::move(surface));
    return *this;
}

VulkanBackendBuilder& VulkanBackendBuilder::withSurface(VulkanSurface::surface_callback callback)
{
    return this->withSurface(std::move(VulkanSurface::createSurface(*this->instance(), callback)));
}

VulkanBackendBuilder& VulkanBackendBuilder::withAdapter(const UInt32& adapterId)
{
    auto adapter = this->instance()->findAdapter(adapterId);
    
    if (adapter == nullptr)
        throw std::invalid_argument("The argument `adapterId` is invalid.");
    
    m_impl->setAdapter(adapter);
    return *this;
}

VulkanBackendBuilder& VulkanBackendBuilder::withAdapterOrDefault(const Optional<UInt32>& adapterId)
{
    auto adapter = this->instance()->findAdapter(adapterId);
    
    if (adapter == nullptr)
        adapter = this->instance()->findAdapter(std::nullopt);

    m_impl->setAdapter(adapter);
    return *this;
}