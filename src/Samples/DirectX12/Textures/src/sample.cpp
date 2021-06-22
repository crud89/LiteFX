#include "sample.h"
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

enum DescriptorSets : UInt32
{
    Constant = 0,                                       // All buffers that are immutable.
    Samplers = 1,                                       // All samplers that are immutable.
    PerFrame = 2,                                       // All buffers that are updated each frame.
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

void SampleApp::initRenderGraph()
{
    m_renderPass = m_device->buildRenderPass()
        .renderTarget(RenderTargetType::Present, Format::B8G8R8A8_SRGB, MultiSamplingLevel::x1, { 0.f, 0.f, 0.f, 1.f }, true, false, false)
        .go();
}

void SampleApp::initPipelines()
{
    m_pipeline = m_renderPass->makePipeline(Pipelines::Basic, "Basic")
        .withViewport(m_viewport)
        .withScissor(m_scissor)
        .layout()
            .shaderProgram()
                .addVertexShaderModule("shaders/textures_vs.dxi")
                .addFragmentShaderModule("shaders/textures_ps.dxi")
                .go()
            .addDescriptorSet(DescriptorSets::Constant, ShaderStage::Vertex | ShaderStage::Fragment)
                .addUniform(0, sizeof(CameraBuffer))
                .addImage(1)
                .go()
            .addDescriptorSet(DescriptorSets::Samplers, ShaderStage::Vertex | ShaderStage::Fragment)
                .addSampler(0)
                .go()
            .addDescriptorSet(DescriptorSets::PerFrame, ShaderStage::Vertex)
                .addUniform(0, sizeof(TransformBuffer))
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
                .addAttribute(0, BufferFormat::XYZ32F, offsetof(Vertex, Position), AttributeSemantic::Position)
                .addAttribute(1, BufferFormat::XYZW32F, offsetof(Vertex, Color), AttributeSemantic::Color)
                .addAttribute(2, BufferFormat::XY32F, offsetof(Vertex, TextureCoordinate0), AttributeSemantic::TextureCoordinate)
                .go()
            .go()
        .go();

    m_inputAssembler = m_pipeline->inputAssembler();
}

void SampleApp::initBuffers()
{
    // Get the pipeline instance.
    auto commandBuffer = m_device->bufferQueue().createCommandBuffer(true);

    // Create the staging buffer.
    // NOTE: The mapping works, because vertex and index buffers have an alignment of 0, so we can treat the whole buffer as a single element the size of the 
    //       whole buffer.
    auto stagedVertices = m_device->factory().createVertexBuffer(m_inputAssembler->vertexBufferLayout(0), BufferUsage::Staging, vertices.size());
    stagedVertices->map(vertices.data(), vertices.size() * sizeof(::Vertex), 0);

    // Create the actual vertex buffer and transfer the staging buffer into it.
    m_vertexBuffer = m_device->factory().createVertexBuffer(m_inputAssembler->vertexBufferLayout(0), BufferUsage::Resource, vertices.size());
    m_vertexBuffer->transferFrom(*commandBuffer, *stagedVertices, 0, 0, vertices.size());

    // Create the staging buffer for the indices. For infos about the mapping see the note about the vertex buffer mapping above.
    auto stagedIndices = m_device->factory().createIndexBuffer(m_inputAssembler->indexBufferLayout(), BufferUsage::Staging, indices.size());
    stagedIndices->map(indices.data(), indices.size() * m_inputAssembler->indexBufferLayout().elementSize(), 0);

    // Create the actual index buffer and transfer the staging buffer into it.
    m_indexBuffer = m_device->factory().createIndexBuffer(m_inputAssembler->indexBufferLayout(), BufferUsage::Resource, indices.size());
    m_indexBuffer->transferFrom(*commandBuffer, *stagedIndices, 0, 0, indices.size());

    // Initialize the camera buffer. The camera buffer is constant, so we only need to create one buffer, that can be read from all frames. Since this is a 
    // write-once/read-multiple scenario, we also transfer the buffer to the more efficient memory heap on the GPU.
    auto& constantBindingLayout = m_pipeline->layout().layout(DescriptorSets::Constant);
    auto& samplerBindingLayout = m_pipeline->layout().layout(DescriptorSets::Samplers);
    m_cameraStagingBuffer = m_device->factory().createConstantBuffer(constantBindingLayout.layout(0), BufferUsage::Staging, 1);
    m_cameraBuffer = m_device->factory().createConstantBuffer(constantBindingLayout.layout(0), BufferUsage::Resource, 1);

    // Allocate the descriptor set and bind the camera buffer to it.
    m_samplerBindings = samplerBindingLayout.allocate();
    m_constantBindings = constantBindingLayout.allocate();
    m_constantBindings->update(*m_cameraBuffer, 0);

    // Update the camera. Since the descriptor set already points to the proper buffer, all changes are implicitly visible.
    this->updateCamera(*commandBuffer);

    // Load the texture.
    this->loadTexture();

    // Next, we create the descriptor sets for the transform buffer. The transform changes with every frame. Since we have three frames in flight, we
    // create a buffer with three elements and bind the appropriate element to the descriptor set for every frame.
    auto& transformBindingLayout = m_pipeline->layout().layout(DescriptorSets::PerFrame);
    m_perFrameBindings = transformBindingLayout.allocate(3);
    m_transformBuffer = m_device->factory().createConstantBuffer(transformBindingLayout.layout(0), BufferUsage::Dynamic, 3);
    std::ranges::for_each(m_perFrameBindings, [this, i = 0](const UniquePtr<DirectX12DescriptorSet>& descriptorSet) mutable { descriptorSet->update(*m_transformBuffer, i++); });

    // End and submit the command buffer.
    commandBuffer->end(true, true);
}

void SampleApp::loadTexture()
{
    // Load the image.
    using ImageDataPtr = UniquePtr<stbi_uc, decltype(&::stbi_image_free)>;

    int width, height, channels;
    auto imageData = ImageDataPtr(::stbi_load("assets/logo_quad.tga", &width, &height, &channels, STBI_rgb_alpha), ::stbi_image_free);

    if (imageData == nullptr)
        throw std::runtime_error("Texture could not be loaded: \"assets/logo_quad.tga\".");

    // Create the texture from the constant buffer descriptor set, since we only load the texture once and use it for all frames.
    m_texture = m_constantBindings->makeTexture(1, Format::R8G8B8A8_UNORM, Size2d(width, height));

    // Create a staging buffer for the texture.
    auto stagedTexture = m_device->factory().createBuffer(BufferType::Other, BufferUsage::Staging, m_texture->size());
    stagedTexture->map(imageData.get(), m_texture->size(), 0);

    // Transfer the texture using the graphics queue to ensure that the image transition is supported for the descriptor set shader stages.
    auto commandBuffer = m_device->graphicsQueue().createCommandBuffer(true);
    m_texture->transferFrom(*commandBuffer, *stagedTexture);

    // Submit the command buffer and wait for it to execute.
    // NOTE: If the command buffer goes out of scope, it will get destroyed and its fence released. This causes validation errors, since the command buffer must
    //       have finished before being released. Instead of waiting here, it is probably better for applications that do repeated texture uploads to store a
    //       command buffer instance and re-use it without waiting after each submit. It is even more efficient, to put multiple texture transfers into the 
    //       command buffer before submitting.
    commandBuffer->end(true, true);

    // Create a sampler state for the texture.
    m_sampler = m_samplerBindings->makeSampler(0);

    // Update the descriptor set.
    m_constantBindings->update(*m_texture);
    m_samplerBindings->update(*m_sampler);
}

void SampleApp::updateCamera(const DirectX12CommandBuffer& commandBuffer)
{
    // Calculate the camera view/projection matrix.
    auto aspectRatio = m_viewport->getRectangle().width() / m_viewport->getRectangle().height();
    glm::mat4 view = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.0001f, 1000.0f);
    camera.ViewProjection = projection * view;
    projection[1][1] *= -1.f;   // Fix GLM clip coordinate scaling.
    m_cameraStagingBuffer->map(reinterpret_cast<const void*>(&camera), sizeof(camera));
    m_cameraBuffer->transferFrom(commandBuffer, *m_cameraStagingBuffer.get());
}

void SampleApp::run()
{
    // Start by creating the surface and selecting the adapter.
    auto backend = this->findBackend<DirectX12Backend>(BackendType::Rendering);
    auto adapter = backend->findAdapter(m_adapterId);

    if (adapter == nullptr)
        adapter = backend->findAdapter(std::nullopt);

    auto surface = makeUnique<DirectX12Surface>(::glfwGetWin32Window(m_window.get()));

    // Get the proper frame buffer size.
    int width, height;
    ::glfwGetFramebufferSize(m_window.get(), &width, &height);

    // Create viewport and scissors.
    m_viewport = makeShared<Viewport>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
    m_scissor = makeShared<Scissor>(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

    // Create the device with the initial frame buffer size and triple buffering.
    m_device = backend->createDevice(*adapter, *surface, *backend, Format::B8G8R8A8_UNORM, Size2d(width, height), 3);

    // Initialize resources.
    this->initRenderGraph();
    this->initPipelines();
    this->initBuffers();

    // Run application loop until the window is closed.
    while (!::glfwWindowShouldClose(m_window.get()))
    {
        this->handleEvents();
        this->drawFrame();
    }

    // Shut down the device.
    m_device->wait();

    // Destroy all resources.
    m_constantBindings = nullptr;
    m_samplerBindings = nullptr;
    m_perFrameBindings.clear();
    m_cameraBuffer = nullptr;
    m_cameraStagingBuffer = nullptr;
    m_transformBuffer = nullptr;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_texture = nullptr;
    m_sampler = nullptr;

    // Destroy the pipeline, render pass and the device.
    m_pipeline = nullptr;
    m_renderPass = nullptr;
    m_device = nullptr;

    // Destroy the window.
    ::glfwDestroyWindow(m_window.get());
    ::glfwTerminate();
}

void SampleApp::initialize()
{
    ::glfwSetWindowUserPointer(m_window.get(), this);
    ::glfwSetFramebufferSizeCallback(m_window.get(), ::onResize);
}

void SampleApp::resize(int width, int height)
{
    App::resize(width, height);

    if (m_device == nullptr)
        return;

    // In order to re-create the swap chain, we need to wait for all frames in flight to finish.
    m_device->wait();

    // Resize the frame buffer and recreate the swap chain.
    auto surfaceFormat = m_device->swapChain().surfaceFormat();
    auto renderArea = Size2d(width, height);
    m_device->swapChain().reset(surfaceFormat, renderArea, 3);
    // NOTE: Important to do this in order, since dependencies (i.e. input attachments) are re-created and might be mapped to images that do no longer exist when a dependency
    //       gets re-created. This is hard to detect, since some frame buffers can have a constant size, that does not change with the render area and do not need to be 
    //       re-created. We should either think of a clever implicit dependency management for this, or at least document this behavior!
    m_renderPass->resizeFrameBuffers(renderArea);

    // Also resize viewport and scissor.
    m_viewport->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));
    m_scissor->setRectangle(RectF(0.f, 0.f, static_cast<Float>(width), static_cast<Float>(height)));

    // Also update the camera.
    auto commandBuffer = m_device->bufferQueue().createCommandBuffer(true);
    this->updateCamera(*commandBuffer);
    commandBuffer->end(true, true);
}

void SampleApp::handleEvents()
{
    ::glfwPollEvents();
}

void SampleApp::drawFrame()
{
    // Store the initial time this method has been called first.
    static auto start = std::chrono::high_resolution_clock::now();

    // Swap the back buffers for the next frame.
    auto backBuffer = m_device->swapChain().swapBackBuffer();

    // Begin rendering on the render pass and use the only pipeline we've created for it.
    m_renderPass->begin(backBuffer);
    m_pipeline->use();

    // Get the amount of time that has passed since the first frame.
    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();

    // Compute world transform and update the transform buffer.
    transform.World = glm::rotate(glm::mat4(1.0f), time * glm::radians(42.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_transformBuffer->map(reinterpret_cast<const void*>(&transform), sizeof(transform), backBuffer);

    // Bind both descriptor sets to the pipeline.
    m_pipeline->bind(*m_constantBindings);
    m_pipeline->bind(*m_samplerBindings);
    m_pipeline->bind(*m_perFrameBindings[backBuffer]);

    // Bind the vertex and index buffers.
    m_pipeline->bind(*m_vertexBuffer);
    m_pipeline->bind(*m_indexBuffer);

    // Draw the object and present the frame by ending the render pass.
    m_pipeline->drawIndexed(m_indexBuffer->elements());
    m_renderPass->end();
}