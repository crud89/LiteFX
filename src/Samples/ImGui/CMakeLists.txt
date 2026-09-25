###################################################################################################
#####                                                                                         #####
#####         LiteFX.Samples.ImGui - Contains an example for integrating Dear ImGui.          #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.ImGui VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Resolve package dependencies.
FIND_PACKAGE(imgui CONFIG REQUIRED)

# Collect header & source files.
SET(SAMPLE_IMGUI_HEADERS
    "src/sample.h"
)

SET(SAMPLE_IMGUI_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_IMGUI_HEADERS}
    SOURCES ${SAMPLE_IMGUI_SOURCES}
    LIBRARIES imgui::imgui
)

# Define shader program(s).
TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
    VERTEX "shaders/imgui_vs.hlsl"
    PIXEL  "shaders/imgui_fs.hlsl"
    SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
    FOLDER "Samples/Shaders"
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
)
