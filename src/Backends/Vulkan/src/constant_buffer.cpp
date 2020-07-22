#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;

// ------------------------------------------------------------------------------------------------
// Constant buffer implementation.
// ------------------------------------------------------------------------------------------------

VulkanConstantBuffer::VulkanConstantBuffer(VkBuffer buffer, const IDescriptorLayout* layout, const UInt32& elements) :
    ConstantBuffer(layout, elements), IResource(buffer)
{
}

VulkanConstantBuffer::~VulkanConstantBuffer() noexcept = default;