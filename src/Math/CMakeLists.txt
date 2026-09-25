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
SET(MATH_HEADERS
    "include/litefx/vector.hpp"
    "include/litefx/matrix.hpp"
    "include/litefx/math.hpp"
)

SET(MATH_SOURCES
    "src/vector.cpp"
    "src/matrix.cpp"
    "src/size.cpp"
    "src/rect.cpp"
)

# Add library project.
LITEFX_ADD_LIBRARY(${PROJECT_NAME}
    EXPORT_NAME Math
    SOURCES ${MATH_SOURCES}
    HEADERS ${MATH_HEADERS}
    EXPORT_MACRO LITEFX_MATH_API
    EXPORT_HEADER "litefx/math_export.h"
    NATVIS ".natvis"
)

# Force `size_t` as GLM size type.
IF(LITEFX_BUILD_WITH_GLM)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC GLM_FORCE_SIZE_T_LENGTH)
ENDIF(LITEFX_BUILD_WITH_GLM)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MATH_HEADERS} ${MATH_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
    FOLDER "SDK"
    VERSION ${LITEFX_VERSION}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE "src/")

# Link project dependencies.
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    PUBLIC LiteFX::Core ${DEPENDENCY_PACKAGES}
)

# Setup installer.
INSTALL(TARGETS ${PROJECT_NAME} EXPORT LiteFX FILE_SET HEADERS)
