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
    { { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
    { { 0.5f, -0.5f, 0.0f },  { 0.0f, 1.0f, 1.0f, 1.0f } },
    { { 0.5f, 0.5f, 0.0f },   { 1.0f, 0.0f, 1.0f, 1.0f } },
    { { -0.5f, 0.5f, 0.0f },  { 1.0f, 1.0f, 1.0f, 1.0f } }
};

const Array<UInt16> indices = { 2, 1, 0, 3, 2, 0 };

const UInt32 pixelSize = 4;

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

void SampleApp::createPipeline()
{
    m_pipeline = m_device->build<VulkanRenderPipeline>()
        .make<VulkanRenderPipelineLayout>()
            .make<VulkanRasterizer>()
                .withPolygonMode(PolygonMode::Solid)
                .withCullMode(CullMode::BackFaces)
                .withCullOrder(CullOrder::ClockWise)
                .withLineWidth(1.f)
                .go()
            .make<VulkanViewport>()
                .withRectangle(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                .addScissor(RectF(0.f, 0.f, static_cast<Float>(m_device->getBufferWidth()), static_cast<Float>(m_device->getBufferHeight())))
                .go()
            .make<VulkanInputAssembler>()
                .withTopology(PrimitiveTopology::TriangleList)
                .make<VulkanVertexBufferLayout>(sizeof(Vertex), 0)
                    .addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position))
                    .addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color))
                    .go()
                .make<VulkanIndexBufferLayout>(IndexType::UInt16)
                    .go()
                .go()
            .make<VulkanShaderProgram>()
                .addVertexShaderModule("shaders/default.vert.spv")
                .addFragmentShaderModule("shaders/default.frag.spv")
                .make<VulkanDescriptorSetLayout>(DescriptorSets::PerFrame, ShaderStage::Vertex | ShaderStage::Fragment)
                    .addDescriptor(DescriptorType::Uniform, 0, sizeof(CameraBuffer))
                    .go()
                .make<VulkanDescriptorSetLayout>(DescriptorSets::PerInstance, ShaderStage::Vertex)
                    .addDescriptor(DescriptorType::Uniform, 0, sizeof(TransformBuffer))
                    .go()
                .make<VulkanDescriptorSetLayout>(DescriptorSets::PerMaterial, ShaderStage::Fragment)
                    .addDescriptor(DescriptorType::Sampler, 0, pixelSize)
                    .go()
                .go()
            .go()
        .make<VulkanRenderPass>()
            .withPresentTarget()
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

    // Create a texture with 1 mip map.
    
    //auto textureBuffer = m_pipeline->makeBuffer()

    //m_textureBuffer = m_pipeline->makeBufferPool(BufferUsage::Resource, DescriptorSets::PerMaterial);
    //textureBuffer->transfer(m_device->getTransferQueue(), m_textureBuffer->getBuffer(0), textureBuffer->getSize());
}

void SampleApp::initBuffers()
{
    // Create the staging buffer.
    auto stagingBuffer = m_pipeline->makeVertexBuffer(BufferUsage::Staging, vertices.size());
    stagingBuffer->map(vertices.data(), vertices.size() * sizeof(::Vertex));

    // Create the actual vertex buffer and transfer the staging buffer into it.
    m_vertexBuffer = m_pipeline->makeVertexBuffer(BufferUsage::Resource, vertices.size());
    stagingBuffer->transfer(m_device->getTransferQueue(), m_vertexBuffer.get(), vertices.size() * sizeof(::Vertex));

    // Create the staging buffer for the indices.
    stagingBuffer = m_pipeline->makeIndexBuffer(BufferUsage::Staging, indices.size(), IndexType::UInt16);
    stagingBuffer->map(indices.data(), indices.size() * sizeof(UInt16));

    // Create the actual index buffer and transfer the staging buffer into it.
    m_indexBuffer = m_pipeline->makeIndexBuffer(BufferUsage::Resource, indices.size(), IndexType::UInt16);
    stagingBuffer->transfer(m_device->getTransferQueue(), m_indexBuffer.get(), indices.size() * sizeof(UInt16));

    // Create a uniform buffers for the camera and transform information.
    m_cameraBuffer = m_pipeline->makeBufferPool(BufferUsage::Dynamic, DescriptorSets::PerFrame);
    m_transformBuffer = m_pipeline->makeBufferPool(BufferUsage::Dynamic, DescriptorSets::PerInstance);
    m_textureBuffer = m_pipeline->makeBufferPool(BufferUsage::Resource, DescriptorSets::PerMaterial);
}

void SampleApp::run() 
{
    m_device = this->getRenderBackend()->createDevice<VulkanDevice>(Format::B8G8R8A8_SRGB);
    this->createPipeline();
    this->initBuffers();
    this->loadTexture();

    while (!::glfwWindowShouldClose(m_window.get()))
    {
        this->handleEvents();
        this->drawFrame();
    }

    // Shut down the device.
    m_device->wait();

    // Destroy all buffers.
    m_transformBuffer = nullptr;
    m_cameraBuffer = nullptr;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;

    // Destroy the pipeline and the device.
    m_pipeline = nullptr;
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
        auto layout = m_pipeline->getLayout();
        auto viewport = layout->remove(layout->getViewports().front());
        viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        viewport->getScissors().clear();
        viewport->getScissors().push_back(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
        layout->use(std::move(viewport));
        
        // Recreate the pipeline.
        m_pipeline->reset();
    }
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    // Begin rendering.
    m_pipeline->beginFrame();

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
    m_cameraBuffer->getBuffer(0)->map(reinterpret_cast<const void*>(&camera), sizeof(camera));
    m_pipeline->bind(m_cameraBuffer.get());

    // Draw the model.
    m_pipeline->bind(m_vertexBuffer.get());
    m_pipeline->bind(m_indexBuffer.get());
    
    // Compute world transform.
    transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_transformBuffer->getBuffer(0)->map(reinterpret_cast<const void*>(&transform), sizeof(transform));
    m_pipeline->bind(m_transformBuffer.get());

    // Bind the texture buffer.
    //m_pipeline->bind(m_textureBuffer.get());

    // Draw the object.
    m_pipeline->getRenderPass()->drawIndexed(indices.size());

    // End the frame.
    m_pipeline->endFrame();
}