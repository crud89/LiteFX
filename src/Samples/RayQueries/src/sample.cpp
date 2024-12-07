#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "sample.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <random>

// Currently there's nine instances of two geometries.
consteval const UInt32 NUM_INSTANCES = 9;

enum class DescriptorSets : UInt32 // NOLINT(performance-enum-size)
{
    StaticData   = 0, // Camera and acceleration structures.
    Materials    = 1, // The bind-less material properties array.
    Sampler      = 2  // Skybox sampler state.
};

const Array<Vertex> _vertices =
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

const Array<UInt16> _indices = {
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
    glm::vec2 ViewportSize;
} camera;

struct MaterialData {
    glm::vec4 Color = { 0.1f, 0.1f, 0.1f, 1.0f };
} materials[NUM_INSTANCES];

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

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
    using RenderPass = TRenderBackend::render_pass_type;
    using RenderPipeline = TRenderBackend::render_pipeline_type;
    using ShaderProgram = TRenderBackend::shader_program_type;
    using InputAssembler = TRenderBackend::input_assembler_type;
    using FrameBuffer = TRenderBackend::frame_buffer_type;

    // Get the default device.
    auto device = backend->device("Default");

    // Create the frame buffers for all back buffers.
    auto frameBuffers = std::views::iota(0u, device->swapChain().buffers()) |
        std::views::transform([&](UInt32 index) { return device->makeFrameBuffer(std::format("Frame Buffer {0}", index), device->swapChain().renderArea()); }) |
        std::ranges::to<Array<SharedPtr<FrameBuffer>>>();

    // Create input assembler state for the geometry.
    SharedPtr<InputAssembler> inputAssembler = device->buildInputAssembler()
        .topology(PrimitiveTopology::TriangleList)
        .indexType(IndexType::UInt16)
        .vertexBuffer(sizeof(Vertex), 0)
            .withAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
            .withAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color), AttributeSemantic::Color)
            .add();

    inputAssemblerState = std::static_pointer_cast<IInputAssembler>(inputAssembler);

    // Create a geometry render pass.
    UniquePtr<RenderPass> renderPass = device->buildRenderPass("Deferred")
        .renderTarget("Color Target", RenderTargetType::Present, Format::B8G8R8A8_UNORM, RenderTargetFlags::Clear, { 0.1f, 0.1f, 0.1f, 1.f })
        .renderTarget("Depth/Stencil Target", RenderTargetType::DepthStencil, Format::D32_SFLOAT, RenderTargetFlags::Clear, { 1.f, 0.f, 0.f, 0.f });

    // Map all render targets to the frame buffer.
    std::ranges::for_each(frameBuffers, [&renderPass](auto& frameBuffer) { frameBuffer->addImages(renderPass->renderTargets()); });

    // Create the shader program.
    SharedPtr<ShaderProgram> shaderProgram = device->buildShaderProgram()
        .withVertexShaderModule("shaders/rayqueries_vs." + FileExtensions<TRenderBackend>::SHADER)
        .withFragmentShaderModule("shaders/rayqueries_fs." + FileExtensions<TRenderBackend>::SHADER);

    // Create a render pipeline. The render pipeline draws only the screen quad, so we need a different input assembler.
    SharedPtr<InputAssembler> pipelineInputAssembler = device->buildInputAssembler()
        .topology(PrimitiveTopology::TriangleStrip);

    UniquePtr<RenderPipeline> renderPipeline = device->buildRenderPipeline(*renderPass, "Geometry")
        .inputAssembler(pipelineInputAssembler)
        .rasterizer(device->buildRasterizer()
            .polygonMode(PolygonMode::Solid)
            .cullMode(CullMode::Disabled)
            .cullOrder(CullOrder::ClockWise)
            .lineWidth(1.f))
        .layout(shaderProgram->reflectPipelineLayout())
        .shaderProgram(shaderProgram);

    // Add the resources to the device state.
    device->state().add(std::move(renderPass));
    device->state().add(std::move(renderPipeline));
    std::ranges::for_each(frameBuffers, [device](auto& frameBuffer) { device->state().add(std::move(frameBuffer)); });
}

void SampleApp::initBuffers(IRenderBackend* /*backend*/)
{
    // Get a command buffer. Note that we use the graphics queue here, as it also supports transfers, but additionally allows us to build acceleration structures.
    auto commandBuffer = m_device->defaultQueue(QueueType::Graphics).createCommandBuffer(true);

    // Create the vertex buffer and transfer the staging buffer into it.
    auto vertexBuffer = m_device->factory().createVertexBuffer("Vertex Buffer", *m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, static_cast<UInt32>(_vertices.size()), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
    commandBuffer->transfer(_vertices.data(), static_cast<UInt32>(_vertices.size() * sizeof(::Vertex)), *vertexBuffer, 0, static_cast<UInt32>(_vertices.size()));

    // Create the index buffer and transfer the staging buffer into it.
    auto indexBuffer = m_device->factory().createIndexBuffer("Index Buffer", *m_inputAssembler->indexBufferLayout(), ResourceHeap::Resource, static_cast<UInt32>(_indices.size()), ResourceUsage::TransferDestination | ResourceUsage::AccelerationStructureBuildInput);
    commandBuffer->transfer(_indices.data(), static_cast<UInt32>(_indices.size() * m_inputAssembler->indexBufferLayout()->elementSize()), *indexBuffer, 0, static_cast<UInt32>(_indices.size()));

    // Before building the acceleration structures the GPU needs to wait for the transfer to finish.
    auto barrier = m_device->makeBarrier(PipelineStage::Transfer, PipelineStage::AccelerationStructureBuild);
    barrier->transition(*vertexBuffer, ResourceAccess::TransferWrite, ResourceAccess::Common);
    barrier->transition(*indexBuffer, ResourceAccess::TransferWrite, ResourceAccess::Common);
    commandBuffer->barrier(*barrier);

    // Pre-build acceleration structures. We start with 2 bottom-level acceleration structures (BLAS) for our simple geometry and a few top-level acceleration structures (TLAS) for the 
    // instances. The geometries share one vertex and index buffer.
    auto vertices = asShared(std::move(vertexBuffer));
    auto indices = asShared(std::move(indexBuffer));
    auto opaque = asShared(m_device->factory().createBottomLevelAccelerationStructure(AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory));
    opaque->withTriangleMesh({ vertices, indices, nullptr, GeometryFlags::Opaque });

    // Add an empty geometry, so that the geometry index of the second one will increase, causing it to get reflective (as the hit group changes). Not the most elegant solution, but works 
    // for demonstration purposes.
    auto reflective = asShared(m_device->factory().createBottomLevelAccelerationStructure(AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory));
    auto dummyVertexBuffer = m_device->factory().createVertexBuffer(*m_inputAssembler->vertexBufferLayout(0), ResourceHeap::Resource, 1, ResourceUsage::AccelerationStructureBuildInput);
    reflective->withTriangleMesh({ asShared(std::move(dummyVertexBuffer)), SharedPtr<IIndexBuffer>() });
    reflective->withTriangleMesh({ vertices, indices, nullptr, GeometryFlags::Opaque });

    // Allocate a single buffer for all bottom-level acceleration structures.
    // NOTE: We can use the sizes as offsets here directly, as they are already properly aligned when requested from the device.
    UInt64 opaqueSize, opaqueScratchSize, reflectiveSize, reflectiveScratchSize;
    m_device->computeAccelerationStructureSizes(*opaque, opaqueSize, opaqueScratchSize);
    m_device->computeAccelerationStructureSizes(*reflective, reflectiveSize, reflectiveScratchSize);
    auto blasBuffer = asShared(m_device->factory().createBuffer("BLAS", BufferType::AccelerationStructure, ResourceHeap::Resource, opaqueSize + reflectiveSize, 1u, ResourceUsage::AllowWrite));

    // Orient instances randomly.
    std::srand(static_cast<UInt32>(std::time(nullptr)));

    auto tlas = m_device->factory().createTopLevelAccelerationStructure("TLAS", AccelerationStructureFlags::AllowCompaction | AccelerationStructureFlags::MinimizeMemory);
    tlas->withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-3.0f, -3.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 0)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-4.0f, 0.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 1)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-3.0f, 3.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 2)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, -4.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 3)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 4.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 4)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(3.0f, -3.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 5)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(4.0f, 0.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 6)
        .withInstance(opaque, glm::mat4x3(glm::translate(glm::identity<glm::mat4>(), glm::vec3(3.0f, 3.0f, 0.0f)) * glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX)), 7);

    // Add the reflective instance.
    tlas->withInstance(reflective, glm::mat4x3(glm::eulerAngleXYX(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX) * glm::scale(glm::identity<glm::mat4>(), glm::vec3(3.0f))), 8);

    // Create a scratch buffer.
    UInt64 tlasSize, tlasScratchSize;
    m_device->computeAccelerationStructureSizes(*tlas, tlasSize, tlasScratchSize);
    auto scratchBufferSize = std::max(std::max(opaqueScratchSize, reflectiveScratchSize), tlasScratchSize);
    auto scratchBuffer = asShared(m_device->factory().createBuffer(BufferType::Storage, ResourceHeap::Resource, scratchBufferSize, 1, ResourceUsage::AllowWrite));

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

    // Load and upload skybox texture.
    // NOTE: See textures sample for details. We're not creating mip maps here.
    using ImageDataPtr = UniquePtr<stbi_uc, decltype(&::stbi_image_free)>;

    int width, height, channels;
    auto imageData = ImageDataPtr(::stbi_load("assets/rq_skybox.jpg", &width, &height, &channels, STBI_rgb_alpha), ::stbi_image_free);

    if (imageData == nullptr)
        throw std::runtime_error("Texture could not be loaded: \"assets/rq_skybox.jpg\".");

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
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
    auto sampler = m_device->factory().createSampler();
    auto& samplerBindingsLayout = geometryPipeline.layout()->descriptorSet(std::to_underlying(DescriptorSets::Sampler));
    auto samplerBindings = samplerBindingsLayout.allocate({ { .resource = *sampler } });
        
    // Setup random colors for each material. The last one (for the reflective object) can stay the default.
    for (int i{ 0 }; i < NUM_INSTANCES - 1; ++i)
        materials[i].Color = glm::vec4(std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, std::rand() / (float)RAND_MAX, 1.0f);

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
        auto opaqueCompactedSize = Math::align<UInt64>(opaque->size(), 256);
        auto reflectiveCompactedSize = Math::align<UInt64>(reflective->size(), 256);
        auto tlasCompactedSize = Math::align<UInt64>(tlas->size(), 256);
        auto overallSize = opaqueCompactedSize + reflectiveCompactedSize + tlasCompactedSize;

        // Allocate one buffer for all acceleration structures and allocate them individually.
        auto accelerationStructureBuffer = asShared(m_device->factory().createBuffer("Acceleration Structures", BufferType::AccelerationStructure, ResourceHeap::Resource, overallSize, 1u));
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
    m_device->state().add("Sampler Bindings", std::move(samplerBindings));
    m_device->state().add("Material Bindings", std::move(materialBindings));
}

void SampleApp::updateCamera(IBuffer& buffer) const
{
    // Store the initial time this method has been called first.
    static auto start = std::chrono::high_resolution_clock::now();

    // Get the amount of time that has passed since the first frame.
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();
    time *= 0.5f; // Slow down a bit.
    auto position = glm::vec3(std::sinf(time), std::cosf(time), 0.3f) * 7.5f;

    // Calculate the camera view/projection matrix.
    auto viewportRect = m_viewport->getRectangle();
    auto aspectRatio = viewportRect.width() / viewportRect.height();
    glm::mat4 view = glm::lookAt(position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f);
    camera.ViewProjection = projection * view;
    camera.InverseView = glm::inverse(view);
    camera.InverseProjection = glm::inverse(projection);
    camera.ViewportSize = glm::vec2(viewportRect.width(), viewportRect.height());

    // Update the camera buffer.
    buffer.map(static_cast<const void*>(&camera), sizeof(camera));
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
        int width, height;
        ::glfwGetFramebufferSize(window, &width, &height);

        auto adapter = backend->findAdapter(m_adapterId);

        if (adapter == nullptr)
            adapter = backend->findAdapter(std::nullopt);

        auto surface = backend->createSurface(::glfwGetWin32Window(window));

        // Create viewport and scissors.
        m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

        // Create the device.
        m_device = backend->createDevice("Default", *adapter, std::move(surface), Format::B8G8R8A8_UNORM, Size2d(width, height), 3, false, GraphicsDeviceFeatures { .RayQueries = true });

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

void SampleApp::onResize(const void* /*sender*/, ResizeEventArgs e)
{
    // In order to re-create the swap chain, we need to wait for all frames in flight to finish.
    m_device->wait();

    // Resize the frame buffer and recreate the swap chain.
    auto surfaceFormat = m_device->swapChain().surfaceFormat();
    auto renderArea = Size2d(e.width(), e.height());
    auto vsync = m_device->swapChain().verticalSynchronization();
    m_device->swapChain().reset(surfaceFormat, renderArea, 3, vsync);

    // Resize the frame buffers. Note that we could also use an event handler on the swap chain `reseted` event to do this automatically instead.
    m_device->state().frameBuffer("Frame Buffer 0").resize(renderArea);
    m_device->state().frameBuffer("Frame Buffer 1").resize(renderArea);
    m_device->state().frameBuffer("Frame Buffer 2").resize(renderArea);

    // Also resize viewport and scissor.
    m_viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(e.width()), static_cast<Float>(e.height())));
    m_scissor->setRectangle(RectF(0.f, 0.f, static_cast<Float>(e.width()), static_cast<Float>(e.height())));

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
    auto& frameBuffer = m_device->state().frameBuffer(std::format("Frame Buffer {0}", backBuffer));
    auto& renderPass = m_device->state().renderPass("Deferred");
    auto& geometryPipeline = m_device->state().pipeline("Geometry");
    auto& staticDataBindings = m_device->state().descriptorSet("Static Data Bindings");
    auto& materialBindings = m_device->state().descriptorSet("Material Bindings");
    auto& samplerBindings = m_device->state().descriptorSet("Sampler Bindings");
    auto& cameraBuffer = m_device->state().buffer("Camera");

    // Begin rendering on the render pass and use the only pipeline we've created for it.
    renderPass.begin(frameBuffer);
    auto commandBuffer = renderPass.commandBuffer(0);
    commandBuffer->use(geometryPipeline);
    commandBuffer->setViewports(m_viewport.get());
    commandBuffer->setScissors(m_scissor.get());

    // Update the camera buffer.
    this->updateCamera(cameraBuffer);

    // Bind all descriptor sets to the pipeline.
    commandBuffer->bind({ &staticDataBindings, &materialBindings, &samplerBindings });

    // Draw the screen quad and present the frame by ending the render pass. The screen quad is a single triangle that gets clipped.
    commandBuffer->draw(3);
    renderPass.end();
}