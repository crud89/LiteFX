###################################################################################################
#####                                                                                         #####
#####     LiteFX.Samples.Vulkan.DeferredShading - Contains the deferred renderer sample.      #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.Vulkan.DeferredShading VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

IF(NOT BUILD_VULKAN_BACKEND)
    MESSAGE(FATAL_ERROR "The deferred shading sample requires the Vulkan backend. Set the BUILD_VULKAN_BACKEND option to ON and retry.")
ENDIF(NOT BUILD_VULKAN_BACKEND)

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
ADD_EXECUTABLE(${PROJECT_NAME} 
    ${SAMPLE_DEFERRED_SHADING_HEADERS}
    ${SAMPLE_DEFERRED_SHADING_SOURCES}
    ${SAMPLE_DEFERRED_SHADING_SHADERS}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_BACKEND_HEADERS} ${VULKAN_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "Samples/Vulkan"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE glm::glm glad::glad glfw Vulkan::Vulkan CLI11::CLI11 LiteFX.Core LiteFX.Math LiteFX.AppModel LiteFX.Rendering LiteFX.Backends.Vulkan)

# Add shader sources.
TARGET_SHADER_SOURCES(${PROJECT_NAME} LANGUAGE HLSL SOURCES ${SAMPLE_DEFERRED_SHADING_SHADERS})

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFXSamplesConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
)

# Export config.
INSTALL(EXPORT LiteFXSamplesConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXSamplesDeferredShadingConfig.cmake)