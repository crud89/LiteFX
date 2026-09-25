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

# Define shader program(s).
# NOTE: There is a DXC bug that emits SPV_KHR_ray_query extension for ray tracing shaders, even if it is not required (https://github.com/microsoft/DirectXShaderCompiler/issues/4385). This
#       bug has been closed, but was not actually fixed. To prevent emitting this extension, we specify the supported extensions manually. If you are targeting only hardware that supports
#       DXR tier 1.1 you can remove the compile options below. However, some GPUs are locked out in this case (e.g. GTX 10x series with compute emulation for ray-tracing features).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    RAYTRACING "shaders/raytracing_gen.hlsl" "shaders/raytracing_hit.hlsl" "shaders/raytracing_miss.hlsl"
    INCLUDES   "shaders/raytracing_common.hlsli"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    COMPILE_OPTIONS "-disable-payload-qualifiers"
    SPIRV_COMPILE_OPTIONS "-fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_EXT_descriptor_indexing"
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)

# Define assets.
TARGET_ADD_ASSET_DIRECTORY(${PROJECT_NAME}
    NAME "assets"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ASSETS "assets/rt_skybox.jpg"
)
