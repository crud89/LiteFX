###################################################################################################
#####                                                                                         #####
#####           LiteFX.Samples.PushConstants - Contains the push constants sample.            #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.PushConstants VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(SAMPLE_PUSH_CONSTANTS_HEADERS
    "src/sample.h"
)

SET(SAMPLE_PUSH_CONSTANTS_SOURCES
    "src/main.cpp"
    "src/sample.cpp"
)

# Define sample executable.
DEFINE_SAMPLE(${PROJECT_NAME}
    HEADERS ${SAMPLE_PUSH_CONSTANTS_HEADERS}
    SOURCES ${SAMPLE_PUSH_CONSTANTS_SOURCES}
)

IF(LITEFX_BUILD_VULKAN_BACKEND)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.VS SOURCE "shaders/push_constants_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.FS SOURCE "shaders/push_constants_fs.hlsl" LANGUAGE HLSL TYPE FRAGMENT COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.VS PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.FS PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    
    TARGET_LINK_SHADERS(${PROJECT_NAME} 
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Vk.Shaders.VS ${PROJECT_NAME}.Vk.Shaders.FS
    )
ENDIF(LITEFX_BUILD_VULKAN_BACKEND)

IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.VS SOURCE "shaders/push_constants_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.PS SOURCE "shaders/push_constants_fs.hlsl" LANGUAGE HLSL TYPE PIXEL    COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.VS PROPERTIES FOLDER "Samples/Shaders/DirectX 12")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.PS PROPERTIES FOLDER "Samples/Shaders/DirectX 12")

    TARGET_LINK_SHADERS(${PROJECT_NAME} 
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Dx.Shaders.VS ${PROJECT_NAME}.Dx.Shaders.PS
    )
ENDIF(LITEFX_BUILD_DIRECTX_12_BACKEND)
