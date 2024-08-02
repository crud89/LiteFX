###################################################################################################
#####                                                                                         #####
#####                     LiteFX.Math - Contains the base math library.                       #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Math VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: ${PROJECT_NAME}...")

# Find optional dependencies.
SET(DEPENDENCY_PACKAGES "")

IF(LITEFX_BUILD_WITH_GLM)
    FIND_PACKAGE(glm CONFIG REQUIRED)
    LIST(APPEND DEPENDENCY_PACKAGES "glm::glm")
ENDIF(LITEFX_BUILD_WITH_GLM)

IF(LITEFX_BUILD_WITH_DIRECTX_MATH)
    FIND_PACKAGE(directxmath CONFIG REQUIRED)
    LIST(APPEND DEPENDENCY_PACKAGES "Microsoft::DirectXMath")
ENDIF(LITEFX_BUILD_WITH_DIRECTX_MATH)

# Collect header & source files.
SET(VULKAN_MATH_HEADERS
    "include/litefx/vector.hpp"
    "include/litefx/matrix.hpp"
    "include/litefx/math.hpp"
)

SET(VULKAN_MATH_SOURCES
    "src/vector.cpp"
    "src/matrix.cpp"
    "src/size.cpp"
    "src/rect.cpp"
)

# Add shared library project.
ADD_LIBRARY(${PROJECT_NAME}
    ${VULKAN_MATH_HEADERS}
    ${VULKAN_MATH_SOURCES}

    ".natvis"
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_MATH_HEADERS} ${VULKAN_MATH_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
    PUBLIC_HEADER "${VULKAN_MATH_HEADERS}"
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
    PUBLIC LiteFX.Core ${DEPENDENCY_PACKAGES}
)

# Pre-define export specifier, to prevent dllimport/dllexport from being be emitted.
IF(NOT BUILD_SHARED_LIBS)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC -DLITEFX_MATH_API=)
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
EXPORT(TARGETS ${PROJECT_NAME} FILE LiteFXMathConfig.cmake)