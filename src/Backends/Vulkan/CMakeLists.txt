###################################################################################################
#####                                                                                         #####
#####             LiteFX.Backends.Vulkan - Contains the Vulkan rendering backend.             #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Backends.Vulkan VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Resolve package dependencies.
FIND_PACKAGE(Vulkan REQUIRED)
FIND_PACKAGE(unofficial-vulkan-memory-allocator CONFIG REQUIRED)

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
    "src/adaper.cpp"
    "src/device.cpp"
    "src/factory.cpp"
    "src/queue.cpp"
    "src/surface.cpp"
    "src/swapchain.cpp"
    "src/frame_buffer.cpp"
    "src/input_attachment_mapping.cpp"
    "src/render_pass.cpp"
    "src/render_pipeline.cpp"
    "src/compute_pipeline.cpp"
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
)

# Add builder sources.
IF(BUILD_DEFINE_BUILDERS)
    SET(VULKAN_BACKEND_SOURCES
        ${VULKAN_BACKEND_SOURCES}
        "src/builders/compute_pipeline_builder.cpp"
        "src/builders/compute_pipeline_layout_builder.cpp"
        "src/builders/descriptor_set_layout_builder.cpp"
        "src/builders/input_assembler_builder.cpp"
        "src/builders/push_constants_layout_builder.cpp"
        "src/builders/rasterizer_builder.cpp"
        "src/builders/render_pass_builder.cpp"
        "src/builders/render_pipeline_builder.cpp"
        "src/builders/render_pipeline_layout_builder.cpp"
        "src/builders/shader_program_builder.cpp"
        "src/builders/vertex_buffer_layout_builder.cpp"
    )
ENDIF(BUILD_DEFINE_BUILDERS)

# Add shared library project.
ADD_LIBRARY(${PROJECT_NAME} SHARED
    ${VULKAN_BACKEND_HEADERS}
    ${VULKAN_BACKEND_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_BACKEND_HEADERS} ${VULKAN_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "Backends"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    PUBLIC_HEADER "${VULKAN_BACKEND_HEADERS}"
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
    PUBLIC LiteFX.Core LiteFX.Math LiteFX.Rendering LiteFX.AppModel Vulkan::Vulkan 
    PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator
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
INSTALL(EXPORT LiteFX DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXBackendsVulkanConfig.cmake)