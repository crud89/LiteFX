###################################################################################################
#####                                                                                         #####
#####        LiteFX.Samples.ResourceAliasing - Contains the resource aliasing sample.         #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.ResourceAliasing VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_RESOURCE_ALIASING_HEADERS
    "src/sample.h"
)

SET(SAMPLE_RESOURCE_ALIASING_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_RESOURCE_ALIASING_HEADERS}
    SOURCES ${SAMPLE_RESOURCE_ALIASING_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/ra_pass1_vs.hlsl"
    PIXEL  "shaders/ra_pass1_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)

TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/ra_pass2_vs.hlsl"
    PIXEL  "shaders/ra_pass2_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
