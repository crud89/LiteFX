###################################################################################################
#####                                                                                         #####
#####        LiteFX.Samples.DX12.BasicRendering - Contains the basic rendering sample.        #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.DX12.BasicRendering VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

IF(NOT BUILD_DIRECTX_12_BACKEND)
    MESSAGE(FATAL_ERROR "This sample requires the DirectX 12 backend. Set the BUILD_DIRECTX_12_BACKEND option to ON and retry.")
ENDIF(NOT BUILD_DIRECTX_12_BACKEND)

IF(NOT BUILD_WITH_GLM)
    MESSAGE(FATAL_ERROR "This sample requires the glm converters for the math module. Set the BUILD_WITH_GLM option to ON and retry.")
ENDIF(NOT BUILD_WITH_GLM)

# Resolve package dependencies.
FIND_PACKAGE(glfw3 CONFIG REQUIRED)
FIND_PACKAGE(cli11 CONFIG REQUIRED)

CONFIGURE_FILE("../config.tmpl" "${CMAKE_CURRENT_BINARY_DIR}/src/config.h")

# Collect header & source files.
SET(SAMPLE_DIRECTX_12_BASIC_RENDERING_HEADERS
    "src/sample.h"
    "${CMAKE_CURRENT_BINARY_DIR}/src/config.h"
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
ADD_EXECUTABLE(${PROJECT_NAME} 
    ${SAMPLE_DIRECTX_12_BASIC_RENDERING_HEADERS}
    ${SAMPLE_DIRECTX_12_BASIC_RENDERING_SOURCES}
    ${SAMPLE_DIRECTX_12_BASIC_RENDERING_SHADERS}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DIRECTX_12_BACKEND_HEADERS} ${DIRECTX_12_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "Samples/DirectX 12"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE "src/" "${CMAKE_CURRENT_BINARY_DIR}/src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE LiteFX.Core LiteFX.Math LiteFX.AppModel LiteFX.Rendering LiteFX.Backends.DirectX12 glfw CLI11::CLI11)

# Add shader sources.
TARGET_SHADER_SOURCES(${PROJECT_NAME} LANGUAGE HLSL SOURCES ${SAMPLE_DIRECTX_12_BASIC_RENDERING_SHADERS})

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFXSamplesConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

# Export config.
INSTALL(EXPORT LiteFXSamplesConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXSamplesDXBasicRenderingConfig.cmake)