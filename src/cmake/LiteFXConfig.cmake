###################################################################################################
#####                                                                                         #####
##### LiteFX package configuration file. This file is automatically created during build and  #####
##### must not be altered!                                                                    #####
#####                                                                                         #####
###################################################################################################

@PACKAGE_INIT@

# Set build variables.
SET(LITEFX_HAS_DIRECTX12_BACKEND @BUILD_DIRECTX_12_BACKEND@)
SET(LITEFX_HAS_VULKAN_BACKEND @BUILD_VULKAN_BACKEND@)
SET(LITEFX_HAS_DIRECTX_MATH @BUILD_WITH_DIRECTX_MATH@)
SET(LITEFX_HAS_GLM @BUILD_WITH_GLM@)

# Keep track of the imported libraries for convenience.
SET(LITEFX_DEPENDENCIES fmt::fmt spdlog::spdlog LiteFX.Core LiteFX.Logging LiteFX.AppModel LiteFX.Math LiteFX.Graphics LiteFX.Rendering)

# Lookup package dependencies.
INCLUDE(CMakeFindDependencyMacro)

IF(LITEFX_HAS_DIRECTX12_BACKEND)
  LIST(APPEND LITEFX_DEPENDENCIES LiteFX.Backends.DirectX12)

  FIND_DEPENDENCY(directx-headers CONFIG)
  FIND_PACKAGE(d3d12-memory-allocator CONFIG)
ENDIF(LITEFX_HAS_DIRECTX12_BACKEND)

IF(LITEFX_HAS_VULKAN_BACKEND)
  LIST(APPEND LITEFX_DEPENDENCIES LiteFX.Backends.Vulkan)

  FIND_DEPENDENCY(Vulkan)
  FIND_PACKAGE(unofficial-vulkan-memory-allocator CONFIG)
ENDIF(LITEFX_HAS_VULKAN_BACKEND)

IF(LITEFX_HAS_DIRECTX_MATH)
  FIND_DEPENDENCY(directxmath CONFIG)
ENDIF(LITEFX_HAS_DIRECTX_MATH)

IF(LITEFX_HAS_GLM)
  FIND_DEPENDENCY(glm CONFIG)
ENDIF(LITEFX_HAS_GLM)

FIND_DEPENDENCY(fmt CONFIG)
FIND_DEPENDENCY(spdlog CONFIG)

FIND_PROGRAM(BUILD_GLSLC_COMPILER glslc PATHS $ENV{VULKAN_SDK}/bin)
FIND_PROGRAM(BUILD_DXC_COMPILER dxc PATHS $ENV{VULKAN_SDK}/bin NO_DEFAULT_PATH)

IF(NOT BUILD_DXC_COMPILER)
	FIND_PROGRAM(BUILD_DXC_COMPILER dxc)
ENDIF(NOT BUILD_DXC_COMPILER)

# Include target export configuration.
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/LiteFX.cmake")

# Also include helper scripts here. Simply define LITEFX_WITHOUT_HELPERS, to prevent those scripts from being imported.
IF(NOT DEFINED LITEFX_WITHOUT_HELPERS)
  INCLUDE("${CMAKE_CURRENT_LIST_DIR}/Assets.cmake")
  INCLUDE("${CMAKE_CURRENT_LIST_DIR}/Shaders.cmake")
ENDIF(NOT DEFINED LITEFX_WITHOUT_HELPERS)

CHECK_REQUIRED_COMPONENTS("@PROJECT_NAME@")