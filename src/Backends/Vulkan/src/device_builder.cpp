#include <litefx/rendering.hpp>

using namespace LiteFX::Rendering;

// TODO:
//// Find a graphics queue.
//auto queue = adapter->findQueue(QueueType::Graphics, surface);
//
//if (queue == nullptr)
//    throw std::runtime_error("Unable to find a fitting command queue to present the specified surface.");




//class GraphicsDeviceBuilder::GraphicsDeviceBuilderImpl {
//private:
//    const IRenderBackend* m_backend;
//    const ISurface* m_surface = nullptr;
//    const IGraphicsAdapter* m_adapter = nullptr;
//
//public:
//    GraphicsDeviceBuilderImpl(const IRenderBackend* backend) noexcept : m_backend(backend) { }
//
//public:
//    const IRenderBackend* getBackend() const noexcept
//    {
//        return m_backend;
//    }
//
//    const ISurface* getSurface() const noexcept
//    {
//        return m_surface;
//    }
//
//    void setSurface(const ISurface* surface) noexcept
//    {
//        m_surface = surface;
//    }
//
//    const IGraphicsAdapter* getAdapter() const noexcept
//    {
//        return m_adapter;
//    }
//
//    void setAdapter(const IGraphicsAdapter* adapter) noexcept
//    {
//        m_adapter = adapter;
//    }
//};
//
//GraphicsDeviceBuilder::GraphicsDeviceBuilder(const IRenderBackend* backend) :
//    m_impl(makePimpl<GraphicsDeviceBuilderImpl>(backend))
//{
//    if (backend == nullptr)
//        throw std::invalid_argument("The argument `backend` must be initialized.");
//}
//
//GraphicsDeviceBuilder::~GraphicsDeviceBuilder() noexcept = default;
//
//const IRenderBackend* GraphicsDeviceBuilder::getBackend() const noexcept
//{
//    return m_impl->getBackend();
//}
//
//const ISurface* GraphicsDeviceBuilder::getSurface() const noexcept
//{
//    return m_impl->getSurface();
//}
//
//const IGraphicsAdapter* GraphicsDeviceBuilder::getAdapter() const noexcept
//{
//    return m_impl->getAdapter();
//}
//
//GraphicsDeviceBuilder& GraphicsDeviceBuilder::forSurface(const ISurface * surface)
//{
//    if (surface == nullptr)
//        throw std::invalid_argument("The argument `surface` must be initialized.");
//
//    m_impl->setSurface(surface);
//
//    return *this;
//}
//
//GraphicsDeviceBuilder& GraphicsDeviceBuilder::useAdapter(const uint32_t & adapterId)
//{
//    auto adapter = m_impl->getBackend()->getAdapter(adapterId);
//
//    if (adapter == nullptr)
//        throw std::invalid_argument("The argument `adapterId` is invalid.");
//
//    m_impl->setAdapter(adapter);
//
//    return *this;
//}
//
//GraphicsDeviceBuilder& GraphicsDeviceBuilder::useAdapterOrDefault(const Optional<uint32_t> & adapterId)
//{
//    auto adapter = m_impl->getBackend()->getAdapter(adapterId);
//
//    if (adapter == nullptr)
//        adapter = m_impl->getBackend()->getAdapter(std::nullopt);
//
//    m_impl->setAdapter(adapter);
//
//    return *this;
//}