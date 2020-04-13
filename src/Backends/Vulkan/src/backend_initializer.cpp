#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

class VulkanBackendInitializer::VulkanBackendInitializerImpl {
private:
    UniquePtr<ISurface> m_surface;
    const IGraphicsAdapter* m_adapter;

public:
    VulkanBackendInitializerImpl() noexcept : m_surface(nullptr), m_adapter(nullptr) { }

public:
    const ISurface* getSurface() const noexcept { return m_surface.get(); }
    void setSurface(UniquePtr<ISurface>&& surface) noexcept { m_surface = std::move(surface); }
    const IGraphicsAdapter* getAdapter() const noexcept { return m_adapter; }
    void setAdapter(const IGraphicsAdapter* adapter) noexcept { m_adapter = adapter; }
};

VulkanBackendInitializer::VulkanBackendInitializer(builder_type& parent, UniquePtr<backend_type>&& instance) noexcept :
    BackendInitializer(parent, std::move(instance)), m_impl(makePimpl<VulkanBackendInitializerImpl>())
{
}

VulkanBackendInitializer::~VulkanBackendInitializer() noexcept = default;

AppBuilder& VulkanBackendInitializer::go()
{
    // TODO: Create a device and execute further initialization logic.

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