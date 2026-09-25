###################################################################################################
#####                                                                                         #####
#####                  LiteFX.Graphics - Contains graphics data structures.                   #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Graphics VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(GRAPHICS_HEADERS
    "include/litefx/graphics_api.hpp"
    "include/litefx/graphics.hpp"

    "include/litefx/gfx/blitter.hpp"
    "include/litefx/gfx/vertex.hpp"
)

SET(GRAPHICS_SOURCES
    "src/blitter_vk.cpp"
    "src/blitter_d3d12.cpp"
)

# Add library project
LITEFX_ADD_LIBRARY(${PROJECT_NAME}
    EXPORT_NAME Graphics
    SOURCES ${GRAPHICS_SOURCES}
    HEADERS ${GRAPHICS_HEADERS}
    EXPORT_MACRO LITEFX_GRAPHICS_API
    EXPORT_HEADER "litefx/graphics_export.h"
    NATVIS ".natvis"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GRAPHICS_HEADERS} ${GRAPHICS_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX::Core LiteFX::Logging LiteFX::Math LiteFX::Rendering
)

# Link supported backends.
IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC LiteFX::DirectX12)

    # Add shader modules.
    ADD_SHADER_LIBRARY(${PROJECT_NAME}.Shaders SOURCE_FILE "shader_resources.hpp" NAMESPACE "LiteFX::Graphics::Shaders")
    ADD_SHADER_MODULE(${PROJECT_NAME}.Blit SOURCE "shaders/blit.hlsl" LANGUAGE HLSL TYPE COMPUTE COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC LIBRARY ${PROJECT_NAME}.Shaders)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Shaders PROPERTIES FOLDER "SDK/Graphics/Shaders")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Blit PROPERTIES FOLDER "SDK/Graphics/Shaders")

    TARGET_LINK_SHADER_LIBRARIES(${PROJECT_NAME}
        LIBRARIES ${PROJECT_NAME}.Shaders
    )
ENDIF(LITEFX_BUILD_DIRECTX_12_BACKEND)

IF(LITEFX_BUILD_VULKAN_BACKEND)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC LiteFX::Vulkan)
ENDIF(LITEFX_BUILD_VULKAN_BACKEND)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFX FILE_SET HEADERS)
