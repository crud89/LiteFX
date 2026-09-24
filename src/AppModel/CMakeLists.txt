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

# Add library project.
LITEFX_ADD_LIBRARY(${PROJECT_NAME}
    EXPORT_NAME AppModel
    SOURCES ${APP_MODEL_SOURCES}
    HEADERS ${APP_MODEL_HEADERS}
    EXPORT_MACRO LITEFX_APPMODEL_API
    EXPORT_HEADER "litefx/app_export.h"
    NATVIS ".natvis"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${APP_MODEL_HEADERS} ${APP_MODEL_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX::Core LiteFX::Logging
)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFX FILE_SET HEADERS)
