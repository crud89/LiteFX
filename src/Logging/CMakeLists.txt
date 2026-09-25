###################################################################################################
#####                                                                                         #####
#####                LiteFX.Logging - Contains common logging infrastructure.                 #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Logging VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Resolve package dependencies.
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
    "src/termination.cpp"
)

# Add library project.
LITEFX_ADD_LIBRARY(${PROJECT_NAME}
    EXPORT_NAME Logging
    SOURCES ${LOGGING_SOURCES}
    HEADERS ${LOGGING_HEADERS}
    EXPORT_MACRO LITEFX_LOGGING_API
    EXPORT_HEADER "litefx/logging_export.h"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LOGGING_HEADERS} ${LOGGING_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX::Core spdlog::spdlog
)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFX FILE_SET HEADERS)
