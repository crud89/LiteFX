###################################################################################################
#####                                                                                         #####
#####          LiteFX.Samples.BasicRendering - Contains the basic rendering sample.           #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.BasicRendering VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_BASIC_RENDERING_HEADERS
    "src/sample.h"
)

SET(SAMPLE_BASIC_RENDERING_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_BASIC_RENDERING_HEADERS}
    SOURCES ${SAMPLE_BASIC_RENDERING_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    SOURCE "shaders/basic.hlsl"
    VERTEX VSMain
    PIXEL  PSMain
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
