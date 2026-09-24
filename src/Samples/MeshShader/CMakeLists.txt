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

IF(LITEFX_BUILD_VULKAN_BACKEND)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.TS SOURCE "shaders/mesh_shader_ts.hlsl" LANGUAGE HLSL TYPE TASK     COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.MS SOURCE "shaders/mesh_shader_ms.hlsl" LANGUAGE HLSL TYPE MESH     COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Vk.Shaders.FS SOURCE "shaders/mesh_shader_fs.hlsl" LANGUAGE HLSL TYPE FRAGMENT COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.TS PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.MS PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Vk.Shaders.FS PROPERTIES FOLDER "Samples/Shaders/Vulkan")
    
    TARGET_LINK_SHADERS(${PROJECT_NAME} 
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Vk.Shaders.TS ${PROJECT_NAME}.Vk.Shaders.MS ${PROJECT_NAME}.Vk.Shaders.FS
    )
ENDIF(LITEFX_BUILD_VULKAN_BACKEND)

IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.AS SOURCE "shaders/mesh_shader_ts.hlsl" LANGUAGE HLSL TYPE AMPLIFICATION COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.MS SOURCE "shaders/mesh_shader_ms.hlsl" LANGUAGE HLSL TYPE MESH          COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    ADD_SHADER_MODULE(${PROJECT_NAME}.Dx.Shaders.PS SOURCE "shaders/mesh_shader_fs.hlsl" LANGUAGE HLSL TYPE PIXEL         COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.AS PROPERTIES FOLDER "Samples/Shaders/DirectX 12")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.MS PROPERTIES FOLDER "Samples/Shaders/DirectX 12")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}.Dx.Shaders.PS PROPERTIES FOLDER "Samples/Shaders/DirectX 12")

    TARGET_LINK_SHADERS(${PROJECT_NAME} 
        INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
        SHADERS ${PROJECT_NAME}.Dx.Shaders.AS ${PROJECT_NAME}.Dx.Shaders.MS ${PROJECT_NAME}.Dx.Shaders.PS
    )
ENDIF(LITEFX_BUILD_DIRECTX_12_BACKEND)
