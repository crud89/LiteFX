###################################################################################################
#####                                                                                         #####
#####          LiteFX.Samples.DXBasicRendering - Contains the basic rendering sample.           #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.DXBasicRendering VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Samples.DXBasicRendering...")

IF(NOT BUILD_DIRECTX_12_BACKEND)
    MESSAGE(FATAL_ERROR "This sample requires the DirectX 12 backend. Set the BUILD_DIRECTX_12_BACKEND option to ON and retry.")
ENDIF(NOT BUILD_DIRECTX_12_BACKEND)

# Install packages.
MESSAGE (STATUS "Installing package dependencies...")
VCPKG_INSTALL (glfw3 cli11)

# Resolve package dependencies.
FIND_PACKAGE(glfw3 CONFIG REQUIRED)
FIND_PACKAGE(cli11 CONFIG REQUIRED)

# Collect header & source files.
SET(SAMPLE_DIRECTX_12_BASIC_RENDERING_HEADERS
    "src/sample.h"
)

SET(SAMPLE_DIRECTX_12_BASIC_RENDERING_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

SET(SAMPLE_DIRECTX_12_BASIC_RENDERING_SHADERS
    "shaders/basic.vert"
    "shaders/basic.frag"
)

# Add shared library project.
ADD_EXECUTABLE(LiteFX.Samples.DXBasicRendering 
    ${SAMPLE_DIRECTX_12_BASIC_RENDERING_HEADERS}
    ${SAMPLE_DIRECTX_12_BASIC_RENDERING_SOURCES}
    ${SAMPLE_DIRECTX_12_BASIC_RENDERING_SHADERS}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DIRECTX_12_BACKEND_HEADERS} ${DIRECTX_12_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Samples.DXBasicRendering PROPERTIES
    FOLDER "Samples"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Samples.DXBasicRendering PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Samples.DXBasicRendering PRIVATE glfw CLI11::CLI11 LiteFX.Core LiteFX.Math LiteFX.AppModel LiteFX.Rendering LiteFX.Backends.DirectX12 DirectX-Headers DirectX-Guids)

# Add shader sources.
TARGET_SHADER_SOURCES(LiteFX.Samples.DXBasicRendering LANGUAGE HLSL SOURCES ${SAMPLE_DIRECTX_12_BASIC_RENDERING_SHADERS})

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(LiteFX.Samples.DXBasicRendering REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS LiteFX.Samples.DXBasicRendering EXPORT LiteFXSamplesConfig
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
EXPORT(TARGETS LiteFX.Samples.DXBasicRendering FILE LiteFXSamplesDXBasicRenderingConfig.cmake)