###################################################################################################
#####                                                                                         #####
#####                  LiteFX.Graphics - Contains graphics data structures.                   #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Graphics VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Graphics...")

# Collect header & source files.
SET(GRAPHICS_HEADERS
    "include/litefx/graphics_api.hpp"
    "include/litefx/graphics_formatters.hpp"
    "include/litefx/graphics.hpp"
)

SET(GRAPHICS_SOURCES
    "src/vertex.cpp"
)

# Add shared library project.
ADD_LIBRARY(LiteFX.Graphics SHARED
    ${GRAPHICS_HEADERS}
    ${GRAPHICS_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GRAPHICS_HEADERS} ${GRAPHICS_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Graphics PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Graphics 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Graphics 
    PUBLIC LiteFX.Core LiteFX.Logging LiteFX.Math
)

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(LiteFX.Graphics REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS LiteFX.Graphics EXPORT LiteFXGraphicsConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXGraphicsConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Graphics FILE LiteFXGraphicsConfig.cmake)