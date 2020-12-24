###################################################################################################
#####                                                                                         #####
#####         LiteFX.Backends.DirectX12 - Contains the DirectX 12 rendering backend.          #####
#####                                                                                         #####
###################################################################################################

PROJECT(LiteFX.Backends.DirectX12 VERSION ${LITEFX_VERSION} LANGUAGES CXX)
MESSAGE(STATUS "Initializing: LiteFX.Backends.DirectX12...")

# Install packages.
MESSAGE(STATUS "Installing package dependencies...")
#VCPKG_INSTALL()

# Resolve package dependencies.
#FIND_PACKAGE()

# Collect header & source files.
SET(DIRECTX12_BACKEND_HEADERS
    "include/litefx/backends/dx12_api.hpp"
    "include/litefx/backends/dx12_builders.hpp"
    "include/litefx/backends/dx12_formatters.hpp"
    "include/litefx/backends/dx12.hpp"
)

SET(DIRECTX12_BACKEND_SOURCES
    "src/backend.cpp"
    "src/adapter.cpp"
)

# Add shared library project.
ADD_LIBRARY(LiteFX.Backends.DirectX12 SHARED
    ${DIRECTX12_BACKEND_HEADERS}
    ${DIRECTX12_BACKEND_SOURCES}
)

# Create source groups for better code organization.
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DIRECTX12_BACKEND_HEADERS} ${DIRECTX12_BACKEND_SOURCES})

# Setup project properties.
SET_TARGET_PROPERTIES(LiteFX.Backends.DirectX12 PROPERTIES
    FOLDER "Backends"
    VERSION ${LITEFX_VERSION}
    SOVERSION ${LITEFX_YEAR}
)

# Setup target include directories.
TARGET_INCLUDE_DIRECTORIES(LiteFX.Backends.DirectX12 
    PRIVATE 
        "src/"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

# Link project dependencies.
TARGET_LINK_LIBRARIES(LiteFX.Backends.DirectX12
    PUBLIC LiteFX.Core LiteFX.Math LiteFX.Rendering LiteFX.AppModel
    PRIVATE DirectX-Headers DirectX-Guids d3d12 dxcore dxgi
)

# Re-use pre-compiled core header.
IF(BUILD_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS(LiteFX.Backends.DirectX12 REUSE_FROM LiteFX.Core)
ENDIF(BUILD_PRECOMPILED_HEADERS)

# Setup installer.
INSTALL(TARGETS LiteFX.Backends.DirectX12 EXPORT LiteFXConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBRARY_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINARY_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR}
)

INSTALL(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDE_DIR})
INSTALL(EXPORT LiteFXConfig DESTINATION ${CMAKE_INSTALL_EXPORT_DIR})

# Export config.
EXPORT(TARGETS LiteFX.Backends.DirectX12 FILE LiteFXBackendsDirectX12Config.cmake)