###################################################################################################
#####                                                                                         #####
#####      LiteFX.Samples.DynamicDescriptors - Contains the dynamic descriptors sample.       #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.DynamicDescriptors VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Dynamic descriptors require shader model 6.8 or later.
STRING(REPLACE "_" ";" SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL})
LIST(GET SHADER_MODEL 0 SHADER_MODEL_MAJOR)
LIST(GET SHADER_MODEL 1 SHADER_MODEL_MINOR)
SET(SAMPLE_SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL})

# NOTE: The dynamic descriptors syntax actually only requries SM 6.6, but we use SV_StartInstanceLocation so that we do not have to pass around instance indices.
IF(${SHADER_MODEL_MAJOR} LESS 6 OR (${SHADER_MODEL_MAJOR} EQUAL 6 AND ${SHADER_MODEL_MINOR} LESS 8))
    MESSAGE(WARNING "The dynamic descriptors sample requires shader model 6.8 or later, but ${SHADER_MODEL_MAJOR}.${SHADER_MODEL_MINOR} is selected. The sample will be built with shader model 6.8 and will not work if your system does not support it.")
    SET(SAMPLE_SHADER_MODEL "6_8")
ENDIF(${SHADER_MODEL_MAJOR} LESS 6 OR (${SHADER_MODEL_MAJOR} EQUAL 6 AND ${SHADER_MODEL_MINOR} LESS 8))

# Collect header & source files.
SET(SAMPLE_DYNAMIC_DESCRIPTORS_HEADERS
    "src/sample.h"
)

SET(SAMPLE_DYNAMIC_DESCRIPTORS_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_DYNAMIC_DESCRIPTORS_HEADERS}
    SOURCES ${SAMPLE_DYNAMIC_DESCRIPTORS_SOURCES}
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/dynamic_descriptors_vs.hlsl"
    PIXEL  "shaders/dynamic_descriptors_fs.hlsl"
    SHADER_MODEL ${SAMPLE_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
