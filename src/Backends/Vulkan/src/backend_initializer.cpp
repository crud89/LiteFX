#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanBackendInitializer::VulkanBackendInitializerImpl {
private:
    UniquePtr<ISurface> m_surface = nullptr;
    const IGraphicsAdapter* m_adapter = nullptr;
    Format m_format = Format::B8G8R8A8_UNORM_SRGB;

public:
    VulkanBackendInitializerImpl() = default;

public:
    const ISurface* getSurface() const noexcept { return m_surface.get(); }
    void setSurface(UniquePtr<ISurface>&& surface) noexcept { m_surface = std::move(surface); }
    const IGraphicsAdapter* getAdapter() const noexcept { return m_adapter; }
    void setAdapter(const IGraphicsAdapter* adapter) noexcept { m_adapter = adapter; }
    const Format& getFormat() const noexcept { return m_format; }
    void setFormat(const Format& format) noexcept { m_format = format; }
};

VulkanBackendInitializer::VulkanBackendInitializer(builder_type& parent, UniquePtr<backend_type>&& instance) noexcept :
    BackendInitializer(parent, std::move(instance)), m_impl(makePimpl<VulkanBackendInitializerImpl>())
{
}

VulkanBackendInitializer::~VulkanBackendInitializer() noexcept = default;

AppBuilder& VulkanBackendInitializer::go()
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

    return BackendInitializer<VulkanBackend>::go();
}

VulkanBackendInitializer& VulkanBackendInitializer::withSurface(UniquePtr<ISurface>&& surface)
{
    m_impl->setSurface(std::move(surface));
    return *this;
}

VulkanBackendInitializer& VulkanBackendInitializer::withSurface(VulkanSurface::surface_callback callback)
{
    return this->withSurface(std::move(VulkanSurface::createSurface(*this->instance(), callback)));
}

VulkanBackendInitializer& VulkanBackendInitializer::withAdapter(const UInt32& adapterId)
{
    auto adapter = this->instance()->getAdapter(adapterId);
    
    if (adapter == nullptr)
        throw std::invalid_argument("The argument `adapterId` is invalid.");
    
    m_impl->setAdapter(adapter);
    return *this;
}

VulkanBackendInitializer& VulkanBackendInitializer::withAdapterOrDefault(const Optional<UInt32>& adapterId)
{
    auto adapter = this->instance()->getAdapter(adapterId);
    
    if (adapter == nullptr)
        adapter = this->instance()->getAdapter(std::nullopt);

    m_impl->setAdapter(adapter);
    return *this;
}

VulkanBackendInitializer& VulkanBackendInitializer::useDeviceFormat(const Format& format)
{
    m_impl->setFormat(format);
    return *this;
}