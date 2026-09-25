###################################################################################################
#####                                                                                         #####
#####            LiteFX.Samples.RenderPasses - Contains the render passes sample.             #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.RenderPasses VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_RENDER_PASSES_HEADERS
    "src/sample.h"
)

SET(SAMPLE_RENDER_PASSES_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_RENDER_PASSES_HEADERS}
    SOURCES ${SAMPLE_RENDER_PASSES_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/geometry_pass_vs.hlsl"
    PIXEL  "shaders/geometry_pass_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)

TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/lighting_pass_vs.hlsl"
    PIXEL  "shaders/lighting_pass_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
