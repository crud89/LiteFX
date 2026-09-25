###################################################################################################
#####                                                                                         #####
#####          LiteFX.Samples.Multithreading - Contains the multi-threading sample.           #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.Multithreading VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_MULTITHREADING_HEADERS
    "src/sample.h"
)

SET(SAMPLE_MULTITHREADING_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_MULTITHREADING_HEADERS}
    SOURCES ${SAMPLE_MULTITHREADING_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/multithreading_vs.hlsl"
    PIXEL  "shaders/multithreading_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
