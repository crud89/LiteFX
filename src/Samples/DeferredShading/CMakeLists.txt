###################################################################################################
#####                                                                                         #####
#####        LiteFX.Samples.DeferredShading - Contains the deferred renderer sample.          #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.DeferredShading VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Samples.DeferredShading...")

IF(NOT BUILD_VULKAN_BACKEND)
    MESSAGE(FATAL_ERROR "The deferred shading sample requires the Vulkan backend. Set the BUILD_VULKAN_BACKEND option to ON and retry.")
ENDIF(NOT BUILD_VULKAN_BACKEND)

# Install packages.
MESSAGE (STATUS "Installing package dependencies...")
VCPKG_INSTALL (glm glad vulkan glfw3 cli11)

# Resolve package dependencies.
FIND_PACKAGE(glm CONFIG REQUIRED)
FIND_PACKAGE(glad CONFIG REQUIRED)
FIND_PACKAGE(Vulkan REQUIRED)
FIND_PACKAGE(glfw3 CONFIG REQUIRED)
FIND_PACKAGE(cli11 CONFIG REQUIRED)

# Collect header & source files.
SET(SAMPLE_DEFERRED_SHADING_HEADERS
    "src/sample.h"
)

SET(SAMPLE_DEFERRED_SHADING_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

SET(SAMPLE_DEFERRED_SHADING_SHADERS
    "shaders/deferred_shading_geometry_pass.vert"
    "shaders/deferred_shading_geometry_pass.frag"
    "shaders/deferred_shading_lighting_pass.vert"
    "shaders/deferred_shading_lighting_pass.frag"
)

# Add shared library project.
ADD_EXECUTABLE(LiteFX.Samples.DeferredShading 
    ${SAMPLE_DEFERRED_SHADING_HEADERS}
    ${SAMPLE_DEFERRED_SHADING_SOURCES}
    ${SAMPLE_DEFERRED_SHADING_SHADERS}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_BACKEND_HEADERS} ${VULKAN_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Samples.DeferredShading PROPERTIES
    FOLDER "Samples"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Samples.DeferredShading PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Samples.DeferredShading PRIVATE glm glad::glad glfw Vulkan::Vulkan CLI11::CLI11 LiteFX.Core LiteFX.Math LiteFX.AppModel LiteFX.Rendering LiteFX.Backends.Vulkan)

# Add shader SAMPLE_DEFERRED_SHADING_SOURCES.
TARGET_SHADER_SOURCES(LiteFX.Samples.DeferredShading LANGUAGE HLSL SOURCES ${SAMPLE_DEFERRED_SHADING_SHADERS})

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(LiteFX.Samples.DeferredShading REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS LiteFX.Samples.DeferredShading EXPORT LiteFXSamplesDeferredShadingConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

#INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXSamplesDeferredShadingConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Install GLFW
SET (GLFW_LOCATION "")
GET_TARGET_PROPERTY(GLFW_LOCATION glfw LOCATION_${CMAKE_BUILD_TYPE})
INSTALL(FILES ${GLFW_LOCATION} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Samples.DeferredShading FILE LiteFXSamplesDeferredShadingConfig.cmake)