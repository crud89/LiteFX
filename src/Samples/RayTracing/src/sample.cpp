#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "sample.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <random>

// Currently there's nine instances of two geometries.
constexpr UInt32 NUM_INSTANCES = 9;

enum class DescriptorSets : UInt32 // NOLINT(performance-enum-size)
{
    StaticData   = 0, // Camera and acceleration structures.
    FrameBuffer  = 1, // The frame buffer descriptor to write into.
    Materials    = 2, // The bind-less material properties array.
    GeometryData = 3, // The shader-local per-geometry data.
    Sampler      = 4  // Skybox sampler state.
};

const Array<Vertex> vertices {
    { { -0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, {  1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, {  1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, {  1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, {  1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.33f, 0.33f, 0.33f, 1.0f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } }
};

const Array<UInt16> indices {
    0, 1, 2, 1, 3, 2,       // Front
    4, 6, 5, 5, 6, 7,       // Back
    8, 9, 10, 9, 11, 10,    // Right
    12, 14, 13, 13, 14, 15, // Left
    16, 17, 18, 17, 19, 18, // Bottom
    20, 22, 21, 21, 22, 23  // Top
};

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

struct CameraBuffer {
    glm::mat4 ViewProjection;
    glm::mat4 InverseView;
    glm::mat4 InverseProjection;
} camera;

struct MaterialData { // NOLINT(cppcoreguidelines-avoid-c-arrays)
    glm::vec4 Color = { 0.1f, 0.1f, 0.1f, 1.0f };
} materials[NUM_INSTANCES];

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

struct alignas(8) GeometryData { // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    UInt32 Index;
    UInt32 Reflective;
    UInt32 Padding[2]; // NOLINT(cppcoreguidelines-avoid-c-arrays)
};

template<typename TRenderBackend> requires
    meta::implements<TRenderBackend, IRenderBackend>
struct FileExtensions {
    static const String SHADER;
};

#ifdef LITEFX_BUILD_VULKAN_BACKEND
template<>
const String FileExtensions<VulkanBackend>::SHADER = "spv";
#endif // LITEFX_BUILD_VULKAN_BACKEND
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
template<>
const String FileExtensions<DirectX12Backend>::SHADER = "dxi";
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

template<typename TRenderBackend> requires
    meta::implements<TRenderBackend, IRenderBackend>
void initRenderGraph(TRenderBackend* backend, SharedPtr<IInputAssembler>& inputAssemblerState)
{
    using RayTracingPipeline = TRenderBackend::ray_tracing_pipeline_type;
    using ShaderProgram = TRenderBackend::shader_program_type;
    using InputAssembler = TRenderBackend::input_assembler_type;

    // Get the default device.
    auto device = backend->device("Default");

    // Create input assembler state.
    SharedPtr<InputAssembler> inputAssembler = device->buildInputAssembler()
        .topology(PrimitiveTopology::TriangleList)
        .indexType(IndexType::UInt16)
        .vertexBuffer(sizeof(Vertex), 0)
            .withAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
            .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color), AttributeSemantic::Color)
            .add();

    inputAssemblerState = std::static_pointer_cast<IInputAssembler>(inputAssembler);

    // Create the shader program.
    // NOTE: The hit shader here receives per-invocation data at the descriptor bound to register 0, space/set 1.
    SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
        .withRayGenerationShaderModule("shaders/raytracing_gen." + FileExtensions<TRenderBackend>::SHADER)
        .withClosestHitShaderModule("shaders/raytracing_hit." + FileExtensions<TRenderBackend>::SHADER, DescriptorBindingPoint { .Register = 0, .Space = std::to_underlying(DescriptorSets::GeometryData) })
        .withMissShaderModule("shaders/raytracing_miss." + FileExtensions<TRenderBackend>::SHADER);

    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    // Build a shader record collection and create a ray-tracing pipeline.
    // NOTE: The local data (payload) for the shader invocation must be defined before building the shader binding table. A shader module may occur multiple times with different 
    // payloads, which can become hard to read and debug, hence it is preferred to use local shader data as sparingly as possible. In this particular case we pass the geometry 
    // index to the shader and since out BLAS (defined later) only contains a single geometry, we only need one entry here. If you only target hardware that supports DXR 1.1 or,
    // you can eliminate the payload entirely by calling the `GeometryIndex()` intrinsic from the shader.
    UniquePtr<RayTracingPipeline> rayTracingPipeline = device->buildRayTracingPipeline("RT Geometry",
        shaderProgram->buildShaderRecordCollection()
            .withShaderRecord("shaders/raytracing_gen." + FileExtensions<TRenderBackend>::SHADER)
            .withShaderRecord("shaders/raytracing_miss." + FileExtensions<TRenderBackend>::SHADER)
            .withMeshGeometryHitGroupRecord(std::nullopt, "shaders/raytracing_hit." + FileExtensions<TRenderBackend>::SHADER, GeometryData { .Index = 0, .Reflective = 0 })  // First geometry hit group for first BLAS.
            .withMeshGeometryHitGroupRecord(std::nullopt, "shaders/raytracing_hit." + FileExtensions<TRenderBackend>::SHADER, GeometryData { .Index = 1, .Reflective = 1 })) // Second geometry hit group for second BLAS.
        .maxBounces(16)                       // Important: If changed, the closest hit shader also needs to be updated!
        .maxPayloadSize(sizeof(Float) * 5)    // See HitInfo in raytracing_common.hlsli
        .maxAttributeSize(sizeof(Float) * 2)  // See Attributes in raytracing_common.hlsli
        .layout(shaderProgram->reflectPipelineLayout());
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
    
    // Add the resources to the device state.
    device->state().add(std::move(rayTracingPipeline));
}

void SampleApp::initBuffers(IRenderBackend* /*backend*/)
{
    // Get a command buffer. Note that we use the graphics queue here, as it also supports transfers, but additionally allows us to build acceleration structures.
    auto commandBuffer = m_device->defaultQueue(QueueType::Graphics).createCommandBuffer(true);

    // Create the vertex buffer and transfer the staging buffer into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", *m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, static_cast<UInt32>(vertices.size()), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
    commandBuffer->transfer(vertices.data(), static_cast<UInt32>(vertices.size() * sizeof(::Vertex)), *vertexBuffer, 0, static_cast<UInt32>(vertices.size()));

    // Create the index buffer and transfer the staging buffer into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *m_inputAssembler->indexBufferLayout(), ResourceHeap::Resource, static_cast<UInt32>(indices.size()), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
    commandBuffer->transfer(indices.data(), static_cast<UInt32>(indices.size() * m_inputAssembler->indexBufferLayout()->elementSize()), *indexBuffer, 0, static_cast<UInt32>(indices.size()));

    // Before building the acceleration structures the GPU needs to wait for the transfer to finish.
    auto barrier = m_device->makeBarrier(PipelineStage::Transfer, PipelineStage::AccelerationStructureBuild);
    barrier->transition(*vertexBuffer, ResourceAccess::TransferWrite, ResourceAccess::Common);
    barrier->transition(*indexBuffer, ResourceAccess::TransferWrite, ResourceAccess::Common);
    commandBuffer->barrier(*barrier);

    // Pre-build acceleration structures. We start with 2 bottom-level acceleration structures (BLAS) for our simple geometry and a few top-level acceleration structures (TLAS) for the 
    // instances. The geometries share one vertex and index buffer.
    auto opaque = asShared(m_device->factory().createBottomLevelAccelerationStructure(AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory));
    opaque->withTriangleMesh({ vertexBuffer, indexBuffer });

    // Add an empty geometry, so that the geometry index of the second one will increase, causing it to get reflective (as the hit group changes). Not the most elegant solution, but works 
    // for demonstration purposes.
    auto reflective = asShared(m_device->factory().createBottomLevelAccelerationStructure(AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory));
    auto dummyVertexBuffer = m_device->factory().createVertexBuffer(*m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, 1, ResourceUsage::AccelerationStructureBuildInput);
    reflective->withTriangleMesh({ std::move(dummyVertexBuffer), SharedPtr<IIndexBuffer>() });
    reflective->withTriangleMesh({ vertexBuffer, indexBuffer, nullptr, GeometryFlags::Opaque });

    // Allocate a single buffer for all bottom-level acceleration structures.
    // NOTE: We can use the sizes as offsets here directly, as they are already properly aligned when requested from the device.
    UInt64 opaqueSize{}, opaqueScratchSize{}, reflectiveSize{}, reflectiveScratchSize{};
    m_device->computeAccelerationStructureSizes(*opaque, opaqueSize, opaqueScratchSize);
    m_device->computeAccelerationStructureSizes(*reflective, reflectiveSize, reflectiveScratchSize);
    auto blasBuffer = m_device->factory().createBuffer("BLAS", BufferType::AccelerationStructure, ResourceHeap::Resource, opaqueSize + reflectiveSize, 1u, ResourceUsage::AllowWrite);

    // Orient instances randomly.
    std::srand(static_cast<UInt32>(std::time(nullptr)));

    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    auto tlas = m_device->factory().createTopLevelAccelerationStructure("TLAS", AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory);
    tlas->withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-3.0f, -3.0f, 0.0f)) * glm::eulerAngleXYX(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX)), 0)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-4.0f, 0.0f, 0.0f)) * glm::eulerAngleXYX(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX)), 1)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-3.0f, 3.0f, 0.0f)) * glm::eulerAngleXYX(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX)), 2)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, -4.0f, 0.0f)) * glm::eulerAngleXYX(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX)), 3)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 4.0f, 0.0f)) * glm::eulerAngleXYX(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX)), 4)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(3.0f, -3.0f, 0.0f)) * glm::eulerAngleXYX(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX)), 5)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(4.0f, 0.0f, 0.0f)) * glm::eulerAngleXYX(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX)), 6)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(3.0f, 3.0f, 0.0f)) * glm::eulerAngleXYX(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX)), 7);

    // Add the reflective instance.
    tlas->withInstance(reflective, glm::mat4x3(glm::eulerAngleXYX(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX) * glm::scale(glm::identity<glm::mat4>(), glm::vec3(3.0f))), 8);
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
    
    // Create a scratch buffer.
    UInt64 tlasSize{}, tlasScratchSize{};
    m_device->computeAccelerationStructureSizes(*tlas, tlasSize, tlasScratchSize);
    auto scratchBufferSize = std::max(std::max(opaqueScratchSize, reflectiveScratchSize), tlasScratchSize);
    auto scratchBuffer = m_device->factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, scratchBufferSize, 1, ResourceUsage::AllowWrite);

    // Build the BLAS and the TLAS. We need to barrier in between both to prevent simultaneous scratch buffer writes.
    opaque->build(*commandBuffer, scratchBuffer, blasBuffer, 0, opaqueSize);
    barrier = m_device->makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
    barrier->transition(*scratchBuffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureWrite);
    commandBuffer->barrier(*barrier);
    reflective->build(*commandBuffer, scratchBuffer, blasBuffer, opaqueSize, reflectiveSize);
    barrier = m_device->makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
    barrier->transition(*scratchBuffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureWrite);
    commandBuffer->barrier(*barrier);
    tlas->build(*commandBuffer, scratchBuffer);

    // Create a shader binding table from the pipeline and transfer it into a GPU buffer (not necessarily required for such a small SBT, but for demonstration purposes).
    auto& geometryPipeline = dynamic_cast<IRayTracingPipeline&>(m_device->state().pipeline("RT Geometry"));
    auto stagingSBT = geometryPipeline.allocateShaderBindingTable(m_offsets);
    auto shaderBindingTable = m_device->factory().createBuffer("Shader Binding Table", BufferType::ShaderBindingTable, ResourceHeap::Resource, stagingSBT->elementSize(), stagingSBT->elements(), ResourceUsage::TransferDestination);
    commandBuffer->transfer(std::move(stagingSBT), *shaderBindingTable, 0, 0, shaderBindingTable->elements());

    // Load and upload skybox texture.
    // NOTE: See textures sample for details. We're not creating mip maps here.
    using ImageDataPtr = UniquePtr<stbi_uc, decltype(&::stbi_image_free)>;

    int width{}, height{}, channels{};
    auto imageData = ImageDataPtr(::stbi_load("assets/rt_skybox.jpg", &width, &height, &channels, STBI_rgb_alpha), ::stbi_image_free);

    if (imageData == nullptr)
        throw std::runtime_error("Texture could not be loaded: \"assets/rt_skybox.jpg\".");

    auto texture = m_device->factory().createTexture("Skybox", Format::R8G8B8A8_UNORM, Size2d(width, height), ImageDimensions::DIM_2, 1, 1, MultiSamplingLevel::x1, ResourceUsage::TransferDestination);
    barrier = m_device->makeBarrier(PipelineStage::None, PipelineStage::Transfer);
    barrier->transition(*texture, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
    commandBuffer->barrier(*barrier);

    // Transfer the skybox texture.
    commandBuffer->transfer(imageData.get(), texture->size(0), *texture);
    barrier = m_device->makeBarrier(PipelineStage::Transfer, PipelineStage::None);
    barrier->transition(*texture, ResourceAccess::TransferWrite, ResourceAccess::None, ImageLayout::CopyDestination, ImageLayout::ShaderResource);
    commandBuffer->barrier(*barrier);

    // Create a sampler for the skybox (note that a static sampler would make more sense here, but let's not care too much, as it's a demo).
    auto sampler = m_device->factory().createSampler();
    auto& samplerBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::Sampler));
    auto samplerBindings = samplerBindingsLayout.allocate({ { .resource = *sampler } });
        
    // Create and bind the back buffer resource to the ray-tracing pipeline output. Here we use a 3D texture with three layers (one for each back buffer) and only bind each array slice to the resource later.
    auto& swapChain = m_device->swapChain();
    auto backBuffers = m_device->factory().createTexture("Back Buffers", swapChain.surfaceFormat(), swapChain.renderArea(), ImageDimensions::DIM_2, 1u, swapChain.buffers(), MultiSamplingLevel::x1, ResourceUsage::AllowWrite | ResourceUsage::TransferSource);
    auto& outputBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::FrameBuffer));
    auto outputBindings = outputBindingsLayout.allocate(swapChain.buffers(), [&backBuffers](UInt32 set) -> Generator<DescriptorBinding> {
        co_yield { .resource = *backBuffers, .firstElement = set, .elements = 1 };
    }) | std::ranges::to<Array<UniquePtr<IDescriptorSet>>>();

    // Setup random colors for each material. The last one (for the reflective object) can stay the default.
    for (UInt32 i{ 0 }; i < NUM_INSTANCES - 1; ++i)
        materials[i].Color = glm::vec4(static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, static_cast<float>(std::rand()) / (float)RAND_MAX, 1.0f); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)

    // Bind the material data.
    auto& materialBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::Materials));
    auto materialBuffer = m_device->factory().createBuffer("Material Buffer", materialBindingsLayout, 0, ResourceHeap::Dynamic, sizeof(MaterialData), NUM_INSTANCES);
    auto materialBindings = materialBindingsLayout.allocate(NUM_INSTANCES, { { .resource = *materialBuffer } });
    materialBuffer->map(static_cast<const void*>(&materials[0]), sizeof(MaterialData) * NUM_INSTANCES);

    // End and submit the command buffer and wait for it to finish.
    auto fence = commandBuffer->submit();
    m_device->defaultQueue(QueueType::Graphics).waitFor(fence);

    // Compact the acceleration structures and setup static bindings.
    {
        // Get compacted sizes to allocate enough memory in one single buffer.
        // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
        auto opaqueCompactedSize = Math::align<UInt64>(opaque->size(), 256);
        auto reflectiveCompactedSize = Math::align<UInt64>(reflective->size(), 256);
        auto tlasCompactedSize = Math::align<UInt64>(tlas->size(), 256);
        auto overallSize = opaqueCompactedSize + reflectiveCompactedSize + tlasCompactedSize;
        // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

        // Allocate one buffer for all acceleration structures and allocate them individually.
        auto accelerationStructureBuffer = m_device->factory().createBuffer("Acceleration Structures", BufferType::AccelerationStructure, ResourceHeap::Resource, overallSize, 1u);
        auto compactedOpaque = m_device->factory().createBottomLevelAccelerationStructure("Opaque BLAS");
        auto compactedReflective = m_device->factory().createBottomLevelAccelerationStructure("Reflective BLAS");
        auto compactedTlas = m_device->factory().createTopLevelAccelerationStructure("TLAS");

        // Create a new command buffer to record compaction commands.
        commandBuffer = m_device->defaultQueue(QueueType::Graphics).createCommandBuffer(true);

        // Copy and compress the acceleration structures individually. This will copy the acceleration structures into one buffer as follows: [ tlas, opaque, reflective ]. Building info will not
        // be copied and updates are not supported.
        opaque->copy(*commandBuffer, *compactedOpaque, true, accelerationStructureBuffer, tlasCompactedSize);
        reflective->copy(*commandBuffer, *compactedReflective, true, accelerationStructureBuffer, tlasCompactedSize + opaqueCompactedSize);
        tlas->copy(*commandBuffer, *compactedTlas, true, accelerationStructureBuffer);

        // Submit the command buffer.
        fence = commandBuffer->submit();

        // Initialize the camera buffer. The camera buffer is constant, so we only need to create one buffer, that can be read from all frames. Since this is a 
        // write-once/read-multiple scenario, we also transfer the buffer to the more efficient memory heap on the GPU.
        auto& staticDataBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::StaticData));
        auto cameraBuffer = m_device->factory().createBuffer("Camera", staticDataBindingsLayout, 0, ResourceHeap::Dynamic);
        auto staticDataBindings = staticDataBindingsLayout.allocate({ { .resource = *cameraBuffer }, { .resource = *compactedTlas }, { .resource = *texture } });

        // Update the camera. Since the descriptor set already points to the proper buffer, all changes are implicitly visible.
        this->updateCamera(*cameraBuffer);

        // Store compacted acceleration structure and static bindings.
        m_device->state().add(std::move(compactedOpaque));
        m_device->state().add(std::move(compactedReflective));
        m_device->state().add(std::move(compactedTlas));
        m_device->state().add(std::move(cameraBuffer));
        m_device->state().add("Static Data Bindings", std::move(staticDataBindings));
    }

    // Wait for the second fence.
    m_device->defaultQueue(QueueType::Graphics).waitFor(fence);
    
    // Add everything to the state.
    m_device->state().add(std::move(texture));
    m_device->state().add(std::move(sampler));
    m_device->state().add(std::move(materialBuffer));
    m_device->state().add(std::move(shaderBindingTable));
    m_device->state().add(std::move(backBuffers));
    m_device->state().add("Sampler Bindings", std::move(samplerBindings));
    std::ranges::for_each(outputBindings, [this, i = 0](auto& binding) mutable { m_device->state().add(std::format("Output Bindings {0}", i++), std::move(binding)); });
    m_device->state().add("Material Bindings", std::move(materialBindings));
}

void SampleApp::updateCamera(IBuffer& buffer) const
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    // Store the initial time this method has been called first.
    static auto start = std::chrono::high_resolution_clock::now();

    // Get the amount of time that has passed since the first frame.
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();
    time *= 0.5f; // Slow down a bit.
    auto position = glm::vec3(std::sinf(time), std::cosf(time), 0.3f) * 7.5f;

    // Calculate the camera view/projection matrix.
    auto aspectRatio = static_cast<Float>(m_device->swapChain().renderArea().width()) / static_cast<Float>(m_device->swapChain().renderArea().height());
    glm::mat4 view = glm::lookAt(position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f);
    camera.ViewProjection = projection * view;
    camera.InverseView = glm::inverse(view);
    camera.InverseProjection = glm::inverse(projection);

    // Update the camera buffer.
    buffer.map(static_cast<const void*>(&camera), sizeof(camera));
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

void SampleApp::onStartup()
{
    // Run application loop until the window is closed.
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        this->handleEvents();
        this->drawFrame();
        this->updateWindowTitle();
    }
}

void SampleApp::onShutdown()
{
    // Destroy the window.
    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}

void SampleApp::onInit()
{
    ::glfwSetWindowUserPointer(m_window.get(), this);

    ::glfwSetFramebufferSizeCallback(m_window.get(), [](GLFWwindow* window, int width, int height) {
        auto app = static_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
        app->resize(width, height); 
    });

    ::glfwSetKeyCallback(m_window.get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = static_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
        app->keyDown(key, scancode, action, mods);
    });

    // Create a callback for backend startup and shutdown.
    auto startCallback = [this]<typename TBackend>(TBackend* backend) {
        // Store the window handle.
        auto window = m_window.get();

        // Get the proper frame buffer size.
        int width{}, height{};
        ::glfwGetFramebufferSize(window, &width, &height);

        auto adapter = backend->findAdapter(m_adapterId);

        if (adapter == nullptr)
            adapter = backend->findAdapter(std::nullopt);

        auto surface = backend->createSurface(::glfwGetWin32Window(window));

        // Create the device.
        m_device = std::addressof(backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, Size2d(width, height), 3, false, GraphicsDeviceFeatures { .RayTracing = true }));

        // Initialize resources.
        ::initRenderGraph(backend, m_inputAssembler);
        this->initBuffers(backend);

        return true;
    };

    auto stopCallback = []<typename TBackend>(TBackend* backend) {
        backend->releaseDevice("Default");
    };

#ifdef LITEFX_BUILD_VULKAN_BACKEND
    // Register the Vulkan backend de-/initializer.
    this->onBackendStart<VulkanBackend>(startCallback);
    this->onBackendStop<VulkanBackend>(stopCallback);
#endif // LITEFX_BUILD_VULKAN_BACKEND

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
    // We do not need to provide a root signature for shader reflection (refer to the project wiki for more information: https://github.com/crud89/LiteFX/wiki/Shader-Development).
    DirectX12ShaderProgram::suppressMissingRootSignatureWarning();

    // Register the DirectX 12 backend de-/initializer.
    this->onBackendStart<DirectX12Backend>(startCallback);
    this->onBackendStop<DirectX12Backend>(stopCallback);
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND
}

void SampleApp::onResize(const void* /*sender*/, const ResizeEventArgs& e)
{
    // In order to re-create the swap chain, we need to wait for all frames in flight to finish.
    m_device->wait();

    // Resize the frame buffer and recreate the swap chain.
    auto surfaceFormat = m_device->swapChain().surfaceFormat();
    auto renderArea = Size2d(e.width(), e.height());
    auto vsync = m_device->swapChain().verticalSynchronization();
    m_device->swapChain().reset(surfaceFormat, renderArea, 3, vsync);

    // Recreate output images and re-bind them to the output descriptors.
    auto backBuffers = m_device->factory().createTexture("Back Buffers", m_device->swapChain().surfaceFormat(), m_device->swapChain().renderArea(), ImageDimensions::DIM_2, 1u, m_device->swapChain().buffers(), MultiSamplingLevel::x1, ResourceUsage::AllowWrite | ResourceUsage::TransferSource);
    
    for (UInt32 i = 0; i < m_device->swapChain().buffers(); ++i)
    {
        auto& outputBindings = m_device->state().descriptorSet(std::format("Output Bindings {0}", i));
        outputBindings.update(0, *backBuffers, 0, 0, 1, i, 1);
    }

    m_device->state().release(m_device->state().image("Back Buffers"));
    m_device->state().add(std::move(backBuffers));

    // Also update the camera.
    this->updateCamera(m_device->state().buffer("Camera"));
}

void SampleApp::keyDown(int key, int /*scancode*/, int action, int /*mods*/)
{
#ifdef LITEFX_BUILD_VULKAN_BACKEND
    if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
        this->startBackend<VulkanBackend>();
#endif // LITEFX_BUILD_VULKAN_BACKEND

#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
        this->startBackend<DirectX12Backend>();
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

    if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
    {
        static RectI windowRect;

        // Check if we're switching from fullscreen to windowed or the other way around.
        if (::glfwGetWindowMonitor(m_window.get()) == nullptr)
        {
            // Find the monitor, that contains most of the window.
            RectI clientRect, monitorRect;
            GLFWmonitor* currentMonitor = nullptr;
            const GLFWvidmode* currentVideoMode = nullptr;
            int monitorCount{};

            ::glfwGetWindowPos(m_window.get(), &clientRect.x(), &clientRect.y());
            ::glfwGetWindowSize(m_window.get(), &clientRect.width(), &clientRect.height());
            auto monitors = ::glfwGetMonitors(&monitorCount);
            int highestOverlap = 0;

            for (int i(0); i < monitorCount; ++i)
            {
                auto monitor = monitors[i]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                auto mode = ::glfwGetVideoMode(monitor);
                ::glfwGetMonitorPos(monitor, &monitorRect.x(), &monitorRect.y());
                monitorRect.width() = mode->width;
                monitorRect.height() = mode->height;

                auto overlap =
                    std::max(0, std::min(clientRect.x() + clientRect.width(), monitorRect.x() + monitorRect.width()) - std::max(clientRect.x(), monitorRect.x())) *
                    std::max(0, std::min(clientRect.y() + clientRect.height(), monitorRect.y() + monitorRect.height()) - std::max(clientRect.y(), monitorRect.y()));

                if (highestOverlap < overlap)
                {
                    highestOverlap = overlap;
                    currentMonitor = monitor;
                    currentVideoMode = mode;
                }
            }

            // Save the current window rect in order to restore it later.
            windowRect = clientRect;

            // Switch to fullscreen.
            if (currentVideoMode != nullptr)
                ::glfwSetWindowMonitor(m_window.get(), currentMonitor, 0, 0, currentVideoMode->width, currentVideoMode->height, currentVideoMode->refreshRate);
        }
        else
        {
            // NOTE: If we were to launch in fullscreen mode, we should use something like `max(windowRect.width(), defaultWidth)`.
            ::glfwSetWindowMonitor(m_window.get(), nullptr, windowRect.x(), windowRect.y(), windowRect.width(), windowRect.height(), 0);
        }
    }

    if (key == GLFW_KEY_F7 && action == GLFW_PRESS)
    {
        // Wait for the device.
        m_device->wait();

        // Toggle VSync on the swap chain.
        auto& swapChain = m_device->swapChain();
        swapChain.reset(swapChain.surfaceFormat(), swapChain.renderArea(), swapChain.buffers(), !swapChain.verticalSynchronization());
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        // Close the window with the next loop.
        ::glfwSetWindowShouldClose(m_window.get(), GLFW_TRUE);
    }
}

void SampleApp::updateWindowTitle()
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto frameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - lastTime).count();

    std::stringstream title;
    title << this->name() << " | " << "Backend: " << this->activeBackend(BackendType::Rendering)->name() << " | " << static_cast<UInt32>(1000.0f / frameTime) << " FPS"; // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    ::glfwSetWindowTitle(m_window.get(), title.str().c_str());
    lastTime = std::chrono::high_resolution_clock::now();
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    // Swap the back buffers for the next frame.
    auto backBuffer = m_device->swapChain().swapBackBuffer();
    auto& backBufferImage = *m_device->swapChain().image(backBuffer);

    // Query state. For performance reasons, those state variables should be cached for more complex applications, instead of looking them up every frame.
    auto& geometryPipeline = m_device->state().pipeline("RT Geometry");
    auto& staticDataBindings = m_device->state().descriptorSet("Static Data Bindings");
    auto& materialBindings = m_device->state().descriptorSet("Material Bindings");
    auto& samplerBindings = m_device->state().descriptorSet("Sampler Bindings");
    auto& outputBindings = m_device->state().descriptorSet(std::format("Output Bindings {0}", backBuffer));
    auto& shaderBindingTable = m_device->state().buffer("Shader Binding Table");
    auto& backBuffers = m_device->state().image("Back Buffers");
    auto& cameraBuffer = m_device->state().buffer("Camera");

    // Update the camera.
    this->updateCamera(cameraBuffer);

    // Wait for all transfers to finish.
    auto& graphicsQueue = m_device->defaultQueue(QueueType::Graphics);
    graphicsQueue.beginDebugRegion("Ray-Tracing");
    auto commandBuffer = graphicsQueue.createCommandBuffer(true);

    // Transition back buffer image into read-write state.
    auto barrier = m_device->makeBarrier(PipelineStage::None, PipelineStage::Raytracing);
    barrier->transition(backBuffers, 0, 1, backBuffer, 1, 0, ResourceAccess::None, ResourceAccess::ShaderReadWrite, ImageLayout::Undefined, ImageLayout::ReadWrite);
    commandBuffer->barrier(*barrier);

    // Begin rendering on the render pass and use the only pipeline we've created for it.
    commandBuffer->use(geometryPipeline);

    // Bind both descriptor sets to the pipeline.
    commandBuffer->bind({ &outputBindings, &staticDataBindings, &materialBindings, &samplerBindings });

    // Draw the object and present the frame by ending the render pass.
    commandBuffer->traceRays(static_cast<UInt32>(m_device->swapChain().renderArea().width()), static_cast<UInt32>(m_device->swapChain().renderArea().height()), 1, m_offsets, shaderBindingTable, &shaderBindingTable, &shaderBindingTable);

    // Transition the image back into `CopySource` layout.
    barrier = m_device->makeBarrier(PipelineStage::Raytracing, PipelineStage::Transfer);
    barrier->transition(backBuffers, 0, 1, backBuffer, 1, 0, ResourceAccess::ShaderReadWrite, ResourceAccess::TransferRead, ImageLayout::ReadWrite, ImageLayout::CopySource);
    barrier->transition(backBufferImage, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
    commandBuffer->barrier(*barrier);

    // Copy the back buffer into the current swap chain image.
    commandBuffer->transfer(backBuffers, backBufferImage, backBuffers.subresourceId(0, backBuffer, 0));

    // Transition the image back into `Present` layout.
    barrier = m_device->makeBarrier(PipelineStage::Transfer, PipelineStage::Resolve);
    barrier->transition(backBufferImage, ResourceAccess::TransferWrite, ResourceAccess::Common, ImageLayout::CopyDestination, ImageLayout::Present);
    commandBuffer->barrier(*barrier);

    // Present.
    auto fence = graphicsQueue.submit(commandBuffer);
    graphicsQueue.endDebugRegion();
    m_device->swapChain().present(fence);
}