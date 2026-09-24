###################################################################################################
#####                                                                                         #####
#####          LiteFX.Samples.RayTracing - Contains the hardware ray tracing sample.          #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.RayTracing VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_RAY_TRACING_HEADERS
    "src/sample.h"
)

SET(SAMPLE_RAY_TRACING_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_RAY_TRACING_HEADERS}
    SOURCES ${SAMPLE_RAY_TRACING_SOURCES}
)

IF(LITEFX_BUILD_VULKAN_BACKEND)
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
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Vk.Shaders.RG ${PROJECT_NAME}.Vk.Shaders.RH ${PROJECT_NAME}.Vk.Shaders.RM
    )
ENDIF(LITEFX_BUILD_VULKAN_BACKEND)

IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.RG SOURCE "shaders/raytracing_gen.hlsl"  LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC INCLUDES "shaders/raytracing_common.hlsli" COMPILE_OPTIONS "-disable-payload-qualifiers")
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.RH SOURCE "shaders/raytracing_hit.hlsl"  LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC INCLUDES "shaders/raytracing_common.hlsli" COMPILE_OPTIONS "-disable-payload-qualifiers")
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.RM SOURCE "shaders/raytracing_miss.hlsl" LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC INCLUDES "shaders/raytracing_common.hlsli" COMPILE_OPTIONS "-disable-payload-qualifiers")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.RG PROPERTIES FOLDER "Samples/Shaders/DirectX 12")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.RH PROPERTIES FOLDER "Samples/Shaders/DirectX 12")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.RM PROPERTIES FOLDER "Samples/Shaders/DirectX 12")

    TARGET_LINK_SHADERS(${PROJECT_NAME} 
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Dx.Shaders.RG ${PROJECT_NAME}.Dx.Shaders.RH ${PROJECT_NAME}.Dx.Shaders.RM
    )
ENDIF(LITEFX_BUILD_DIRECTX_12_BACKEND)

# Define assets.
TARGET_ADD_ASSET_DIRECTORY(${PROJECT_NAME}
    NAME "assets"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ASSETS "assets/rt_skybox.jpg"
)
