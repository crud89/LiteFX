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
    "src/convert.cpp"
    "src/buffer_attribute.cpp"
    "src/depth_stencil_state.cpp"
    "src/rasterizer.cpp"
    "src/viewport.cpp"
    "src/scissor.cpp"
    "src/render_target.cpp"
    "src/state_resource.cpp"
    "src/device_state.cpp"
    "src/timing_event.cpp"
)

# Add shared library project.
ADD_LIBRARY(${PROJECT_NAME} SHARED
    ${RENDERING_HEADERS}
    ${RENDERING_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RENDERING_HEADERS} ${RENDERING_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    PUBLIC_HEADER "${RENDERING_HEADERS}"
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Debug"
    MAP_IMPORTED_CONFIG_MINSIZEREL "Release"
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} 
    PUBLIC LiteFX.Core LiteFX.Math LiteFX.Graphics LiteFX.AppModel LiteFX.Logging
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
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}/litefx/
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

# Export config.
INSTALL(EXPORT LiteFX DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXRenderingConfig.cmake)