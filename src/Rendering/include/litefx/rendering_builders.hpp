#pragma once

#include "rendering_api.hpp"
#include "rendering.hpp"

#if defined(BUILD_DEFINE_BUILDERS)
namespace LiteFX::Rendering {

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
            constexpr inline auto toContinueWith(PipelineStage stage) -> TParent {
                this->m_parent.stagesCallback(this->m_from, stage);
                return std::move(this->m_parent);
            }
        };

        /// <summary>
        /// A builder that sets up a global resource barrier.
        /// </summary>
        /// <typeparam name="TParent">The type of the parent barrier builder.</typeparam>
        template <typename TParent> requires
            rtti::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] GlobalBarrierBuilder {
        private:
            ResourceAccess m_access;
            TParent m_parent;

            /// <summary>
            /// Initializes a builder that sets up a global resource barrier.
            /// </summary>
            /// <param name="parent">The parent builder instance.</param>
            /// <param name="access">The resource access state of all resources to wait for with this barrier.</param>
            constexpr inline GlobalBarrierBuilder(TParent&& parent, ResourceAccess access) noexcept :
                m_parent(std::move(parent)), m_access(access) { }

        public:
            friend class BarrierBuilder;

            /// <summary>
            /// Specifies the resource accesses that are waited for in a global barrier before it can be executed.
            /// </summary>
            /// <param name="access">The resource accesses that are waited for until the barrier can be executed.</param>
            constexpr inline auto untilFinishedWith(ResourceAccess access) -> TParent {
                this->m_parent.globalBarrierCallback(access, m_access);
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
            ResourceAccess m_access;
            IBuffer& m_buffer;
            TParent m_parent;

            /// <summary>
            /// Initializes a builder that sets up a barrier for a specific buffer.
            /// </summary>
            /// <param name="parent">The parent builder instance.</param>
            /// <param name="buffer">The buffer for this barrier.</param>
            /// <param name="access">The resource access state of the buffer to wait for with this barrier.</param>
            constexpr inline BufferBarrierBuilder(TParent&& parent, IBuffer& buffer, ResourceAccess access) noexcept :
                m_parent(std::move(parent)), m_buffer(buffer), m_access(access) { }

        public:
            friend class BarrierBuilder;

            /// <summary>
            /// Specifies the resource accesses that are waited for in a buffer before the barrier can be executed.
            /// </summary>
            /// <param name="access">The resource accesses that are waited for in a buffer before the barrier can be executed.</param>
            constexpr inline auto untilFinishedWith(ResourceAccess access) -> TParent {
                this->m_parent.bufferBarrierCallback(m_buffer, access, m_access);
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
            ResourceAccess m_access;
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
            /// <param name="levels">The number of levels to transition.</param>
            /// <param name="layer">The layer of the first sub-resource to transition.</param>
            /// <param name="layers">The number of layers to transition.</param>
            /// <param name="plane">The plane of the sub-resource to transition.</param>
            constexpr inline ImageLayoutBarrierBuilder(TParent&& parent, IImage& image, ResourceAccess access, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) noexcept :
                m_parent(std::move(parent)), m_image(image), m_access(access), m_layout(layout), m_level(level), m_levels(levels), m_layer(layer), m_layers(layers), m_plane(plane) { }

        public:
            friend class BarrierBuilder;
            friend class ImageBarrierBuilder<TParent>;

            /// <summary>
            /// Specifies the resource accesses that are waited for on the image sub-resources before the barrier can be executed.
            /// </summary>
            /// <param name="access">The resource accesses that are waited for on the image sub-resources before the barrier can be executed.</param>
            constexpr inline auto whenFinishedWith(ResourceAccess access) -> TParent {
                this->m_parent.imageBarrierCallback(m_image, access, m_access, m_layout, m_level, m_levels, m_layer, m_layers, m_plane);
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
            ResourceAccess m_access;
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
                m_parent(std::move(parent)), m_image(image), m_access(access), m_level{ 0 }, m_levels{ 0 }, m_layer{ 0 }, m_layers{ 0 }, m_plane{ 0 } { }

        public:
            friend class BarrierBuilder;

            /// <summary>
            /// Specifies the layout to transition an image to when executing the barrier.
            /// </summary>
            /// <param name="layout">The layout to transition an image to when executing the barrier.</param>
            constexpr inline auto transitionLayout(ImageLayout layout) -> ImageLayoutBarrierBuilder<TParent> {
                return ImageLayoutBarrierBuilder<TParent>{ std::move(m_parent), m_image, m_access, layout, m_level, m_levels, m_layer, m_layers, m_plane };
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
        /// <param name="levels">The number of levels to transition.</param>
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
        /// <param name="levels">The number of levels to transition.</param>
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
        constexpr inline [[nodiscard]] auto waitFor(this TSelf&& self, PipelineStage stage) -> SecondStageBuilder<TSelf> {
            return SecondStageBuilder<TSelf>{ std::move(self), stage };
        }

        /// <summary>
        /// Specifies the resource accesses that are blocked in a global barrier until the barrier has executed.
        /// </summary>
        /// <param name="access">The resource accesses that are blocked until the barrier has executed.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto blockAccessTo(this TSelf&& self, ResourceAccess access) -> GlobalBarrierBuilder<TSelf> {
            return GlobalBarrierBuilder<TSelf>{ std::move(self), access };
        }

        /// <summary>
        /// Specifies the resource accesses that are blocked for <paramref name="buffer" /> until the barrier has executed.
        /// </summary>
        /// <param name="buffer">The buffer to wait for.</param>
        /// <param name="access">The resource accesses that are blocked until the barrier has executed.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto blockAccessTo(this TSelf&& self, IBuffer& buffer, ResourceAccess access) -> BufferBarrierBuilder<TSelf> {
            return BufferBarrierBuilder<TSelf>{ std::move(self), buffer, access };
        }

        /// <summary>
        /// Specifies the resource accesses that are blocked for <paramref name="buffer" /> until the barrier has executed.
        /// </summary>
        /// <param name="buffer">The buffer to wait for.</param>
        /// <param name="subresource">The sub-resource to block.</param>
        /// <param name="access">The resource accesses that are blocked until the barrier has executed.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto blockAccessTo(this TSelf&& self, IBuffer& buffer, UInt32 subresource, ResourceAccess access) -> BufferBarrierBuilder<TSelf> {
            return BufferBarrierBuilder<TSelf>{ std::move(self), buffer, subresource, access };
        }

        /// <summary>
        /// Specifies the resource accesses that are blocked for <paramref name="image" /> until the barrier has executed.
        /// </summary>
        /// <param name="image">The buffer to wait for.</param>
        /// <param name="access">The resource accesses that are blocked until the barrier has executed.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto blockAccessTo(this TSelf&& self, IImage& image, ResourceAccess access) -> ImageBarrierBuilder<TSelf> {
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
        /// Stores the shader program state while building.
        /// </summary>
        struct ShaderProgramState {
            /// <summary>
            /// The shader modules of the program.
            /// </summary>
            Array<UniquePtr<shader_module_type>> modules;
        } m_state;

        /// <summary>
        /// Called to create a new shader module in the program that is stored in a file.
        /// </summary>
        /// <param name="type">The type of the shader module.</param>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        /// <returns>The shader module instance.</return>
        constexpr inline virtual UniquePtr<shader_module_type> makeShaderModule(ShaderStage type, const String& fileName, const String& entryPoint) = 0;

        /// <summary>
        /// Called to create a new shader module in the program that is loaded from a stream.
        /// </summary>
        /// <param name="type">The type of the shader module.</param>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        /// <returns>The shader module instance.</return>
        constexpr inline virtual UniquePtr<shader_module_type> makeShaderModule(ShaderStage type, std::istream& stream, const String& name, const String& entryPoint) = 0;

    public:
        /// <summary>
        /// Adds a shader module to the program.
        /// </summary>
        /// <param name="type">The type of the shader module.</param>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withShaderModule(this TSelf&& self, ShaderStage type, const String& fileName, const String& entryPoint = "main") -> TSelf& {
            self.m_state.modules.push_back(std::move(static_cast<ShaderProgramBuilder&>(self).makeShaderModule(type, fileName, entryPoint)));
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
        constexpr inline [[nodiscard]] auto withShaderModule(this TSelf&& self, ShaderStage type, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf& {
            self.m_state.modules.push_back(std::move(static_cast<ShaderProgramBuilder&>(self).makeShaderModule(type, stream, name, entryPoint)));
            return self;
        }

        /// <summary>
        /// Adds a vertex shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withVertexShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::Vertex, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a vertex shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withVertexShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::Vertex, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a tessellation control shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withTessellationControlShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a tessellation control shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withTessellationControlShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::TessellationControl, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a tessellation evaluation shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withTessellationEvaluationShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a tessellation evaluation shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withTessellationEvaluationShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::TessellationEvaluation, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a geometry shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withGeometryShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::Geometry, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a geometry shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withGeometryShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::Geometry, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a fragment shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withFragmentShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::Fragment, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a fragment shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withFragmentShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::Fragment, stream, name, entryPoint);
        }

        /// <summary>
        /// Adds a compute shader module to the program.
        /// </summary>
        /// <param name="fileName">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withComputeShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::Compute, fileName, entryPoint);
        }

        /// <summary>
        /// Adds a compute shader module to the program.
        /// </summary>
        /// <param name="stream">The file stream of the module.</param>
        /// <param name="name">The file name of the module.</param>
        /// <param name="entryPoint">The name of the entry point for the module.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto withComputeShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf& {
            return self.withShaderModule(ShaderStage::Compute, stream, name, entryPoint);
        }
    };

    /// <summary>
    /// Builds a <see cref="Rasterizer" />.
    /// </summary>
    /// <typeparam name="TRasterizer">The type of the rasterizer. Must implement <see cref="IRasterizer" />.</typeparam>
    /// <seealso cref="IRasterizer" />
    template <typename TRasterizer> requires
        rtti::implements<TRasterizer, IRasterizer>
    class RasterizerBuilder : public Builder<TRasterizer, std::nullptr_t, SharedPtr<TRasterizer>> {
    public:
        using Builder<TRasterizer, std::nullptr_t, SharedPtr<TRasterizer>>::Builder;
        using rasterizer_type = TRasterizer;

    protected:
        /// <summary>
        /// Stores the rasterizer state while building.
        /// </summary>
        struct RasterizerState {
            /// <summary>
            /// The polygon draw mode.
            /// </summary>
            PolygonMode polygonMode;

            /// <summary>
            /// The polygon cull mode.
            /// </summary>
            CullMode cullMode;

            /// <summary>
            /// The polygon cull order.
            /// </summary>
            CullOrder cullOrder;

            /// <summary>
            /// The line width, if line rasterization is supported.
            /// </summary>
            Float lineWidth;

            /// <summary>
            /// The depth bias state.
            /// </summary>
            DepthStencilState::DepthBias depthBias;

            /// <summary>
            /// The depth state.
            /// </summary>
            DepthStencilState::DepthState depthState;

            /// <summary>
            /// The stencil state.
            /// </summary>
            DepthStencilState::StencilState stencilState;
        } m_state;

    public:
        /// <summary>
        /// Initializes the rasterizer state with the provided polygon mode.
        /// </summary>
        /// <param name="mode">The polygon mode to initialize the rasterizer state with.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto polygonMode(this TSelf&& self, PolygonMode mode) noexcept -> TSelf& {
            self.m_state.polygonMode = mode;
            return self;
        }

        /// <summary>
        /// Initializes the rasterizer state with the provided cull mode.
        /// </summary>
        /// <param name="mode">The cull mode to initialize the rasterizer state with.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto cullMode(this TSelf&& self, CullMode mode) noexcept -> TSelf& {
            self.m_state.cullMode = mode;
            return self;
        }

        /// <summary>
        /// Initializes the rasterizer state with the provided cull order.
        /// </summary>
        /// <param name="order">The cull order to initialize the rasterizer state with.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto cullOrder(this TSelf&& self, CullOrder order) noexcept -> TSelf& {
            self.m_state.cullOrder = order;
            return self;
        }

        /// <summary>
        /// Initializes the rasterizer state with the provided line width.
        /// </summary>
        /// <param name="width">The line width to initialize the rasterizer state with.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto lineWidth(this TSelf&& self, Float width) noexcept -> TSelf& {
            self.m_state.lineWidth = width;
            return self;
        }

        /// <summary>
        /// Initializes the rasterizer depth bias.
        /// </summary>
        /// <param name="depthBias">The depth bias the rasterizer should use.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto depthBias(this TSelf&& self, const DepthStencilState::DepthBias& depthBias) noexcept -> TSelf& {
            self.m_state.depthBias = depthBias;
            return self;
        }

        /// <summary>
        /// Initializes the rasterizer depth state.
        /// </summary>
        /// <param name="depthState">The depth state of the rasterizer.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto depthState(this TSelf&& self, const DepthStencilState::DepthState& depthState) noexcept -> TSelf& {
            self.m_state.depthState = depthState;
            return self;
        }

        /// <summary>
        /// Initializes the rasterizer stencil state.
        /// </summary>
        /// <param name="stencilState">The stencil state of the rasterizer.</param>
        template<typename TSelf>
        constexpr inline [[nodiscard]] auto stencilState(this TSelf&& self, const DepthStencilState::StencilState& stencilState) noexcept -> TSelf& {
            self.m_state.stencilState = stencilState;
            return self;
        }
    };

    /// <summary>
    /// Builds a <see cref="VertexBufferLayout" />.
    /// </summary>
    /// <typeparam name="TVertexBufferLayout">The type of the vertex buffer layout. Must implement <see cref="IVertexBufferLayout" />.</typeparam>
    /// <seealso cref="IVertexBufferLayout" />
    template <typename TVertexBufferLayout, typename TParent> requires
        rtti::implements<TVertexBufferLayout, IVertexBufferLayout>
    class VertexBufferLayoutBuilder : public Builder<TVertexBufferLayout, TParent> {
    public:
        using Builder<TVertexBufferLayout, TParent>::Builder;
        using vertex_buffer_layout_type = TVertexBufferLayout;

    protected:
        /// <summary>
        /// Stores the vertex buffer layout state while building.
        /// </summary>
        struct VertexBufferLayoutState {
            /// <summary>
            /// The vertex buffer attributes of the layout.
            /// </summary>
            Array<UniquePtr<BufferAttribute>> attributes;
        } m_state;

    public:
        /// <summary>
        /// Adds an attribute to the vertex buffer layout.
        /// </summary>
        /// <param name="attribute">The attribute to add to the layout.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withAttribute(this TSelf&& self, UniquePtr<BufferAttribute>&& attribute) -> TSelf& {
            self.m_state.attributes.push_back(std::move(attribute));
            return self;
        }

        /// <summary>
        /// Adds an attribute to the vertex buffer layout.
        /// </summary>
        /// <remarks>
        /// This overload implicitly determines the location based on the number of attributes already defined. It should only be used if all locations can be implicitly deducted.
        /// </remarks>
        /// <param name="format">The format of the attribute.</param>
        /// <param name="offset">The offset of the attribute within a buffer element.</param>
        /// <param name="semantic">The semantic of the attribute.</param>
        /// <param name="semanticIndex">The semantic index of the attribute.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withAttribute(this TSelf&& self, BufferFormat format, UInt32 offset, AttributeSemantic semantic = AttributeSemantic::Unknown, UInt32 semanticIndex = 0) -> TSelf& {
            self.withAttribute(std::move(makeUnique<BufferAttribute>(static_cast<UInt32>(self.m_state.attributes.size()), offset, format, semantic, semanticIndex)));
            return self;
        }

        /// <summary>
        /// Adds an attribute to the vertex buffer layout.
        /// </summary>
        /// <param name="location">The location, the attribute is bound to.</param>
        /// <param name="format">The format of the attribute.</param>
        /// <param name="offset">The offset of the attribute within a buffer element.</param>
        /// <param name="semantic">The semantic of the attribute.</param>
        /// <param name="semanticIndex">The semantic index of the attribute.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withAttribute(this TSelf&& self, UInt32 location, BufferFormat format, UInt32 offset, AttributeSemantic semantic = AttributeSemantic::Unknown, UInt32 semanticIndex = 0) -> TSelf& {
            self.withAttribute(std::move(makeUnique<BufferAttribute>(location, offset, format, semantic, semanticIndex)));
            return self;
        }
    };

    /// <summary>
    /// Builds a <see cref="DescriptorSetLayout" /> for a <see cref="PipelineLayout" />.
    /// </summary>
    /// <typeparam name="TDescriptorSetLayout">The type of the descriptor set layout. Must implement <see cref="DescriptorSetLayout" />.</typeparam>
    /// <seealso cref="DescriptorSetLayout" />
    /// <seealso cref="PipelineLayout" />
    template <typename TDescriptorSetLayout, typename TParent> requires
        rtti::implements<TDescriptorSetLayout, DescriptorSetLayout<typename TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSetLayout::descriptor_set_type>>
    class DescriptorSetLayoutBuilder : public Builder<TDescriptorSetLayout, TParent> {
    public:
        using Builder<TDescriptorSetLayout, TParent>::Builder;
        using descriptor_set_layout_type = TDescriptorSetLayout;
        using descriptor_layout_type = descriptor_set_layout_type::descriptor_layout_type;
        using descriptor_set_type = descriptor_set_layout_type::descriptor_set_type;

    protected:
        /// <summary>
        /// Stores the descriptor set layout state while building.
        /// </summary>
        struct DescriptorSetLayoutState {
            /// <summary>
            /// The space of the descriptor set.
            /// </summary>
            UInt32 space;

            /// <summary>
            /// The pool size (if supported), of the descriptor pool that allocates the descriptors in the descriptor set.
            /// </summary>
            /// <remarks>
            /// Descriptor pools are only supported in Vulkan. For DirectX 12, this setting is ignored.
            /// </remarks>
            UInt32 poolSize;
            
            /// <summary>
            /// The maximum size of unbounded (i.e., bindless) descriptor arrays.
            /// </summary>
            /// <remarks>
            /// This setting is only required in Vulkan. For DirectX 12 it is ignored.
            /// </remarks>
            UInt32 maxUnboundedArraySize;

            /// <summary>
            /// The shader stages, the descriptor set is accessible from.
            /// </summary>
            ShaderStage stages;

            /// <summary>
            /// The layouts of the descriptors within the descriptor set.
            /// </summary>
            Array<UniquePtr<descriptor_layout_type>> descriptorLayouts;
        } m_state;

        /// <summary>
        /// Creates a descriptor to the descriptor set layout.
        /// </summary>
        /// <param name="type">The type of the descriptor.</param>
        /// <param name="binding">The binding point for the descriptor.</param>
        /// <param name="descriptorSize">The size of a single descriptor.</param>
        /// <param name="descriptors">The number of descriptors to bind.</param>
        /// <returns>The descriptor layout instance.</returns>
        constexpr inline virtual UniquePtr<descriptor_layout_type> makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors) = 0;

        /// <summary>
        /// Creates a static sampler for the descriptor bound to <see cref="binding" />.
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
        /// <returns>The descriptor layout instance for the static sampler.</returns>
        constexpr inline virtual UniquePtr<descriptor_layout_type> makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) = 0;

    public:
        /// <summary>
        /// Adds a descriptor to the descriptor set layout.
        /// </summary>
        /// <param name="layout">The descriptor layout to add.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withDescriptor(this TSelf&& self, UniquePtr<descriptor_layout_type>&& layout) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(layout));
            return self;
        }

        /// <summary>
        /// Adds a descriptor to the descriptor set layout.
        /// </summary>
        /// <param name="type">The type of the descriptor.</param>
        /// <param name="binding">The binding point for the descriptor.</param>
        /// <param name="descriptorSize">The size of a single descriptor.</param>
        /// <param name="descriptors">The number of descriptors to bind.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withDescriptor(this TSelf&& self, DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors = 1) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(self.makeDescriptor(type, binding, descriptorSize, descriptors)));
            return self;
        }

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
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withStaticSampler(this TSelf&& self, UInt32 binding, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float minLod = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float anisotropy = 0.f) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(self.makeDescriptor(binding, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy)));
            return self;
        }

        /// <summary>
        /// Adds an uniform/constant buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptorSize">The size of a single descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withConstantBuffer(this TSelf&& self, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors = 1) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(self.makeDescriptor(DescriptorType::ConstantBuffer, binding, descriptorSize, descriptors)));
            return self;
        }

        /// <summary>
        /// Adds a texel buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withBuffer(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1, bool writable = false) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(self.makeDescriptor(writable ? DescriptorType::RWBuffer : DescriptorType::Buffer, binding, 0, descriptors)));
            return self;
        }

        /// <summary>
        /// Adds a storage/structured buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withStructuredBuffer(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1, bool writable = false) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(self.makeDescriptor(writable ? DescriptorType::RWStructuredBuffer : DescriptorType::StructuredBuffer, binding, 0, descriptors)));
            return self;
        }

        /// <summary>
        /// Adds a byte address buffer descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withByteAddressBuffer(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1, bool writable = false) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(self.makeDescriptor(writable ? DescriptorType::RWByteAddressBuffer : DescriptorType::ByteAddressBuffer, binding, 0, descriptors)));
            return self;
        }

        /// <summary>
        /// Adds an image/texture descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        /// <param name="writable"><c>true</c>, if the buffer should be writable.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withTexture(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1, bool writable = false) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(self.makeDescriptor(writable ? DescriptorType::RWTexture : DescriptorType::Texture, binding, 0, descriptors)));
            return self;
        }

        /// <summary>
        /// Adds an input attachment descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withInputAttachment(this TSelf&& self, UInt32 binding) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(self.makeDescriptor(DescriptorType::InputAttachment, binding, 0)));
            return self;
        }

        /// <summary>
        /// Adds a sampler descriptor.
        /// </summary>
        /// <param name="binding">The binding point or register index of the descriptor.</param>
        /// <param name="descriptors">The number of descriptors in the array.</param>
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto withSampler(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(self.makeDescriptor(DescriptorType::Sampler, binding, 0, descriptors)));
            return self;
        }

        /// <summary>
        /// Sets the space, the descriptor set is bound to.
        /// </summary>
        /// <param name="space">The space, the descriptor set is bound to.</param>
        template <typename TSelf>
        constexpr inline auto space(this TSelf&& self, UInt32 space) noexcept -> TSelf& {
            self.m_state.space = space;
            return self;
        }

        /// <summary>
        /// Sets the shader stages, the descriptor set is accessible from.
        /// </summary>
        /// <param name="stages">The shader stages, the descriptor set is accessible from.</param>
        template <typename TSelf>
        constexpr inline auto shaderStages(this TSelf&& self, ShaderStage stages) noexcept -> TSelf& {
            self.m_state.stages = stages;
            return self;
        }

        /// <summary>
        /// Sets the size of the descriptor pools used for descriptor set allocations. Ignored for DirectX 12, but required for interface compatibility.
        /// </summary>
        /// <param name="poolSize">The size of the descriptor pools used for descriptor set allocations.</param>
        template <typename TSelf>
        constexpr inline auto poolSize(this TSelf&& self, UInt32 poolSize) noexcept -> TSelf& {
            self.m_state.poolSize = poolSize;
            return self;
        }

    public:
        /// <summary>
        /// Adds a descriptor layout to the descriptor set.
        /// </summary>
        /// <param name="layout">The layout of the descriptor.</param>
        /// <seealso cref="DescriptorLayout" />
        template <typename TSelf>
        constexpr inline [[nodiscard]] auto use(this TSelf&& self, UniquePtr<descriptor_layout_type>&& layout) -> TSelf& {
            self.m_state.descriptorLayouts.push_back(std::move(layout));
            return self;
        }
    };

    /// <summary>
    /// Builds a <see cref="PushConstantsLayout" /> for a <see cref="PipelineLayout" />.
    /// </summary>
    /// <typeparam name="TPushConstantsLayout">The type of the push constants layout. Must implement <see cref="PushConstantsLayout" />.</typeparam>
    /// <seealso cref="PushConstantsLayout" />
    template <typename TPushConstantsLayout, typename TParent> requires
        rtti::implements<TPushConstantsLayout, PushConstantsLayout<typename TPushConstantsLayout::push_constants_range_type>>
    class PushConstantsLayoutBuilder : public Builder<TPushConstantsLayout, TParent> {
    public:
        using Builder<TPushConstantsLayout, TParent>::Builder;
        using push_constants_layout_type = TPushConstantsLayout;
        using push_constants_range_type = push_constants_layout_type::push_constants_range_type;

    protected:
        /// <summary>
        /// Stores the push constants layout state while building.
        /// </summary>
        struct PushConstantsLayoutState {
            /// <summary>
            /// The push constant ranges of the layout.
            /// </summary>
            Array<UniquePtr<push_constants_range_type>> ranges;
        } m_state;

        /// <summary>
        /// Creates a new push constants range.
        /// </summary>
        /// <param name="shaderStages">The shader stage, for which the range is defined.</param>
        /// <param name="offset">The offset of the range.</param>
        /// <param name="size">The size of the range.</param>
        /// <param name="space">The descriptor space, the range is bound to.</param>
        /// <param name="binding">The binding point for the range.</param>
        /// <returns>The instance of the push constant range.</returns>
        virtual inline UniquePtr<push_constants_range_type> makeRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) = 0;

    public:
        /// <summary>
        /// Adds a new push constants range.
        /// </summary>
        /// <param name="shaderStages">The shader stage, for which the range is defined.</param>
        /// <param name="offset">The offset of the range.</param>
        /// <param name="size">The size of the range.</param>
        /// <param name="space">The descriptor space, the range is bound to.</param>
        /// <param name="binding">The binding point for the range.</param>
        template <typename TSelf>
        constexpr inline auto withRange(this TSelf&& self, ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) -> TSelf& {
            self.m_state.ranges.push_back(std::move(self.makeRange(shaderStages, offset, size, space, binding)));
            return self;
        }
    };

    /// <summary>
    /// Base class for a builder of a <see cref="PipelineLayout" />.
    /// </summary>
    /// <typeparam name="TPipelineLayout">The type of the pipeline layout. Must implement <see cref="PipelineLayout" />.</typeparam>
    /// <seealso cref="PipelineLayout" />
    template <typename TPipelineLayout> requires
        rtti::implements<TPipelineLayout, PipelineLayout<typename TPipelineLayout::descriptor_set_layout_type, typename TPipelineLayout::push_constants_layout_type>>
    class PipelineLayoutBuilder : public Builder<TPipelineLayout, std::nullptr_t, SharedPtr<TPipelineLayout>> {
    public:
        using Builder<TPipelineLayout, std::nullptr_t, SharedPtr<TPipelineLayout>>::Builder;
        using pipeline_layout_type = TPipelineLayout;
        using descriptor_set_layout_type = pipeline_layout_type::descriptor_set_layout_type;
        using push_constants_layout_type = pipeline_layout_type::push_constants_layout_type;

    protected:
        /// <summary>
        /// Stores the pipeline layout state while building.
        /// </summary>
        struct PipelineLayoutState {
            /// <summary>
            /// The descriptor set layouts of the pipeline state.
            /// </summary>
            Array<UniquePtr<descriptor_set_layout_type>> descriptorSetLayouts;

            /// <summary>
            /// The push constant layout of the pipeline state.
            /// </summary>
            UniquePtr<push_constants_layout_type> pushConstantsLayout;
        } m_state;

    public:
        /// <summary>
        /// Adds a descriptor set to the pipeline layout.
        /// </summary>
        /// <param name="layout">The layout of the descriptor set.</param>
        /// <seealso cref="DescriptorSetLayout" />
        template <typename TSelf>
        constexpr inline auto use(this TSelf&& self, UniquePtr<descriptor_set_layout_type>&& layout) -> TSelf& {
            self.m_state.descriptorSetLayouts.push_back(std::move(layout));
            return self;
        }

        /// <summary>
        /// Adds a push constants range to the pipeline layout.
        /// </summary>
        /// <param name="layout">The layout of the push constants range.</param>
        /// <seealso cref="PushConstantsLayout" />
        template <typename TSelf>
        constexpr inline auto use(this TSelf&& self, UniquePtr<push_constants_layout_type>&& layout) -> TSelf& {
            self.m_state.pushConstantsLayout = std::move(layout);
            return self;
        }
    };

    /// <summary>
    /// Builds a <see cref="InputAssembler" />.
    /// </summary>
    /// <typeparam name="TInputAssembler">The type of the input assembler state. Must implement <see cref="InputAssembler" />.</typeparam>
    /// <seealso cref="InputAssembler" />
    template <typename TInputAssembler> requires
        rtti::implements<TInputAssembler, InputAssembler<typename TInputAssembler::vertex_buffer_layout_type, typename TInputAssembler::index_buffer_layout_type>>
    class InputAssemblerBuilder : public Builder<TInputAssembler, std::nullptr_t, SharedPtr<TInputAssembler>> {
    public:
        using Builder<TInputAssembler, std::nullptr_t, SharedPtr<TInputAssembler>>::Builder;
        using input_assembler_type = TInputAssembler;
        using vertex_buffer_layout_type = input_assembler_type::vertex_buffer_layout_type;
        using index_buffer_layout_type = input_assembler_type::index_buffer_layout_type;

    protected:
        /// <summary>
        /// Stores the input assembler state while building.
        /// </summary>
        struct InputAssemblerState {
            /// <summary>
            /// The primitive topology.
            /// </summary>
            PrimitiveTopology topology;

            /// <summary>
            /// The vertex buffer layouts.
            /// </summary>
            Array<UniquePtr<vertex_buffer_layout_type>> vertexBufferLayouts;
            
            /// <summary>
            /// The index buffer layout.
            /// </summary>
            UniquePtr<index_buffer_layout_type> indexBufferLayout;
        } m_state;

    public:
        /// <summary>
        /// Specifies the topology to initialize the input assembler with.
        /// </summary>
        /// <param name="topology">The topology to initialize the input assembler with.</param>
        template <typename TSelf>
        constexpr inline auto topology(this TSelf&& self, PrimitiveTopology topology) -> TSelf& {
            self.m_state.topology = topology;
            return self;
        }

        /// <summary>
        /// Adds a vertex buffer layout to the input assembler. Can be called multiple times.
        /// </summary>
        /// <param name="layout">The layout to add to the input assembler.</param>
        template <typename TSelf>
        constexpr inline auto use(this TSelf&& self, UniquePtr<vertex_buffer_layout_type>&& layout) -> TSelf& {
            self.m_state.vertexBufferLayouts.push_back(std::move(layout));
            return self;
        }

        /// <summary>
        /// Adds an index buffer layout to the input assembler. Can only be called once.
        /// </summary>
        /// <param name="layout"></param>
        /// <exception cref="RuntimeException">Thrown if another index buffer layout has already been specified.</excpetion>
        template <typename TSelf>
        constexpr inline auto use(this TSelf&& self, UniquePtr<index_buffer_layout_type>&& layout) -> TSelf& {
            self.m_state.indexBufferLayout = std::move(layout);
            return self;
        }
    };

    /// <summary>
    /// Describes the interface of a render pipeline builder.
    /// </summary>
    /// <typeparam name="TRenderPipeline">The type of the render pipeline. Must implement <see cref="RenderPipeline" />.</typeparam>
    /// <seealso cref="RenderPipeline" />
    template <typename TRenderPipeline> requires
        rtti::implements<TRenderPipeline, RenderPipeline<typename TRenderPipeline::pipeline_layout_type, typename TRenderPipeline::shader_program_type, typename TRenderPipeline::input_assembler_type, typename TRenderPipeline::rasterizer_type>>
    class RenderPipelineBuilder : public Builder<TRenderPipeline> {
    public:
        using Builder<TRenderPipeline>::Builder;
        using render_pipeline_type = TRenderPipeline;
        using pipeline_layout_type = render_pipeline_type::pipeline_layout_type;
        using shader_program_type = render_pipeline_type::shader_program_type;
        using input_assembler_type = render_pipeline_type::input_assembler_type;
        using rasterizer_type = render_pipeline_type::rasterizer_type;

    protected:
        /// <summary>
        /// Stores the render pipeline state while building.
        /// </summary>
        struct RenderPipelineState {
            /// <summary>
            /// The shader program of the render pipeline.
            /// </summary>
            SharedPtr<shader_program_type> shaderProgram;

            /// <summary>
            /// The render pipeline layout.
            /// </summary>
            SharedPtr<pipeline_layout_type> pipelineLayout;

            /// <summary>
            /// The rasterizer state.
            /// </summary>
            SharedPtr<rasterizer_type> rasterizer;

            /// <summary>
            /// The input assembler state.
            /// </summary>
            SharedPtr<input_assembler_type> inputAssembler;

            /// <summary>
            /// The alpha-to-coverage setting.
            /// </summary>
            bool enableAlphaToCoverage{ false };
        } m_state;

    public:
        /// <summary>
        /// Adds a shader program to the pipeline layout.
        /// </summary>
        /// <remarks>
        /// Note that a pipeline must only have one shader program. If this method is called twice, the second call will overwrite the shader
        /// program set by the first call.
        /// </remarks>
        /// <param name="program">The program to add to the pipeline layout.</param>
        template <typename TSelf>
        constexpr inline auto shaderProgram(this TSelf&& self, SharedPtr<shader_program_type> program) -> TSelf& {
            self.m_state.shaderProgram = program;
            return self;
        }

        /// <summary>
        /// Uses the provided pipeline layout to initialize the render pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="layout">The pipeline layout to initialize the render pipeline with.</param>
        template <typename TSelf>
        constexpr inline auto layout(this TSelf&& self, SharedPtr<pipeline_layout_type> layout) -> TSelf& {
            self.m_state.pipelineLayout = layout;
            return self;
        }

        /// <summary>
        /// Uses the provided rasterizer state to initialize the render pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="rasterizer">The rasterizer state to initialize the render pipeline with.</param>
        template <typename TSelf>
        constexpr inline auto rasterizer(this TSelf&& self, SharedPtr<rasterizer_type> rasterizer) -> TSelf& {
            self.m_state.rasterizer = rasterizer;
            return self;
        }

        /// <summary>
        /// Uses the provided input assembler state to initialize the render pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="inputAssembler">The input assembler state to initialize the render pipeline with.</param>
        template <typename TSelf>
        constexpr inline auto inputAssembler(this TSelf&& self, SharedPtr<input_assembler_type> inputAssembler) -> TSelf& {
            self.m_state.inputAssembler = inputAssembler;
            return self;
        }

        /// <summary>
        /// Enables <i>Alpha-to-Coverage</i> multi-sampling on the pipeline.
        /// </summary>
        /// <remarks>
        /// For more information on <i>Alpha-to-Coverage</i> multi-sampling see the remarks of <see cref="IRenderPipeline::alphaToCoverage" />.
        /// </remarks>
        /// <param name="enable">Whether or not to use <i>Alpha-to-Coverage</i> multi-sampling.</param>
        template <typename TSelf>
        constexpr inline auto enableAlphaToCoverage(this TSelf&& self, bool enable = true) -> TSelf& {
            self.m_state.enableAlphaToCoverage = enable;
            return self;
        }
    };

    /// <summary>
    /// Describes the interface of a render pipeline builder.
    /// </summary>
    /// <typeparam name="TComputePipeline">The type of the compute pipeline. Must implement <see cref="ComputePipeline" />.</typeparam>
    /// <seealso cref="ComputePipeline" />
    template <typename TComputePipeline> requires
        rtti::implements<TComputePipeline, ComputePipeline<typename TComputePipeline::pipeline_layout_type, typename TComputePipeline::shader_program_type>>
    class ComputePipelineBuilder : public Builder<TComputePipeline> {
    public:
        using Builder<TComputePipeline>::Builder;
        using compute_pipeline_type = TComputePipeline;
        using pipeline_layout_type = compute_pipeline_type::pipeline_layout_type;
        using shader_program_type = compute_pipeline_type::shader_program_type;

    protected:
        /// <summary>
        /// Stores the compute pipeline state while building.
        /// </summary>
        struct ComputePipelineState {
            /// <summary>
            /// The compute pipeline shader program.
            /// </summary>
            SharedPtr<shader_program_type> shaderProgram;

            /// <summary>
            /// The compute pipeline layout.
            /// </summary>
            SharedPtr<pipeline_layout_type> pipelineLayout;
        } m_state;

    public:
        /// <summary>
        /// Adds a shader program to the pipeline.
        /// </summary>
        /// <remarks>
        /// Note that a pipeline must only have one shader program. If this method is called twice, the second call will overwrite the shader
        /// program set by the first call.
        /// </remarks>
        /// <param name="program">The program to add to the pipeline layout.</param>
        template <typename TSelf>
        constexpr inline auto shaderProgram(this TSelf&& self, SharedPtr<shader_program_type> program) -> TSelf& {
            self.m_state.shaderProgram = program;
            return self;
        }

        /// <summary>
        /// Uses the provided pipeline layout to initialize the compute pipeline. Can be invoked only once.
        /// </summary>
        /// <param name="layout">The pipeline layout to initialize the compute pipeline with.</param>
        template <typename TSelf>
        constexpr inline auto layout(this TSelf&& self, SharedPtr<pipeline_layout_type> layout) -> TSelf& {
            self.m_state.pipelineLayout = layout;
            return self;
        }
    };

    /// <summary>
    /// Describes the interface of a render pass builder.
    /// </summary>
    /// <typeparam name="TDerived">The type of the implementation of the builder.</typeparam>
    /// <typeparam name="TRenderPass">The type of the render pass. Must implement <see cref="RenderPass" />.</typeparam>
    /// <seealso cref="RenderPass" />
    template <typename TRenderPass> requires
        rtti::implements<TRenderPass, RenderPass<typename TRenderPass::render_pipeline_type, typename TRenderPass::frame_buffer_type, typename TRenderPass::input_attachment_mapping_type>>
    class RenderPassBuilder : public Builder<TRenderPass> {
    public:
        using Builder<TRenderPass>::Builder;
        using render_pass_type = TRenderPass;
        using input_attachment_mapping_type = render_pass_type::input_attachment_mapping_type;

    protected:
        /// <summary>
        /// Stores the render pass state while building.
        /// </summary>
        struct RenderPassState {
            /// <summary>
            /// The number of command buffers provided by the render pass.
            /// </summary>
            UInt32 commandBufferCount{ 0 };

            /// <summary>
            /// The multi-sampling level for the render pass frame buffer.
            /// </summary>
            MultiSamplingLevel multiSamplingLevel{ MultiSamplingLevel::x1 };

            /// <summary>
            /// The render targets of the render pass.
            /// </summary>
            Array<RenderTarget> renderTargets;

            /// <summary>
            /// The input attachments of the render pass.
            /// </summary>
            Array<input_attachment_mapping_type> inputAttachments;
        } m_state;

        /// <summary>
        /// Creates a new input attachment mapping between a render target of a specified render pass and an input location of the render pass that is currently built.
        /// </summary>
        /// <param name="inputLocation">The input location of the mapped render target.</param>
        /// <param name="renderPass">The render pass that produces the render target.</param>
        /// <param name="renderTarget">The render target of the render pass.</param>
        /// <returns>The input attachment mapping that describes the relation between the earlier render pass render target and the input location.</returns>
        virtual inline input_attachment_mapping_type makeInputAttachment(UInt32 inputLocation, const render_pass_type& renderPass, const RenderTarget& renderTarget) = 0;

    public:
        /// <summary>
        /// Sets the number of command buffers allocated by the render pass.
        /// </summary>
        /// <param name="count">The number of command buffers.</param>
        template <typename TSelf>
        constexpr inline auto commandBuffers(this TSelf&& self, UInt32 count) -> TSelf& {
            self.m_state.commandBufferCount = count;
            return self;
        }

        /// <summary>
        /// Sets the multi-sampling level for the render targets.
        /// </summary>
        /// <param name="samples">The number of samples for each render target.</param>
        template <typename TSelf>
        constexpr inline auto multiSamplingLevel(this TSelf&& self, MultiSamplingLevel samples) -> TSelf& {
            self.m_state.multiSamplingLevel = samples;
            return self;
        }

        /// <summary>
        /// Adds a render target to the render pass by assigning it an incremental location number.
        /// </summary>
        /// <param name="type">The type of the render target.</param>
        /// <param name="format">The color format of the render target.</param>
        /// <param name="clearValues">The fixed clear value for the render target.</param>
        /// <param name="clearColor"><c>true</c>, if the render target color or depth should be cleared.</param>
        /// <param name="clearStencil"><c>true</c>, if the render target stencil should be cleared.</param>
        /// <param name="isVolatile"><c>true</c> to mark the render target as volatile, so is not required to be preserved after the render pass has ended.</param>
        template <typename TSelf>
        constexpr inline auto renderTarget(this TSelf&& self, RenderTargetType type, Format format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) -> TSelf& {
            self.renderTarget("", static_cast<UInt32>(self.m_state.renderTargets.size()), type, format, clearValues, clearColor, clearStencil, isVolatile);
            return self;
        }

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
        template <typename TSelf>
        constexpr inline auto renderTarget(this TSelf&& self, const String& name, RenderTargetType type, Format format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) -> TSelf& {
            self.renderTarget(name, static_cast<UInt32>(self.m_state.renderTargets.size()), type, format, clearValues, clearColor, clearStencil, isVolatile);
            return self;
        }

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
        template <typename TSelf>
        constexpr inline auto renderTarget(this TSelf&& self, UInt32 location, RenderTargetType type, Format format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) -> TSelf& {
            self.renderTarget("", location, type, format, clearValues, clearColor, clearStencil, isVolatile);
            return self;
        }

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
        template <typename TSelf>
        constexpr inline auto renderTarget(this TSelf&& self, const String& name, UInt32 location, RenderTargetType type, Format format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) -> TSelf& {
            self.m_state.renderTargets.push_back(RenderTarget(name, location, type, format, clearColor, clearValues, clearStencil, isVolatile));
            return self;
        }

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
        template <typename TSelf>
        constexpr inline auto renderTarget(this TSelf&& self, input_attachment_mapping_type& output, RenderTargetType type, Format format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) -> TSelf& {
            self.renderTarget("", output, static_cast<UInt32>(self.m_state.m_renderTargets.size()), type, format, clearValues, clearColor, clearStencil, isVolatile);
            return self;
        }

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
        template <typename TSelf>
        constexpr inline auto renderTarget(this TSelf&& self, const String& name, input_attachment_mapping_type& output, RenderTargetType type, Format format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) -> TSelf& {
            self.renderTarget(name, output, static_cast<UInt32>(self.m_state.renderTargets.size()), type, format, clearValues, clearColor, clearStencil, isVolatile);
            return self;
        }

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
        template <typename TSelf>
        constexpr inline auto renderTarget(this TSelf&& self, input_attachment_mapping_type& output, UInt32 location, RenderTargetType type, Format format, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }, bool clearColor = true, bool clearStencil = true, bool isVolatile = false) -> TSelf& {
            self.renderTarget("", output, location, type, format, clearValues, clearColor, clearStencil, isVolatile);
            return self;
        }

        /// <summary>
        /// Adds an input attachment to the render pass.
        /// </summary>
        /// <param name="inputAttachment">The input attachment to add.</param>
        template <typename TSelf>
        constexpr inline auto inputAttachment(this TSelf&& self, const input_attachment_mapping_type& inputAttachment) -> TSelf& {
            self.m_state.inputAttachments.push_back(inputAttachment);
            return self;
        }

        /// <summary>
        /// Adds an input attachment to the render pass.
        /// </summary>
        /// <param name="inputLocation">The location from which the input attachment gets accessed.</param>
        /// <param name="renderPass">The render pass, the input attachment is created from.</param>
        /// <param name="outputLocation">The location to which the input attachment is written by <paramref name="renderPass" />.</param>
        template <typename TSelf>
        constexpr inline auto inputAttachment(this TSelf&& self, UInt32 inputLocation, const render_pass_type& renderPass, UInt32 outputLocation) -> TSelf& {
            self.inputAttachment(static_cast<RenderPassBuilder&>(self).makeInputAttachment(inputLocation, renderPass, renderPass.renderTarget(outputLocation)));
            return self;
        }

        /// <summary>
        /// Adds an input attachment to the render pass.
        /// </summary>
        /// <param name="inputLocation">The location from which the input attachment gets accessed.</param>
        /// <param name="renderPass">The render pass, the input attachment is created from.</param>
        /// <param name="renderTarget">The render target that is bound as input attachment.</param>
        template <typename TSelf>
        constexpr inline auto inputAttachment(this TSelf&& self, UInt32 inputLocation, const render_pass_type& renderPass, RenderTarget renderTarget) -> TSelf& {
            self.inputAttachment(static_cast<RenderPassBuilder&>(self).makeInputAttachment(inputLocation, renderPass, renderTarget));
            return self;
        }
    };

}
#endif // defined(BUILD_DEFINE_BUILDERS)