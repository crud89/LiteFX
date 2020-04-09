###################################################################################################
#####                                                                                         #####
#####             LiteFX - An extensible computer graphics and rendering engine.              #####
#####                                                                                         #####
###################################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
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

# Include sub-projects.
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(AppModel)
ADD_SUBDIRECTORY(Math)
ADD_SUBDIRECTORY(Rendering)

# Include optional modules.
IF (BUILD_OPEN_GL_BACKEND)
    ADD_SUBDIRECTORY(Backends/OpenGL)
ENDIF(BUILD_OPEN_GL_BACKEND)

IF (BUILD_VULKAN_BACKEND)
    ADD_SUBDIRECTORY(Backends/Vulkan)
ENDIF(BUILD_VULKAN_BACKEND)

# Include samples.
IF(BUILD_SAMPLE_BASIC_RENDERING)
    add_subdirectory(Samples/BasicRendering)
ENDIF(BUILD_SAMPLE_BASIC_RENDERING)