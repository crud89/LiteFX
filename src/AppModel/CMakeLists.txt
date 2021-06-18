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
)

SET(APP_MODEL_SOURCES
    "src/app.cpp"
    "src/appversion.cpp"
 )

ADD_LIBRARY(${PROJECT_NAME} SHARED
    ${APP_MODEL_HEADERS}
    ${APP_MODEL_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${APP_MODEL_HEADERS} ${APP_MODEL_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    PUBLIC_HEADER "${APP_MODEL_HEADERS}"
)

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX.Core LiteFX.Logging
)

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFXConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}/litefx/
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

# Export config.
INSTALL(EXPORT LiteFXConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXAppModelConfig.cmake)