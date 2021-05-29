###################################################################################################
#####                                                                                         #####
#####             LiteFX - An extensible computer graphics and rendering engine.              #####
#####                                                                                         #####
###################################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

SET(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)		# Install vcpkg dependencies automatically (experimental - might be changed or removed later; see: https://github.com/microsoft/vcpkg/issues/1653). 
SET(VCPKG_MODULE_DIRECTORY "${CMAKE_SOURCE_DIR}/modules/vcpkg" CACHE STRING "VCPKG module directory." FORCE)
SET(CMAKE_TOOLCHAIN_FILE "${VCPKG_MODULE_DIRECTORY}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "VCPKG toolchain file." FORCE)

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

# Include samples.
IF(BUILD_EXAMPLES AND BUILD_VULKAN_BACKEND AND BUILD_WITH_GLM)
    ADD_SUBDIRECTORY(Samples/Vulkan/BasicRendering)
    ADD_SUBDIRECTORY(Samples/Vulkan/Textures)
    #ADD_SUBDIRECTORY(Samples/Vulkan/DeferredShading)
ENDIF(BUILD_EXAMPLES AND BUILD_VULKAN_BACKEND AND BUILD_WITH_GLM)

IF(BUILD_EXAMPLES AND BUILD_DIRECTX_12_BACKEND AND BUILD_WITH_DIRECTX_MATH)
    ADD_SUBDIRECTORY(Samples/DirectX12/BasicRendering)
ENDIF(BUILD_EXAMPLES AND BUILD_DIRECTX_12_BACKEND AND BUILD_WITH_DIRECTX_MATH)