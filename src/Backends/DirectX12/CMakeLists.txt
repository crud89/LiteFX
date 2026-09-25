###################################################################################################
#####                                                                                         #####
#####         LiteFX.Backends.DirectX12 - Contains the DirectX 12 rendering backend.          #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Backends.DirectX12 VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Resolve package dependencies.
FIND_PACKAGE(directx-headers CONFIG REQUIRED)
FIND_PACKAGE(directx12-agility CONFIG REQUIRED)
FIND_PACKAGE(D3D12MemoryAllocator CONFIG REQUIRED)
FIND_PACKAGE(directx-dxc CONFIG REQUIRED)

# Collect header & source files.
SET(DIRECTX12_BACKEND_HEADERS
    "include/litefx/backends/dx12_api.hpp"
    "include/litefx/backends/dx12_formatters.hpp"
    "include/litefx/backends/dx12_builders.hpp"
    "include/litefx/backends/dx12.hpp"
)

SET(DIRECTX12_BACKEND_SOURCES
    "src/buffer.h"
    "src/buffer.cpp"
    "src/image.h"
    "src/image.cpp"
    "src/convert.cpp"
    "src/backend.cpp"
    "src/adapter.cpp"
    "src/surface.cpp"
    "src/device.cpp"
    "src/factory.cpp"
    "src/queue.cpp"
    "src/swapchain.cpp"
    "src/frame_buffer.cpp"
    "src/render_pass.cpp"
    "src/command_buffer.cpp"
    "src/barrier.cpp"
    "src/render_pipeline.cpp"
    "src/compute_pipeline.cpp"
    "src/ray_tracing_pipeline.cpp"
    "src/pipeline_layout.cpp"
    "src/rasterizer.cpp"
    "src/input_assembler.cpp"
    "src/vertex_buffer_layout.cpp"
    "src/index_buffer_layout.cpp"
    "src/shader_module.cpp"
    "src/shader_program.cpp"
    "src/descriptor_layout.cpp"
    "src/descriptor_set.cpp"
    "src/descriptor_set_layout.cpp"
    "src/push_constants_range.cpp"
    "src/push_constants_layout.cpp"
    "src/blas.cpp"
    "src/tlas.cpp"
    "src/virtual_allocator.hpp"
)

# Add library project.
LITEFX_ADD_LIBRARY(${PROJECT_NAME}
    EXPORT_NAME DirectX12
    SOURCES ${DIRECTX12_BACKEND_SOURCES}
    HEADERS ${DIRECTX12_BACKEND_HEADERS}
    EXPORT_MACRO LITEFX_DIRECTX12_API
    EXPORT_HEADER "litefx/backends/dx12_export.h"
    NATVIS ".natvis"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DIRECTX12_BACKEND_HEADERS} ${DIRECTX12_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "Backends"
    VERSION ${LITEFX_VERSION}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX::Core LiteFX::Math LiteFX::Rendering LiteFX::AppModel Microsoft::DirectX-Headers Microsoft::DirectX-Guids dxgi dxcore Microsoft::DirectX12-Agility Microsoft::DirectXShaderCompiler
    PRIVATE GPUOpen::D3D12MemoryAllocator
)

# Link PIX runtime, if available.
IF(LITEFX_BUILD_WITH_PIX_RUNTIME)
    FIND_PACKAGE(winpixevent CONFIG REQUIRED)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE Microsoft::WinPixEventRuntime)
ENDIF(LITEFX_BUILD_WITH_PIX_RUNTIME)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFX FILE_SET HEADERS)
