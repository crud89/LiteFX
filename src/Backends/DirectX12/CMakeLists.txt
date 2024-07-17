###################################################################################################
#####                                                                                         #####
#####         LiteFX.Backends.DirectX12 - Contains the DirectX 12 rendering backend.          #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Backends.DirectX12 VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Resolve package dependencies.
FIND_PACKAGE(directx-headers CONFIG REQUIRED)
FIND_PACKAGE(directx-agility-sdk CONFIG REQUIRED)
FIND_PACKAGE(d3d12-memory-allocator CONFIG REQUIRED)
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
)

# Add shared library project.
ADD_LIBRARY(${PROJECT_NAME}
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
    PUBLIC_HEADER "${DIRECTX12_BACKEND_HEADERS}"
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Debug"
    MAP_IMPORTED_CONFIG_MINSIZEREL "Release"
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
    PUBLIC LiteFX.Core LiteFX.Math LiteFX.Rendering LiteFX.AppModel Microsoft::DirectX-Headers Microsoft::DirectX-Guids dxgi Microsoft::DirectXAgilitySDK Microsoft::DirectXShaderCompiler Microsoft::DXIL 
    PRIVATE unofficial::d3d12-memory-allocator
)

# Pre-define export specifier, to prevent dllimport/dllexport from being be emitted.
IF(NOT BUILD_SHARED_LIBS)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC "LITEFX_DIRECTX12_API")
ENDIF(NOT BUILD_SHARED_LIBS)

# Link PIX runtime, if available.
IF(LITEFX_BUILD_WITH_PIX_RUNTIME)
    FIND_PACKAGE(WinPixEventRuntime CONFIG REQUIRED)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE Microsoft::WinPixEventRuntime)
ENDIF(LITEFX_BUILD_WITH_PIX_RUNTIME)

# Add shader modules.
ADD_SHADER_LIBRARY(${PROJECT_NAME}.Shaders SOURCE_FILE "shader_resources.hpp" NAMESPACE "LiteFX::Backends::DirectX12::Shaders")
ADD_SHADER_MODULE(${PROJECT_NAME}.Blit SOURCE "shaders/blit.hlsl" LANGUAGE HLSL TYPE COMPUTE COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC LIBRARY ${PROJECT_NAME}.Shaders)
SET_TARGET_PROPERTIES(${PROJECT_NAME}.Shaders PROPERTIES FOLDER "Backends/Shaders")
SET_TARGET_PROPERTIES(${PROJECT_NAME}.Blit PROPERTIES FOLDER "Backends/Shaders")

TARGET_LINK_SHADER_LIBRARIES(${PROJECT_NAME} 
    LIBRARIES ${PROJECT_NAME}.Shaders
)

# Copy the DXC runtime to the project build dir.
IF(DXCOMPILER_DLL)
    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXCOMPILER_DLL}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying ${DXCOMPILER_DLL} to runtime directory..."
    )

    INSTALL(FILES "${DXCOMPILER_DLL}" DESTINATION ${CMAKE_INSTALL_BINARY_DIR})
ENDIF(DXCOMPILER_DLL)

IF(DXIL_DLL)
    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXIL_DLL}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying ${DXIL_DLL} to runtime directory..."
    )

    INSTALL(FILES "${DXIL_DLL}" DESTINATION ${CMAKE_INSTALL_BINARY_DIR})
ENDIF(DXIL_DLL)

# Re-use pre-compiled core header.
IF(LITEFX_BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(LITEFX_BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT EXPORT LiteFX
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}/litefx/backends
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

# Export config.
INSTALL(EXPORT LiteFX DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXBackendsDirectX12Config.cmake)