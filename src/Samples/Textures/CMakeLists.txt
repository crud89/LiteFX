###################################################################################################
#####                                                                                         #####
#####                LiteFX.Samples.Textures - Contains the textures sample.                  #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.Textures VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_TEXTURES_HEADERS
    "src/sample.h"
)

SET(SAMPLE_TEXTURES_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_TEXTURES_HEADERS}
    SOURCES ${SAMPLE_TEXTURES_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/textures_vs.hlsl"
    PIXEL  "shaders/textures_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)

# Define assets.
TARGET_ADD_ASSET_DIRECTORY(${PROJECT_NAME}
    NAME "assets"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ASSETS "assets/logo_quad.tga"
)
