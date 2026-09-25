###################################################################################################
#####                                                                                         #####
#####           LiteFX.Samples.PushConstants - Contains the push constants sample.            #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.PushConstants VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_PUSH_CONSTANTS_HEADERS
    "src/sample.h"
)

SET(SAMPLE_PUSH_CONSTANTS_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_PUSH_CONSTANTS_HEADERS}
    SOURCES ${SAMPLE_PUSH_CONSTANTS_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/push_constants_vs.hlsl"
    PIXEL  "shaders/push_constants_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
