###################################################################################################
#####                                                                                         #####
#####                LiteFX.Rendering - Contains the rendering infrastructure.                #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Rendering VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Rendering...")

# Install packages.
MESSAGE(STATUS "Installing package dependencies...")

SET(TARGET_LINK_LIBS "")

IF(BUILD_WITH_GLM)
    VCPKG_INSTALL(glm)
    FIND_PACKAGE(glm CONFIG REQUIRED)
    LIST(APPEND TARGET_LINK_LIBS "glm")
ENDIF(BUILD_WITH_GLM)

# Collect header & source files.
SET(RENDERING_HEADERS
    "include/litefx/rendering_api.hpp"
    "include/litefx/rendering_formatters.hpp"
    "include/litefx/rendering_pipelines.hpp"
    "include/litefx/rendering.hpp"
)

SET(RENDERING_SOURCES
    "src/backend.cpp"
    "src/device.cpp"
    "src/buffer_attribute.cpp"
    "src/pipeline.cpp"
    "src/pipeline_layout.cpp"
    "src/input_assembler.cpp"
    "src/rasterizer.cpp"
    "src/viewport.cpp"
    "src/render_target.cpp"
    "src/buffer.cpp"
    "src/vertex_buffer.cpp"
    "src/index_buffer.cpp"
    "src/constant_buffer.cpp"
    "src/image.cpp"
    "src/texture.cpp"
    "src/sampler.cpp"
)

# Add shared library project.
ADD_LIBRARY(LiteFX.Rendering SHARED
    ${RENDERING_HEADERS}
    ${RENDERING_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RENDERING_HEADERS} ${RENDERING_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Rendering PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Rendering 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Rendering 
    PUBLIC LiteFX.Core LiteFX.Math LiteFX.Graphics LiteFX.AppModel LiteFX.Logging ${TARGET_LINK_LIBS}
)

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(LiteFX.Rendering REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS LiteFX.Rendering EXPORT LiteFXRenderingConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXRenderingConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Rendering FILE LiteFXRenderingConfig.cmake)