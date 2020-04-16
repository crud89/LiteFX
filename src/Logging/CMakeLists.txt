###################################################################################################
#####                                                                                         #####
#####                LiteFX.Logging - Contains common logging infrastructure.                 #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Logging VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Logging...")

# Install packages.
MESSAGE (STATUS "Installing package dependencies...")
VCPKG_INSTALL (spdlog)

# Resolve package dependencies.
# NOTE: spdlog also imports the fmt::fmt target.
FIND_PACKAGE(spdlog CONFIG REQUIRED)

# Collect header & source files.
SET(LOGGING_HEADERS
    "include/litefx/logging.hpp"
)

SET(LOGGING_SOURCES
    "src/logger.cpp"
    "src/logger_factory.cpp"
    "src/console.cpp"
    "src/rolling_file.cpp"
)

# Add shared library project.
ADD_LIBRARY(LiteFX.Logging SHARED
    ${LOGGING_HEADERS}
    ${LOGGING_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LOGGING_HEADERS} ${LOGGING_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Logging PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Logging 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Logging
    PUBLIC LiteFX.Core spdlog::spdlog
    PRIVATE spdlog::spdlog_header_only
)

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(LiteFX.Logging REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS LiteFX.Logging EXPORT LiteFXLoggingConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXLoggingConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Install GLFW
SET (FMT_LOCATION "")
GET_TARGET_PROPERTY(FMT_LOCATION fmt::fmt LOCATION_${CMAKE_BUILD_TYPE})
INSTALL(FILES ${FMT_LOCATION} DESTINATION ${CMAKE_INSTALL_BINARY_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Logging FILE LiteFXLoggingConfig.cmake)