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

# Add shared library project.
ADD_LIBRARY(${PROJECT_NAME} SHARED
    ${LOGGING_HEADERS}
    ${LOGGING_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LOGGING_HEADERS} ${LOGGING_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    PUBLIC_HEADER "${LOGGING_HEADERS}"
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Debug"
    MAP_IMPORTED_CONFIG_MINSIZEREL "Release"
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX.Core spdlog::spdlog
    PRIVATE spdlog::spdlog_header_only
)

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT EXPORT LiteFX
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}/litefx/
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

# Export config.
INSTALL(EXPORT LiteFX DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXLoggingConfig.cmake)