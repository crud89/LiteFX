###################################################################################################
#####                                                                                         #####
#####               LiteFX.Core - Defines the common includes for the project.                #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Core VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Resolve package dependencies.
FIND_PACKAGE(fmt CONFIG REQUIRED)

CONFIGURE_FILE("config.tmpl" "${CMAKE_CURRENT_BINARY_DIR}/include/litefx/config.h")
CONFIGURE_FILE("version.tmpl" "${CMAKE_CURRENT_BINARY_DIR}/include/litefx/version.h")
CONFIGURE_FILE("core.tmpl" "${CMAKE_CURRENT_BINARY_DIR}/include/litefx/core.h")

# Collect header & source files.
SET(CORE_HEADERS
    "include/litefx/litefx.h"
    "include/litefx/common.h"
)

SET(CORE_SOURCES
    "src/generator.hpp"
)

SET(CORE_MODULES
    "modules/core.ixx"
    "modules/alias.ixx"
    "modules/string.ixx"
    "modules/concepts.ixx"
    "modules/exceptions.ixx"
    "modules/enumerable.ixx"
    "modules/pimpl.ixx"
    "modules/builder.ixx"
    "modules/resource.ixx"
)

ADD_LIBRARY(${PROJECT_NAME} SHARED
    ${CORE_HEADERS}
    ${CORE_SOURCES}
    "${CMAKE_CURRENT_BINARY_DIR}/include/litefx/config.h"
    "${CMAKE_CURRENT_BINARY_DIR}/include/litefx/version.h"
    "${CMAKE_CURRENT_BINARY_DIR}/include/litefx/core.h"
)

TARGET_SOURCES(${PROJECT_NAME} 
    PUBLIC
        FILE_SET core_modules TYPE CXX_MODULES
        FILES ${CORE_MODULES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CORE_HEADERS} ${CORE_MODULES} ${CORE_SOURCES})

# Setup project properties. Relevant properties:
# - FOLDER: IDE folder (if supported).
# - VERSION/SOVERSION: The current build version.
# - DEFINE_SYMBOL: Disables the "_EXPORTS" define, which would conflict with the precompiled header.
# - PUBLIC_HEADER: Specifies a list of headers to copy during install.
# Do not export
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    DEFINE_SYMBOL ""
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Debug"
    MAP_IMPORTED_CONFIG_MINSIZEREL "Release"
    PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/litefx/config.h;${CMAKE_CURRENT_BINARY_DIR}/include/litefx/version.h;${CMAKE_CURRENT_BINARY_DIR}/include/litefx/core.h"
)

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC fmt::fmt)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT EXPORT LiteFX
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}/litefx/
    FILE_SET core_modules DESTINATION ${CMAKE_INSTALL_MODULE_DIR}
    CXX_MODULES_BMI DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}/modules
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

# Export config.
INSTALL(EXPORT LiteFX DESTINATION ${CMAKE_INSTALL_EXPORT_DIR} CXX_MODULES_DIRECTORY ${CMAKE_INSTALL_EXPORT_DIR}/modules)
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXCoreConfig.cmake)