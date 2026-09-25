###################################################################################################
#####                                                                                         #####
#####              LiteFX.Samples.Compute - Contains the compute shader sample.               #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.Compute VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_COMPUTE_HEADERS
    "src/sample.h"
)

SET(SAMPLE_COMPUTE_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_COMPUTE_HEADERS}
    SOURCES ${SAMPLE_COMPUTE_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/compute_geom_vs.hlsl"
    PIXEL  "shaders/compute_geom_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)

TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/compute_present_vs.hlsl"
    PIXEL  "shaders/compute_present_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)

TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    COMPUTE "shaders/compute_lum_cs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
