#pragma once

#include "rendering_api.hpp"
#include "rendering.hpp"

#if defined(LITEFX_BUILD_DEFINE_BUILDERS)
namespace LiteFX::Rendering {

    /// @brief Base class for a builder that builds a @ref Barrier.
    ///
    /// @tparam TBarrier The type of the barrier. Must implement @ref Barrier.
    /// @see Barrier
    /// @see IBarrier
    template <typename TBarrier> requires
        meta::implements<TBarrier, Barrier<typename TBarrier::buffer_type, typename TBarrier::image_type>>
    class BarrierBuilder : public Builder<TBarrier> {
    public:
        template <typename TParent> requires
            meta::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] ImageBarrierBuilder;

        /// @brief A builder that sets up the pipeline stages to wait for and to continue with on a barrier.
        ///
        /// @tparam TParent The type of the parent barrier builder.
        template <typename TParent> requires
            meta::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] SecondStageBuilder {
        private:
            TParent m_parent;
            PipelineStage m_from;

            /// @brief Initializes a builder that sets up barrier stages.
            ///
            /// @param parent The parent builder instance.
            /// @param waitFor The pipeline stage to wait for.
            constexpr SecondStageBuilder(TParent&& parent, PipelineStage waitFor) noexcept :
                m_parent(std::move(parent)), m_from(waitFor) { }

        public:
            friend class BarrierBuilder;

            /// @brief Specifies the pipeline stage that are allowed to continue after the barrier has executed.
            ///
            /// @param stage The pipeline stage that are allowed to continue after the barrier has executed.
            /// @return The instance of the parent builder.
            constexpr auto toContinueWith(PipelineStage stage) -> TParent {
                this->m_parent.stagesCallback(this->m_from, stage);
                return std::move(this->m_parent);
            }
        };

        /// @brief A builder that sets up a global resource barrier.
        ///
        /// @tparam TParent The type of the parent barrier builder.
        template <typename TParent> requires
            meta::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] GlobalBarrierBuilder {
        private:
            ResourceAccess m_access;
            TParent m_parent;

            /// @brief Initializes a builder that sets up a global resource barrier.
            ///
            /// @param parent The parent builder instance.
            /// @param access The resource access state of all resources to wait for with this barrier.
            constexpr GlobalBarrierBuilder(TParent&& parent, ResourceAccess access) noexcept :
                m_parent(std::move(parent)), m_access(access) { }

        public:
            friend class BarrierBuilder;

            /// @brief Specifies the resource accesses that are waited for in a global barrier before it can be executed.
            ///
            /// @param access The resource accesses that are waited for until the barrier can be executed.
            constexpr auto untilFinishedWith(ResourceAccess access) -> TParent {
                this->m_parent.globalBarrierCallback(access, m_access);
                return std::move(this->m_parent);
            }
        };

        /// @brief A builder that sets up a resource barrier for a specific buffer.
        ///
        /// @tparam TParent The type of the parent barrier builder.
        template <typename TParent> requires
            meta::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] BufferBarrierBuilder {
        private:
            ResourceAccess m_access;
            SharedPtr<IBuffer> m_buffer;
            TParent m_parent;

            /// @brief Initializes a builder that sets up a barrier for a specific buffer.
            ///
            /// @param parent The parent builder instance.
            /// @param buffer The buffer for this barrier.
            /// @param access The resource access state of the buffer to wait for with this barrier.
            constexpr BufferBarrierBuilder(TParent&& parent, IBuffer& buffer, ResourceAccess access) :
                m_parent(std::move(parent)), m_buffer(buffer.shared_from_this()), m_access(access) { }

        public:
            friend class BarrierBuilder;

            /// @brief Specifies the resource accesses that are waited for in a buffer before the barrier can be executed.
            ///
            /// @param access The resource accesses that are waited for in a buffer before the barrier can be executed.
            constexpr auto untilFinishedWith(ResourceAccess access) -> TParent {
                this->m_parent.bufferBarrierCallback(*m_buffer, access, m_access);
                return std::move(this->m_parent);
            }
        };

        /// @brief A builder that sets up the layout transition barrier for a set of sub-resources of a specific image.
        ///
        /// @tparam TParent The type of the parent barrier builder.
        template <typename TParent> requires
            meta::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] ImageLayoutBarrierBuilder {
        private:
            TParent m_parent;
            ResourceAccess m_access;
            SharedPtr<IImage> m_image;
            ImageLayout m_layout;
            UInt32 m_level{ 0 }, m_levels{ 1 }, m_layer{ 0 }, m_layers{ 1 }, m_plane{ 0 };

            /// @brief Initializes a builder that sets up the layout transition barrier for a set of sub-resources of a specific image.
            ///
            /// @param parent The parent builder instance.
            /// @param image The image for this barrier.
            /// @param access The resource access state of the sub-resources in the image to wait for with this barrier.
            /// @param layout The layout to transition the image sub-resources into.
            /// @param level The level of the first sub-resource to transition.
            /// @param levels The number of levels to transition.
            /// @param layer The layer of the first sub-resource to transition.
            /// @param layers The number of layers to transition.
            /// @param plane The plane of the sub-resource to transition.
            constexpr ImageLayoutBarrierBuilder(TParent&& parent, IImage& image, ResourceAccess access, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) :
                m_parent(std::move(parent)), m_access(access), m_image(image.shared_from_this()), m_layout(layout), m_level(level), m_levels(levels), m_layer(layer), m_layers(layers), m_plane(plane) { }

        public:
            friend class BarrierBuilder;
            friend struct ImageBarrierBuilder<TParent>;

            /// @brief Specifies the resource accesses that are waited for on the image sub-resources before the barrier can be executed.
            ///
            /// @param access The resource accesses that are waited for on the image sub-resources before the barrier can be executed.
            constexpr auto whenFinishedWith(ResourceAccess access) -> TParent {
                this->m_parent.imageBarrierCallback(*m_image, access, m_access, m_layout, m_level, m_levels, m_layer, m_layers, m_plane);
                return std::move(this->m_parent);
            }
        };

        /// @brief A builder that sets up a resource barrier for a specific image.
        ///
        /// @tparam TParent The type of the parent barrier builder.
        template <typename TParent> requires
            meta::implements<TParent, BarrierBuilder<TBarrier>>
        struct [[nodiscard]] ImageBarrierBuilder {
        private:
            TParent m_parent;
            ResourceAccess m_access;
            SharedPtr<IImage> m_image;
            UInt32 m_level{ 0 }, m_levels{ 0 }, m_layer{ 0 }, m_layers{ 0 }, m_plane{ 0 };

            /// @brief Initializes a builder that sets up a resource barrier for a specific image.
            ///
            /// @param parent The parent builder instance.
            /// @param image The image for this barrier.
            /// @param access The resource access state of the sub-resources in the image to wait for with this barrier.
            constexpr ImageBarrierBuilder(TParent&& parent, IImage& image, ResourceAccess access) :
                m_parent(std::move(parent)), m_access(access), m_image(image.shared_from_this()) { }

        public:
            friend class BarrierBuilder;

            /// @brief Specifies the layout to transition an image to when executing the barrier.
            ///
            /// @param layout The layout to transition an image to when executing the barrier.
            constexpr auto transitionLayout(ImageLayout layout) -> ImageLayoutBarrierBuilder<TParent> {
                return ImageLayoutBarrierBuilder<TParent>{ std::move(m_parent), *m_image, m_access, layout, m_level, m_levels, m_layer, m_layers, m_plane };
            }

            /// @brief Specifies the sub-resource to block and transition when executing the barrier.
            ///
            /// @param level The base level of the sub-resource.
            /// @param levels The number of levels to block and transition.
            /// @param layer The base layer of the sub-resource.
            /// @param layers The number of layers to block and transition.
            /// @param plane The plane index of the sub-resource to block and transition.
            constexpr auto subresource(UInt32 level, UInt32 levels, UInt32 layer = 0, UInt32 layers = 1, UInt32 plane = 0) -> ImageBarrierBuilder<TParent>& {
                m_level = level;
                m_levels = levels;
                m_layer = layer;
                m_layers = layers;
                m_plane = plane;

                return *this;
            };
        };

    private:
        /// @brief Function that is called back from the @ref SecondStageBuilder in order to setup the barrier pipeline stages.
        ///
        /// @param waitFor The pipeline stage to wait for with the barrier.
        /// @param continueWith The pipeline stage to allow continuation with the current barrier.
        constexpr void stagesCallback(PipelineStage waitFor, PipelineStage continueWith) {
            this->setupStages(waitFor, continueWith);
        }

        /// @brief Function that is called back from the @ref GlobalBarrierBuilder in order to setup the resource access states for a global resource barrier.
        ///
        /// @param before The resource access state of all resources to wait for with this barrier.
        /// @param after The resource access state of all resources to continue with after this barrier.
        constexpr void globalBarrierCallback(ResourceAccess before, ResourceAccess after) {
            this->setupGlobalBarrier(before, after);
        }

        /// @brief Function that is called back from the @ref BufferBarrierBuilder in order to setup the resource access states for a buffer resource barrier.
        ///
        /// @param buffer The buffer for which the barrier blocks.
        /// @param before The resource access state of the buffer to wait for with this barrier.
        /// @param after The resource access state of the buffer to continue with after this barrier.
        constexpr void bufferBarrierCallback(IBuffer& buffer, ResourceAccess before, ResourceAccess after) {
            this->setupBufferBarrier(buffer, before, after);
        }

        /// @brief Function that is called back from the @ref ImageLayoutBarrierBuilder in order to setup the layout transition and resource access states for a set if sub-resources of a specific image.
        ///
        /// @param image The image for this barrier.
        /// @param before The resource access state of the sub-resources in the image to wait for with this barrier.
        /// @param after The resource access state of the sub-resources in the image to continue with after this barrier.
        /// @param layout The layout to transition the image sub-resources into.
        /// @param level The level of the first sub-resource to transition.
        /// @param levels The number of levels to transition.
        /// @param layer The layer of the first sub-resource to transition.
        /// @param layers The number of layers to transition.
        /// @param plane The plane of the sub-resource to transition.
        constexpr void imageBarrierCallback(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) {
            this->setupImageBarrier(image, before, after, layout, level, levels, layer, layers, plane);
        }

    protected:
        /// @brief Sets the pipeline stages for the built barrier to wait for and to continue with.
        ///
        /// @param waitFor The pipeline stage to wait for with the barrier.
        /// @param continueWith The pipeline stage to allow continuation with the current barrier.
        constexpr virtual void setupStages(PipelineStage waitFor, PipelineStage continueWith) = 0;

        /// @brief Sets up the resource access states to wait for and to continue with the barrier to be built.
        ///
        /// @param before The resource access state of all resources to wait for with this barrier.
        /// @param after The resource access state of all resources to continue with after this barrier.
        constexpr virtual void setupGlobalBarrier(ResourceAccess before, ResourceAccess after) = 0;

        /// @brief Sets up the resource access states to wait for and to continue with for a specific buffer with the barrier to be built.
        ///
        /// @param buffer The buffer for which the barrier blocks.
        /// @param before The resource access state of all resources to wait for with this barrier.
        /// @param after The resource access state of all resources to continue with after this barrier.
        constexpr virtual void setupBufferBarrier(IBuffer& buffer, ResourceAccess before, ResourceAccess after) = 0;

        /// @brief Sets up the image layout transition and resource access states to wait for and continue with the barrier to be built.
        ///
        /// @param image The image for this barrier.
        /// @param before The resource access state of the sub-resources in the image to wait for with this barrier.
        /// @param after The resource access state of the sub-resources in the image to continue with after this barrier.
        /// @param layout The layout to transition the image sub-resources into.
        /// @param level The level of the first sub-resource to transition.
        /// @param levels The number of levels to transition.
        /// @param layer The layer of the first sub-resource to transition.
        /// @param layers The number of layers to transition.
        /// @param plane The plane of the sub-resource to transition.
        constexpr virtual void setupImageBarrier(IImage& image, ResourceAccess before, ResourceAccess after, ImageLayout layout, UInt32 level, UInt32 levels, UInt32 layer, UInt32 layers, UInt32 plane) = 0;

    public:
        using Builder<TBarrier>::Builder;
        using barrier_type = TBarrier;

    public:
        /// @brief Specifies the pipeline stages to wait for before executing the barrier.
        ///
        /// @param stage The pipeline stages to wait for before executing the barrier.
        template <typename TSelf>
        [[nodiscard]] constexpr auto waitFor(this TSelf&& self, PipelineStage stage) -> SecondStageBuilder<TSelf> {
            return SecondStageBuilder<TSelf>{ std::forward<TSelf>(self), stage };
        }

        /// @brief Specifies the resource accesses that are blocked in a global barrier until the barrier has executed.
        ///
        /// @param access The resource accesses that are blocked until the barrier has executed.
        template <typename TSelf>
        [[nodiscard]] constexpr auto blockAccessTo(this TSelf&& self, ResourceAccess access) -> GlobalBarrierBuilder<TSelf> {
            return GlobalBarrierBuilder<TSelf>{ std::forward<TSelf>(self), access };
        }

        /// @brief Specifies the resource accesses that are blocked for @p buffer until the barrier has executed.
        ///
        /// @param buffer The buffer to wait for.
        /// @param access The resource accesses that are blocked until the barrier has executed.
        template <typename TSelf>
        [[nodiscard]] constexpr auto blockAccessTo(this TSelf&& self, IBuffer& buffer, ResourceAccess access) -> BufferBarrierBuilder<TSelf> {
            return BufferBarrierBuilder<TSelf>{ std::forward<TSelf>(self), buffer, access };
        }

        /// @brief Specifies the resource accesses that are blocked for @p buffer until the barrier has executed.
        ///
        /// @param buffer The buffer to wait for.
        /// @param subresource The sub-resource to block.
        /// @param access The resource accesses that are blocked until the barrier has executed.
        template <typename TSelf>
        [[nodiscard]] constexpr auto blockAccessTo(this TSelf&& self, IBuffer& buffer, UInt32 subresource, ResourceAccess access) -> BufferBarrierBuilder<TSelf> {
            return BufferBarrierBuilder<TSelf>{ std::forward<TSelf>(self), buffer, subresource, access };
        }

        /// @brief Specifies the resource accesses that are blocked for @p image until the barrier has executed.
        ///
        /// @param image The buffer to wait for.
        /// @param access The resource accesses that are blocked until the barrier has executed.
        template <typename TSelf>
        [[nodiscard]] constexpr auto blockAccessTo(this TSelf&& self, IImage& image, ResourceAccess access) -> ImageBarrierBuilder<TSelf> {
            return ImageBarrierBuilder<TSelf>{ std::forward<TSelf>(self), image, access };
        }
    };

    /// @brief Base class for a builder that builds a @ref ShaderProgram.
    ///
    /// @tparam TShaderProgram The type of the shader program. Must implement @ref ShaderProgram.
    /// @see ShaderProgram
    template <typename TShaderProgram> requires
        meta::implements<TShaderProgram, ShaderProgram<typename TShaderProgram::shader_module_type>>
    class ShaderProgramBuilder : public Builder<TShaderProgram, std::nullptr_t, SharedPtr<TShaderProgram>> {
    public:
        using Builder<TShaderProgram, std::nullptr_t, SharedPtr<TShaderProgram>>::Builder;
        using shader_program_type = TShaderProgram;
        using shader_module_type = shader_program_type::shader_module_type;

    private:
        /// @brief Stores the shader program state while building.
        struct ShaderProgramState {
            /// @brief The shader modules of the program.
            Array<UniquePtr<shader_module_type>> modules{ };
        } m_state;

    protected:
        /// @brief Returns the current state of the shader program.
        ///
        /// @return A reference of the current state of the shader program.
        inline ShaderProgramState& state() noexcept {
            return m_state;
        }

        /// @brief Called to create a new shader module in the program that is stored in a file.
        ///
        /// @param type The type of the shader module.
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data for ray-tracing shaders.
        /// @return The shader module instance.
        constexpr virtual UniquePtr<shader_module_type> makeShaderModule(ShaderStage type, const String& fileName, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) = 0;

        /// @brief Called to create a new shader module in the program that is loaded from a stream.
        ///
        /// @param type The type of the shader module.
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data for ray-tracing shaders.
        /// @return The shader module instance.
        constexpr virtual UniquePtr<shader_module_type> makeShaderModule(ShaderStage type, std::istream& stream, const String& name, const String& entryPoint, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor) = 0;

    public:
        /// @brief Adds a shader module to the program.
        ///
        /// @param type The type of the shader module.
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data for ray-tracing shaders.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withShaderModule(this TSelf&& self, ShaderStage type, const String& fileName, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt) -> TSelf&& {
            self.m_state.modules.push_back(std::move(static_cast<ShaderProgramBuilder&>(self).makeShaderModule(type, fileName, entryPoint, shaderLocalDescriptor)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a shader module to the program.
        ///
        /// @param type The type of the shader module.
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data for ray-tracing shaders.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withShaderModule(this TSelf&& self, ShaderStage type, std::istream& stream, const String& name, const String& entryPoint = "main", const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt) -> TSelf&& {
            self.m_state.modules.push_back(std::move(static_cast<ShaderProgramBuilder&>(self).makeShaderModule(type, stream, name, entryPoint, shaderLocalDescriptor)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a vertex shader module to the program.
        ///
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withVertexShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Vertex, fileName, entryPoint);
        }

        /// @brief Adds a vertex shader module to the program.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withVertexShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Vertex, stream, name, entryPoint);
        }

        /// @brief Adds a task shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::MeshShaders feature is enabled.
        ///
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withTaskShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Task, fileName, entryPoint);
        }
        /// @brief Adds a task shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::MeshShaders feature is enabled.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withTaskShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Task, stream, name, entryPoint);
        }

        /// @brief Adds a mesh shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::MeshShaders feature is enabled.
        ///
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withMeshShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Mesh, fileName, entryPoint);
        }

        /// @brief Adds a mesh shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::MeshShaders feature is enabled.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withMeshShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Mesh, stream, name, entryPoint);
        }

        /// @brief Adds a tessellation control shader module to the program.
        ///
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withTessellationControlShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::TessellationControl, fileName, entryPoint);
        }

        /// @brief Adds a tessellation control shader module to the program.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withTessellationControlShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::TessellationControl, stream, name, entryPoint);
        }

        /// @brief Adds a tessellation evaluation shader module to the program.
        ///
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withTessellationEvaluationShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::TessellationEvaluation, fileName, entryPoint);
        }

        /// @brief Adds a tessellation evaluation shader module to the program.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withTessellationEvaluationShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::TessellationEvaluation, stream, name, entryPoint);
        }

        /// @brief Adds a geometry shader module to the program.
        ///
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withGeometryShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Geometry, fileName, entryPoint);
        }

        /// @brief Adds a geometry shader module to the program.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withGeometryShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Geometry, stream, name, entryPoint);
        }

        /// @brief Adds a fragment shader module to the program.
        ///
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withFragmentShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Fragment, fileName, entryPoint);
        }

        /// @brief Adds a fragment shader module to the program.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withFragmentShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Fragment, stream, name, entryPoint);
        }

        /// @brief Adds a compute shader module to the program.
        ///
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withComputeShaderModule(this TSelf&& self, const String& fileName, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Compute, fileName, entryPoint);
        }

        /// @brief Adds a compute shader module to the program.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withComputeShaderModule(this TSelf&& self, std::istream& stream, const String& name, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Compute, stream, name, entryPoint);
        }

        /// @brief Adds a ray generation shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param fileName The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withRayGenerationShaderModule(this TSelf&& self, const String& fileName, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::RayGeneration, fileName, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds a ray generation shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withRayGenerationShaderModule(this TSelf&& self, std::istream& stream, const String& name, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::RayGeneration, stream, name, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds a miss shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param fileName The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withMissShaderModule(this TSelf&& self, const String& fileName, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Miss, fileName, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds a miss shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withMissShaderModule(this TSelf&& self, std::istream& stream, const String& name, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Miss, stream, name, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds a callable shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param fileName The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withCallableShaderModule(this TSelf&& self, const String& fileName, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Callable, fileName, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds a callable shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withCallableShaderModule(this TSelf&& self, std::istream& stream, const String& name, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Callable, stream, name, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds an intersection hit shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param fileName The file name of the module.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withIntersectionShaderModule(this TSelf&& self, const String& fileName, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Intersection, fileName, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds an intersection hit shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withIntersectionShaderModule(this TSelf&& self, std::istream& stream, const String& name, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::Intersection, stream, name, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds an any hit shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param fileName The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withAnyHitShaderModule(this TSelf&& self, const String& fileName, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::AnyHit, fileName, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds an any hit shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withAnyHitShaderModule(this TSelf&& self, std::istream& stream, const String& name, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::AnyHit, stream, name, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds a closest hit shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param fileName The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withClosestHitShaderModule(this TSelf&& self, const String& fileName, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::ClosestHit, fileName, entryPoint, shaderLocalDescriptor);
        }

        /// @brief Adds a closest hit shader module to the program.
        ///
        /// This method is only supported if the @ref GraphicsDeviceFeature::RayTracing feature is enabled.
        ///
        /// @param stream The file stream of the module.
        /// @param name The file name of the module.
        /// @param shaderLocalDescriptor The descriptor that binds shader-local data.
        /// @param entryPoint The name of the entry point for the module.
        template<typename TSelf>
        [[nodiscard]] constexpr auto withClosestHitShaderModule(this TSelf&& self, std::istream& stream, const String& name, const Optional<DescriptorBindingPoint>& shaderLocalDescriptor = std::nullopt, const String& entryPoint = "main") -> TSelf&& {
            return std::forward<TSelf>(self).withShaderModule(ShaderStage::ClosestHit, stream, name, entryPoint, shaderLocalDescriptor);
        }
    };

    /// @brief Builds a @ref Rasterizer.
    ///
    /// @tparam TRasterizer The type of the rasterizer. Must implement @ref IRasterizer.
    /// @see IRasterizer
    template <typename TRasterizer> requires
        meta::implements<TRasterizer, IRasterizer>
    class RasterizerBuilder : public Builder<TRasterizer, std::nullptr_t, SharedPtr<TRasterizer>> {
    public:
        using Builder<TRasterizer, std::nullptr_t, SharedPtr<TRasterizer>>::Builder;
        using rasterizer_type = TRasterizer;

    private:
        /// @brief Stores the rasterizer state while building.
        struct RasterizerState {
            /// @brief The polygon draw mode.
            PolygonMode polygonMode{ PolygonMode::Solid };

            /// @brief The polygon cull mode.
            CullMode cullMode{ CullMode::BackFaces };

            /// @brief The polygon cull order.
            CullOrder cullOrder{ CullOrder::ClockWise };

            /// @brief The line width, if line rasterization is supported.
            Float lineWidth{ 1.0f };

            /// @brief The depth clip setting for the rasterizer state.
            bool depthClip{ true };

            /// @brief The depth bias state.
            DepthStencilState::DepthBias depthBias{ };

            /// @brief The depth state.
            DepthStencilState::DepthState depthState{ };

            /// @brief The stencil state.
            DepthStencilState::StencilState stencilState{ };

            /// @brief Toggles conservative rasterization in the rasterizer.
            bool conservativeRasterization{ false };
        } m_state;

    protected:
        /// @brief Returns the current rasterizer state.
        ///
        /// @return A reference of the current rasterizer state.
        inline RasterizerState& state() noexcept {
            return m_state;
        }

    public:
        /// @brief Initializes the rasterizer state with the provided polygon mode.
        ///
        /// @param mode The polygon mode to initialize the rasterizer state with.
        template<typename TSelf>
        [[nodiscard]] constexpr auto polygonMode(this TSelf&& self, PolygonMode mode) noexcept -> TSelf&& {
            self.m_state.polygonMode = mode;
            return std::forward<TSelf>(self);
        }

        /// @brief Initializes the rasterizer state with the provided cull mode.
        ///
        /// @param mode The cull mode to initialize the rasterizer state with.
        template<typename TSelf>
        [[nodiscard]] constexpr auto cullMode(this TSelf&& self, CullMode mode) noexcept -> TSelf&& {
            self.m_state.cullMode = mode;
            return std::forward<TSelf>(self);
        }

        /// @brief Initializes the rasterizer state with the provided cull order.
        ///
        /// @param order The cull order to initialize the rasterizer state with.
        template<typename TSelf>
        [[nodiscard]] constexpr auto cullOrder(this TSelf&& self, CullOrder order) noexcept -> TSelf&& {
            self.m_state.cullOrder = order;
            return std::forward<TSelf>(self);
        }

        /// @brief Initializes the rasterizer state with the provided line width.
        ///
        /// @param width The line width to initialize the rasterizer state with.
        template<typename TSelf>
        [[nodiscard]] constexpr auto lineWidth(this TSelf&& self, Float width) noexcept -> TSelf&& {
            self.m_state.lineWidth = width;
            return std::forward<TSelf>(self);
        }

        /// @brief Initializes the depth clip toggle for the rasterizer state.
        ///
        /// @param depthClip The depth clip toggle for the rasterizer state.
        template<typename TSelf>
        [[nodiscard]] constexpr auto depthClip(this TSelf&& self, bool depthClip) noexcept -> TSelf&& {
            self.m_state.depthClip = depthClip;
            return std::forward<TSelf>(self);
        }

        /// @brief Initializes the rasterizer state with conservative rasterization.
        ///
        /// @param enable `true`, if the rasterizer should use conservative rasterization and `false` otherwise.
        template<typename TSelf>
        [[nodiscard]] constexpr auto conservativeRasterization(this TSelf&& self, bool enable) noexcept -> TSelf&& {
            self.m_state.conservativeRasterization = enable;
            return std::forward<TSelf>(self);
        }

        /// @brief Initializes the rasterizer depth bias.
        ///
        /// @param depthBias The depth bias the rasterizer should use.
        template<typename TSelf>
        [[nodiscard]] constexpr auto depthBias(this TSelf&& self, const DepthStencilState::DepthBias& depthBias) noexcept -> TSelf&& {
            self.m_state.depthBias = depthBias;
            return std::forward<TSelf>(self);
        }

        /// @brief Initializes the rasterizer depth state.
        ///
        /// @param depthState The depth state of the rasterizer.
        template<typename TSelf>
        [[nodiscard]] constexpr auto depthState(this TSelf&& self, const DepthStencilState::DepthState& depthState) noexcept -> TSelf&& {
            self.m_state.depthState = depthState;
            return std::forward<TSelf>(self);
        }

        /// @brief Initializes the rasterizer stencil state.
        ///
        /// @param stencilState The stencil state of the rasterizer.
        template<typename TSelf>
        [[nodiscard]] constexpr auto stencilState(this TSelf&& self, const DepthStencilState::StencilState& stencilState) noexcept -> TSelf&& {
            self.m_state.stencilState = stencilState;
            return std::forward<TSelf>(self);
        }
    };

    /// @brief Builds a @ref VertexBufferLayout.
    ///
    /// @tparam TVertexBufferLayout The type of the vertex buffer layout. Must implement @ref IVertexBufferLayout.
    /// @see IVertexBufferLayout
    template <typename TVertexBufferLayout, typename TParent> requires
        meta::implements<TVertexBufferLayout, IVertexBufferLayout>
    class VertexBufferLayoutBuilder : public Builder<TVertexBufferLayout, TParent, SharedPtr<TVertexBufferLayout>> {
    public:
        using Builder<TVertexBufferLayout, TParent, SharedPtr<TVertexBufferLayout>>::Builder;
        using vertex_buffer_layout_type = TVertexBufferLayout;

    private:
        /// @brief Stores the vertex buffer layout state while building.
        struct VertexBufferLayoutState {
            /// @brief The vertex buffer attributes of the layout.
            Array<BufferAttribute> attributes{ };

            /// @brief The vertex buffer input rate of the layout.
            VertexBufferInputRate inputRate{ VertexBufferInputRate::Vertex };
        } m_state;

    protected:
        /// @brief Returns the current vertex buffer layout state.
        ///
        /// @return A reference of the current vertex buffer layout state.
        inline VertexBufferLayoutState& state() noexcept {
            return m_state;
        }

    public:
        /// @brief Adds an attribute to the vertex buffer layout.
        ///
        /// @param attribute The attribute to add to the layout.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withAttribute(this TSelf&& self, BufferAttribute&& attribute) -> TSelf&& {
            self.m_state.attributes.push_back(std::move(attribute));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an attribute to the vertex buffer layout.
        ///
        /// This overload implicitly determines the location based on the number of attributes already defined. It should only be used if all locations can be implicitly deducted.
        ///
        /// @param format The format of the attribute.
        /// @param offset The offset of the attribute within a buffer element.
        /// @param semantic The semantic of the attribute.
        /// @param semanticIndex The semantic index of the attribute.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withAttribute(this TSelf&& self, BufferFormat format, UInt32 offset, AttributeSemantic semantic = AttributeSemantic::Arbitrary, UInt32 semanticIndex = 0) -> TSelf&& {
            return std::forward<TSelf>(self).withAttribute({ static_cast<UInt32>(self.m_state.attributes.size()), offset, format, semantic, semanticIndex });
        }

        /// @brief Adds an attribute to the vertex buffer layout.
        ///
        /// @param location The location, the attribute is bound to.
        /// @param format The format of the attribute.
        /// @param offset The offset of the attribute within a buffer element.
        /// @param semantic The semantic of the attribute.
        /// @param semanticIndex The semantic index of the attribute.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withAttribute(this TSelf&& self, UInt32 location, BufferFormat format, UInt32 offset, AttributeSemantic semantic = AttributeSemantic::Arbitrary, UInt32 semanticIndex = 0) -> TSelf&& {
            return std::forward<TSelf>(self).withAttribute({ location, offset, format, semantic, semanticIndex });
        }

        /// @brief Specifies the input rate for the vertex buffer layout.
        ///
        /// @param inputRate The rate at which data of the vertex buffer is made available to the vertex shader.
        template <typename TSelf>
        [[nodiscard]] constexpr auto atRate(this TSelf&& self, VertexBufferInputRate inputRate) -> TSelf&& {
            self.m_state.inputRate = inputRate;
            return std::forward<TSelf>(self);
        }
    };

    /// @brief Builds a @ref DescriptorSetLayout for a @ref PipelineLayout.
    ///
    /// @tparam TDescriptorSetLayout The type of the descriptor set layout. Must implement @ref DescriptorSetLayout.
    /// @see DescriptorSetLayout
    /// @see PipelineLayout
    template <typename TDescriptorSetLayout, typename TParent> requires
        meta::implements<TDescriptorSetLayout, DescriptorSetLayout<typename TDescriptorSetLayout::descriptor_layout_type, typename TDescriptorSetLayout::descriptor_set_type>>
    class DescriptorSetLayoutBuilder : public Builder<TDescriptorSetLayout, TParent, SharedPtr<TDescriptorSetLayout>> {
    public:
        using Builder<TDescriptorSetLayout, TParent, SharedPtr<TDescriptorSetLayout>>::Builder;
        using descriptor_set_layout_type = TDescriptorSetLayout;
        using descriptor_layout_type = descriptor_set_layout_type::descriptor_layout_type;
        using descriptor_set_type = descriptor_set_layout_type::descriptor_set_type;

    private:
        /// @brief Stores the descriptor set layout state while building.
        struct DescriptorSetLayoutState {
            /// @brief The space of the descriptor set.
            UInt32 space{};
            
            /// @brief The shader stages, the descriptor set is accessible from.
            ShaderStage stages{ ShaderStage::Other };

            /// @brief The layouts of the descriptors within the descriptor set.
            Array<descriptor_layout_type> descriptorLayouts{};
        } m_state;

    protected:
        /// @brief Returns the current descriptor set layout state.
        ///
        /// @return A reference of the current descriptor set layout state.
        inline DescriptorSetLayoutState& state() noexcept {
            return m_state;
        }

        /// @brief Creates a descriptor to the descriptor set layout.
        ///
        /// @param type The type of the descriptor.
        /// @param binding The binding point for the descriptor.
        /// @param descriptorSize The size of a single descriptor.
        /// @param descriptors The number of descriptors to bind.
        /// @param unbounded `true` if the descriptor should define an unbounded array and `false` otherwise.
        /// @return The descriptor layout instance.
        constexpr virtual descriptor_layout_type makeDescriptor(DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors, bool unbounded) = 0;

        /// @brief Creates a static sampler for the descriptor bound to @ref binding.
        ///
        /// @param binding The binding point for the descriptor.
        /// @param magFilter The magnifying filter operation.
        /// @param minFilter The minifying filter operation.
        /// @param borderU The border address mode into U direction.
        /// @param borderV The border address mode into V direction.
        /// @param borderW The border address mode into W direction.
        /// @param mipMapMode The mip map filter operation.
        /// @param mipMapBias The mip map bias.
        /// @param minLod The closest mip map distance level.
        /// @param maxLod The furthest mip map distance level.
        /// @param anisotropy The maximum anisotropy.
        /// @return The descriptor layout instance for the static sampler.
        constexpr virtual descriptor_layout_type makeDescriptor(UInt32 binding, FilterMode magFilter, FilterMode minFilter, BorderMode borderU, BorderMode borderV, BorderMode borderW, MipMapMode mipMapMode, Float mipMapBias, Float minLod, Float maxLod, Float anisotropy) = 0;

    public:
        /// @brief Adds a descriptor to the descriptor set layout.
        ///
        /// @param layout The descriptor layout to add.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withDescriptor(this TSelf&& self, descriptor_layout_type&& layout) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(layout));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a descriptor to the descriptor set layout.
        ///
        /// @param type The type of the descriptor.
        /// @param binding The binding point for the descriptor.
        /// @param descriptorSize The size of a single descriptor.
        /// @param descriptors The number of descriptors to bind.
        /// @param unbounded `true` if the descriptor should define an unbounded array and `false` otherwise.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withDescriptor(this TSelf&& self, DescriptorType type, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors = 1, bool unbounded = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(type, binding, descriptorSize, descriptors, unbounded)));
            return std::forward<TSelf>(self);
        }

        /// @brief Defines a static sampler at the descriptor bound to @ref binding.
        ///
        /// @param binding The binding point for the descriptor.
        /// @param magFilter The magnifying filter operation.
        /// @param minFilter The minifying filter operation.
        /// @param borderU The border address mode into U direction.
        /// @param borderV The border address mode into V direction.
        /// @param borderW The border address mode into W direction.
        /// @param mipMapMode The mip map filter operation.
        /// @param mipMapBias The mip map bias.
        /// @param minLod The closest mip map distance level.
        /// @param maxLod The furthest mip map distance level.
        /// @param anisotropy The maximum anisotropy.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withStaticSampler(this TSelf&& self, UInt32 binding, FilterMode magFilter = FilterMode::Nearest, FilterMode minFilter = FilterMode::Nearest, BorderMode borderU = BorderMode::Repeat, BorderMode borderV = BorderMode::Repeat, BorderMode borderW = BorderMode::Repeat, MipMapMode mipMapMode = MipMapMode::Nearest, Float mipMapBias = 0.f, Float minLod = 0.f, Float maxLod = std::numeric_limits<Float>::max(), Float anisotropy = 0.f) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(binding, magFilter, minFilter, borderU, borderV, borderW, mipMapMode, mipMapBias, minLod, maxLod, anisotropy)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an uniform/constant buffer descriptor.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param descriptorSize The size of a single descriptor.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withConstantBuffer(this TSelf&& self, UInt32 binding, UInt32 descriptorSize) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(DescriptorType::ConstantBuffer, binding, descriptorSize, 1u, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an uniform/constant buffer descriptor array.
        ///
        /// If the @p unbounded is set to `true`, the @p descriptors parameter defines the upper limit for the number of descriptors in the runtime array. Note that unbounded constant/uniform buffer arrays might
        /// not be supported on older hardware.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param descriptorSize The size of a single descriptor.
        /// @param descriptors The number of descriptors in the array.
        /// @param unbounded `true` if the descriptor should define an unbounded array, `false` otherwise.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withConstantBufferArray(this TSelf&& self, UInt32 binding, UInt32 descriptorSize, UInt32 descriptors, bool unbounded = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(DescriptorType::ConstantBuffer, binding, descriptorSize, descriptors, unbounded)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a texel buffer descriptor.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param writable `true`, if the buffer should be writable.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withBuffer(this TSelf&& self, UInt32 binding, bool writable = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(writable ? DescriptorType::RWBuffer : DescriptorType::Buffer, binding, 0u, 1u, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a texel buffer descriptor array.
        ///
        /// If the @p unbounded is set to `true`, the @p descriptors parameter defines the upper limit for the number of descriptors in the runtime array.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param descriptors The number of descriptors in the array.
        /// @param writable `true`, if the buffer should be writable.
        /// @param unbounded `true` if the descriptor should define an unbounded array, `false` otherwise.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withBufferArray(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1, bool writable = false, bool unbounded = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(writable ? DescriptorType::RWBuffer : DescriptorType::Buffer, binding, 0, descriptors, unbounded)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a storage/structured buffer descriptor.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param writable `true`, if the buffer should be writable.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withStructuredBuffer(this TSelf&& self, UInt32 binding, bool writable = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(writable ? DescriptorType::RWStructuredBuffer : DescriptorType::StructuredBuffer, binding, 0u, 1u, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a storage/structured buffer descriptor array.
        ///
        /// If the @p unbounded is set to `true`, the @p descriptors parameter defines the upper limit for the number of descriptors in the runtime array.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param descriptors The number of descriptors in the array.
        /// @param writable `true`, if the buffer should be writable.
        /// @param unbounded `true` if the descriptor should define an unbounded array, `false` otherwise.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withStructuredBufferArray(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1, bool writable = false, bool unbounded = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(writable ? DescriptorType::RWStructuredBuffer : DescriptorType::StructuredBuffer, binding, 0, descriptors, unbounded)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a byte address buffer descriptor.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param writable `true`, if the buffer should be writable.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withByteAddressBuffer(this TSelf&& self, UInt32 binding, bool writable = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(writable ? DescriptorType::RWByteAddressBuffer : DescriptorType::ByteAddressBuffer, binding, 0u, 1u, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a byte address buffer descriptor array.
        ///
        /// If the @p unbounded is set to `true`, the @p descriptors parameter defines the upper limit for the number of descriptors in the runtime array.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param descriptors The number of descriptors in the array.
        /// @param writable `true`, if the buffer should be writable.
        /// @param unbounded `true` if the descriptor should define an unbounded array, `false` otherwise.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withByteAddressBufferArray(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1, bool writable = false, bool unbounded = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(writable ? DescriptorType::RWByteAddressBuffer : DescriptorType::ByteAddressBuffer, binding, 0, descriptors, unbounded)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an image/texture descriptor.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param writable `true`, if the buffer should be writable.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withTexture(this TSelf&& self, UInt32 binding, bool writable = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(writable ? DescriptorType::RWTexture : DescriptorType::Texture, binding, 0u, 1u, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an image/texture descriptor.
        ///
        /// If the @p unbounded is set to `true`, the @p descriptors parameter defines the upper limit for the number of descriptors in the runtime array.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param descriptors The number of descriptors in the array.
        /// @param writable `true`, if the buffer should be writable.
        /// @param unbounded `true` if the descriptor should define an unbounded array, `false` otherwise.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withTextureArray(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1, bool writable = false, bool unbounded = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(writable ? DescriptorType::RWTexture : DescriptorType::Texture, binding, 0, descriptors, unbounded)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an input attachment descriptor.
        ///
        /// @param binding The binding point or register index of the descriptor.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withInputAttachment(this TSelf&& self, UInt32 binding) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(DescriptorType::InputAttachment, binding, 0, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a ray-tracing acceleration structure.
        ///
        /// This method is only supported, if the @ref GraphicsDeviceFeatures::RayTracing feature is enabled.
        ///
        /// @param binding The binding point or register index of the descriptor.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withAccelerationStructure(this TSelf&& self, UInt32 binding) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(DescriptorType::AccelerationStructure, binding, 0, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a sampler descriptor.
        ///
        /// @param binding The binding point or register index of the descriptor.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withSampler(this TSelf&& self, UInt32 binding) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(DescriptorType::Sampler, binding, 0u, 1u, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a sampler descriptor.
        ///
        /// If the @p unbounded is set to `true`, the @p descriptors parameter defines the upper limit for the number of descriptors in the runtime array.
        ///
        /// @param binding The binding point or register index of the descriptor.
        /// @param descriptors The number of descriptors in the array.
        /// @param unbounded `true` if the descriptor should define an unbounded array, `false` otherwise.
        template <typename TSelf>
        [[nodiscard]] constexpr auto withSamplerArray(this TSelf&& self, UInt32 binding, UInt32 descriptors = 1, bool unbounded = false) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(DescriptorType::Sampler, binding, 0, descriptors, unbounded)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a proxy descriptor that can be used with dynamic resource indexing.
        ///
        /// @param binding The binding point or register index at which to create the proxy descriptor.
        /// @param heapSize The number of descriptors to reserve for the proxy descriptor.
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        template <typename TSelf>
        [[nodiscard]] constexpr auto withResourceHeapAccess(this TSelf&& self, UInt32 binding, UInt32 heapSize) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(DescriptorType::ResourceDescriptorHeap, binding, 0u, heapSize, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a proxy descriptor that can be used with dynamic sampler indexing.
        ///
        /// @param binding The binding point or register index at which to create the proxy descriptor.
        /// @param heapSize The number of descriptors to reserve for the proxy descriptor.
        /// @see GraphicsDeviceFeature::DynamicDescriptors
        template <typename TSelf>
        [[nodiscard]] constexpr auto withSamplerHeapAccess(this TSelf&& self, UInt32 binding, UInt32 heapSize) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(static_cast<DescriptorSetLayoutBuilder&>(self).makeDescriptor(DescriptorType::SamplerDescriptorHeap, binding, 0u, heapSize, false)));
            return std::forward<TSelf>(self);
        }

        /// @brief Sets the space, the descriptor set is bound to.
        ///
        /// @param space The space, the descriptor set is bound to.
        template <typename TSelf>
        constexpr auto space(this TSelf&& self, UInt32 space) noexcept -> TSelf&& {
            self.m_state.space = space;
            return std::forward<TSelf>(self);
        }

        /// @brief Sets the shader stages, the descriptor set is accessible from.
        ///
        /// @param stages The shader stages, the descriptor set is accessible from.
        template <typename TSelf>
        constexpr auto shaderStages(this TSelf&& self, ShaderStage stages) noexcept -> TSelf&& {
            self.m_state.stages = stages;
            return std::forward<TSelf>(self);
        }

    public:
        /// @brief Adds a descriptor layout to the descriptor set.
        ///
        /// @param layout The layout of the descriptor.
        /// @see DescriptorLayout
        template <typename TSelf>
        [[nodiscard]] constexpr auto use(this TSelf&& self, UniquePtr<descriptor_layout_type>&& layout) -> TSelf&& {
            self.m_state.descriptorLayouts.push_back(std::move(layout));
            return std::forward<TSelf>(self);
        }
    };

    /// @brief Builds a @ref PushConstantsLayout for a @ref PipelineLayout.
    ///
    /// @tparam TPushConstantsLayout The type of the push constants layout. Must implement @ref PushConstantsLayout.
    /// @see PushConstantsLayout
    template <typename TPushConstantsLayout, typename TParent> requires
        meta::implements<TPushConstantsLayout, PushConstantsLayout<typename TPushConstantsLayout::push_constants_range_type>>
    class PushConstantsLayoutBuilder : public Builder<TPushConstantsLayout, TParent> {
    public:
        using Builder<TPushConstantsLayout, TParent>::Builder;
        using push_constants_layout_type = TPushConstantsLayout;
        using push_constants_range_type = push_constants_layout_type::push_constants_range_type;

    private:
        /// @brief Stores the push constants layout state while building.
        struct PushConstantsLayoutState {
            /// @brief The push constant ranges of the layout.
            Array<UniquePtr<push_constants_range_type>> ranges{ };
        } m_state;

    protected:
        /// @brief Returns the current push constants layout state.
        ///
        /// @return A reference of the current push constants layout state.
        inline PushConstantsLayoutState& state() noexcept {
            return m_state;
        }

        /// @brief Creates a new push constants range.
        ///
        /// @param shaderStages The shader stage, for which the range is defined.
        /// @param offset The offset of the range.
        /// @param size The size of the range.
        /// @param space The descriptor space, the range is bound to.
        /// @param binding The binding point for the range.
        /// @return The instance of the push constant range.
        virtual UniquePtr<push_constants_range_type> makeRange(ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) = 0;

    public:
        /// @brief Adds a new push constants range.
        ///
        /// @param shaderStages The shader stage, for which the range is defined.
        /// @param offset The offset of the range.
        /// @param size The size of the range.
        /// @param space The descriptor space, the range is bound to.
        /// @param binding The binding point for the range.
        template <typename TSelf>
        constexpr auto withRange(this TSelf&& self, ShaderStage shaderStages, UInt32 offset, UInt32 size, UInt32 space, UInt32 binding) -> TSelf&& {
            self.m_state.ranges.push_back(std::move(static_cast<PushConstantsLayoutBuilder&>(self).makeRange(shaderStages, offset, size, space, binding)));
            return std::forward<TSelf>(self);
        }
    };

    /// @brief Base class for a builder of a @ref PipelineLayout.
    ///
    /// @tparam TPipelineLayout The type of the pipeline layout. Must implement @ref PipelineLayout.
    /// @see PipelineLayout
    template <typename TPipelineLayout> requires
        meta::implements<TPipelineLayout, PipelineLayout<typename TPipelineLayout::descriptor_set_layout_type, typename TPipelineLayout::push_constants_layout_type>>
    class PipelineLayoutBuilder : public Builder<TPipelineLayout, std::nullptr_t, SharedPtr<TPipelineLayout>> {
    public:
        using Builder<TPipelineLayout, std::nullptr_t, SharedPtr<TPipelineLayout>>::Builder;
        using pipeline_layout_type = TPipelineLayout;
        using descriptor_set_layout_type = pipeline_layout_type::descriptor_set_layout_type;
        using push_constants_layout_type = pipeline_layout_type::push_constants_layout_type;

    private:
        /// @brief Stores the pipeline layout state while building.
        struct PipelineLayoutState {
            /// @brief The descriptor set layouts of the pipeline state.
            Array<SharedPtr<descriptor_set_layout_type>> descriptorSetLayouts{ };

            /// @brief The push constant layout of the pipeline state.
            UniquePtr<push_constants_layout_type> pushConstantsLayout{ };
        } m_state;

    protected:
        /// @brief Returns the current pipeline layout state.
        ///
        /// @return A reference of the current pipeline layout state.
        inline PipelineLayoutState& state() noexcept {
            return m_state;
        }

    public:
        /// @brief Adds a descriptor set to the pipeline layout.
        ///
        /// @param layout The layout of the descriptor set.
        /// @see DescriptorSetLayout
        template <typename TSelf>
        constexpr auto use(this TSelf&& self, SharedPtr<descriptor_set_layout_type>&& layout) -> TSelf&& {
            self.m_state.descriptorSetLayouts.push_back(std::move(layout));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a push constants range to the pipeline layout.
        ///
        /// @param layout The layout of the push constants range.
        /// @see PushConstantsLayout
        template <typename TSelf>
        constexpr auto use(this TSelf&& self, UniquePtr<push_constants_layout_type>&& layout) -> TSelf&& {
            self.m_state.pushConstantsLayout = std::move(layout);
            return std::forward<TSelf>(self);
        }
    };

    /// @brief Builds a @ref InputAssembler.
    ///
    /// @tparam TInputAssembler The type of the input assembler state. Must implement @ref InputAssembler.
    /// @see InputAssembler
    template <typename TInputAssembler> requires
        meta::implements<TInputAssembler, InputAssembler<typename TInputAssembler::vertex_buffer_layout_type, typename TInputAssembler::index_buffer_layout_type>>
    class InputAssemblerBuilder : public Builder<TInputAssembler, std::nullptr_t, SharedPtr<TInputAssembler>> {
    public:
        using Builder<TInputAssembler, std::nullptr_t, SharedPtr<TInputAssembler>>::Builder;
        using input_assembler_type = TInputAssembler;
        using vertex_buffer_layout_type = input_assembler_type::vertex_buffer_layout_type;
        using index_buffer_layout_type = input_assembler_type::index_buffer_layout_type;

    private:
        /// @brief Stores the input assembler state while building.
        struct InputAssemblerState {
            /// @brief The primitive topology.
            PrimitiveTopology topology{ PrimitiveTopology::PointList };

            /// @brief The vertex buffer layouts.
            Array<SharedPtr<vertex_buffer_layout_type>> vertexBufferLayouts{};
            
            /// @brief The index buffer layout.
            SharedPtr<index_buffer_layout_type> indexBufferLayout{};

            /// @brief The number of control points in a `PrimitiveType::PatchList`.
            UInt32 controlPoints{ 1u };
        } m_state;

    protected:
        /// @brief Returns the current input assembler state.
        ///
        /// @return A reference of the current input assembler state.
        inline InputAssemblerState& state() noexcept {
            return m_state;
        }

    public:
        /// @brief Specifies the topology to initialize the input assembler with.
        ///
        /// @param topology The topology to initialize the input assembler with.
        template <typename TSelf>
        constexpr auto topology(this TSelf&& self, PrimitiveTopology topology) -> TSelf&& {
            self.m_state.topology = topology;
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a vertex buffer layout to the input assembler. Can be called multiple times.
        ///
        /// @param layout The vertex buffer layout to add to the input assembler.
        template <typename TSelf>
        constexpr auto use(this TSelf&& self, SharedPtr<vertex_buffer_layout_type>&& layout) -> TSelf&& {
            self.m_state.vertexBufferLayouts.push_back(std::move(layout));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an index buffer layout to the input assembler. Can only be called once.
        ///
        /// @param layout The index buffer layout to add to the input assembler.
        /// @throws RuntimeException Thrown if another index buffer layout has already been specified.
        template <typename TSelf>
        constexpr auto use(this TSelf&& self, SharedPtr<index_buffer_layout_type>&& layout) -> TSelf&& {
            self.m_state.indexBufferLayout = std::move(layout);
            return std::forward<TSelf>(self);
        }

        /// @brief Specifies the number of control points to initialize the input assembler with.
        ///
        /// @param controlPoints The control points to initialize the input assembler with.
        template <typename TSelf>
        constexpr auto topology(this TSelf&& self, UInt32 controlPoints) -> TSelf&& {
            self.m_state.controlPoints = controlPoints;
            return std::forward<TSelf>(self);
        }
    };

    /// @brief Describes the interface of a render pipeline builder.
    ///
    /// @tparam TRenderPipeline The type of the render pipeline. Must implement @ref RenderPipeline.
    /// @see RenderPipeline
    template <typename TRenderPipeline> requires
        meta::implements<TRenderPipeline, RenderPipeline<typename TRenderPipeline::pipeline_layout_type, typename TRenderPipeline::shader_program_type, typename TRenderPipeline::input_assembler_type, typename TRenderPipeline::rasterizer_type>>
    class RenderPipelineBuilder : public Builder<TRenderPipeline> {
    public:
        using Builder<TRenderPipeline>::Builder;
        using render_pipeline_type = TRenderPipeline;
        using pipeline_layout_type = render_pipeline_type::pipeline_layout_type;
        using shader_program_type = render_pipeline_type::shader_program_type;
        using input_assembler_type = render_pipeline_type::input_assembler_type;
        using rasterizer_type = render_pipeline_type::rasterizer_type;

    private:
        /// @brief Stores the render pipeline state while building.
        struct RenderPipelineState {
            /// @brief The shader program of the render pipeline.
            SharedPtr<shader_program_type> shaderProgram{ };

            /// @brief The render pipeline layout.
            SharedPtr<pipeline_layout_type> pipelineLayout{ };

            /// @brief The rasterizer state.
            SharedPtr<rasterizer_type> rasterizer{ };

            /// @brief The input assembler state.
            SharedPtr<input_assembler_type> inputAssembler{ };

            /// @brief The alpha-to-coverage setting.
            bool enableAlphaToCoverage{ false };

            /// @brief The multi-sampling level of the render pipeline.
            MultiSamplingLevel samples { MultiSamplingLevel::x1 };
        } m_state;

    protected:
        /// @brief Returns the current render pipeline state.
        ///
        /// @return A reference of the current render pipeline state.
        inline RenderPipelineState& state() noexcept {
            return m_state;
        }

    public:
        /// @brief Adds a shader program to the pipeline layout.
        ///
        /// Note that a pipeline must only have one shader program. If this method is called twice, the second call will overwrite the shader program set by the first call.
        ///
        /// @param program The program to add to the pipeline layout.
        template <typename TSelf>
        constexpr auto shaderProgram(this TSelf&& self, SharedPtr<shader_program_type> program) -> TSelf&& {
            self.m_state.shaderProgram = std::move(program);
            return std::forward<TSelf>(self);
        }

        /// @brief Uses the provided pipeline layout to initialize the render pipeline. Can be invoked only once.
        ///
        /// @param layout The pipeline layout to initialize the render pipeline with.
        template <typename TSelf>
        constexpr auto layout(this TSelf&& self, SharedPtr<pipeline_layout_type> layout) -> TSelf&& {
            self.m_state.pipelineLayout = std::move(layout);
            return std::forward<TSelf>(self);
        }

        /// @brief Uses the provided rasterizer state to initialize the render pipeline. Can be invoked only once.
        ///
        /// @param rasterizer The rasterizer state to initialize the render pipeline with.
        template <typename TSelf>
        constexpr auto rasterizer(this TSelf&& self, SharedPtr<rasterizer_type> rasterizer) -> TSelf&& {
            self.m_state.rasterizer = std::move(rasterizer);
            return std::forward<TSelf>(self);
        }

        /// @brief Uses the provided input assembler state to initialize the render pipeline. Can be invoked only once.
        ///
        /// @param inputAssembler The input assembler state to initialize the render pipeline with.
        template <typename TSelf>
        constexpr auto inputAssembler(this TSelf&& self, SharedPtr<input_assembler_type> inputAssembler) -> TSelf&& {
            self.m_state.inputAssembler = std::move(inputAssembler);
            return std::forward<TSelf>(self);
        }

        /// @brief Enables *Alpha-to-Coverage* multi-sampling on the pipeline.
        ///
        /// For more information on *Alpha-to-Coverage* multi-sampling see the remarks of @ref IRenderPipeline::alphaToCoverage.
        ///
        /// @param enable Whether or not to use *Alpha-to-Coverage* multi-sampling.
        template <typename TSelf>
        constexpr auto enableAlphaToCoverage(this TSelf&& self, bool enable = true) -> TSelf&& {
            self.m_state.enableAlphaToCoverage = enable;
            return std::forward<TSelf>(self);
        }

        /// @brief Sets the multi-sampling level of the render pipeline.
        ///
        /// @param samples The multi-sampling level of the render pipeline.
        template <typename TSelf>
        constexpr auto samples(this TSelf&& self, MultiSamplingLevel samples) -> TSelf&& {
            self.m_state.samples = samples;
            return std::forward<TSelf>(self);
        }
    };

    /// @brief Describes the interface of a compute pipeline builder.
    ///
    /// @tparam TComputePipeline The type of the compute pipeline. Must implement @ref ComputePipeline.
    /// @see ComputePipeline
    template <typename TComputePipeline> requires
        meta::implements<TComputePipeline, ComputePipeline<typename TComputePipeline::pipeline_layout_type, typename TComputePipeline::shader_program_type>>
    class ComputePipelineBuilder : public Builder<TComputePipeline> {
    public:
        using Builder<TComputePipeline>::Builder;
        using compute_pipeline_type = TComputePipeline;
        using pipeline_layout_type = compute_pipeline_type::pipeline_layout_type;
        using shader_program_type = compute_pipeline_type::shader_program_type;

    private:
        /// @brief Stores the compute pipeline state while building.
        struct ComputePipelineState {
            /// @brief The compute pipeline shader program.
            SharedPtr<shader_program_type> shaderProgram{ };

            /// @brief The compute pipeline layout.
            SharedPtr<pipeline_layout_type> pipelineLayout{ };
        } m_state;

    protected:
        /// @brief Returns the current compuite pipeline state.
        ///
        /// @return A reference of the current compute pipeline state.
        inline ComputePipelineState& state() noexcept {
            return m_state;
        }

    public:
        /// @brief Adds a shader program to the pipeline.
        ///
        /// Note that a pipeline must only have one shader program. If this method is called twice, the second call will overwrite the shader program set by the first call.
        ///
        /// @param program The program to add to the pipeline layout.
        template <typename TSelf>
        constexpr auto shaderProgram(this TSelf&& self, SharedPtr<shader_program_type> program) -> TSelf&& {
            self.m_state.shaderProgram = std::move(program);
            return std::forward<TSelf>(self);
        }

        /// @brief Uses the provided pipeline layout to initialize the compute pipeline. Can be invoked only once.
        ///
        /// @param layout The pipeline layout to initialize the compute pipeline with.
        template <typename TSelf>
        constexpr auto layout(this TSelf&& self, SharedPtr<pipeline_layout_type> layout) -> TSelf&& {
            self.m_state.pipelineLayout = std::move(layout);
            return std::forward<TSelf>(self);
        }
    };

    /// @brief Describes the interface of a ray-tracing pipeline builder.
    ///
    /// @tparam TRayTracingPipeline The type of the ray-tracing pipeline. Must implement @ref RayTracingPipeline.
    /// @see RayTracingPipeline
    template <typename TRayTracingPipeline> requires
        meta::implements<TRayTracingPipeline, RayTracingPipeline<typename TRayTracingPipeline::pipeline_layout_type, typename TRayTracingPipeline::shader_program_type>>
    class RayTracingPipelineBuilder : public Builder<TRayTracingPipeline> {
    public:
        using Builder<TRayTracingPipeline>::Builder;
        using raytracing_pipeline_type = TRayTracingPipeline;
        using pipeline_layout_type = raytracing_pipeline_type::pipeline_layout_type;
        using shader_program_type = raytracing_pipeline_type::shader_program_type;

    private:
        /// @brief Stores the ray-tracing pipeline state while building.
        struct RayTracingPipelineState {
            /// @brief The ray-tracing pipeline layout.
            SharedPtr<pipeline_layout_type> pipelineLayout { };

            /// @brief The maximum number of ray bounces in the pipeline.
            UInt32 maxRecursionDepth { 10 }; // NOLINT(cppcoreguidelines-avoid-magic-numbers)

            /// @brief The maximum size for ray payloads in the pipeline.
            UInt32 maxPayloadSize { 0 }; // NOLINT(cppcoreguidelines-avoid-magic-numbers)

            /// @brief The maximum size for ray attributes in the pipeline.
            UInt32 maxAttributeSize { 32 }; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        } m_state;

    protected:
        /// @brief Returns the current ray-tracing pipeline state.
        ///
        /// @return A reference of the current ray-tracing pipeline state.
        inline RayTracingPipelineState& state() noexcept {
            return m_state;
        }

    public:
        /// @brief Uses the provided pipeline layout to initialize the ray-tracing pipeline. Can be invoked only once.
        ///
        /// @param layout The pipeline layout to initialize the ray-tracing pipeline with.
        template <typename TSelf>
        constexpr auto layout(this TSelf&& self, SharedPtr<pipeline_layout_type> layout) -> TSelf&& {
            self.m_state.pipelineLayout = std::move(layout);
            return std::forward<TSelf>(self);
        }

        /// @brief Sets the maximum number of ray bounces allowed in the pipeline.
        ///
        /// @param maxRecursionDepth The maximum number of ray bounces allowed in the pipeline.
        template <typename TSelf>
        constexpr auto maxBounces(this TSelf&& self, UInt32 maxRecursionDepth) -> TSelf&& {
            self.m_state.maxRecursionDepth = maxRecursionDepth;
            return std::forward<TSelf>(self);
        }

        /// @brief Sets the maximum size for a ray payload in the pipeline. Ignored [Payload Access Qualifiers](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#payload-access-qualifiers) are used.
        ///
        /// @param maxPayloadSize The maximum size for ray payloads in the pipeline.
        template <typename TSelf>
        constexpr auto maxPayloadSize(this TSelf&& self, UInt32 maxPayloadSize) -> TSelf&& {
            self.m_state.maxPayloadSize = maxPayloadSize;
            return std::forward<TSelf>(self);
        }

        /// @brief Sets the maximum size for a ray attribute in the pipeline.
        ///
        /// @param maxAttributeSize The maximum size for ray attributes in the pipeline.
        template <typename TSelf>
        constexpr auto maxAttributeSize(this TSelf&& self, UInt32 maxAttributeSize) -> TSelf&& {
            self.m_state.maxAttributeSize = maxAttributeSize;
            return std::forward<TSelf>(self);
        }
    };

    /// @brief Describes the interface of a render pass builder.
    ///
    /// @tparam TDerived The type of the implementation of the builder.
    /// @tparam TRenderPass The type of the render pass. Must implement @ref RenderPass.
    /// @see RenderPass
    template <typename TRenderPass> requires
        meta::implements<TRenderPass, RenderPass<typename TRenderPass::command_queue_type, typename TRenderPass::frame_buffer_type>>
    class RenderPassBuilder : public Builder<TRenderPass, std::nullptr_t, SharedPtr<TRenderPass>> {
    public:
        using Builder<TRenderPass, std::nullptr_t, SharedPtr<TRenderPass>>::Builder;
        using render_pass_type = TRenderPass;
        using command_queue_type = render_pass_type::command_queue_type;

    private:
        /// @brief Stores the render pass state while building.
        struct RenderPassState {
            /// @brief The number of command buffers provided by the render pass.
            UInt32 commandBufferCount{ 0 };

            /// @brief The render targets of the render pass.
            Array<RenderTarget> renderTargets{ };

            /// @brief The input attachments of the render pass.
            Array<RenderPassDependency> inputAttachments{ };

            /// @brief The command queue, the render pass will execute on.
            SharedPtr<const command_queue_type> commandQueue{ nullptr };

            /// @brief The binding point for input attachment samplers, if required.
            Optional<DescriptorBindingPoint> inputAttachmentSamplerBinding{ std::nullopt };

            /// @brief A mask that indicates the enabled view instances during rendering, if view instancing is enabled.
            ///
            /// @see GraphicsDeviceFeatures::ViewInstancing
            UInt32 viewMask{ 0b0000 };
        } m_state;

    protected:
        /// @brief Returns the current render pass state.
        ///
        /// @return A reference of the current render pass state.
        inline RenderPassState& state() noexcept {
            return m_state;
        }

        /// @brief Creates a new input attachment mapping between a render target of a specified render pass and an input location of the render pass that is currently built.
        ///
        /// @param binding The descriptor binding on which to bind the mapped render target.
        /// @param renderTarget The render target of the render pass.
        /// @return The input attachment mapping that describes the relation between the earlier render pass render target and the input location.
        virtual RenderPassDependency makeInputAttachment(DescriptorBindingPoint binding, const RenderTarget& renderTarget) = 0;

    public:
        /// @brief Sets the command queue, the render pass will execute on.
        ///
        /// This method can be called to set the command queue, the render pass will submit it's commands to. If no queue is provided, the render pass will execute on the default graphics queue of the parent
        /// device.
        ///
        /// @param queue The command queue, the render pass will execute on.
        template <typename TSelf>
        constexpr auto executeOn(this TSelf&& self, const command_queue_type& queue) -> TSelf&& {
            self.m_state.commandQueue = queue.shared_from_this();
            return std::forward<TSelf>(self);
        }

        /// @brief Sets the number of command buffers allocated by the render pass.
        ///
        /// @param count The number of command buffers.
        template <typename TSelf>
        constexpr auto commandBuffers(this TSelf&& self, UInt32 count) -> TSelf&& {
            self.m_state.commandBufferCount = count;
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a render target to the render pass by assigning it an incremental location number.
        ///
        /// @param type The type of the render target.
        /// @param format The color format of the render target.
        /// @param flags The flags that control the behavior of the render target.
        /// @param clearValues The fixed clear value for the render target.
        template <typename TSelf>
        constexpr auto renderTarget(this TSelf&& self, RenderTargetType type, Format format, RenderTargetFlags flags = RenderTargetFlags::None, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }) -> TSelf&& {
            self.renderTarget("", static_cast<UInt32>(self.m_state.renderTargets.size()), type, format, flags, clearValues);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a render target to the render pass by assigning it an incremental location number.
        ///
        /// @param name The name of the render target.
        /// @param type The type of the render target.
        /// @param format The color format of the render target.
        /// @param flags The flags that control the behavior of the render target.
        /// @param clearValues The fixed clear value for the render target.
        template <typename TSelf>
        constexpr auto renderTarget(this TSelf&& self, const String& name, RenderTargetType type, Format format, RenderTargetFlags flags = RenderTargetFlags::None, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }) -> TSelf&& {
            self.renderTarget(name, static_cast<UInt32>(self.m_state.renderTargets.size()), type, format, flags, clearValues);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a render target to the render pass.
        ///
        /// @param location The location of the render target.
        /// @param type The type of the render target.
        /// @param format The color format of the render target.
        /// @param flags The flags that control the behavior of the render target.
        /// @param clearValues The fixed clear value for the render target.
        template <typename TSelf>
        constexpr auto renderTarget(this TSelf&& self, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags = RenderTargetFlags::None, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }) -> TSelf&& {
            self.renderTarget("", location, type, format, flags, clearValues);
            return std::forward<TSelf>(self);
        }

        /// @brief Adds a render target to the render pass.
        ///
        /// @param name The name of the render target.
        /// @param location The location of the render target.
        /// @param type The type of the render target.
        /// @param flags The flags that control the behavior of the render target.
        /// @param format The color format of the render target.
        /// @param clearValues The fixed clear value for the render target.
        template <typename TSelf>
        constexpr auto renderTarget(this TSelf&& self, const String& name, UInt32 location, RenderTargetType type, Format format, RenderTargetFlags flags = RenderTargetFlags::None, const Vector4f& clearValues = { 0.0f, 0.0f, 0.0f, 0.0f }) -> TSelf&& {
            self.m_state.renderTargets.push_back(RenderTarget(name, location, type, format, flags, clearValues));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an input attachment to the render pass.
        ///
        /// @param binding The descriptor binding on which to bind the mapped render target.
        /// @param renderPass The render pass, the input attachment is created from.
        /// @param outputLocation The location to which the input attachment is written by @p renderPass.
        template <typename TSelf>
        constexpr auto inputAttachment(this TSelf&& self, DescriptorBindingPoint binding, const render_pass_type& renderPass, UInt32 outputLocation) -> TSelf&& {
            self.m_state.inputAttachments.push_back(static_cast<RenderPassBuilder&>(self).makeInputAttachment(binding, renderPass.renderTarget(outputLocation)));
            return std::forward<TSelf>(self);
        }

        /// @brief Adds an input attachment to the render pass.
        ///
        /// @param binding The descriptor binding on which to bind the mapped render target.
        /// @param renderTarget The render target that is bound as input attachment.
        template <typename TSelf>
        auto inputAttachment(this TSelf&& self, DescriptorBindingPoint binding, RenderTarget renderTarget) -> TSelf&& {
            self.m_state.inputAttachments.push_back(static_cast<RenderPassBuilder&>(self).makeInputAttachment(binding, renderTarget));
            return std::forward<TSelf>(self);
        }

        /// @brief Specifies where to bind the sampler for input attachments, if one is required.
        ///
        /// @param bindingPoint The register and space of the descriptor to bind the input attachment sampler to.
        template <typename TSelf>
        constexpr auto inputAttachmentSamplerBinding(this TSelf&& self, const DescriptorBindingPoint& bindingPoint) -> TSelf&& {
            self.m_state.inputAttachmentSamplerBinding = bindingPoint;
            return std::forward<TSelf>(self);
        }

        /// @brief Specifies the mask of enabled view instances.
        ///
        /// @param viewMask A mask that indicates the enabled view instances for the render pass.
        template <typename TSelf>
        constexpr auto inputAttachmentSamplerBinding(this TSelf&& self, UInt32 viewMask) -> TSelf&& {
            self.m_state.viewMask = viewMask;
            return std::forward<TSelf>(self);
        }
    };

}
#endif // defined(LITEFX_BUILD_DEFINE_BUILDERS)