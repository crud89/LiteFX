###################################################################################################
#####                                                                                         #####
#####               LiteFX.Core - Defines the common includes for the project.                #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Core VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Core...")

CONFIGURE_FILE("config.tmpl" "${CMAKE_CURRENT_SOURCE_DIR}/include/litefx/config.h")
CONFIGURE_FILE("version.tmpl" "${CMAKE_CURRENT_SOURCE_DIR}/include/litefx/version.h")
CONFIGURE_FILE("core.tmpl" "${CMAKE_CURRENT_SOURCE_DIR}/include/litefx/core.h")

# Collect header & source files.
SET(CORE_HEADERS
    "include/litefx/containers.hpp"
    "include/litefx/core.h"
    "include/litefx/config.h"
    "include/litefx/version.h"
    "include/litefx/litefx.h"
)

SET(CORE_SOURCES
    "src/core.cpp"
 )

ADD_LIBRARY(LiteFX.Core SHARED
    ${CORE_HEADERS}
    ${CORE_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CORE_HEADERS} ${CORE_SOURCES})

# Setup project properties. Relevant properties:
# - FOLDER: IDE folder (if supported).
# - VERSION/SOVERSION: The current build version.
# - DEFINE_SYMBOL: Disables the "_EXPORTS" define, which would conflict with the precompiled header.
# Do not export
SET_TARGET_PROPERTIES(LiteFX.Core PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    DEFINE_SYMBOL ""
)

TARGET_INCLUDE_DIRECTORIES(LiteFX.Core
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

TARGET_PRECOMPILE_HEADERS(LiteFX.Core 
    PRIVATE 
        <litefx/core.h>
)

# Setup installer.
INSTALL(TARGETS LiteFX.Core EXPORT LiteFXCoreConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXCoreConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Core FILE LiteFXCoreConfig.cmake)