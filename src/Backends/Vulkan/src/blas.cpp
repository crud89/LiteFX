#include <litefx/backends/vulkan.hpp>

using namespace LiteFX::Rendering::Backends;
using TriangleMesh  = IBottomLevelAccelerationStructure::TriangleMesh;
using BoundingBoxes = IBottomLevelAccelerationStructure::BoundingBoxes;

extern PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructure;
extern PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructure;

// ------------------------------------------------------------------------------------------------
// Implementation.
// ------------------------------------------------------------------------------------------------

class VulkanBottomLevelAccelerationStructure::VulkanBottomLevelAccelerationStructureImpl : public Implement<VulkanBottomLevelAccelerationStructure> {
public:
    friend class VulkanBottomLevelAccelerationStructure;

private:
    Array<TriangleMesh>  m_triangleMeshes { };
    Array<BoundingBoxes> m_boundingBoxes  { };
    AccelerationStructureFlags m_flags;
    UniquePtr<IVulkanBuffer> m_buffer;
    UInt64 m_scratchBufferSize { };
    const VulkanDevice* m_device { nullptr };

public:
    VulkanBottomLevelAccelerationStructureImpl(VulkanBottomLevelAccelerationStructure* parent, AccelerationStructureFlags flags) :
        base(parent), m_flags(flags)
    {
        if (LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastBuild) && LITEFX_FLAG_IS_SET(flags, AccelerationStructureFlags::PreferFastTrace)) [[unlikely]]
            throw InvalidArgumentException("flags", "Cannot combine acceleration structure flags `PreferFastBuild` and `PreferFastTrace`.");
    }

public:
    Array<std::pair<UInt32, VkAccelerationStructureGeometryKHR>> build() const
    {
        return [this]() -> std::generator<std::pair<UInt32, VkAccelerationStructureGeometryKHR>> {
            // Build up mesh descriptions.
            for (UInt32 i{ 0 }; auto& mesh : m_triangleMeshes)
            {
                // Find the position attribute.
                auto attributes = mesh.VertexBuffer->layout().attributes();
                auto positionAttribute = std::ranges::find_if(attributes, [](const BufferAttribute* attribute) { return attribute->semantic() == AttributeSemantic::Position; });

                if (positionAttribute == attributes.end()) [[unlikely]]
                    throw RuntimeException("A vertex buffer must contain a position attribute to be used in a bottom-level acceleration structure.");

                if ((*positionAttribute)->offset() != 0) [[unlikely]]
                    throw RuntimeException("The position attribute must not have a non-zero offset in the vertex buffer layout.");

                UInt32 primitiveCount = mesh.IndexBuffer == nullptr ? mesh.VertexBuffer->elements() / 3 : mesh.IndexBuffer->elements() / 3;

                co_yield std::make_pair(primitiveCount, VkAccelerationStructureGeometryKHR {
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                    .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
                    .geometry = {
                        .triangles = {
                            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                            .vertexFormat = Vk::getFormat((*positionAttribute)->format()),
                            .vertexData = mesh.VertexBuffer->virtualAddress(),
                            .vertexStride = mesh.VertexBuffer->alignedElementSize(),
                            .maxVertex = mesh.VertexBuffer->elements(),
                            .indexType = mesh.IndexBuffer == nullptr ? VK_INDEX_TYPE_NONE_KHR : (mesh.IndexBuffer->layout().indexType() == IndexType::UInt16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32),
                            .indexData = mesh.IndexBuffer == nullptr ? 0 : mesh.IndexBuffer->virtualAddress(),
                            .transformData = mesh.TransformBuffer == nullptr ? 0 : mesh.TransformBuffer->virtualAddress()
                        }
                    },
                    .flags = std::bit_cast<VkGeometryFlagsKHR>(mesh.Flags)
                });
            }

            // Build up AABB descriptions.
            for (UInt32 i{ 0 }; auto& bb : m_boundingBoxes)
            {
                if (bb.Buffer == nullptr) [[unlikely]]
                    throw RuntimeException("Cannot build bottom-level acceleration structure from uninitialized bounding boxes.");

                UInt32 primitiveCount = bb.Buffer->elements();

                co_yield std::make_pair(primitiveCount, VkAccelerationStructureGeometryKHR {
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                    .geometryType = VK_GEOMETRY_TYPE_AABBS_KHR,
                    .geometry = {
                        .aabbs = {
                            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
                            .data = bb.Buffer->virtualAddress(),
                            .stride = bb.Buffer->alignedElementSize()
                        }
                    },
                    .flags = std::bit_cast<VkGeometryFlagsKHR>(bb.Flags)
                });
            }
        }() | std::ranges::to<Array<std::pair<UInt32, VkAccelerationStructureGeometryKHR>>>();
    }
};

// ------------------------------------------------------------------------------------------------
// Shared interface.
// ------------------------------------------------------------------------------------------------

VulkanBottomLevelAccelerationStructure::VulkanBottomLevelAccelerationStructure(AccelerationStructureFlags flags, StringView name) :
    m_impl(makePimpl<VulkanBottomLevelAccelerationStructureImpl>(this, flags)), Resource(VK_NULL_HANDLE), StateResource(name)
{
}

VulkanBottomLevelAccelerationStructure::~VulkanBottomLevelAccelerationStructure() noexcept
{
    if (this->handle() != VK_NULL_HANDLE)
        ::vkDestroyAccelerationStructure(m_impl->m_device->handle(), this->handle(), nullptr);
}

AccelerationStructureFlags VulkanBottomLevelAccelerationStructure::flags() const noexcept
{
    return m_impl->m_flags;
}

UInt64 VulkanBottomLevelAccelerationStructure::requiredScratchMemory() const noexcept
{
    return m_impl->m_scratchBufferSize;
}

const IVulkanBuffer* VulkanBottomLevelAccelerationStructure::buffer() const noexcept
{
    return m_impl->m_buffer.get();
}

void VulkanBottomLevelAccelerationStructure::allocateBuffer(const VulkanDevice& device)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("The buffer for this acceleration structure has already been allocated.");

    // Store the device.
    m_impl->m_device = &device;

    // Compute buffer sizes.
    UInt64 bufferSize{ };
    device.computeAccelerationStructureSizes(*this, bufferSize, m_impl->m_scratchBufferSize);

    // Allocate the buffer.
    m_impl->m_buffer = device.factory().createBuffer(BufferType::AccelerationStructure, ResourceHeap::Resource, bufferSize, 1, ResourceUsage::AllowWrite);

    // Create a handle for the acceleration structure.
    VkAccelerationStructureCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .buffer = std::as_const(*m_impl->m_buffer).handle(),
        .size = m_impl->m_buffer->alignedElementSize(),
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
    };

    ::vkCreateAccelerationStructure(device.handle(), &info, nullptr, &this->handle());
}

const Array<TriangleMesh>& VulkanBottomLevelAccelerationStructure::triangleMeshes() const noexcept
{
    return m_impl->m_triangleMeshes;
}

void VulkanBottomLevelAccelerationStructure::addTriangleMesh(const TriangleMesh& mesh)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("An acceleration structure cannot be modified after buffers for it have been created.");

    if (!m_impl->m_boundingBoxes.empty()) [[unlikely]]
        throw RuntimeException("A bottom-level acceleration structure can only contain either bounding boxes or triangle meshes, but not both at the same time.");

    m_impl->m_triangleMeshes.push_back(mesh);
}

const Array<BoundingBoxes>& VulkanBottomLevelAccelerationStructure::boundingBoxes() const noexcept
{
    return m_impl->m_boundingBoxes;
}

void VulkanBottomLevelAccelerationStructure::addBoundingBox(const BoundingBoxes& aabb)
{
    if (m_impl->m_buffer != nullptr) [[unlikely]]
        throw RuntimeException("An acceleration structure cannot be modified after buffers for it have been created.");

    if (!m_impl->m_triangleMeshes.empty()) [[unlikely]]
        throw RuntimeException("A bottom-level acceleration structure can only contain either bounding boxes or triangle meshes, but not both at the same time.");

    m_impl->m_boundingBoxes.push_back(aabb);
}

Array<std::pair<UInt32, VkAccelerationStructureGeometryKHR>> VulkanBottomLevelAccelerationStructure::buildInfo() const
{
    return m_impl->build();
}

void VulkanBottomLevelAccelerationStructure::makeBuffer(const IGraphicsDevice& device)
{
    this->allocateBuffer(dynamic_cast<const VulkanDevice&>(device));
}