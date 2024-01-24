###################################################################################################
#####                                                                                         #####
#####             LiteFX - An extensible computer graphics and rendering engine.              #####
#####                                                                                         #####
###################################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

# Check if the build dir does not match the source dir (disallow in-source builds).
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   MESSAGE(SEND_ERROR "In-source builds are not allowed.")
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

# Setup modules.
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/modules.cmake")

# Declare top-level project.
PROJECT(LiteFX LANGUAGES CXX)

# Include configuration files.
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/version.cmake")
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/options.cmake")
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/config.cmake") 
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/shaders.cmake")
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/assets.cmake")

# Include sub-projects.
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(Logging)
ADD_SUBDIRECTORY(AppModel)
ADD_SUBDIRECTORY(Math)
ADD_SUBDIRECTORY(Graphics)
ADD_SUBDIRECTORY(Rendering)

# Include optional modules.
IF(LITEFX_BUILD_VULKAN_BACKEND)
    ADD_SUBDIRECTORY(Backends/Vulkan)
ENDIF(LITEFX_BUILD_VULKAN_BACKEND)

IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    ADD_SUBDIRECTORY(Backends/DirectX12)
ENDIF(LITEFX_BUILD_DIRECTX_12_BACKEND)

IF(LITEFX_BUILD_EXAMPLES AND NOT LITEFX_BUILD_DEFINE_BUILDERS)
    MESSAGE(WARNING "Examples require backend builders to be defined in order to compile them properly.")
ENDIF(LITEFX_BUILD_EXAMPLES AND NOT LITEFX_BUILD_DEFINE_BUILDERS)

# Include samples.
IF(LITEFX_BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(Samples/BasicRendering)
    ADD_SUBDIRECTORY(Samples/RenderPasses)
    ADD_SUBDIRECTORY(Samples/PushConstants)
    ADD_SUBDIRECTORY(Samples/UniformArrays)
    ADD_SUBDIRECTORY(Samples/Textures)
    ADD_SUBDIRECTORY(Samples/Multisampling)
    ADD_SUBDIRECTORY(Samples/Multithreading)
    ADD_SUBDIRECTORY(Samples/Bindless)
    ADD_SUBDIRECTORY(Samples/Compute)
    ADD_SUBDIRECTORY(Samples/Indirect)

    IF(LITEFX_BUILD_MESH_SHADER_SUPPORT)
        ADD_SUBDIRECTORY(Samples/MeshShader)
    ENDIF(LITEFX_BUILD_MESH_SHADER_SUPPORT)
ENDIF(LITEFX_BUILD_EXAMPLES)

# Include tests.
IF(LITEFX_BUILD_TESTS)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(Tests)
ENDIF(LITEFX_BUILD_TESTS)

# Install license notice.
INSTALL(FILES "${CMAKE_SOURCE_DIR}/../LICENSE" "${CMAKE_SOURCE_DIR}/../NOTICE" DESTINATION "/")