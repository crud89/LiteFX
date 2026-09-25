###################################################################################################
#####                                                                                         #####
#####              LiteFX.Samples.Rayqueries - Contains the ray queries sample.               #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.RayQueries VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_RAY_QUERIES_HEADERS
    "src/sample.h"
)

SET(SAMPLE_RAY_QUERIES_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_RAY_QUERIES_HEADERS}
    SOURCES ${SAMPLE_RAY_QUERIES_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/rayqueries_vs.hlsl"
    PIXEL  "shaders/rayqueries_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)

# Define assets.
TARGET_ADD_ASSET_DIRECTORY(${PROJECT_NAME}
    NAME "assets"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ASSETS "assets/rq_skybox.jpg"
)
