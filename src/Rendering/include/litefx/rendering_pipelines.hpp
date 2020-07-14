#pragma once

#include <litefx/rendering_api.hpp>

namespace LiteFX::Rendering {
    using namespace LiteFX;
    using namespace LiteFX::Math;
    using namespace LiteFX::Graphics;

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API BufferAttribute {
        LITEFX_IMPLEMENTATION(BufferAttributeImpl);

    public:
        BufferAttribute();
        BufferAttribute(const UInt32& location, const UInt32& offset, const BufferFormat& format);
        BufferAttribute(BufferAttribute&&) noexcept;
        BufferAttribute(const BufferAttribute&);
        virtual ~BufferAttribute() noexcept;

    public:
        virtual const UInt32& getLocation() const noexcept;
        virtual const BufferFormat& getFormat() const noexcept;
        virtual const UInt32& getOffset() const noexcept;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IBufferLayout {
    public:
        virtual Array<const BufferAttribute*> getAttributes() const noexcept = 0;
        virtual const size_t& getElementSize() const noexcept = 0;
        virtual const UInt32& getBinding() const noexcept = 0;
        virtual const BufferType& getType() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IBufferSet {
    public:
        virtual Array<const IBufferLayout*> getLayouts() const noexcept = 0;
        virtual Array<const IBufferLayout*> getLayouts(const BufferType& type) const noexcept = 0;
        virtual const IBufferLayout* getLayout(const UInt32& binding) const noexcept = 0;
        virtual const UInt32& getSetId() const noexcept = 0;
        virtual const BufferSetType& getType() const noexcept = 0;
        virtual UniquePtr<IBufferPool> createBufferPool(const BufferUsage& usage) const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API BufferLayout : public IBufferLayout {
        LITEFX_IMPLEMENTATION(BufferLayoutImpl);

    public:
        BufferLayout(const BufferType& type, const size_t& elementSize, const UInt32& binding = 0);
        BufferLayout(BufferLayout&&) = delete;
        BufferLayout(const BufferLayout&) = delete;
        virtual ~BufferLayout() noexcept;

    public:
        virtual void add(UniquePtr<BufferAttribute>&& attribute);
        virtual void remove(const BufferAttribute* attribute);

    public:
        virtual Array<const BufferAttribute*> getAttributes() const noexcept override;
        virtual const size_t& getElementSize() const noexcept override;
        virtual const UInt32& getBinding() const noexcept override;
        virtual const BufferType& getType() const noexcept override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API BufferSet : public IBufferSet {
        LITEFX_IMPLEMENTATION(BufferSetImpl);

    public:
        BufferSet(const BufferSetType& type, const UInt32& id = 0);
        BufferSet(BufferSet&&) = delete;
        BufferSet(const BufferSet&) = delete;
        virtual ~BufferSet() noexcept;

    public:
        virtual void add(UniquePtr<IBufferLayout>&& layout);
        virtual void remove(const IBufferLayout* layout);

    public:
        virtual Array<const IBufferLayout*> getLayouts() const noexcept override;
        virtual Array<const IBufferLayout*> getLayouts(const BufferType& type) const noexcept override;
        virtual const IBufferLayout* getLayout(const UInt32& binding) const noexcept override;
        virtual const UInt32& getSetId() const noexcept override;
        virtual const BufferSetType& getType() const noexcept override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IBuffer {
    public:
        virtual ~IBuffer() noexcept = default;

    public:
        virtual const UInt32& getElementSize() const noexcept = 0;
        virtual const UInt32& getElements() const noexcept = 0;
        virtual const UInt32& getBinding() const noexcept = 0;
        virtual const UInt32 getSize() const noexcept = 0;
        virtual const BufferType& getType() const noexcept = 0;

    public:
        virtual void map(const void* const data, const size_t& size) = 0;
        virtual void transfer(const ICommandQueue* commandQueue, IBuffer* target, const size_t& size, const size_t& offset = 0, const size_t& targetOffset = 0) const = 0;
        virtual void bind(const IRenderPass* renderPass) const = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API Buffer : public IBuffer {
    private:
        const BufferType m_type {};
        const UInt32 m_elementSize{ 0 }, m_elements{ 0 }, m_binding{ 0 };

    public:
        Buffer(const BufferType& type, const UInt32& elements, const UInt32& elementSize, const UInt32& binding) :
            m_type(type), m_elementSize(elementSize), m_elements(elements), m_binding(binding) { }
        Buffer(const Buffer& _other) = delete;
        Buffer(Buffer&& _other) = delete;
        virtual ~Buffer() noexcept = default;

    public:
        virtual const UInt32& getElementSize() const noexcept override { return m_elementSize; }
        virtual const UInt32& getElements() const noexcept override    { return m_elements; }
        virtual const UInt32& getBinding() const noexcept override     { return m_binding; }
        virtual const UInt32 getSize() const noexcept override         { return m_elementSize * m_elements; }
        virtual const BufferType& getType() const noexcept override    { return m_type; }
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IBufferPool {
    public:
        virtual ~IBufferPool() noexcept = default;

    public:
        virtual IBuffer* getBuffer(const UInt32& binding) const noexcept = 0;
        virtual const BufferUsage& getUsage() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IRenderPipeline {
    public:
        virtual ~IRenderPipeline() noexcept = default;
    
    public:
        virtual const IRenderPipelineLayout* getLayout() const noexcept = 0;
        virtual const IShaderProgram* getProgram() const noexcept = 0;
        virtual const IRenderPass* getRenderPass() const noexcept = 0;
        virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) = 0;
        virtual void use(UniquePtr<IShaderProgram>&& program) = 0;
        virtual void use(UniquePtr<IRenderPass>&& renderPass) = 0;
        virtual void beginFrame() const = 0;
        virtual void endFrame() = 0;
        virtual UniquePtr<IBuffer> makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding = 0) const = 0;
        virtual UniquePtr<IBuffer> makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const = 0;
        virtual UniquePtr<IBufferPool> makeBufferPool(const BufferUsage& usage, const UInt32& bufferSet) const = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IRenderPipelineLayout {
    public:
        virtual ~IRenderPipelineLayout() noexcept = default;

    public:
        virtual Array<const IViewport*> getViewports() const noexcept = 0;
        virtual void use(UniquePtr<IViewport>&& viewport) = 0;
        virtual UniquePtr<IViewport> remove(const IViewport* viewport) noexcept = 0;
        virtual const IRasterizer* getRasterizer() const noexcept = 0;
        virtual void use(UniquePtr<IRasterizer>&& rasterizer) = 0;
        virtual const IInputAssembler* getInputAssembler() const noexcept = 0;
        virtual void use(UniquePtr<IInputAssembler>&& inputAssembler) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IShaderModule {
    public:
        virtual ~IShaderModule() noexcept = default;

    public:
        virtual const ShaderType& getType() const noexcept = 0;
        virtual const String& getFileName() const noexcept = 0;
        virtual const String& getEntryPoint() const noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IShaderProgram {
    public:
        virtual ~IShaderProgram() noexcept = default;
    
    public:
        virtual Array<const IShaderModule*> getModules() const noexcept = 0;
        virtual void use(UniquePtr<IShaderModule>&& module) = 0;
        virtual UniquePtr<IShaderModule> remove(const IShaderModule* module) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IInputAssembler {
    public:
        virtual ~IInputAssembler() noexcept = default;

    public:
        virtual Array<const IBufferSet*> getBufferSets() const = 0;
        virtual Array<const IBufferSet*> getBufferSets(const BufferSetType& type) const = 0;
        virtual const IBufferSet* getBufferSet(const UInt32& setId) const = 0;
        virtual void use(UniquePtr<IBufferSet>&& layout) = 0;
        virtual UniquePtr<IBufferSet> remove(const IBufferSet* layout) = 0;
        virtual const PrimitiveTopology getTopology() const noexcept = 0;
        virtual void setTopology(const PrimitiveTopology& topology) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IRasterizer {
    public:
        virtual ~IRasterizer() noexcept = default;

    public:
        virtual PolygonMode getPolygonMode() const noexcept = 0;
        virtual void setPolygonMode(const PolygonMode& mode) noexcept = 0;
        virtual CullMode getCullMode() const noexcept = 0;
        virtual void setCullMode(const CullMode& mode) noexcept = 0;
        virtual CullOrder getCullOrder() const noexcept = 0;
        virtual void setCullOrder(const CullOrder& order) noexcept = 0;
        virtual Float getLineWidth() const noexcept = 0;
        virtual void setLineWidth(const Float& width) noexcept = 0;
        virtual bool getDepthBiasEnabled() const noexcept = 0;
        virtual void setDepthBiasEnabled(const bool& enable) noexcept = 0;
        virtual float getDepthBiasClamp() const noexcept = 0;
        virtual void setDepthBiasClamp(const float& clamp) noexcept = 0;
        virtual float getDepthBiasConstantFactor() const noexcept = 0;
        virtual void setDepthBiasConstantFactor(const float& factor) noexcept = 0;
        virtual float getDepthBiasSlopeFactor() const noexcept = 0;
        virtual void setDepthBiasSlopeFactor(const float& factor) noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IViewport {
    public:
        virtual ~IViewport() noexcept = default;

    public:
        virtual RectF getRectangle() const noexcept = 0;
        virtual void setRectangle(const RectF& rectangle) noexcept = 0;
        virtual const Array<RectF>& getScissors() const noexcept = 0;
        virtual Array<RectF>& getScissors() noexcept = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API IRenderPass {
    public:
        virtual ~IRenderPass() noexcept = default;

    public:
        virtual const ICommandBuffer* getCommandBuffer() const noexcept = 0;

    public:
        virtual void addTarget(UniquePtr<IRenderTarget>&& target) = 0;
        virtual const Array<const IRenderTarget*> getTargets() const noexcept = 0;
        virtual UniquePtr<IRenderTarget> removeTarget(const IRenderTarget* target) = 0;
        virtual void begin() const = 0;
        virtual void end(const bool& present = false) = 0;
        virtual void draw(const UInt32& vertices, const UInt32& instances = 1, const UInt32& firstVertex = 0, const UInt32& firstInstance = 0) const = 0;
        virtual void drawIndexed(const UInt32& indices, const UInt32& instances = 1, const UInt32& firstIndex = 0, const Int32& vertexOffset = 0, const UInt32& firstInstance = 0) const = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API Rasterizer : public IRasterizer {
        LITEFX_IMPLEMENTATION(RasterizerImpl);

    public:
        Rasterizer();
        Rasterizer(Rasterizer&&) noexcept = delete;
        Rasterizer(const Rasterizer&) noexcept = delete;
        virtual ~Rasterizer() noexcept;

    public:
        virtual PolygonMode getPolygonMode() const noexcept override;
        virtual void setPolygonMode(const PolygonMode& mode) noexcept override;
        virtual CullMode getCullMode() const noexcept override;
        virtual void setCullMode(const CullMode& mode) noexcept override;
        virtual CullOrder getCullOrder() const noexcept override;
        virtual void setCullOrder(const CullOrder& order) noexcept override;
        virtual Float getLineWidth() const noexcept override;
        virtual void setLineWidth(const Float& width) noexcept override;
        virtual bool getDepthBiasEnabled() const noexcept override;
        virtual void setDepthBiasEnabled(const bool& enable) noexcept override;
        virtual float getDepthBiasClamp() const noexcept override;
        virtual void setDepthBiasClamp(const float& clamp) noexcept override;
        virtual float getDepthBiasConstantFactor() const noexcept override;
        virtual void setDepthBiasConstantFactor(const float& factor) noexcept override;
        virtual float getDepthBiasSlopeFactor() const noexcept override;
        virtual void setDepthBiasSlopeFactor(const float& factor) noexcept override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API InputAssembler : public IInputAssembler {
        LITEFX_IMPLEMENTATION(InputAssemblerImpl);

    public:
        InputAssembler();
        InputAssembler(InputAssembler&&) noexcept = delete;
        InputAssembler(const InputAssembler&) noexcept = delete;
        virtual ~InputAssembler() noexcept;

    public:
        virtual Array<const IBufferSet*> getBufferSets() const override;
        virtual Array<const IBufferSet*> getBufferSets(const BufferSetType& type) const override;
        virtual const IBufferSet* getBufferSet(const UInt32& setId) const override;
        virtual void use(UniquePtr<IBufferSet>&& layout) override;
        virtual UniquePtr<IBufferSet> remove(const IBufferSet* layout) override;
        virtual const PrimitiveTopology getTopology() const noexcept override;
        virtual void setTopology(const PrimitiveTopology& topology) override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API Viewport : public IViewport {
        LITEFX_IMPLEMENTATION(ViewportImpl);

    public:
        Viewport(const RectF& clientRect = { });
        Viewport(Viewport&&) noexcept = delete;
        Viewport(const Viewport&) noexcept = delete;
        virtual ~Viewport() noexcept;

    public:
        virtual RectF getRectangle() const noexcept override;
        virtual void setRectangle(const RectF& rectangle) noexcept override;
        virtual const Array<RectF>& getScissors() const noexcept override;
        virtual Array<RectF>& getScissors() noexcept override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API RenderPipelineLayout : public IRenderPipelineLayout {
        LITEFX_IMPLEMENTATION(RenderPipelineLayoutImpl);

    public:
        RenderPipelineLayout();
        RenderPipelineLayout(RenderPipelineLayout&&) = delete;
        RenderPipelineLayout(const RenderPipelineLayout&) = delete;
        virtual ~RenderPipelineLayout() noexcept;

    public:
        virtual Array<const IViewport*> getViewports() const noexcept override;
        virtual void use(UniquePtr<IViewport>&& viewport) override;
        virtual UniquePtr<IViewport> remove(const IViewport* viewport) noexcept override;
        virtual const IRasterizer* getRasterizer() const noexcept override;
        virtual void use(UniquePtr<IRasterizer>&& rasterizer) override;
        virtual const IInputAssembler* getInputAssembler() const noexcept override;
        virtual void use(UniquePtr<IInputAssembler>&& inputAssembler) override;
    };

    /// <summary>
    /// 
    /// </summary>
    class LITEFX_RENDERING_API RenderPipeline : public IRenderPipeline {
        LITEFX_IMPLEMENTATION(RenderPipelineImpl);

    public:
        RenderPipeline(const IGraphicsDevice* device);
        explicit RenderPipeline(const IGraphicsDevice* device, UniquePtr<IRenderPipelineLayout>&& layout);
        RenderPipeline(RenderPipeline&&) noexcept = delete;
        RenderPipeline(const RenderPipeline&) noexcept = delete;
        virtual ~RenderPipeline() noexcept;

    public:
        virtual const IRenderPipelineLayout* getLayout() const noexcept override;
        virtual const IShaderProgram* getProgram() const noexcept override;
        virtual const IRenderPass* getRenderPass() const noexcept override;
        virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) override;
        virtual void use(UniquePtr<IShaderProgram>&& program) override;
        virtual void use(UniquePtr<IRenderPass>&& renderPass) override;
        virtual void beginFrame() const override;
        virtual void endFrame() override;
        virtual UniquePtr<IBuffer> makeVertexBuffer(const BufferUsage& usage, const UInt32& elements, const UInt32& binding = 0) const override;
        virtual UniquePtr<IBuffer> makeIndexBuffer(const BufferUsage& usage, const UInt32& elements, const IndexType& indexType) const override;
        virtual UniquePtr<IBufferPool> makeBufferPool(const BufferUsage& usage, const UInt32& bufferSet) const override;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TPipeline>
    class RenderPipelineBuilder : public Builder<TDerived, TPipeline> {
    public:
        using builder_type::Builder;

    public:
        virtual void use(UniquePtr<IRenderPipelineLayout>&& layout) = 0;
        virtual void use(UniquePtr<IShaderProgram>&& program) = 0;
        virtual void use(UniquePtr<IRenderPass>&& renderPass) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TShaderProgram, typename TParent>
    class ShaderProgramBuilder : public Builder<TDerived, TShaderProgram, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual ShaderProgramBuilder& addShaderModule(const ShaderType& type, const String& fileName, const String& entryPoint = "main") = 0;
        virtual ShaderProgramBuilder& addVertexShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual ShaderProgramBuilder& addTessellationControlShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual ShaderProgramBuilder& addTessellationEvaluationShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual ShaderProgramBuilder& addGeometryShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual ShaderProgramBuilder& addFragmentShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
        virtual ShaderProgramBuilder& addComputeShaderModule(const String& fileName, const String& entryPoint = "main") = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TRenderPass, typename TParent>
    class RenderPassBuilder : public Builder<TDerived, TRenderPass, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual void use(UniquePtr<IRenderTarget>&& target) = 0;
        virtual TDerived& withColorTarget(const MultiSamplingLevel& samples = MultiSamplingLevel::x1) = 0;
        //virtual TDerived& withDepthTarget() = 0;
        virtual TDerived& addTarget(const RenderTargetType& type, const Format& format, const MultiSamplingLevel& samples, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TPipelineLayout, typename TParent>
    class RenderPipelineLayoutBuilder : public Builder<TDerived, TPipelineLayout, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual void use(UniquePtr<IRasterizer>&& rasterizer) = 0;
        virtual void use(UniquePtr<IInputAssembler>&& inputAssembler) = 0;
        virtual void use(UniquePtr<IViewport>&& viewport) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TRasterizer, typename TParent>
    class RasterizerBuilder : public Builder<TDerived, TRasterizer, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual TDerived& withPolygonMode(const PolygonMode& mode = PolygonMode::Solid) = 0;
        virtual TDerived& withCullMode(const CullMode& cullMode = CullMode::BackFaces) = 0;
        virtual TDerived& withCullOrder(const CullOrder& cullOrder = CullOrder::CounterClockWise) = 0;
        virtual TDerived& withLineWidth(const Float& lineWidth = 1.f) = 0;
        virtual TDerived& withDepthBias(const bool& enable = false) = 0;
        virtual TDerived& withDepthBiasClamp(const Float& clamp = 0.f) = 0;
        virtual TDerived& withDepthBiasConstantFactor(const Float& factor = 0.f) = 0;
        virtual TDerived& withDepthBiasSlopeFactor(const Float& factor = 0.f) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TViewport, typename TParent>
    class ViewportBuilder : public Builder<TDerived, TViewport, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual TDerived& withRectangle(const RectF& rectangle) = 0;
        virtual TDerived& addScissor(const RectF& scissor) = 0;
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TInputAssembler, typename TParent>
    class InputAssemblerBuilder : public Builder<TDerived, TInputAssembler, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual TDerived& addBufferSet(UniquePtr<IBufferSet>&& set) = 0;
        virtual TDerived& withTopology(const PrimitiveTopology& topology) = 0;
        virtual void use(UniquePtr<IBufferSet>&& set) {
            this->addBufferSet(std::move(set));
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TBufferSet, typename TParent>
    class BufferSetBuilder : public Builder<TDerived, TBufferSet, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual TDerived& addLayout(UniquePtr<IBufferLayout>&& layout) = 0;
        virtual void use(UniquePtr<BufferLayout>&& layout) {
            this->addLayout(std::move(layout));
        }
    };

    /// <summary>
    /// 
    /// </summary>
    template <typename TDerived, typename TBufferLayout, typename TParent>
    class BufferLayoutBuilder : public Builder<TDerived, TBufferLayout, TParent> {
    public:
        using builder_type::Builder;

    public:
        virtual TDerived& addAttribute(UniquePtr<BufferAttribute>&& attribute) = 0;
    };

}