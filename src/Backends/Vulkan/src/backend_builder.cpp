#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

AppBuilder& VulkanBackendBuilder::go()
{
    //auto adapter = this->instance()->getAdapter();
    //auto surface = this->instance()->getSurface();

    //if (adapter == nullptr)
    //    throw std::runtime_error("No adapter has been defined to use for this backend.");

    //if (surface == nullptr)
    //    throw std::runtime_error("No surface has been defined to use for this backend.");

    //// TODO:
    ////// Find a graphics queue.
    ////auto queue = adapter->findQueue(QueueType::Graphics, surface);
    ////
    ////if (queue == nullptr)
    ////    throw std::runtime_error("Unable to find a fitting command queue to present the specified surface.");
    ////
    ////this->instance()->useDevice(makeUnique<VulkanDevice>(adapter, surface, queue, m_impl->getFormat(), this->instance()->getExtensions()));

    return builder_type::go();
}

VulkanBackendBuilder& VulkanBackendBuilder::withSurface(UniquePtr<ISurface>&& surface)
{
    //m_impl->setSurface(std::move(surface));
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
    
    //m_impl->setAdapter(adapter);
    return *this;
}

VulkanBackendBuilder& VulkanBackendBuilder::withAdapterOrDefault(const Optional<UInt32>& adapterId)
{
    auto adapter = this->instance()->findAdapter(adapterId);
    
    if (adapter == nullptr)
        adapter = this->instance()->findAdapter(std::nullopt);

    //m_impl->setAdapter(adapter);
    return *this;
}