#include "sample.h"
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

enum DescriptorSets : UInt32
{
    PerFrame = 0,                                       // All buffers that are updated for each frame.
    PerInstance = 1,                                    // All buffers that are updated for each rendered instance.
    PerMaterial = 2,                                    // All buffers that are updated for each material.
    VertexData = std::numeric_limits<UInt32>::max()     // Unused, but required to correctly address buffer sets.
};

enum Pipelines : UInt32
{
    Basic = 0                                           // Default render pipeline.
};

const Array<Vertex> vertices =
{
    { { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.0f },  { 0.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
    { { 0.5f, 0.5f, 0.0f },   { 1.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
    { { -0.5f, 0.5f, 0.0f },  { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }
};

const Array<UInt16> indices = { 2, 1, 0, 3, 2, 0 };

struct CameraBuffer {
    glm::mat4 ViewProjection;
} camera;

struct TransformBuffer {
    glm::mat4 World;
} transform;

static void onResize(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<SampleApp*>(::glfwGetWindowUserPointer(window));
    app->resize(width, height);
}

void SampleApp::createRenderPasses()
{
    m_renderPass = m_device->buildRenderPass()
        .attachTarget(RenderTargetType::Present, Format::B8G8R8A8_SRGB, MultiSamplingLevel::x1, { 0.f, 0.f, 0.f, 0.f }, true, false, false)
        .addPipeline(Pipelines::Basic, "Basic")
            .layout()
                .shaderProgram()
                    .addVertexShaderModule("shaders/textures.vert.spv")
                    .addFragmentShaderModule("shaders/textures.frag.spv")
                    .go()
                .addDescriptorSet(DescriptorSets::PerFrame, ShaderStage::Vertex | ShaderStage::Fragment)
                    .addUniform(0, sizeof(CameraBuffer))
                    .go()
                .addDescriptorSet(DescriptorSets::PerInstance, ShaderStage::Vertex)
                    .addUniform(0, sizeof(TransformBuffer))
                    .go()
                .addDescriptorSet(DescriptorSets::PerMaterial, ShaderStage::Fragment)
                    .addImage(0)
                    .addSampler(1)
                    .go()
                .go()
            .rasterizer()
                .withPolygonMode(PolygonMode::Solid)
                .withCullMode(CullMode::BackFaces)
                .withCullOrder(CullOrder::ClockWise)
                .withLineWidth(1.f)
                .go()
            .inputAssembler()
                .withTopology(PrimitiveTopology::TriangleList)
                .withIndexType(IndexType::UInt16)
                .addVertexBuffer(sizeof(Vertex), 0)
                    .addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position))
                    .addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color))
                    .addAttribute(2, BufferFormat::XY32F, offsetof(Vertex, TextureCoordinate0))
                    .go()
                .go()
            .withViewport(m_viewport)
            .withScissor(m_scissor)
            .go()
        .go();
}

void SampleApp::loadTexture()
{
    // Get the pipeline instance.
    auto pipeline = m_renderPass->getPipeline(Pipelines::Basic);

    using ImageDataPtr = UniquePtr<stbi_uc, decltype(&::stbi_image_free)>;
    
    int width, height, channels;
    auto imageData = ImageDataPtr(::stbi_load("assets/logo_quad.tga", &width, &height, &channels, STBI_rgb_alpha), ::stbi_image_free);

    if (imageData == nullptr)
        throw std::runtime_error("Texture could not be loaded: \"assets/logo_quad.tga\".");

    // Create the texture and transfer the pixel contents to it.
    m_perMaterialBindings = pipeline->makeDescriptorSet(DescriptorSets::PerMaterial);
    m_texture = m_perMaterialBindings->makeTexture(0, Format::R8G8B8A8_SRGB, Size2d(width, height));
    auto stagedTexture = m_device->createBuffer(BufferType::Other, BufferUsage::Staging, m_texture->getSize());
    stagedTexture->map(imageData.get(), m_texture->getSize());

    // Transfer the texture using the graphics queue.
    m_texture->transferFrom(m_device->graphicsQueue(), stagedTexture.get(), m_texture->getSize(), 0);
    m_perMaterialBindings->updateAll(m_texture.get());

    // Create a sampler.
    m_sampler = m_perMaterialBindings->makeSampler(1);
    m_perMaterialBindings->updateAll(m_sampler.get());
}

void SampleApp::initBuffers()
{
    // Get the pipeline instance.
    auto pipeline = m_renderPass->getPipeline(Pipelines::Basic);

    // Create the staging buffer.
    auto stagedVertices = pipeline->makeVertexBuffer(BufferUsage::Staging, vertices.size());
    stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex));

    // Create the actual vertex buffer and transfer the staging buffer into it.
    m_vertexBuffer = pipeline->makeVertexBuffer(BufferUsage::Resource, vertices.size());
    m_vertexBuffer->transferFrom(m_device->bufferQueue(), stagedVertices.get(), stagedVertices->getSize());

    // Create the staging buffer for the indices.
    auto stagedIndices = pipeline->makeIndexBuffer(BufferUsage::Staging, indices.size(), IndexType::UInt16);
    stagedIndices->map(indices.data(), indices.size() * sizeof(UInt16));

    // Create the actual index buffer and transfer the staging buffer into it.
    m_indexBuffer = pipeline->makeIndexBuffer(BufferUsage::Resource, indices.size(), IndexType::UInt16);
    m_indexBuffer->transferFrom(m_device->bufferQueue(), stagedIndices.get(), stagedIndices->getSize());

    // Create a uniform buffers for the camera and transform information.
    m_perFrameBindings = pipeline->makeDescriptorSet(DescriptorSets::PerFrame);
    m_cameraBuffer = m_perFrameBindings->makeBuffer(0, BufferUsage::Dynamic);
    m_perObjectBindings = pipeline->makeDescriptorSet(DescriptorSets::PerInstance);
    m_transformBuffer = m_perObjectBindings->makeBuffer(0, BufferUsage::Dynamic);
}

void SampleApp::run() 
{
    // Get the proper frame buffer size.
    int width, height;
    ::glfwGetFramebufferSize(m_window.get(), &width, &height);

    // Create viewport and scissors.
    m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
    m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

    // Create the device with the initial frame buffer size and triple buffering.
    m_device = this->getRenderBackend()->createDevice<VulkanDevice>(Format::B8G8R8A8_SRGB, Size2d(width, height), 3);

    // Initialize resources.
    this->createRenderPasses();
    this->initBuffers();
    this->loadTexture();

    // Run application loop until the window is closed.
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        this->handleEvents();
        this->drawFrame();
    }

    // Shut down the device.
    m_device->wait();

    // Destroy all resources.
    m_perObjectBindings = nullptr;
    m_perFrameBindings = nullptr;
    m_perMaterialBindings = nullptr;
    m_cameraBuffer = nullptr;
    m_transformBuffer = nullptr;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_texture = nullptr;
    m_sampler = nullptr;

    // Destroy the pipeline and the device.
    m_renderPass = nullptr;
    m_device = nullptr;

    // Destroy the window.
    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}

void SampleApp::initialize()
{
    ::glfwSetFramebufferSizeCallback(m_window.get(), ::onResize); 
}

void SampleApp::resize(int width, int height)
{
    App::resize(width, height);

    if (m_device == nullptr)
        return;

    // Resize the frame buffer and recreate the swap chain.
    m_device->resize(width, height);
    m_renderPass->resetFramebuffer();

    // Also resize viewport and scissor.
    m_viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
    m_scissor->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    // Begin rendering.
    m_renderPass->begin();

    // Get the pipeline and bind it.
    auto pipeline = m_renderPass->getPipeline(Pipelines::Basic);
    pipeline->use();

    // Update transform buffer.
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();
    auto aspectRatio = static_cast<float>(m_device->getBufferWidth()) / static_cast<float>(m_device->getBufferHeight());

    // Compute camera view and projection.
    glm::mat4 view       = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f);
    projection[1][1] *= -1.f;   // Fix GLM clip coordinate scaling.
    camera.ViewProjection = projection * view;
    m_cameraBuffer->map(reinterpret_cast<const void*>(&camera), sizeof(camera));
    m_perFrameBindings->update(m_cameraBuffer.get());
    pipeline->bind(m_perFrameBindings.get());

    // Draw the model.
    pipeline->bind(m_vertexBuffer.get());
    pipeline->bind(m_indexBuffer.get());
    
    // Compute world transform.
    transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_transformBuffer->map(reinterpret_cast<const void*>(&transform), sizeof(transform));
    m_perObjectBindings->update(m_transformBuffer.get()); 
    pipeline->bind(m_perObjectBindings.get());

    // Bind the texture buffer.
    pipeline->bind(m_perMaterialBindings.get());

    // Draw the object.
    m_renderPass->drawIndexed(indices.size());

    // End the frame.
    m_renderPass->end(true);
}