###################################################################################################
#####                                                                                         #####
#####             LiteFX - An extensible computer graphics and rendering engine.              #####
#####                                                                                         #####
###################################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
PROJECT(LiteFX LANGUAGES CXX)

# Check if the build dir does not match the source dir (disallow in-source builds).
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   MESSAGE(SEND_ERROR "In-source builds are not allowed.")
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

# Include configuration files.
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/version.cmake")
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/options.cmake")
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/config.cmake") 
INCLUDE("${CMAKE_SOURCE_DIR}/cmake/modules.cmake")
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
IF(BUILD_VULKAN_BACKEND)
    ADD_SUBDIRECTORY(Backends/Vulkan)
ENDIF(BUILD_VULKAN_BACKEND)

IF(BUILD_DIRECTX_12_BACKEND)
    ADD_SUBDIRECTORY(Backends/DirectX12)
ENDIF(BUILD_DIRECTX_12_BACKEND)

IF(BUILD_EXAMPLES AND NOT BUILD_DEFINE_BUILDERS)
    MESSAGE(WARNING "Examples require backend builders to be defined in order to compile them properly.")
ENDIF(BUILD_EXAMPLES AND NOT BUILD_DEFINE_BUILDERS)

# Include samples.
IF(BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(Samples/BasicRendering)
    ADD_SUBDIRECTORY(Samples/RenderPasses)
    ADD_SUBDIRECTORY(Samples/PushConstants)
    ADD_SUBDIRECTORY(Samples/UniformArrays)
    ADD_SUBDIRECTORY(Samples/Textures)
    ADD_SUBDIRECTORY(Samples/Multisampling)
    ADD_SUBDIRECTORY(Samples/Multithreading)
    ADD_SUBDIRECTORY(Samples/Bindless)
ENDIF(BUILD_EXAMPLES)

# Install license notice.
INSTALL(FILES "${CMAKE_SOURCE_DIR}/../LICENSE" "${CMAKE_SOURCE_DIR}/../NOTICE" DESTINATION "/")