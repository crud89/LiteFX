###################################################################################################
#####                                                                                         #####
#####          LiteFX.Samples.Bindless - Contains the bind-less descriptors sample.           #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.Bindless VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_BINDLESS_HEADERS
    "src/sample.h"
)

SET(SAMPLE_BINDLESS_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_BINDLESS_HEADERS}
    SOURCES ${SAMPLE_BINDLESS_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/bindless_vs.hlsl"
    PIXEL  "shaders/bindless_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)