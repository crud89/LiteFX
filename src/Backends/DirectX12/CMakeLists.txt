###################################################################################################
#####                                                                                         #####
#####         LiteFX.Backends.DirectX12 - Contains the DirectX 12 rendering backend.          #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Backends.DirectX12 VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Resolve package dependencies.
FIND_PACKAGE(directx-headers CONFIG REQUIRED)    
FIND_PACKAGE(d3d12-memory-allocator CONFIG REQUIRED)

# Collect header & source files.
SET(DIRECTX12_BACKEND_HEADERS
    "include/litefx/backends/dx12_api.hpp"
    "include/litefx/backends/dx12_formatters.hpp"
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
    "src/input_attachment_mapping.cpp"
    "src/render_pass.cpp"
    "src/command_buffer.cpp"
    "src/pipeline.cpp"
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
)

# Add shared library project.
ADD_LIBRARY(${PROJECT_NAME} SHARED
    ${DIRECTX12_BACKEND_HEADERS}
    ${DIRECTX12_BACKEND_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DIRECTX12_BACKEND_HEADERS} ${DIRECTX12_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "Backends"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    PUBLIC_HEADER "${VULKAN_BACKEND_HEADERS}"
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX.Core LiteFX.Math LiteFX.Rendering LiteFX.AppModel Microsoft::DirectX-Headers Microsoft::DirectX-Guids d3d12 dxcore dxgi d3dcompiler
    PRIVATE unofficial::d3d12-memory-allocator
)

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT EXPORT LiteFX
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}/litefx/backends
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

# Export config.
INSTALL(EXPORT EXPORT LiteFX DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXBackendsDirectX12Config.cmake)