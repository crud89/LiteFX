###################################################################################################
#####                                                                                         #####
#####           LiteFX.AppModel - Contains the application base model definition.             #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.AppModel VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.AppModel...")

# Collect header & source files.
SET(APP_MODEL_HEADERS
    "include/litefx/app.hpp"
    "include/litefx/app_api.hpp"
)

SET(APP_MODEL_SOURCES
    "src/app.cpp"
    "src/appversion.cpp"
    "src/app_builder.cpp"
 )

ADD_LIBRARY(LiteFX.AppModel SHARED
    ${APP_MODEL_HEADERS}
    ${APP_MODEL_SOURCES}
)

ADD_DEPENDENCIES(LiteFX.AppModel LiteFX.Core)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${APP_MODEL_HEADERS} ${APP_MODEL_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.AppModel PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

TARGET_INCLUDE_DIRECTORIES(LiteFX.AppModel
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.AppModel
    PUBLIC LiteFX.Core LiteFX.Logging
    PRIVATE ${PRIVATE_LINK_LIBS}
)

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(LiteFX.AppModel REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS LiteFX.AppModel EXPORT LiteFXConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Export config.
EXPORT(TARGETS LiteFX.AppModel FILE LiteFXAppModelConfig.cmake)