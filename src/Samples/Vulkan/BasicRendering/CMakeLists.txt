###################################################################################################
#####                                                                                         #####
#####          LiteFX.Samples.VkBasicRendering - Contains the basic rendering sample.           #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.VkBasicRendering VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Samples.VkBasicRendering...")

IF(NOT BUILD_VULKAN_BACKEND)
    MESSAGE(FATAL_ERROR "This sample requires the Vulkan backend. Set the BUILD_VULKAN_BACKEND option to ON and retry.")
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
SET(SAMPLE_VULKAN_BASIC_RENDERING_HEADERS
    "src/sample.h"
)

SET(SAMPLE_VULKAN_BASIC_RENDERING_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

SET(SAMPLE_VULKAN_BASIC_RENDERING_SHADERS
    "shaders/basic.vert"
    "shaders/basic.frag"
)

# Add shared library project.
ADD_EXECUTABLE(LiteFX.Samples.VkBasicRendering 
    ${SAMPLE_VULKAN_BASIC_RENDERING_HEADERS}
    ${SAMPLE_VULKAN_BASIC_RENDERING_SOURCES}
    ${SAMPLE_VULKAN_BASIC_RENDERING_SHADERS}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_BACKEND_HEADERS} ${VULKAN_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Samples.VkBasicRendering PROPERTIES
    FOLDER "Samples"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Samples.VkBasicRendering PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Samples.VkBasicRendering PRIVATE glm glad::glad glfw Vulkan::Vulkan CLI11::CLI11 LiteFX.Core LiteFX.Math LiteFX.AppModel LiteFX.Rendering LiteFX.Backends.Vulkan)

# Add shader sources.
TARGET_SHADER_SOURCES(LiteFX.Samples.VkBasicRendering LANGUAGE HLSL SOURCES ${SAMPLE_VULKAN_BASIC_RENDERING_SHADERS})

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(LiteFX.Samples.VkBasicRendering REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS LiteFX.Samples.VkBasicRendering EXPORT LiteFXSamplesConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

#INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXSamplesConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Install GLFW
SET (GLFW_LOCATION "")
GET_TARGET_PROPERTY(GLFW_LOCATION glfw LOCATION_${CMAKE_BUILD_TYPE})
INSTALL(FILES ${GLFW_LOCATION} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Samples.VkBasicRendering FILE LiteFXSamplesVkBasicRenderingConfig.cmake)