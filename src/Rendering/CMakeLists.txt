###################################################################################################
#####                                                                                         #####
#####                LiteFX.Rendering - Contains the rendering infrastructure.                #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Rendering VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(RENDERING_HEADERS
    "include/litefx/rendering_api.hpp"
    "include/litefx/rendering_formatters.hpp"
    "include/litefx/rendering.hpp"
    "include/litefx/rendering_builders.hpp"
)

SET(RENDERING_SOURCES
    "src/buffer_attribute.cpp"
    "src/depth_stencil_state.cpp"
    "src/rasterizer.cpp"
    "src/viewport.cpp"
    "src/scissor.cpp"
    "src/render_target.cpp"
    "src/render_pass_dependency.cpp"
    "src/state_resource.cpp"
    "src/device_state.cpp"
    "src/timing_event.cpp"
    "src/shader_record_collection.cpp"
)

# Add library project.
LITEFX_ADD_LIBRARY(${PROJECT_NAME}
    EXPORT_NAME Rendering
    SOURCES ${RENDERING_SOURCES}
    HEADERS ${RENDERING_HEADERS}
    EXPORT_MACRO LITEFX_RENDERING_API
    EXPORT_HEADER "litefx/rendering_export.h"
    NATVIS ".natvis"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RENDERING_HEADERS} ${RENDERING_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX::Core LiteFX::Math LiteFX::AppModel LiteFX::Logging
)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFX FILE_SET HEADERS)
