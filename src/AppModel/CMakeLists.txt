###################################################################################################
#####                                                                                         #####
#####           LiteFX.AppModel - Contains the application base model definition.             #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.AppModel VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Collect header & source files.
SET(APP_MODEL_HEADERS
    "include/litefx/app.hpp"
    "include/litefx/app_api.hpp"
    "include/litefx/app_formatters.hpp"
)

SET(APP_MODEL_SOURCES
    "src/app.cpp"
    "src/appversion.cpp"
 )

ADD_LIBRARY(${PROJECT_NAME}
    ${APP_MODEL_HEADERS}
    ${APP_MODEL_SOURCES}

    ".natvis"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${APP_MODEL_HEADERS} ${APP_MODEL_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    PUBLIC_HEADER "${APP_MODEL_HEADERS}"
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Debug"
    MAP_IMPORTED_CONFIG_MINSIZEREL "Release"
)

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX.Core LiteFX.Logging
)

# Pre-define export specifier, to prevent dllimport/dllexport from being be emitted.
IF(NOT BUILD_SHARED_LIBS)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC -DLITEFX_APPMODEL_API=)
ENDIF(NOT BUILD_SHARED_LIBS)

# Re-use pre-compiled core header.
IF(LITEFX_BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(LITEFX_BUILD_PRECOMPILED_HEADERS)

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
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXAppModelConfig.cmake)