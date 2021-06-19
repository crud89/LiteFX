###################################################################################################
#####                                                                                         #####
#####           LiteFX.Samples.Vulkan.Textures - Contains the texturing sample.               #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.Vulkan.Textures VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

IF(NOT BUILD_VULKAN_BACKEND)
    MESSAGE(FATAL_ERROR "The texturing sample requires the Vulkan backend. Set the BUILD_VULKAN_BACKEND option to ON and retry.")
ENDIF(NOT BUILD_VULKAN_BACKEND)

IF(NOT BUILD_WITH_GLM)
    MESSAGE(FATAL_ERROR "This sample requires the glm converters for the math module. Set the BUILD_WITH_GLM option to ON and retry.")
ENDIF(NOT BUILD_WITH_GLM)

# Resolve package dependencies.
FIND_PACKAGE(glfw3 CONFIG REQUIRED)
FIND_PACKAGE(cli11 CONFIG REQUIRED)
FIND_PACKAGE(Stb REQUIRED)

# Collect header & source files.
SET(SAMPLE_TEXTURES_HEADERS
    "src/sample.h"
)

SET(SAMPLE_TEXTURES_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Add shared library project.
ADD_EXECUTABLE(${PROJECT_NAME} 
    ${SAMPLE_TEXTURES_HEADERS}
    ${SAMPLE_TEXTURES_SOURCES}
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
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE LiteFX.Core LiteFX.Math LiteFX.AppModel LiteFX.Rendering LiteFX.Backends.Vulkan glfw CLI11::CLI11)

# Add shader sources.
ADD_SHADER_MODULE(${PROJECT_NAME}.Shaders.VS SOURCE "shaders/textures_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS SPIRV SHADER_MODEL ${BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(${PROJECT_NAME}.Shaders.FS SOURCE "shaders/textures_fs.hlsl" LANGUAGE HLSL TYPE FRAGMENT COMPILE_AS SPIRV SHADER_MODEL ${BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
SET_TARGET_PROPERTIES(${PROJECT_NAME}.Shaders.VS PROPERTIES FOLDER "Samples/Vulkan/Shaders")
SET_TARGET_PROPERTIES(${PROJECT_NAME}.Shaders.FS PROPERTIES FOLDER "Samples/Vulkan/Shaders")

TARGET_LINK_SHADERS(${PROJECT_NAME} 
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS ${PROJECT_NAME}.Shaders.VS ${PROJECT_NAME}.Shaders.FS
)

# Define assets.
TARGET_ADD_ASSETS(${PROJECT_NAME}
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${ASSET_DEFAULT_SUBDIR}"
    ASSETS "assets/logo_quad.tga"
)

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
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXSamplesTexturesConfig.cmake)