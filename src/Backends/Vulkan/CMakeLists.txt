###################################################################################################
#####                                                                                         #####
#####             LiteFX.Backends.Vulkan - Contains the Vulkan rendering backend.             #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Backends.Vulkan VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Backends.Vulkan...")

# Install packages.
MESSAGE (STATUS "Installing package dependencies...")
VCPKG_INSTALL (glm glad vulkan)

# Resolve package dependencies.
FIND_PACKAGE(glm CONFIG REQUIRED)
FIND_PACKAGE(glad CONFIG REQUIRED)
FIND_PACKAGE(Vulkan REQUIRED)

# Collect header & source files.
SET(VULKAN_BACKEND_HEADERS
    "include/litefx/backends/vulkan.hpp"
)

SET(VULKAN_BACKEND_SOURCES
    "src/convert.cpp"
    "src/backend.cpp"
    "src/adaper.cpp"
    "src/device.cpp"
    "src/queue.cpp"
    "src/surface.cpp"
    "src/swapchain.cpp"
    "src/texture.cpp"
)

# Add shared library project.
ADD_LIBRARY(LiteFX.Backends.Vulkan SHARED
    ${VULKAN_BACKEND_HEADERS}
    ${VULKAN_BACKEND_SOURCES}
)

ADD_DEPENDENCIES(LiteFX.Backends.Vulkan LiteFX.Core LiteFX.Rendering)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_BACKEND_HEADERS} ${VULKAN_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Backends.Vulkan PROPERTIES
    FOLDER "Backends"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Backends.Vulkan 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Backends.Vulkan
    PUBLIC LiteFX.Core LiteFX.Rendering
    PRIVATE glm glad::glad Vulkan::Vulkan
)

# Setup installer.
INSTALL(TARGETS LiteFX.Backends.Vulkan EXPORT LiteFXBackendsVulkanConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXBackendsVulkanConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Backends.Vulkan FILE LiteFXBackendsVulkanConfig.cmake)