###################################################################################################
#####                                                                                         #####
#####          LiteFX.Samples.RayTracing - Contains the hardware ray tracing sample.          #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.RayTracing VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

IF(NOT LITEFX_BUILD_WITH_GLM)
    MESSAGE(FATAL_ERROR "This sample requires the glm converters for the math module. Set the LITEFX_BUILD_WITH_GLM option to ON and retry.")
ENDIF(NOT LITEFX_BUILD_WITH_GLM)

# Resolve package dependencies.
FIND_PACKAGE(glfw3 CONFIG REQUIRED)
FIND_PACKAGE(CLI11 CONFIG REQUIRED)
FIND_PATH(RENDERDOC_INCLUDE_DIR "renderdoc_app.h")

IF(NOT RENDERDOC_INCLUDE_DIR AND LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER)
    MESSAGE(WARNING "RenderDoc is not installed on the system. Loader will not be created.")
    SET(LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER OFF CACHE BOOL "" FORCE)
ENDIF(NOT RENDERDOC_INCLUDE_DIR AND LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER)

CONFIGURE_FILE("../config.tmpl" "${CMAKE_CURRENT_BINARY_DIR}/src/config.h")

# Collect header & source files.
SET(SAMPLE_RAY_TRACING_HEADERS
    "src/sample.h"
)

SET(SAMPLE_RAY_TRACING_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Add shared library project.
ADD_EXECUTABLE(${PROJECT_NAME} 
    ${SAMPLE_RAY_TRACING_HEADERS}
    ${SAMPLE_RAY_TRACING_SOURCES}
    "${CMAKE_CURRENT_BINARY_DIR}/src/config.h"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SAMPLE_RAY_TRACING_HEADERS} ${SAMPLE_RAY_TRACING_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "Samples"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src/")

IF(LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE ${RENDERDOC_INCLUDE_DIR})
ENDIF(LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER)

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE LiteFX.Core LiteFX.Math LiteFX.AppModel LiteFX.Rendering glfw CLI11::CLI11)

IF(LITEFX_BUILD_VULKAN_BACKEND)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE LiteFX.Backends.Vulkan)

    # NOTE: There is a DXC bug that emits SPV_KHR_ray_query extension for ray tracing shaders, even if it is not required (https://github.com/microsoft/DirectXShaderCompiler/issues/4385). This
    #       bug has been closed, but was not actually fixed. To prevent emitting this extension, we specify the supported extensions manually. If you are targeting only hardware that supports
    #       DXR tier 1.1 you can remove the compile options below. However, some GPUs are locked out in this case (e.g. GTX 10x series with compute emulation for ray-tracing features).
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.RG SOURCE "shaders/raytracing_gen.hlsl"  LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC COMPILE_OPTIONS "-fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_EXT_descriptor_indexing -disable-payload-qualifiers" INCLUDES "shaders/raytracing_common.hlsli")
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.RH SOURCE "shaders/raytracing_hit.hlsl"  LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC COMPILE_OPTIONS "-fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_EXT_descriptor_indexing -disable-payload-qualifiers" INCLUDES "shaders/raytracing_common.hlsli")
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.RM SOURCE "shaders/raytracing_miss.hlsl" LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC COMPILE_OPTIONS "-fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_EXT_descriptor_indexing -disable-payload-qualifiers" INCLUDES "shaders/raytracing_common.hlsli")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.RG PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.RH PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.RM PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    
    TARGET_LINK_SHADERS(${PROJECT_NAME} 
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Vk.Shaders.RG ${PROJECT_NAME}.Vk.Shaders.RH ${PROJECT_NAME}.Vk.Shaders.RM
    )
ENDIF(LITEFX_BUILD_VULKAN_BACKEND)

IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE LiteFX.Backends.DirectX12)
    
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.RG SOURCE "shaders/raytracing_gen.hlsl"  LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC INCLUDES "shaders/raytracing_common.hlsli" COMPILE_OPTIONS "-disable-payload-qualifiers")
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.RH SOURCE "shaders/raytracing_hit.hlsl"  LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC INCLUDES "shaders/raytracing_common.hlsli" COMPILE_OPTIONS "-disable-payload-qualifiers")
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.RM SOURCE "shaders/raytracing_miss.hlsl" LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC INCLUDES "shaders/raytracing_common.hlsli" COMPILE_OPTIONS "-disable-payload-qualifiers")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.RG PROPERTIES FOLDER "Samples/Shaders/DirectX 12")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.RH PROPERTIES FOLDER "Samples/Shaders/DirectX 12")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.RM PROPERTIES FOLDER "Samples/Shaders/DirectX 12")

    TARGET_LINK_SHADERS(${PROJECT_NAME} 
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Dx.Shaders.RG ${PROJECT_NAME}.Dx.Shaders.RH ${PROJECT_NAME}.Dx.Shaders.RM
    )
ENDIF(LITEFX_BUILD_DIRECTX_12_BACKEND)

# Define assets.
TARGET_ADD_ASSET_DIRECTORY(${PROJECT_NAME}
    NAME "assets"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}"
    ASSETS "assets/rt_skybox.jpg"
)

# Re-use pre-compiled core header.
IF(LITEFX_BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(LITEFX_BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFXSamples
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

# Export config.
INSTALL(EXPORT LiteFXSamples DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXSamplesRayTracingConfig.cmake)