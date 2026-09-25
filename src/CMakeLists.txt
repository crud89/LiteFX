###################################################################################################
#####                                                                                         #####
#####             LiteFX - An extensible computer graphics and rendering engine.              #####
#####                                                                                         #####
###################################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 4.0)

# Check if the build dir does not match the source dir (disallow in-source builds).
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   MESSAGE(SEND_ERROR "In-source builds are not allowed.")
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

# Include version.
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.cmake")

# Declare top-level project.
PROJECT(LiteFX VERSION ${LITEFX_VERSION} LANGUAGES CXX)

# Include configuration files.
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules.cmake")
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Options.cmake")
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake")
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Runtime.cmake")
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Shaders.cmake")
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Assets.cmake")

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
    ADD_SUBDIRECTORY(Samples)
ENDIF(LITEFX_BUILD_EXAMPLES)

# Include tests.
IF(LITEFX_BUILD_TESTS)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(Tests)
ENDIF(LITEFX_BUILD_TESTS)

# Install license notice.
INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE" "${CMAKE_CURRENT_SOURCE_DIR}/../NOTICE" DESTINATION ".")

# Export package config.
INSTALL(EXPORT LiteFX FILE LiteFX.cmake NAMESPACE LiteFX:: DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})