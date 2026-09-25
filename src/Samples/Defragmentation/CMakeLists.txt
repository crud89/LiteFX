###################################################################################################
#####                                                                                         #####
#####          LiteFX.Samples.Defragmentation - Contains the defragmentation sample.          #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.Defragmentation VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_DEFRAGMENTATION_HEADERS
    "src/sample.h"
)

SET(SAMPLE_DEFRAGMENTATION_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_DEFRAGMENTATION_HEADERS}
    SOURCES ${SAMPLE_DEFRAGMENTATION_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/defragmentation_vs.hlsl"
    PIXEL  "shaders/defragmentation_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
