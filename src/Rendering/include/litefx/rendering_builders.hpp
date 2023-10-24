#pragma once

#include "rendering_api.hpp"
#include "rendering.hpp"

#if defined(BUILD_DEFINE_BUILDERS)
namespace LiteFX::Rendering {

    // TODO: Refactor this to something similar as here: https://godbolt.org/z/fjdPE65YY.

    /// <summary>
    /// Base class for a builder that builds a <see cref="Barrier" />.
    /// </summary>
    /// <typeparam name="TBarrier">The type of the barrier. Must implement <see cref="Barrier" />.</typeparam>
    /// <seealso cref="Barrier" />
    /// <seealso cref="IBarrier" />
    template <typename TBarrier> requires
        rtti::implements<TBarrier, Barrier<typename TBarrier::buffer_type, typename TBarrier::image_type>>
    class BarrierBuilder : public Builder<TBarrier> {
    public:
        template <typename TParent> requires
            rtti::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] ImageBarrierBuilder;

        /// <summary>
        /// A builder that sets up the pipeline stages to wait for and to continue with on a barrier.
        /// </summary>
        /// <typeparam name="TParent">The type of the parent barrier builder.</typeparam>
        template <typename TParent> requires
            rtti::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] SecondStageBuilder {
        private:
            PipelineStage m_from;
            TParent m_parent;

            /// <summary>
            /// Initializes a builder that sets up barrier stages.
            /// </summary>
            /// <param name="parent">The parent builder instance.</param>
            /// <param name="waitFor">The pipeline stage to wait for.</param>
            constexpr inline SecondStageBuilder(TParent&& parent, PipelineStage waitFor) noexcept :
                m_parent(std::move(parent)), m_from(waitFor) { }

        public:
            friend class BarrierBuilder;

            /// <summary>
            /// Specifies the pipeline stage that are allowed to continue after the barrier has executed.
            /// </summary>
            /// <param name="stage">The pipeline stage that are allowed to continue after the barrier has executed.</param>
            /// <returns>The instance of the parent builder.</returns>
            constexpr inline auto toContinueWith(PipelineStage stage) -> TParent&& {
                this->m_parent.stagesCallback(this->m_from, stage);
                return std::move(this->m_parent);
            }
        };

        /// <summary>
        /// A builder that sets up a global ressource barrier.
        /// </summary>
        /// <typeparam name="TParent">The type of the parent barrier builder.</typeparam>
        template <typename TParent> requires
            rtti::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] GlobalBarrierBuilder {
        private:
            ResourceAccess m_waitFor;
            TParent m_parent;

            /// <summary>
            /// Initializes a builder that sets up a global resource barrier.
            /// </summary>
            /// <param name="parent">The parent builder instance.</param>
            /// <param name="access">The resource access state of all resources to wait for with this barrier.</param>
            constexpr inline GlobalBarrierBuilder(TParent&& parent, ResourceAccess access) noexcept :
                m_parent(std::move(parent)), m_waitFor(access) { }

        public:
            friend class BarrierBuilder;

            /// <summary>
            /// Specifies the resource accesses that are waited for in a global barrier before it can be executed.
            /// </summary>
            /// <param name="access">The resource accesses that are waited for until the barrier can be executed.</param>
            constexpr inline auto untilFinishedWith(ResourceAccess access) -> TParent&& {
                this->m_parent.globalBarrierCallback(m_waitFor, access);
                return std::move(this->m_parent);
            }
        };

        /// <summary>
        /// A builder that sets up a resource barrier for a specific buffer.
        /// </summary>
        /// <typeparam name="TParent">The type of the parent barrier builder.</typeparam>
        template <typename TParent> requires
            rtti::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] BufferBarrierBuilder {
        private:
            ResourceAccess m_waitFor;
            IBuffer& m_buffer;
            TParent m_parent;

            /// <summary>
            /// Initializes a builder that sets up a barrier for a specific buffer.
            /// </summary>
            /// <param name="parent">The parent builder instance.</param>
            /// <param name="buffer">The buffer for this barrier.</param>
            /// <param name="access">The resource access state of the buffer to wait for with this barrier.</param>
            constexpr inline BufferBarrierBuilder(TParent&& parent, IBuffer& buffer, ResourceAccess access) noexcept :
                m_parent(std::move(parent)), m_buffer(buffer), m_waitFor(access) { }

        public:
            friend class BarrierBuilder;

            /// <summary>
            /// Specifies the resource accesses that are waited for in a buffer before the barrier can be executed.
            /// </summary>
            /// <param name="access">The resource accesses that are waited for in a buffer before the barrier can be executed.</param>
            constexpr inline auto untilFinishedWith(ResourceAccess access) -> TParent&& {
                this->m_parent.bufferBarrierCallback(m_buffer, m_waitFor, access);
                return std::move(this->m_parent);
            }
        };

        /// <summary>
        /// A builder that sets up the layout transition barrier for a set of sub-resources of a specific image.
        /// </summary>
        /// <typeparam name="TParent">The type of the parent barrier builder.</typeparam>
        template <typename TParent> requires
            rtti::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] ImageLayoutBarrierBuilder {
        private:
            ResourceAccess m_waitFor;
            IImage& m_image;
            TParent m_parent;
            ImageLayout m_layout;
            UInt32 m_level, m_levels, m_layer, m_layers, m_plane;

            /// <summary>
            /// Initializes a builder that sets up the layout transition barrier for a set of sub-resources of a specific image.
            /// </summary>
            /// <param name="parent">The parent builder instance.</param>
            /// <param name="image">The image for this barrier.</param>
            /// <param name="access">The resource access state of the sub-resources in the image to wait for with this barrier.</param>
            /// <param name="layout">The layout to transition the image sub-resources into.</param>
            /// <param name="level">The level of the first sub-resource to transition.</param>
            /// <param name="levels">The number of levels to to transition.</param>
            /// <param name="layer">The layer of the first sub-resource to transition.</param>
            /// <param name="layers">The number of layers to transition.</param>
            /// <param name="plane">The plane of the sub-resource to transition.</param>
            constexpr inline ImageLayoutBarrierBuilder(TParent&& parent, IImage& image, ResourceAccess access, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) noexcept :
                m_parent(std::move(parent)), m_image(image), m_waitFor(access), m_layout(layout), m_level(level), m_levels(levels), m_layer(layer), m_layers(layers), m_plane(plane) { }

        public:
            friend class BarrierBuilder;
            friend class ImageBarrierBuilder<TParent>;

            /// <summary>
            /// Specifies the resource accesses that are waited for on the image sub-resources before the barrier can be executed.
            /// </summary>
            /// <param name="access">The resource accesses that are waited for on the image sub-resources before the barrier can be executed.</param>
            constexpr inline auto whenFinishedWith(ResourceAccess access) -> TParent&& {
                this->m_parent.imageBarrierCallback(m_image, m_waitFor, access, m_layout, m_level, m_levels, m_layer, m_layers, m_plane);
                return std::move(this->m_parent);
            }
        };

        /// <summary>
        /// A builder that sets up a resource barrier for a specific image.
        /// </summary> 
        /// <typeparam name="TParent">The type of the parent barrier builder.</typeparam>
        template <typename TParent> requires
            rtti::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] ImageBarrierBuilder {
        private:
            ResourceAccess m_waitFor;
            IImage& m_image;
            TParent m_parent;
            UInt32 m_level, m_levels, m_layer, m_layers, m_plane;

            /// <summary>
            /// Initializes a builder that sets up a resource barrier for a specific image.
            /// </summary>
            /// <param name="parent">The parent builder instance.</param>
            /// <param name="image">The image for this barrier.</param>
            /// <param name="access">The resource access state of the sub-resources in the image to wait for with this barrier.</param>
            constexpr inline ImageBarrierBuilder(TParent&& parent, IImage& image, ResourceAccess access) noexcept :
                m_parent(std::move(parent)), m_image(image), m_waitFor(access), m_level{ 0 }, m_levels{ 0 }, m_layer{ 0 }, m_layers{ 0 }, m_plane{ 0 } { }

        public:
            /// <summary>
            /// Specifies the layout to transition an image to when executing the barrier.
            /// </summary>
            /// <param name="layout">The layout to transition an image to when executing the barrier.</param>
            constexpr inline auto transitionLayout(ImageLayout layout) -> ImageBarrierBuilder<TParent> {
                return ImageBarrierBuilder<TParent>{ std::move(m_parent), m_image, m_waitFor, layout, m_level, m_levels, m_layer, m_layers, m_plane };
            }

            /// <summary>
            /// Specifies the sub-resource to block and transition when executing the barrier.
            /// </summary>
            /// <param name="level">The base level of the sub-resource.</param>
            /// <param name="levels">The number of levels to block and transition.</param>
            /// <param name="layer">The base layer of the sub-resource.</param>
            /// <param name="layers">The number of layers to block and transition.</param>
            /// <param name="plane">The plane index of the sub-resource to block and transition.</param>
            constexpr inline auto subresource(UInt32 level, UInt32 levels, UInt32 layer = 0, UInt32 layers = 1, UInt32 plane = 0) -> ImageBarrierBuilder<TParent>& {
                m_level = level;
                m_levels = levels;
                m_layer = layer;
                m_layers = layers;
                m_plane = plane;

                return *this;
            };
        };

    private:
        /// <summary>
        /// Function that is called back from the <see cref="SecondStageBuilder" /> in order to setup the barrier pipeline stages.
        /// </summary>
        /// <param name="waitFor">The pipeline stage to wait for with the barrier.</param>
        /// <param name="continueWith">The pipeline stage to allow continuation with the current barrier.</param>
        constexpr inline void stagesCallback(PipelineStage waitFor, PipelineStage continueWith) {
            this->setupStages(waitFor, continueWith);
        }

        /// <summary>
        /// Function that is called back from the <see cref="GlobalBarrierBuilder" /> in order to setup the resource access states for a global resource barrier.
        /// </summary>
        /// <param name="before">The resource access state of all resources to wait for with this barrier.</param>
        /// <param name="after">The resource access state of all resources to continue with after this barrier.</param>
        constexpr inline void globalBarrierCallback(ResourceAccess before, ResourceAccess after) {
            this->setupGlobalBarrier(before, after);
        }

        /// <summary>
        /// Function that is called back from the <see cref="BufferBarrierBuilder" /> in order to setup the resource access states for a buffer resource barrier.
        /// </summary>
        /// <param name="buffer">The buffer for which the barrier blocks.</param>
        /// <param name="before">The resource access state of the buffer to wait for with this barrier.</param>
        /// <param name="after">The resource access state of the buffer to continue with after this barrier.</param>
        constexpr inline void bufferBarrierCallback(IBuffer& buffer, ResourceAccess before, ResourceAccess after) {
            this->setupBufferBarrier(buffer, before, after);
        }

        /// <summary>
        /// Function that is called back from the <see cref="ImageLayoutBarrierBuilder" /> in order to setup the layout transition and resource access states for a set if sub-resources of a specific image.
        /// </summary>
        /// <param name="image">The image for this barrier.</param>
        /// <param name="before">The resource access state of the sub-resources in the image to wait for with this barrier.</param>
        /// <param name="after">The resource access state of the sub-resources in the image to continue with after this barrier.</param>
        /// <param name="layout">The layout to transition the image sub-resources into.</param>
        /// <param name="level">The level of the first sub-resource to transition.</param>
        /// <param name="levels">The number of levels to to transition.</param>
        /// <param name="layer">The layer of the first sub-resource to transition.</param>
        /// <param name="layers">The number of layers to transition.</param>
        /// <param name="plane">The plane of the sub-resource to transition.</param>
        constexpr inline void imageBarrierCallback(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) {
            this->setupImageBarrier(image, before, after, layout, level, levels, layer, layers, plane);
        }

    protected:
        /// <summary>
        /// Sets the pipeline stages for the built barrier to wait for and to continue with.
        /// </summary>
        /// <param name="waitFor">The pipeline stage to wait for with the barrier.</param>
        /// <param name="continueWith">The pipeline stage to allow continuation with the current barrier.</param>
        constexpr inline virtual void setupStages(PipelineStage waitFor, PipelineStage continueWith) = 0;

        /// <summary>
        /// Sets up the resource access states to wait for and to continue with the barrier to be built.
        /// </summary>
        /// <param name="before">The resource access state of all resources to wait for with this barrier.</param>
        /// <param name="after">The resource access state of all resources to continue with after this barrier.</param>
        constexpr inline virtual void setupGlobalBarrier(ResourceAccess before, ResourceAccess after) = 0;

        /// <summary>
        /// Sets up the resource access states to wait for and to continue with for a specific buffer with the barrier to be built.
        /// </summary>
        /// <param name="buffer">The buffer for which the barrier blocks.</param>
        /// <param name="before">The resource access state of all resources to wait for with this barrier.</param>
        /// <param name="after">The resource access state of all resources to continue with after this barrier.</param>
        constexpr inline virtual void setupBufferBarrier(IBuffer& buffer, ResourceAccess before, ResourceAccess after) = 0;

        /// <summary>
        /// Sets up the image layout transition and resource access states to wait for and continue with the barrier to be built.
        /// </summary>
        /// <param name="image">The image for this barrier.</param>
        /// <param name="before">The resource access state of the sub-resources in the image to wait for with this barrier.</param>
        /// <param name="after">The resource access state of the sub-resources in the image to continue with after this barrier.</param>
        /// <param name="layout">The layout to transition the image sub-resources into.</param>
        /// <param name="level">The level of the first sub-resource to transition.</param>
        /// <param name="levels">The number of levels to to transition.</param>
        /// <param name="layer">The layer of the first sub-resource to transition.</param>
        /// <param name="layers">The number of layers to transition.</param>
        /// <param name="plane">The plane of the sub-resource to transition.</param>
        constexpr inline virtual void setupImageBarrier(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) = 0;

    public:
        using Builder<TBarrier>::Builder;
        using barrier_type = TBarrier;

    public:
        /// <summary>
        /// Specifies the pipeline stages to wait for before executing the barrier.
        /// </summary>
        /// <param name="stage">The pipeline stages to wait for before executing the barrier.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto waitFor(this TSelf&& self, PipelineStage stage) -> SecondStageBuilder<TSelf>&& {
            return SecondStageBuilder<TSelf>{ std::move(self), stage };
        }

        /// <summary>
        /// Specifies the resource accesses that are blocked in a global barrier until the barrier has executed.
        /// </summary>
        /// <param name="access">The resource accesses that are blocked until the barrier has executed.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto blockAccessTo(this TSelf&& self, ResourceAccess access) -> GlobalBarrierBuilder<TSelf>&& {
            return GlobalBarrierBuilder<TSelf>{ std::move(self), access };
        }

        /// <summary>
        /// Specifies the resource accesses that are blocked for <paramref name="buffer" /> until the barrier has executed.
        /// </summary>
        /// <param name="buffer">The buffer to wait for.</param>
        /// <param name="access">The resource accesses that are blocked until the barrier has executed.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto blockAccessTo(this TSelf&& self, IBuffer& buffer, ResourceAccess access) -> BufferBarrierBuilder<TSelf>&& {
            return BufferBarrierBuilder<TSelf>{ std::move(self), buffer, access };
        }

        /// <summary>
        /// Specifies the resource accesses that are blocked for <paramref name="buffer" /> until the barrier has executed.
        /// </summary>
        /// <param name="buffer">The buffer to wait for.</param>
        /// <param name="subresource">The sub-resource to block.</param>
        /// <param name="access">The resource accesses that are blocked until the barrier has executed.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto blockAccessTo(this TSelf&& self, IBuffer& buffer, UInt32 subresource, ResourceAccess access) -> BufferBarrierBuilder<TSelf>&& {
            return BufferBarrierBuilder<TSelf>{ std::move(self), buffer, subresource, access };
        }

        /// <summary>
        /// Specifies the resource accesses that are blocked for <paramref name="image" /> until the barrier has executed.
        /// </summary>
        /// <param name="image">The buffer to wait for.</param>
        /// <param name="access">The resource accesses that are blocked until the barrier has executed.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto blockAccessTo(this TSelf&& self, IImage& image, ResourceAccess access) -> ImageBarrierBuilder<TSelf>&& {
            return ImageBarrierBuilder<TSelf>{ std::move(self), image, access };
        }
    };

    /// <summary>
    /// Base class for a builder that builds a <see cref="ShaderProgram" />.
    /// </summary>
    /// <typeparam name="TShaderProgram">The type of the shader program. Must implement <see cref="ShaderProgram" />.</typeparam>
    /// <seealso cref="ShaderProgram" />
    template <typename TShaderProgram> requires
        rtti::implements<TShaderProgram, ShaderProgram<typename TShaderProgram::shader_module_type>>
    class ShaderProgramBuilder : public Builder<TShaderProgram, std::nullptr_t, SharedPtr<TShaderProgram>> {
    public:
        using Builder<TShaderProgram, std::nullptr_t, SharedPtr<TShaderProgram>>::Builder;
        using shader_program_type = TShaderProgram;
        using shader_module_type = shader_program_type::shader_module_type;

    protected:
        /// <summary>
        /// Called to register a new shader module in the program that is stored in a file.
        /// </summary>
        /// <param name="type">The type of the shader module.</param>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        constexpr virtual void addShaderModuleFromFile(const ShaderStage& type, const String& fileName, const String& entryPoint) = 0;

        /// <summary>
        /// Called to register a new shader module in the program that is loaded from a stream.
        /// </summary>
        /// <param name="type">The type of the shader module.</param>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        constexpr virtual void addShaderModuleFromFile(const ShaderStage& type, std::istream& stream, const String& name, const String& entryPoint) = 0;

    public:
        /// <summary>
        /// Adds a shader module to the program.
        /// </summary>
        /// <param name="type">The type of the shader module.</param>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withShaderModule(this TSelf&& self, const ShaderStage& type, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            self.addShaderModuleFromFile(type, fileName, entryPoint);
            return self;
        }

        /// <summary>
        /// Adds a shader module to the program.
        /// </summary>
        /// <param name="type">The type of the shader module.</param>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withShaderModule(this TSelf&& self, const ShaderStage& type, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            self.addShaderModuleFromStream(type, stream, name, entryPoint);
            return self;
        }

        /// <summary>
        /// Adds a vertex shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withVertexShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(fileName, ShaderStage::Vertex, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a vertex shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withVertexShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(ShaderStage::Vertex, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a tessellation control shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withTessellationControlShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(fileName, ShaderStage::TessellationControl, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a tessellation control shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withTessellationControlShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(ShaderStage::TessellationControl, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a tessellation evaluation shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withTessellationEvaluationShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(fileName, ShaderStage::TessellationEvaluation, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a tessellation evaluation shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withTessellationEvaluationShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(ShaderStage::TessellationEvaluation, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a geometry shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withGeometryShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(fileName, ShaderStage::Geometry, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a geometry shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withGeometryShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(ShaderStage::Geometry, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a fragment shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withFragmentShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(fileName, ShaderStage::Fragment, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a fragment shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withFragmentShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(ShaderStage::Fragment, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a compute shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withComputeShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(fileName, ShaderStage::Compute, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a compute shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withComputeShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return self.withShaderModule(ShaderStage::Compute, stream, name, entryPoint);
        }
    };

    /// <summary>
    /// Builds a <see cref="Rasterizer" />.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TRasterizer">The type of the rasterizer. Must implement <see cref="IRasterizer" />.</typeparam>
    /// <seealso cref="IRasterizer" />
    template <typename TDerived, typename TRasterizer> requires
        rtti::implements<TRasterizer, IRasterizer>
    class RasterizerBuilder : public Builder<TDerived, TRasterizer, std::nullptr_t, SharedPtr<TRasterizer>> {
    public:
        using Builder<TDerived, TRasterizer, std::nullptr_t, SharedPtr<TRasterizer>>::Builder;
        using rasterizer_type = TRasterizer;

    public:
        /// <summary>
        /// Initializes the rasterizer state with the provided polygon mode.
        /// </summary>
        /// <param name="mode">The polygon mode to initialize the rasterizer state with.</param>
        virtual TDerived& polygonMode(const PolygonMode& mode) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer state with the provided cull mode.
        /// </summary>
        /// <param name="mode">The cull mode to initialize the rasterizer state with.</param>
        virtual TDerived& cullMode(const CullMode& mode) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer state with the provided cull order.
        /// </summary>
        /// <param name="order">The cull order to initialize the rasterizer state with.</param>
        virtual TDerived& cullOrder(const CullOrder& order) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer state with the provided line width.
        /// </summary>
        /// <param name="width">The line width to initialize the rasterizer state with.</param>
        virtual TDerived& lineWidth(const Float& width) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer depth bias.
        /// </summary>
        /// <param name="depthBias">The depth bias the rasterizer should use.</param>
        virtual TDerived& depthBias(const DepthStencilState::DepthBias& depthBias) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer depth state.
        /// </summary>
        /// <param name="depthState">The depth state of the rasterizer.</param>
        virtual TDerived& depthState(const DepthStencilState::DepthState& depthState) noexcept = 0;

        /// <summary>
        /// Initializes the rasterizer stencil state.
        /// </summary>
        /// <param name="stencilState">The stencil state of the rasterizer.</param>
        virtual TDerived& stencilState(const DepthStencilState::StencilState& stencilState) noexcept = 0;
    };

    /// <summary>
    /// Builds a <see cref="VertexBufferLayout" />.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout" />.</typeparam>
    /// <seealso cref="IVertexBufferLayout" />
    template <typename TDerived, typename TVertexBufferLayout, typename TParent> requires
        rtti::implements<TVertexBufferLayout, IVertexBufferLayout>
    class VertexBufferLayoutBuilder : public Builder<TDerived, TVertexBufferLayout, TParent> {
    public:
        using Builder<TDerived, TVertexBufferLayout, TParent>::Builder;
        using vertex_buffer_layout_type = TVertexBufferLayout;

    public:
        /// <summary>
        /// Adds an attribute to the vertex buffer layout.
        /// </summary>
        /// <param name="attribute">The attribute to add to the layout.</param>
        virtual TDerived& withAttribute(UniquePtr<BufferAttribute>&& attribute) = 0;
    };

    /// <summary>
    /// Builds a <see cref="DescriptorSetLayout" /> for a <see cref="PipelineLayout" />.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TDescriptorSetLayout">The type of the descriptor set layout. Must implement <see cref="DescriptorSetLayout" />.</typeparam>
    /// <seealso cref="DescriptorSetLayout" />
    /// <seealso cref="PipelineLayout" />
    template <typename TDerived, typename TDescriptorSetLayout, typename TParent> requires
        rtti::implements<TDescriptorSetLayout, DescriptorSetLayout<typename TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSetLayout::descriptor_set_type>>
    class DescriptorSetLayoutBuilder : public Builder<TDerived, TDescriptorSetLayout, TParent> {
    public:
        using Builder<TDerived, TDescriptorSetLayout, TParent>::Builder;
        using descriptor_set_layout_type = TDescriptorSetLayout;
        using descriptor_layout_type = descriptor_set_layout_type::descriptor_layout_type;
        using descriptor_set_type = descriptor_set_layout_type::descriptor_set_type;

    public:
        /// <summary>
        /// Adds a descriptor to the descriptor set layout.
        /// </summary>
        /// <param name="layout">The descriptor layout to add.</param>
        virtual TDerived& withDescriptor(UniquePtr<descriptor_layout_type>&& layout) = 0;

        /// <summary>
        /// Adds a descriptor to the descriptor set layout.
        /// </summary>
        /// <param name="type">The type of the descriptor.</param>
        /// <param name="binding">The binding point for the descriptor.</param>
        /// <param name="descriptorSize">The size of a single descriptor.</param>
        /// <param name="descriptors">The number of descriptors to bind.</param>
        virtual TDerived& withDescriptor(const DescriptorType& type, const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors = 1) = 0;

        /// <summary>
        /// Defines a static sampler at the descriptor bound to <see cref="binding" />.
        /// </summary>
        /// <param name="binding">The binding point for the descriptor.</param>
        /// <param name="magFilter">The magnifying filter operation.</param>
        /// <param name="minFilter">The minifying filter operation.</param>
        /// <param name="borderU">The border address mode into U direction.</param>
        /// <param name="borderV">The border address mode into V direction.</param>
        /// <param name="borderW">The border address mode into W direction.</param>
        /// <param name="mipMapMode">The mip map filter operation.</param>
        /// <param name="mipMapBias">The mip map bias.</param>
        /// <param name="minLod">The closest mip map distance level.</param>
        /// <param name="maxLod">The furthest mip map distance level. </param>
        /// <param name="anisotropy">The maximum anisotropy.</param>
        virtual TDerived& withStaticSampler(const UInt32& binding, const FilterMode& magFilter = FilterMode::Nearest, const FilterMode& minFilter = FilterMode::Nearest, const BorderMode& borderU = BorderMode::Repeat, const BorderMode& borderV = BorderMode::Repeat, const BorderMode& borderW = BorderMode::Repeat, const MipMapMode& mipMapMode = MipMapMode::Nearest, const Float& mipMapBias = 0.f, const Float& minLod = 0.f, const Float& maxLod = std::numeric_limits<Float>::max(), const Float& anisotropy = 0.f) = 0;

    public:
        /// <summary>
        /// Adds an uniform/constant buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptorSize">The size of a single descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        virtual TDerived& withConstantBuffer(const UInt32& binding, const UInt32& descriptorSize, const UInt32& descriptors = 1) {
            return this->withDescriptor(DescriptorType::ConstantBuffer, binding, descriptorSize, descriptors);
        }

        /// <summary>
        /// Adds a texel buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        virtual TDerived& withBuffer(const UInt32& binding, const UInt32& descriptors = 1, const bool& writable = false) {
            return this->withDescriptor(writable ? DescriptorType::RWBuffer : DescriptorType::Buffer, binding, 0, descriptors);
        }

        /// <summary>
        /// Adds a storage/structured buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        virtual TDerived& withStructuredBuffer(const UInt32& binding, const UInt32& descriptors = 1, const bool& writable = false) {
            return this->withDescriptor(writable ? DescriptorType::RWStructuredBuffer : DescriptorType::StructuredBuffer, binding, 0, descriptors);
        }

        /// <summary>
        /// Adds a byte address buffer buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        virtual TDerived& withByteAddressBuffer(const UInt32& binding, const UInt32& descriptors = 1, const bool& writable = false) {
            return this->withDescriptor(writable ? DescriptorType::RWByteAddressBuffer : DescriptorType::ByteAddressBuffer, binding, 0, descriptors);
        }

        /// <summary>
        /// Adds an image/texture descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        virtual TDerived& withTexture(const UInt32& binding, const UInt32& descriptors = 1, const bool& writable = false) {
            return this->withDescriptor(writable ? DescriptorType::RWTexture : DescriptorType::Texture, binding, 0, descriptors);
        }

        /// <summary>
        /// Adds an input attachment descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        virtual TDerived& withInputAttachment(const UInt32& binding) {
            return this->withDescriptor(DescriptorType::InputAttachment, binding, 0);
        }

        /// <summary>
        /// Adds a sampler descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        virtual TDerived& withSampler(const UInt32& binding, const UInt32& descriptors = 1) {
            return this->withDescriptor(DescriptorType::Sampler, binding, 0, descriptors);
        }

        /// <summary>
        /// Adds a descriptor layout to the descriptor set.
        /// </summary>
        /// <param name="layout">The layout of the descriptor.</param>
        /// <seealso cref="DescriptorLayout" />
        virtual void use(UniquePtr<descriptor_layout_type>&& layout) {
            this->withDescriptor(std::move(layout));
        }
    };

    /// <summary>
    /// Builds a <see cref="PushConstantsLayout" /> for a <see cref="PipelineLayout" />.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TPushConstantsLayout">The type of the push constants layout. Must implement <see cref="PushConstantsLayout" />.</typeparam>
    /// <seealso cref="PushConstantsLayout" />
    template <typename TDerived, typename TPushConstantsLayout, typename TParent> requires
        rtti::implements<TPushConstantsLayout, PushConstantsLayout<typename TPushConstantsLayout::push_constants_range_type>>
    class PushConstantsLayoutBuilder : public Builder<TDerived, TPushConstantsLayout, TParent> {
    public:
        using Builder<TDerived, TPushConstantsLayout, TParent>::Builder;
        using push_constants_layout_type = TPushConstantsLayout;
        using push_constants_range_type = push_constants_layout_type::push_constants_range_type;

    public:
        /// <summary>
        /// Adds a new push constants range.
        /// </summary>
        /// <param name="shaderStages">The shader stage, for which the range is defined.</param>
        /// <param name="offset">The offset of the range.</param>
        /// <param name="size">The size of the range.</param>
        /// <param name="space">The descriptor space, the range is bound to.</param>
        /// <param name="binding">The binding point for the range.</param>
        virtual TDerived& withRange(const ShaderStage& shaderStages, const UInt32& offset, const UInt32& size, const UInt32& space, const UInt32& binding) = 0;
    };

    /// <summary>
    /// Base class for a builder of a <see cref="PipelineLayout" />.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TPipelineLayout">The type of the pipeline layout. Must implement <see cref="PipelineLayout" />.</typeparam>
    /// <seealso cref="PipelineLayout" />
    template <typename TDerived, typename TPipelineLayout> requires
        rtti::implements<TPipelineLayout, PipelineLayout<typename TPipelineLayout::descriptor_set_layout_type, typename TPipelineLayout::push_constants_layout_type>>
    class PipelineLayoutBuilder : public Builder<TDerived, TPipelineLayout, std::nullptr_t, SharedPtr<TPipelineLayout>> {
    public:
        using Builder<TDerived, TPipelineLayout, std::nullptr_t, SharedPtr<TPipelineLayout>>::Builder;
        using pipeline_layout_type = TPipelineLayout;
        using descriptor_set_layout_type = pipeline_layout_type::descriptor_set_layout_type;
        using push_constants_layout_type = pipeline_layout_type::push_constants_layout_type;

    public:
        /// <summary>
        /// Adds a descriptor set to the pipeline layout.
        /// </summary>
        /// <param name="layout">The layout of the descriptor set.</param>
        /// <seealso cref="DescriptorSetLayout" />
        virtual void use(UniquePtr<descriptor_set_layout_type>&& layout) = 0;

        /// <summary>
        /// Adds a push constants range to the pipeline layout.
        /// </summary>
        /// <param name="layout">The layout of the push constants range.</param>
        /// <seealso cref="PushConstantsLayout" />
        virtual void use(UniquePtr<push_constants_layout_type>&& layout) = 0;
    };

    /// <summary>
    /// Builds a <see cref="InputAssembler" />.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TInputAssembler">The type of the input assembler state. Must implement <see cref="InputAssembler" />.</typeparam>
    /// <seealso cref="InputAssembler" />
    template <typename TDerived, typename TInputAssembler> requires
        rtti::implements<TInputAssembler, InputAssembler<typename TInputAssembler::vertex_buffer_layout_type, typename TInputAssembler::index_buffer_layout_type>>
    class InputAssemblerBuilder : public Builder<TDerived, TInputAssembler, std::nullptr_t, SharedPtr<TInputAssembler>> {
    public:
        using Builder<TDerived, TInputAssembler, std::nullptr_t, SharedPtr<TInputAssembler>>::Builder;
        using input_assembler_type = TInputAssembler;
        using vertex_buffer_layout_type = input_assembler_type::vertex_buffer_layout_type;
        using index_buffer_layout_type = input_assembler_type::index_buffer_layout_type;

    public:
        /// <summary>
        /// Specifies the topology to initialize the input assembler with.
        /// </summary>
        /// <param name="topology">The topology to initialize the input assembler with.</param>
        virtual TDerived& topology(const PrimitiveTopology& topology) = 0;

        /// <summary>
        /// Adds a vertex buffer layout to the input assembler. Can be called multiple times.
        /// </summary>
        /// <param name="layout">The layout to add to the input assembler.</param>
        virtual void use(UniquePtr<vertex_buffer_layout_type>&& layout) = 0;

        /// <summary>
        /// Adds an index buffer layout to the input assembler. Can only be called once.
        /// </summary>
        /// <param name="layout"></param>
        /// <exception cref="RuntimeException">Thrown if another index buffer layout has already been specified.</excpetion>
        virtual void use(UniquePtr<index_buffer_layout_type>&& layout) = 0;
    };

    /// <summary>
    /// Describes the interface of a render pipeline builder.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TRenderPipeline">The type of the render pipeline. Must implement <see cref="RenderPipeline" />.</typeparam>
    /// <seealso cref="RenderPipeline" />
    template <typename TDerived, typename TRenderPipeline> requires
        rtti::implements<TRenderPipeline, RenderPipeline<typename TRenderPipeline::pipeline_layout_type, typename TRenderPipeline::shader_program_type, typename TRenderPipeline::input_assembler_type, typename TRenderPipeline::rasterizer_type>>
    class RenderPipelineBuilder : public Builder<TDerived, TRenderPipeline> {
    public:
        using Builder<TDerived, TRenderPipeline>::Builder;
        using render_pipeline_type = TRenderPipeline;
        using pipeline_layout_type = render_pipeline_type::pipeline_layout_type;
        using shader_program_type = render_pipeline_type::shader_program_type;
        using input_assembler_type = render_pipeline_type::input_assembler_type;
        using rasterizer_type = render_pipeline_type::rasterizer_type;

    public:
        /// <summary>
        /// Adds a shader program to the pipeline layout.
        /// </summary>
        /// <remarks>
        /// Note that a pipeline must only have one shader program. If this method is called twice, the second call will overwrite the shader
        /// program set by the first call.
        /// </remarks>
        /// <param name="program">The program to add to the pipeline layout.</param>
        virtual TDerived& shaderProgram(SharedPtr<shader_program_type> program) = 0;

        /// <summary>
        /// Uses the provided pipeline layout to initialize the render pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="layout">The pipeline layout to initialize the render pipeline with.</param>
        virtual TDerived& layout(SharedPtr<pipeline_layout_type> layout) = 0;

        /// <summary>
        /// Uses the provided rasterizer state to initialize the render pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="rasterizer">The rasterizer state to initialize the render pipeline with.</param>
        virtual TDerived& rasterizer(SharedPtr<rasterizer_type> rasterizer) = 0;

        /// <summary>
        /// Uses the provided input assembler state to initialize the render pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="inputAssembler">The input assembler state to initialize the render pipeline with.</param>
        virtual TDerived& inputAssembler(SharedPtr<input_assembler_type> inputAssembler) = 0;

        /// <summary>
        /// Enables <i>Alpha-to-Coverage</i> multi-sampling on the pipeline.
        /// </summary>
        /// <remarks>
        /// For more information on <i>Alpha-to-Coverage</i> multi-sampling see the remarks of <see cref="IRenderPipeline::alphaToCoverage" />.
        /// </remarks>
        /// <param name="enable">Whether or not to use <i>Alpha-to-Coverage</i> multi-sampling.</param>
        virtual TDerived& enableAlphaToCoverage(const bool& enable = true) = 0;
    };

    /// <summary>
    /// Describes the interface of a render pipeline builder.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TComputePipeline">The type of the compute pipeline. Must implement <see cref="ComputePipeline" />.</typeparam>
    /// <seealso cref="ComputePipeline" />
    template <typename TDerived, typename TComputePipeline> requires
        rtti::implements<TComputePipeline, ComputePipeline<typename TComputePipeline::pipeline_layout_type, typename TComputePipeline::shader_program_type>>
    class ComputePipelineBuilder : public Builder<TDerived, TComputePipeline> {
    public:
        using Builder<TDerived, TComputePipeline>::Builder;
        using compute_pipeline_type = TComputePipeline;
        using pipeline_layout_type = compute_pipeline_type::pipeline_layout_type;
        using shader_program_type = compute_pipeline_type::shader_program_type;

    public:
        /// <summary>
        /// Adds a shader program to the pipeline.
        /// </summary>
        /// <remarks>
        /// Note that a pipeline must only have one shader program. If this method is called twice, the second call will overwrite the shader
        /// program set by the first call.
        /// </remarks>
        /// <param name="program">The program to add to the pipeline layout.</param>
        virtual TDerived& shaderProgram(SharedPtr<shader_program_type> program) = 0;

        /// <summary>
        /// Uses the provided pipeline layout to initialize the compute pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="layout">The pipeline layout to initialize the compute pipeline with.</param>
        virtual TDerived& layout(SharedPtr<pipeline_layout_type> layout) = 0;
    };

    /// <summary>
    /// Describes the interface of a render pass builder.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TRenderPass">The type of the render pass. Must implement <see cref="RenderPass" />.</typeparam>
    /// <seealso cref="RenderPass" />
    template <typename TDerived, typename TRenderPass> requires
        rtti::implements<TRenderPass, RenderPass<typename TRenderPass::render_pipeline_type, typename TRenderPass::frame_buffer_type, typename TRenderPass::input_attachment_mapping_type>>
    class RenderPassBuilder : public Builder<TDerived, TRenderPass> {
    public:
        using Builder<TDerived, TRenderPass>::Builder;
        using render_pass_type = TRenderPass;
        using input_attachment_mapping_type = render_pass_type::input_attachment_mapping_type;

    public:
        /// <summary>
        /// Sets the number of command buffers allocated by the render pass.
        /// </summary>
        /// <param name="count">The number of command buffers.</param>
        virtual TDerived& commandBuffers(const UInt32& count) = 0;

        /// <summary>
        /// Sets the multi-sampling level for the render targets.
        /// </summary>
        /// <param name="samples">The number of samples for each render target.</param>
        virtual TDerived& multiSamplingLevel(const MultiSamplingLevel& samples) = 0;

        /// <summary>
        /// Adds a render target to the render pass by assigning it an incremental location number.
        /// </summary>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The color format of the render target.</param>
        /// <param name="clearValues">The fixed clear value for the render target.</param>
        /// <param name="clearColor"><c>true</c>, if the render target color or depth should be cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared.</param>
        /// <param name="isVolatile"><c>true</c> to mark the render target as volatile, so is not required to be preserved after the render pass has ended.</param>
        virtual TDerived& renderTarget(const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;

        /// <summary>
        /// Adds a render target to the render pass by assigning it an incremental location number.
        /// </summary>
        /// <param name="name">The name of the render target.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The color format of the render target.</param>
        /// <param name="clearValues">The fixed clear value for the render target.</param>
        /// <param name="clearColor"><c>true</c>, if the render target color or depth should be cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared.</param>
        /// <param name="isVolatile"><c>true</c> to mark the render target as volatile, so is not required to be preserved after the render pass has ended.</param>
        virtual TDerived& renderTarget(const String& name, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;

        /// <summary>
        /// Adds a render target to the render pass.
        /// </summary>
        /// <param name="location">The location of the render target.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The color format of the render target.</param>
        /// <param name="clearValues">The fixed clear value for the render target.</param>
        /// <param name="clearColor"><c>true</c>, if the render target color or depth should be cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared.</param>
        /// <param name="isVolatile"><c>true</c> to mark the render target as volatile, so is not required to be preserved after the render pass has ended.</param>
        virtual TDerived& renderTarget(const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;

        /// <summary>
        /// Adds a render target to the render pass.
        /// </summary>
        /// <param name="name">The name of the render target.</param>
        /// <param name="location">The location of the render target.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The color format of the render target.</param>
        /// <param name="clearValues">The fixed clear value for the render target.</param>
        /// <param name="clearColor"><c>true</c>, if the render target color or depth should be cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared.</param>
        /// <param name="isVolatile"><c>true</c> to mark the render target as volatile, so is not required to be preserved after the render pass has ended.</param>
        virtual TDerived& renderTarget(const String& name, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;

        /// <summary>
        /// Adds a render target to the render pass, that maps to an input attachment of another render pass. The location is assigned incrementally.
        /// </summary>
        /// <param name="output">The input attachment mapping to map to.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The color format of the render target.</param>
        /// <param name="clearValues">The fixed clear value for the render target.</param>
        /// <param name="clearColor"><c>true</c>, if the render target color or depth should be cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared.</param>
        /// <param name="isVolatile"><c>true</c> to mark the render target as volatile, so is not required to be preserved after the render pass has ended.</param>
        virtual TDerived& renderTarget(input_attachment_mapping_type& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;

        /// <summary>
        /// Adds a render target to the render pass, that maps to an input attachment of another render pass. The location is assigned incrementally.
        /// </summary>
        /// <param name="name">The name of the render target.</param>
        /// <param name="output">The input attachment mapping to map to.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The color format of the render target.</param>
        /// <param name="clearValues">The fixed clear value for the render target.</param>
        /// <param name="clearColor"><c>true</c>, if the render target color or depth should be cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared.</param>
        /// <param name="isVolatile"><c>true</c> to mark the render target as volatile, so is not required to be preserved after the render pass has ended.</param>
        virtual TDerived& renderTarget(const String& name, input_attachment_mapping_type& output, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;

        /// <summary>
        /// Adds a render target to the render pass, that maps to an input attachment of another render pass.
        /// </summary>
        /// <param name="output">The input attachment mapping to map to.</param>
        /// <param name="location">The location of the render target.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The color format of the render target.</param>
        /// <param name="clearValues">The fixed clear value for the render target.</param>
        /// <param name="clearColor"><c>true</c>, if the render target color or depth should be cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared.</param>
        /// <param name="isVolatile"><c>true</c> to mark the render target as volatile, so is not required to be preserved after the render pass has ended.</param>
        virtual TDerived& renderTarget(input_attachment_mapping_type& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;

        /// <summary>
        /// Adds a render target to the render pass, that maps to an input attachment of another render pass.
        /// </summary>
        /// <param name="name">The name of the render target.</param>
        /// <param name="output">The input attachment mapping to map to.</param>
        /// <param name="location">The location of the render target.</param>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The color format of the render target.</param>
        /// <param name="clearValues">The fixed clear value for the render target.</param>
        /// <param name="clearColor"><c>true</c>, if the render target color or depth should be cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared.</param>
        /// <param name="isVolatile"><c>true</c> to mark the render target as volatile, so is not required to be preserved after the render pass has ended.</param>
        virtual TDerived& renderTarget(const String& name, input_attachment_mapping_type& output, const UInt32& location, const RenderTargetType& type, const Format& format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) = 0;
        
        /// <summary>
        /// Adds an input attachment to the render pass.
        /// </summary>
        /// <param name="inputAttachment">The input attachment to add.</param>
        virtual TDerived& inputAttachment(const input_attachment_mapping_type& inputAttachment) = 0;

        /// <summary>
        /// Adds an input attachment to the render pass.
        /// </summary>
        /// <param name="inputLocation">The location from which the input attachment gets accessed.</param>
        /// <param name="renderPass">The render pass, the input attachment is created from.</param>
        /// <param name="outputLocation">The location to which the input attachment is written by <paramref name="renderPass" />.</param>
        virtual TDerived& inputAttachment(const UInt32& inputLocation, const render_pass_type& renderPass, const UInt32& outputLocation) = 0;

        /// <summary>
        /// Adds an input attachment to the render pass.
        /// </summary>
        /// <param name="inputLocation">The location from which the input attachment gets accessed.</param>
        /// <param name="renderPass">The render pass, the input attachment is created from.</param>
        /// <param name="renderTarget">The render target that is bound as input attachment.</param>
        virtual TDerived& inputAttachment(const UInt32& inputLocation, const render_pass_type& renderPass, const RenderTarget& renderTarget) = 0;
    };
}
#endif // defined(BUILD_DEFINE_BUILDERS)