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

IF(LITEFX_BUILD_VULKAN_BACKEND)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.VS SOURCE "shaders/imgui_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.FS SOURCE "shaders/imgui_fs.hlsl" LANGUAGE HLSL TYPE FRAGMENT COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.VS PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.FS PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    
    TARGET_LINK_SHADERS(${PROJECT_NAME} 
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Vk.Shaders.VS ${PROJECT_NAME}.Vk.Shaders.FS
    )
ENDIF(LITEFX_BUILD_VULKAN_BACKEND)

IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.VS SOURCE "shaders/imgui_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.PS SOURCE "shaders/imgui_fs.hlsl" LANGUAGE HLSL TYPE PIXEL    COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.VS PROPERTIES FOLDER "Samples/Shaders/DirectX 12")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.PS PROPERTIES FOLDER "Samples/Shaders/DirectX 12")

    TARGET_LINK_SHADERS(${PROJECT_NAME} 
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Dx.Shaders.VS ${PROJECT_NAME}.Dx.Shaders.PS
    )
ENDIF(LITEFX_BUILD_DIRECTX_12_BACKEND)
