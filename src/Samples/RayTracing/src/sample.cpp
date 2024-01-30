#include "sample.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <random>

// Currently there's nine instances of two geometries.
#define NUM_INSTANCES 9

enum class DescriptorSets : UInt32
{
    StaticData   = 0, // Camera and acceleration structures.
    FrameBuffer  = 1, // The frame buffer descriptor to write into.
    Materials    = 2, // The bind-less material properties array.
    GeometryData = 3, // The shader-local per-geometry data.
    Sampler      = 4  // Skybox sampler state.
};

const Array<Vertex> vertices =
{
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

const Array<UInt16> indices = {
    0, 1, 2, 1, 3, 2,       // Front
    4, 6, 5, 5, 6, 7,       // Back
    8, 9, 10, 9, 11, 10,    // Right
    12, 14, 13, 13, 14, 15, // Left
    16, 17, 18, 17, 19, 18, // Bottom
    20, 22, 21, 21, 22, 23  // Top
};

struct CameraBuffer {
    glm::mat4 ViewProjection;
    glm::mat4 InverseView;
    glm::mat4 InverseProjection;
} camera;

struct MaterialData {
    glm::vec4 Color = { 0.1f, 0.1f, 0.1f, 1.0f };
} materials[NUM_INSTANCES];

struct alignas(8) GeometryData {
    UInt32 Index;
    UInt32 Reflective;
    UInt32 Padding[2];
};

template<typename TRenderBackend> requires
    meta::implements<TRenderBackend, IRenderBackend>
struct FileExtensions {
    static const String SHADER;
};

#ifdef LITEFX_BUILD_VULKAN_BACKEND
const String FileExtensions<VulkanBackend>::SHADER = "spv";
#endif // LITEFX_BUILD_VULKAN_BACKEND
#ifdef LITEFX_BUILD_DIRECTX_12_BACKEND
const String FileExtensions<DirectX12Backend>::SHADER = "dxi";
#endif // LITEFX_BUILD_DIRECTX_12_BACKEND

template<typename TRenderBackend> requires
    meta::implements<TRenderBackend, IRenderBackend>
void initRenderGraph(TRenderBackend* backend, SharedPtr<IInputAssembler>& inputAssemblerState)
{
    using RenderPass = TRenderBackend::render_pass_type;
    using RayTracingPipeline = TRenderBackend::ray_tracing_pipeline_type;
    using PipelineLayout = TRenderBackend::pipeline_layout_type;
    using ShaderProgram = TRenderBackend::shader_program_type;
    using InputAssembler = TRenderBackend::input_assembler_type;
    using Rasterizer = TRenderBackend::rasterizer_type;

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

    // Add the resources to the device state.
    device->state().add(std::move(rayTracingPipeline));
}

void SampleApp::initBuffers(IRenderBackend* backend)
{
    // Get a command buffer. Note that we use the graphics queue here, as it also supports transfers, but additionally allows us to build acceleration structures.
    auto commandBuffer = m_device->defaultQueue(QueueType::Graphics).createCommandBuffer(true);

    // Create the staging buffer.
    // NOTE: The mapping works, because vertex and index buffers have an alignment of 0, so we can treat the whole buffer as a single element the size of the 
    //       whole buffer.
    auto stagedVertices = m_device->factory().createVertexBuffer(*m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Staging, vertices.size());
    stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex), 0);

    // Create the actual vertex buffer and transfer the staging buffer into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", *m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, vertices.size(), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
    commandBuffer->transfer(asShared(std::move(stagedVertices)), *vertexBuffer, 0, 0, vertices.size());

    // Create the staging buffer for the indices. For infos about the mapping see the note about the vertex buffer mapping above.
    auto stagedIndices = m_device->factory().createIndexBuffer(*m_inputAssembler->indexBufferLayout(), ResourceHeap::Staging, indices.size());
    stagedIndices->map(indices.data(), indices.size() * m_inputAssembler->indexBufferLayout()->elementSize(), 0);

    // Create the actual index buffer and transfer the staging buffer into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *m_inputAssembler->indexBufferLayout(), ResourceHeap::Resource, indices.size(), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
    commandBuffer->transfer(asShared(std::move(stagedIndices)), *indexBuffer, 0, 0, indices.size());

    // Before building the acceleration structures the GPU needs to wait for the transfer to finish.
    auto barrier = m_device->makeBarrier(PipelineStage::Transfer, PipelineStage::AccelerationStructureBuild);
    barrier->transition(*vertexBuffer, ResourceAccess::TransferWrite, ResourceAccess::Common);
    barrier->transition(*indexBuffer, ResourceAccess::TransferWrite, ResourceAccess::Common);
    commandBuffer->barrier(*barrier);

    // Pre-build acceleration structures. We start with 2 bottom-level acceleration structures (BLAS) for our simple geometry and a few top-level acceleration structures (TLAS) for the 
    // instances. The geometries share one vertex and index buffer.
    auto vertices = asShared(std::move(vertexBuffer));
    auto indices = asShared(std::move(indexBuffer));
    auto opaque = asShared(std::move(m_device->factory().createBottomLevelAccelerationStructure()));
    opaque->withTriangleMesh({ vertices, indices });
    opaque->allocateBuffer(*m_device);
    auto reflective = asShared(std::move(m_device->factory().createBottomLevelAccelerationStructure()));

    // Add an empty geometry, so that the geometry index of the second one will increase, causing it to get reflective (as the hit group changes). Not the most elegant solution, but
    // works for demonstration purposes.
    auto dummyVertexBuffer = m_device->factory().createVertexBuffer(*m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, 1);
    reflective->withTriangleMesh({ asShared(std::move(dummyVertexBuffer)), SharedPtr<IIndexBuffer>() });
    reflective->withTriangleMesh({ vertices, indices });
    reflective->allocateBuffer(*m_device);

    // Orient instances randomly.
    std::srand(std::time(nullptr));

    auto tlas = m_device->factory().createTopLevelAccelerationStructure("TLAS");
    tlas->withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-3.0f, -3.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 0)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-4.0f, 0.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 1)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-3.0f, 3.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 2)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, -4.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 3)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 4.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 4)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(3.0f, -3.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 5)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(4.0f, 0.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 6)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(3.0f, 3.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 7);

    // Add a non-opaque instance.
    tlas->withInstance(reflective, glm::mat4x3(glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX) * glm::scale(glm::identity<glm::mat4>(), glm::vec3(3.0f))), 8);

    // Allocate a buffer for the TLAS.
    tlas->allocateBuffer(*m_device);

    // Create a scratch buffer.
    auto scratchBufferSize = std::max(std::max(opaque->requiredScratchMemory(), reflective->requiredScratchMemory()), tlas->requiredScratchMemory());
    auto scratchBuffer = asShared(std::move(m_device->factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, scratchBufferSize, 1, ResourceUsage::AllowWrite)));

    // Build the BLAS and the TLAS. We need to barrier in between both to prevent simultaneous scratch buffer writes.
    commandBuffer->buildAccelerationStructure(*opaque, scratchBuffer);
    barrier = m_device->makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
    barrier->transition(*scratchBuffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureWrite);
    commandBuffer->barrier(*barrier);
    commandBuffer->buildAccelerationStructure(*reflective, scratchBuffer);
    barrier = m_device->makeBarrier(PipelineStage::AccelerationStructureBuild, PipelineStage::AccelerationStructureBuild);
    barrier->transition(*scratchBuffer, ResourceAccess::AccelerationStructureWrite, ResourceAccess::AccelerationStructureWrite);
    commandBuffer->barrier(*barrier);
    commandBuffer->buildAccelerationStructure(*tlas, scratchBuffer);

    // TODO: The TLAS allocates an instance buffer that we could release after building the acceleration structure, if we do not need it for future updates.

    // Create a shader binding table from the pipeline and transfer it into a GPU buffer (not necessarily required for such a small SBT, but for demonstration purposes).
    auto& geometryPipeline = dynamic_cast<IRayTracingPipeline&>(m_device->state().pipeline("RT Geometry"));
    auto stagingSBT = geometryPipeline.allocateShaderBindingTable(m_offsets);
    auto shaderBindingTable = m_device->factory().createBuffer("Shader Binding Table", BufferType::ShaderBindingTable, ResourceHeap::Resource, stagingSBT->elementSize(), stagingSBT->elements(), ResourceUsage::TransferDestination);
    commandBuffer->transfer(asShared(std::move(stagingSBT)), *shaderBindingTable, 0, 0, shaderBindingTable->elements());

    // Load and upload skybox texture.
    // NOTE: See textures sample for details. We're not creating mip maps here.
    using ImageDataPtr = UniquePtr<stbi_uc, decltype(&::stbi_image_free)>;

    int width, height, channels;
    auto imageData = ImageDataPtr(::stbi_load("assets/rt_skybox.jpg", &width, &height, &channels, STBI_rgb_alpha), ::stbi_image_free);

    if (imageData == nullptr)
        throw std::runtime_error("Texture could not be loaded: \"assets/rt_skybox.jpg\".");

    auto texture = m_device->factory().createTexture("Skybox", Format::R8G8B8A8_UNORM, Size2d(width, height), ImageDimensions::DIM_2, 1, 1, MultiSamplingLevel::x1, ResourceUsage::TransferDestination);
    barrier = m_device->makeBarrier(PipelineStage::None, PipelineStage::Transfer);
    barrier->transition(*texture, ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
    commandBuffer->barrier(*barrier);
    auto stagedTexture = m_device->factory().createBuffer(BufferType::Other, ResourceHeap::Staging, texture->size(0));
    stagedTexture->map(imageData.get(), texture->size(0), 0);

    // Initialize the camera buffer. The camera buffer is constant, so we only need to create one buffer, that can be read from all frames. Since this is a 
    // write-once/read-multiple scenario, we also transfer the buffer to the more efficient memory heap on the GPU.
    auto& staticDataBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::StaticData));
    auto cameraBuffer = m_device->factory().createBuffer("Camera", staticDataBindingsLayout, 0, ResourceHeap::Dynamic);
    auto staticDataBindings = staticDataBindingsLayout.allocate({ { .resource = *cameraBuffer }, { .resource = *tlas }, { .resource = *texture } });

    // Transfer the skybox texture.
    commandBuffer->transfer(asShared(std::move(stagedTexture)), *texture);
    barrier = m_device->makeBarrier(PipelineStage::Transfer, PipelineStage::None);
    barrier->transition(*texture, ResourceAccess::TransferWrite, ResourceAccess::None, ImageLayout::CopyDestination, ImageLayout::ShaderResource);
    commandBuffer->barrier(*barrier);

    // Update the camera. Since the descriptor set already points to the proper buffer, all changes are implicitly visible.
    this->updateCamera(*commandBuffer, *cameraBuffer);

    // Create a sampler for the skybox (note that a static sampler would make more sense here, but let's not care too much, as it's a demo).
    auto sampler = m_device->factory().createSampler();
    auto& samplerBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::Sampler));
    auto samplerBindings = samplerBindingsLayout.allocate({ { .resource = *sampler } });
        
    // Create and bind the back buffer resource to the ray-tracing pipeline output. Here we use a 3D texture with three layers (one for each back buffer) and only bind each array slice to the resource later.
    auto& swapChain = m_device->swapChain();
    auto backBuffers = m_device->factory().createTexture("Back Buffers", swapChain.surfaceFormat(), swapChain.renderArea(), ImageDimensions::DIM_2, 1u, swapChain.buffers(), MultiSamplingLevel::x1, ResourceUsage::AllowWrite | ResourceUsage::TransferSource);
    auto& outputBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::FrameBuffer));
    auto outputBindings = outputBindingsLayout.allocateMultiple(swapChain.buffers(), [&backBuffers, i = 0u](UInt32 /*descriptorSet*/) mutable -> Enumerable<DescriptorBinding> {
        return { DescriptorBinding { .resource = *backBuffers, .firstElement = i++, .elements = 1 } };
    });

    // Setup random colors for each material. The last one (for the reflective object) can stay the default.
    for (int i{ 0 }; i < NUM_INSTANCES - 1; ++i)
        materials[i].Color = glm::vec4(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, 1.0f);

    // Bind the material data.
    auto& materialBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::Materials));
    auto materialBuffer = m_device->factory().createBuffer("Material Buffer", materialBindingsLayout, 0, ResourceHeap::Dynamic, sizeof(MaterialData), NUM_INSTANCES);
    auto materialBindings = materialBindingsLayout.allocate(NUM_INSTANCES, { { .resource = *materialBuffer } });
    materialBuffer->map(reinterpret_cast<const void*>(&materials[0]), sizeof(MaterialData) * NUM_INSTANCES);

    // End and submit the command buffer and wait for it to finish.
    auto fence = commandBuffer->submit();
    m_device->defaultQueue(QueueType::Graphics).waitFor(fence);
    
    // Add everything to the state.
    m_device->state().add(std::move(tlas)); // No need to store the BLAS, as it is contained in the TLAS.
    m_device->state().add(std::move(cameraBuffer));
    m_device->state().add(std::move(texture));
    m_device->state().add(std::move(sampler));
    m_device->state().add(std::move(materialBuffer));
    m_device->state().add(std::move(shaderBindingTable));
    m_device->state().add(std::move(backBuffers));
    m_device->state().add("Static Data Bindings", std::move(staticDataBindings));
    m_device->state().add("Sampler Bindings", std::move(samplerBindings));
    std::ranges::for_each(outputBindings, [this, i = 0](auto& binding) mutable { m_device->state().add(fmt::format("Output Bindings {0}", i++), std::move(binding)); });
    m_device->state().add("Material Bindings", std::move(materialBindings));
}

void SampleApp::updateCamera(const ICommandBuffer& commandBuffer, IBuffer& buffer) const
{
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
    buffer.map(reinterpret_cast<const void*>(&camera), sizeof(camera));
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
        auto app = reinterpret_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
        app->resize(width, height); 
    });

    ::glfwSetKeyCallback(m_window.get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
        app->keyDown(key, scancode, action, mods);
    });

    // Create a callback for backend startup and shutdown.
    auto startCallback = [this]<typename TBackend>(TBackend * backend) {
        // Store the window handle.
        auto window = m_window.get();

        // Get the proper frame buffer size.
        int width, height;
        ::glfwGetFramebufferSize(window, &width, &height);

        auto adapter = backend->findAdapter(m_adapterId);

        if (adapter == nullptr)
            adapter = backend->findAdapter(std::nullopt);

        auto surface = backend->createSurface(::glfwGetWin32Window(window));

        // Create the device.
        m_device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, Size2d(static_cast<Float>(width), static_cast<Float>(height)), 3);

        // Initialize resources.
        ::initRenderGraph(backend, m_inputAssembler);
        this->initBuffers(backend);

        return true;
    };

    auto stopCallback = []<typename TBackend>(TBackend * backend) {
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

void SampleApp::onResize(const void* sender, ResizeEventArgs e)
{
    // In order to re-create the swap chain, we need to wait for all frames in flight to finish.
    m_device->wait();

    // Resize the frame buffer and recreate the swap chain.
    auto surfaceFormat = m_device->swapChain().surfaceFormat();
    auto renderArea = Size2d(e.width(), e.height());
    m_device->swapChain().reset(surfaceFormat, renderArea, 3);

    // Recreate output images and re-bind them to the output descriptors.
    auto backBuffers = m_device->factory().createTexture("Back Buffers", m_device->swapChain().surfaceFormat(), m_device->swapChain().renderArea(), ImageDimensions::DIM_2, 1u, m_device->swapChain().buffers(), MultiSamplingLevel::x1, ResourceUsage::AllowWrite | ResourceUsage::TransferSource);
    
    for (int i = 0; i < m_device->swapChain().buffers(); ++i)
    {
        auto& outputBindings = m_device->state().descriptorSet(fmt::format("Output Bindings {0}", i));
        outputBindings.update(0, *backBuffers, 0, 0, 1, i, 1);
    }

    m_device->state().release(m_device->state().image("Back Buffers"));
    m_device->state().add(std::move(backBuffers));

    // Also update the camera.
    auto& cameraBuffer = m_device->state().buffer("Camera");
    auto commandBuffer = m_device->defaultQueue(QueueType::Transfer).createCommandBuffer(true);
    this->updateCamera(*commandBuffer, cameraBuffer);
    m_transferFence = commandBuffer->submit();
}

void SampleApp::keyDown(int key, int scancode, int action, int mods)
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
            int monitorCount;

            ::glfwGetWindowPos(m_window.get(), &clientRect.x(), &clientRect.y());
            ::glfwGetWindowSize(m_window.get(), &clientRect.width(), &clientRect.height());
            auto monitors = ::glfwGetMonitors(&monitorCount);
            int highestOverlap = 0;

            for (int i(0); i < monitorCount; ++i)
            {
                auto monitor = monitors[i];
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
    title << this->name() << " | " << "Backend: " << this->activeBackend(BackendType::Rendering)->name() << " | " << static_cast<UInt32>(1000.0f / frameTime) << " FPS";

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

    // Query state. For performance reasons, those state variables should be cached for more complex applications, instead of looking them up every frame.
    auto& geometryPipeline = m_device->state().pipeline("RT Geometry");
    auto& staticDataBindings = m_device->state().descriptorSet("Static Data Bindings");
    auto& materialBindings = m_device->state().descriptorSet("Material Bindings");
    auto& samplerBindings = m_device->state().descriptorSet("Sampler Bindings");
    auto& outputBindings = m_device->state().descriptorSet(fmt::format("Output Bindings {0}", backBuffer));
    auto& shaderBindingTable = m_device->state().buffer("Shader Binding Table");
    auto& backBuffers = m_device->state().image("Back Buffers");
    auto& cameraBuffer = m_device->state().buffer("Camera");

    // Wait for all transfers to finish.
    auto& graphicsQueue = m_device->defaultQueue(QueueType::Graphics);
    graphicsQueue.beginDebugRegion("Ray-Tracing");
    graphicsQueue.waitFor(m_device->defaultQueue(QueueType::Transfer), m_transferFence);
    auto commandBuffer = graphicsQueue.createCommandBuffer(true);

    // Update the camera.
    this->updateCamera(*commandBuffer, cameraBuffer);

    // Transition back buffer image into read-write state.
    auto barrier = m_device->makeBarrier(PipelineStage::None, PipelineStage::Raytracing);
    barrier->transition(backBuffers, 0, 1, backBuffer, 1, 0, ResourceAccess::None, ResourceAccess::ShaderReadWrite, ImageLayout::Undefined, ImageLayout::ReadWrite);
    commandBuffer->barrier(*barrier);

    // Begin rendering on the render pass and use the only pipeline we've created for it.
    commandBuffer->use(geometryPipeline);

    // Bind both descriptor sets to the pipeline.
    commandBuffer->bind(staticDataBindings);
    commandBuffer->bind(outputBindings);
    commandBuffer->bind(materialBindings);
    commandBuffer->bind(samplerBindings);

    // Draw the object and present the frame by ending the render pass.
    commandBuffer->traceRays(m_device->swapChain().renderArea().width(), m_device->swapChain().renderArea().height(), 1, m_offsets, shaderBindingTable, &shaderBindingTable, &shaderBindingTable);

    // Transition the image back into `CopySource` layout.
    barrier = m_device->makeBarrier(PipelineStage::Raytracing, PipelineStage::Transfer);
    barrier->transition(backBuffers, 0, 1, backBuffer, 1, 0, ResourceAccess::ShaderReadWrite, ResourceAccess::TransferRead, ImageLayout::ReadWrite, ImageLayout::CopySource);
    barrier->transition(*m_device->swapChain().image(backBuffer), ResourceAccess::None, ResourceAccess::TransferWrite, ImageLayout::Undefined, ImageLayout::CopyDestination);
    commandBuffer->barrier(*barrier);

    // Copy the back buffer into the current swap chain image.
    commandBuffer->transfer(backBuffers, *m_device->swapChain().image(backBuffer), backBuffers.subresourceId(0, backBuffer, 0));

    // Transition the image back into `Present` layout.
    barrier = m_device->makeBarrier(PipelineStage::Transfer, PipelineStage::Resolve);
    barrier->transition(*m_device->swapChain().image(backBuffer), ResourceAccess::TransferWrite, ResourceAccess::Common, ImageLayout::CopyDestination, ImageLayout::Present);
    commandBuffer->barrier(*barrier);

    // Present.
    auto fence = graphicsQueue.submit(commandBuffer);
    graphicsQueue.endDebugRegion();
    m_device->swapChain().present(fence);
}