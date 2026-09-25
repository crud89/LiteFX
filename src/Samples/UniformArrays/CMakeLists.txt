###################################################################################################
#####                                                                                         #####
#####           LiteFX.Samples.UniformArrays - Contains the uniform arrays sample.            #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.UniformArrays VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_UNIFORM_ARRAYS_HEADERS
    "src/sample.h"
)

SET(SAMPLE_UNIFORM_ARRAYS_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_UNIFORM_ARRAYS_HEADERS}
    SOURCES ${SAMPLE_UNIFORM_ARRAYS_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/ubo_array_vs.hlsl"
    PIXEL  "shaders/ubo_array_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
