###################################################################################################
#####                                                                                         #####
#####             LiteFX.Backends.Vulkan - Contains the Vulkan rendering backend.             #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Backends.Vulkan VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Resolve package dependencies.
FIND_PACKAGE(Vulkan REQUIRED)
FIND_PACKAGE(VulkanMemoryAllocator CONFIG REQUIRED)
FIND_PACKAGE(unofficial-spirv-reflect CONFIG REQUIRED)

# Collect header & source files.
SET(VULKAN_BACKEND_HEADERS
    "include/litefx/backends/vulkan_api.hpp"
    "include/litefx/backends/vulkan_formatters.hpp"
    "include/litefx/backends/vulkan_builders.hpp"
    "include/litefx/backends/vulkan.hpp"
)

SET(VULKAN_BACKEND_SOURCES
    "src/buffer.h"
    "src/image.h"
    "src/convert.cpp"
    "src/backend.cpp"
    "src/adapter.cpp"
    "src/device.cpp"
    "src/factory.cpp"
    "src/queue.cpp"
    "src/surface.cpp"
    "src/swapchain.cpp"
    "src/frame_buffer.cpp"
    "src/render_pass.cpp"
    "src/render_pipeline.cpp"
    "src/compute_pipeline.cpp"
    "src/ray_tracing_pipeline.cpp"
    "src/shader_module.cpp"
    "src/shader_program.cpp"
    "src/pipeline_layout.cpp"
    "src/rasterizer.cpp"
    "src/input_assembler.cpp"
    "src/command_buffer.cpp"
    "src/barrier.cpp"
    "src/vertex_buffer_layout.cpp"
    "src/index_buffer_layout.cpp"
    "src/descriptor_layout.cpp"
    "src/descriptor_set.cpp"
    "src/descriptor_set_layout.cpp"
    "src/buffer.cpp"
    "src/image.cpp"
    "src/push_constants_range.cpp"
    "src/push_constants_layout.cpp"
    "src/blas.cpp"
    "src/tlas.cpp"
    "src/virtual_allocator.hpp"
)

# Add library project.
LITEFX_ADD_LIBRARY(${PROJECT_NAME}
    EXPORT_NAME Vulkan
    SOURCES ${VULKAN_BACKEND_SOURCES}
    HEADERS ${VULKAN_BACKEND_HEADERS}
    EXPORT_MACRO LITEFX_VULKAN_API
    EXPORT_HEADER "litefx/backends/vulkan_export.h"
    NATVIS ".natvis"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_BACKEND_HEADERS} ${VULKAN_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "Backends"
    VERSION ${LITEFX_VERSION}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX::Core LiteFX::Math LiteFX::Rendering LiteFX::AppModel Vulkan::Vulkan
    PRIVATE GPUOpen::VulkanMemoryAllocator unofficial::spirv-reflect
)

# Link against D3D12 to enable flip-model swap chains.
IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC LiteFX::DirectX12)
ENDIF(LITEFX_BUILD_DIRECTX_12_BACKEND)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFX FILE_SET HEADERS)
