###################################################################################################
#####                                                                                         #####
#####          LiteFX.Samples.BasicRendering - Contains the basic rendering sample.           #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Samples.BasicRendering VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Samples.BasicRendering...")

IF(NOT BUILD_VULKAN_BACKEND)
    MESSAGE(FATAL_ERROR "The basic rendering sample requires the Vulkan backend. Set the BUILD_VULKAN_BACKEND option to ON and retry.")
ENDIF(NOT BUILD_VULKAN_BACKEND)

# Install packages.
MESSAGE (STATUS "Installing package dependencies...")
VCPKG_INSTALL (glm glad vulkan glfw3)

# Resolve package dependencies.
FIND_PACKAGE(glm CONFIG REQUIRED)
FIND_PACKAGE(glad CONFIG REQUIRED)
FIND_PACKAGE(Vulkan REQUIRED)
FIND_PACKAGE(glfw3 CONFIG REQUIRED)

# Collect header & source files.
SET(SAMPLE_BASIC_RENDERING_HEADERS
    "src/sampleapp.h"
)

SET(SAMPLE_BASIC_RENDERING_SOURCES
    "src/main.cpp"
    "src/sampleapp.cpp"
)

# Undefine min/max macros by default - in favor of std::min/std::max from <cmath>.
ADD_DEFINITIONS(-DNOMINMAX)

# Add shared library project.
ADD_EXECUTABLE(LiteFX.Samples.BasicRendering 
    ${SAMPLE_BASIC_RENDERING_HEADERS}
    ${SAMPLE_BASIC_RENDERING_SOURCES}
)

ADD_DEPENDENCIES(LiteFX.Samples.BasicRendering LiteFX.Core LiteFX.Rendering LiteFX.Backends.Vulkan)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_BACKEND_HEADERS} ${VULKAN_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Samples.BasicRendering PROPERTIES
    FOLDER "Samples"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Samples.BasicRendering PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Samples.BasicRendering PRIVATE glm glad::glad glfw Vulkan::Vulkan LiteFX.Core LiteFX.Rendering LiteFX.Backends.Vulkan)

# Setup installer.
INSTALL(TARGETS LiteFX.Samples.BasicRendering EXPORT LiteFXSamplesBasicRenderingConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

#INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXSamplesBasicRenderingConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Samples.BasicRendering FILE LiteFXSamplesBasicRenderingConfig.cmake)