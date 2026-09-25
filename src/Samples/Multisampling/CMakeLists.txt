###################################################################################################
#####                                                                                         #####
#####           LiteFX.Samples.Multisampling - Contains the multisampling sample.             #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.Multisampling VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_MULTISAMPLING_HEADERS
    "src/sample.h"
)

SET(SAMPLE_MULTISAMPLING_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_MULTISAMPLING_HEADERS}
    SOURCES ${SAMPLE_MULTISAMPLING_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/multisampling_vs.hlsl"
    PIXEL  "shaders/multisampling_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
