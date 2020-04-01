###################################################################################################
#####                                                                                         #####
#####                LiteFX.Rendering - Contains the rendering infrastructure.                #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Rendering VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Rendering...")

# Collect header & source files.
SET(RENDERING_HEADERS
    "include/litefx/rendering.hpp"
)

SET(RENDERING_SOURCES
    "src/backend.cpp"
)

# Undefine min/max macros by default - in favor of std::min/std::max from <cmath>.
ADD_DEFINITIONS(-DNOMINMAX)

# Add shared library project.
ADD_LIBRARY(LiteFX.Rendering SHARED
    ${RENDERING_HEADERS}
    ${RENDERING_SOURCES}
)

ADD_DEPENDENCIES(LiteFX.Rendering LiteFX.Core)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RENDERING_HEADERS} ${RENDERING_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Rendering PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Rendering 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Rendering LiteFX.Core)

# Setup installer.
INSTALL(TARGETS LiteFX.Rendering EXPORT LiteFXRenderingConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXRenderingConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Rendering FILE LiteFXRenderingConfig.cmake)