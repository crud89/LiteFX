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

# Add shared library project.
ADD_LIBRARY(${PROJECT_NAME}
    ${GRAPHICS_SOURCES}

    ".natvis"
)

TARGET_SOURCES(${PROJECT_NAME} PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS "include/"
    FILES ${GRAPHICS_HEADERS}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GRAPHICS_HEADERS} ${GRAPHICS_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Debug"
    MAP_IMPORTED_CONFIG_MINSIZEREL "Release"
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} 
    PUBLIC LiteFX.Core LiteFX.Logging LiteFX.Math LiteFX.Rendering
)

# Link supported backends.
IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC LiteFX.Backends.DirectX12)
    
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
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC LiteFX.Backends.Vulkan)
ENDIF(LITEFX_BUILD_VULKAN_BACKEND)

# Pre-define export specifier, to prevent dllimport/dllexport from being be emitted.
IF(NOT BUILD_SHARED_LIBS)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC -DLITEFX_GRAPHICS_API=)
ENDIF(NOT BUILD_SHARED_LIBS)

# Re-use pre-compiled core header.
IF(LITEFX_BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(LITEFX_BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT EXPORT LiteFX
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    FILE_SET public_headers DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

# Export config.
INSTALL(EXPORT LiteFX DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXGraphicsConfig.cmake)