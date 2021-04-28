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
        .attachPresentTarget()
        .setPipeline()
            .defineLayout()
                .setShaderProgram()
                    .addVertexShaderModule("shaders/textures.vert.spv")
                    .addFragmentShaderModule("shaders/textures.frag.spv")
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
                .setRasterizer()
                    .withPolygonMode(PolygonMode::Solid)
                    .withCullMode(CullMode::BackFaces)
                    .withCullOrder(CullOrder::ClockWise)
                    .withLineWidth(1.f)
                    .go()
                .setInputAssembler()
                    .withTopology(PrimitiveTopology::TriangleList)
                    .withIndexType(IndexType::UInt16)
                    .addVertexBuffer(sizeof(Vertex), 0)
                        .addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position))
                        .addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color))
                        .addAttribute(2, BufferFormat::XY32F, offsetof(Vertex, TextureCoordinate0))
                        .go()
                    .go()
                .addViewport()
                    .withRectangle(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                    .addScissor(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                    .go()
                .go()
            .go()
        .go();
}

void SampleApp::loadTexture()
{
    using ImageDataPtr = UniquePtr<stbi_uc, decltype(&::stbi_image_free)>;
    
    int width, height, channels;
    auto imageData = ImageDataPtr(::stbi_load("assets/logo_quad.tga", &width, &height, &channels, STBI_rgb_alpha), ::stbi_image_free);

    if (imageData == nullptr)
        throw std::runtime_error("Texture could not be loaded: \"assets/logo_quad.tga\".");

    // Create the texture and transfer the pixel contents to it.
    m_perMaterialBindings = m_renderPass->makeBufferPool(DescriptorSets::PerMaterial);
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
    // Create the staging buffer.
    auto stagedVertices = m_renderPass->makeVertexBuffer(BufferUsage::Staging, vertices.size());
    stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex));

    // Create the actual vertex buffer and transfer the staging buffer into it.
    m_vertexBuffer = m_renderPass->makeVertexBuffer(BufferUsage::Resource, vertices.size());
    m_vertexBuffer->transferFrom(m_device->transferQueue(), stagedVertices.get(), stagedVertices->getSize());

    // Create the staging buffer for the indices.
    auto stagedIndices = m_renderPass->makeIndexBuffer(BufferUsage::Staging, indices.size(), IndexType::UInt16);
    stagedIndices->map(indices.data(), indices.size() * sizeof(UInt16));

    // Create the actual index buffer and transfer the staging buffer into it.
    m_indexBuffer = m_renderPass->makeIndexBuffer(BufferUsage::Resource, indices.size(), IndexType::UInt16);
    m_indexBuffer->transferFrom(m_device->transferQueue(), stagedIndices.get(), stagedIndices->getSize());

    // Create a uniform buffers for the camera and transform information.
    m_perFrameBindings = m_renderPass->makeBufferPool(DescriptorSets::PerFrame);
    m_cameraBuffer = m_perFrameBindings->makeBuffer(0, BufferUsage::Dynamic);
    m_perObjectBindings = m_renderPass->makeBufferPool(DescriptorSets::PerInstance);
    m_transformBuffer = m_perObjectBindings->makeBuffer(0, BufferUsage::Dynamic);
}

void SampleApp::run() 
{
    m_device = this->getRenderBackend()->createDevice<VulkanDevice>(Format::B8G8R8A8_SRGB);
    this->createRenderPasses();
    this->initBuffers();
    this->loadTexture();

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
    else
    {
        // Resize the frame buffer and recreate the swap chain.
        m_device->resize(width, height);

        // Resize the viewport.
        auto layout = m_renderPass->getPipeline()->getLayout();
        auto viewport = layout->remove(layout->getViewports().front());
        viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        viewport->getScissors().clear();
        viewport->getScissors().push_back(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        layout->use(std::move(viewport));
        
        // Recreate the pipeline.
        m_renderPass->reset();
    }
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    // Begin rendering.
    m_renderPass->begin();

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
    m_renderPass->bind(m_perFrameBindings.get());

    // Draw the model.
    m_renderPass->bind(m_vertexBuffer.get());
    m_renderPass->bind(m_indexBuffer.get());
    
    // Compute world transform.
    transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_transformBuffer->map(reinterpret_cast<const void*>(&transform), sizeof(transform));
    m_perObjectBindings->update(m_transformBuffer.get()); 
    m_renderPass->bind(m_perObjectBindings.get());

    // Bind the texture buffer.
    m_renderPass->bind(m_perMaterialBindings.get());

    // Draw the object.
    m_renderPass->drawIndexed(indices.size());

    // End the frame.
    m_renderPass->end(true);
}