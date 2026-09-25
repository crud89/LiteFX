###################################################################################################
#####                                                                                         #####
#####             LiteFX.Samples.MeshShader - Contains the mesh shaders sample.               #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.MeshShader VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_MESH_SHADER_HEADERS
    "src/sample.h"
)

SET(SAMPLE_MESH_SHADER_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_MESH_SHADER_HEADERS}
    SOURCES ${SAMPLE_MESH_SHADER_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    TASK   "shaders/mesh_shader_ts.hlsl"
    MESH   "shaders/mesh_shader_ms.hlsl"
    PIXEL  "shaders/mesh_shader_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
