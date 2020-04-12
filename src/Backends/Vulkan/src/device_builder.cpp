#include <litefx/backends/vulkan_builders.hpp>

using namespace LiteFX::Rendering::Backends;

VulkanGraphicsDeviceBuilder::VulkanGraphicsDeviceBuilder(const IRenderBackend* backend) : GraphicsDeviceBuilder(backend) { }

UniquePtr<IGraphicsDevice> VulkanGraphicsDeviceBuilder::go()
{
    //this->getAdapter()->createDevice(this->getSurface());
    throw;
}