###################################################################################################
#####                                                                                         #####
#####                     LiteFX.Math - Contains the base math library.                       #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Math VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Math...")

# Install packages.
MESSAGE(STATUS "Installing package dependencies...")

SET(PRIVATE_LINK_LIBS "")

IF(BUILD_ENABLE_GLM)
    VCPKG_INSTALL(glm)
    FIND_PACKAGE(glm CONFIG REQUIRED)
    LIST(APPEND PRIVATE_LINK_LIBS "glm")
ENDIF(BUILD_ENABLE_GLM)

# Collect header & source files.
SET(VULKAN_MATH_HEADERS
    "include/litefx/math_types.hpp"
)

SET(VULKAN_MATH_SOURCES
    "src/vector.cpp"
)

# Add shared library project.
ADD_LIBRARY(LiteFX.Math SHARED
    ${VULKAN_MATH_HEADERS}
    ${VULKAN_MATH_SOURCES}
)

ADD_DEPENDENCIES(LiteFX.Math LiteFX.Core)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_MATH_HEADERS} ${VULKAN_MATH_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Math PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Math 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Math
    PUBLIC LiteFX.Core
    PRIVATE ${PRIVATE_LINK_LIBS}
)

TARGET_PRECOMPILE_HEADERS(LiteFX.Math REUSE_FROM LiteFX.Core)

# Setup installer.
INSTALL(TARGETS LiteFX.Math EXPORT LiteFXMathConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXMathConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Math FILE LiteFXMathConfig.cmake)