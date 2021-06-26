###################################################################################################
#####                                                                                         #####
#####              LiteFX.Samples.DX12.Textures - Contains the texturing sample.              #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.DX12.Textures VERSION ${LITEFX_VERSION} LANGUAGES CXX)
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
SET(SAMPLE_DIRECTX_12_TEXTURES_HEADERS
    "src/sample.h"
)

SET(SAMPLE_DIRECTX_12_TEXTURES_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Add shared library project.
ADD_EXECUTABLE(${PROJECT_NAME} 
    ${SAMPLE_DIRECTX_12_TEXTURES_HEADERS}
    ${SAMPLE_DIRECTX_12_TEXTURES_SOURCES}
    "${CMAKE_CURRENT_BINARY_DIR}/src/config.h"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SAMPLE_DIRECTX_12_TEXTURES_HEADERS} ${SAMPLE_DIRECTX_12_TEXTURES_SOURCES})

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
ADD_SHADER_MODULE(${PROJECT_NAME}.Shaders.VS SOURCE "shaders/textures_vs.hlsl" LANGUAGE HLSL TYPE VERTEX COMPILE_AS DXIL SHADER_MODEL ${BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(${PROJECT_NAME}.Shaders.PS SOURCE "shaders/textures_ps.hlsl" LANGUAGE HLSL TYPE PIXEL  COMPILE_AS DXIL SHADER_MODEL ${BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
SET_TARGET_PROPERTIES(${PROJECT_NAME}.Shaders.VS PROPERTIES FOLDER "Samples/DirectX 12/Shaders")
SET_TARGET_PROPERTIES(${PROJECT_NAME}.Shaders.PS PROPERTIES FOLDER "Samples/DirectX 12/Shaders")

TARGET_LINK_SHADERS(${PROJECT_NAME} 
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS ${PROJECT_NAME}.Shaders.VS ${PROJECT_NAME}.Shaders.PS
)

# Define assets.
TARGET_ADD_ASSET_DIRECTORY(${PROJECT_NAME}
    NAME "assets"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}"
    ASSETS "assets/logo_quad.tga"
)

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFXSamples
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
)

# Export config.
INSTALL(EXPORT LiteFXSamples DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXSamplesDXTexturesConfig.cmake)