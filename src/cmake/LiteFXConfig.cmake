###################################################################################################
#####                                                                                         #####
##### LiteFX package configuration file. This file is automatically created during build and  #####
##### must not be altered!                                                                    #####
#####                                                                                         #####
###################################################################################################

@PACKAGE_INIT@

# Set build variables.
SET(LITEFX_HAS_DIRECTX12_BACKEND @LITEFX_BUILD_DIRECTX_12_BACKEND@)
SET(LITEFX_HAS_VULKAN_BACKEND @LITEFX_BUILD_VULKAN_BACKEND@)
SET(LITEFX_HAS_DIRECTX_MATH @LITEFX_BUILD_WITH_DIRECTX_MATH@)
SET(LITEFX_HAS_GLM @LITEFX_BUILD_WITH_GLM@)
SET(LITEFX_HAS_PIX_RUNTIME @LITEFX_BUILD_WITH_PIX_RUNTIME@)

# Keep track of the imported libraries for convenience.
SET(LITEFX_DEPENDENCIES LiteFX.Core LiteFX.Logging LiteFX.AppModel LiteFX.Math LiteFX.Graphics LiteFX.Rendering)

# Lookup package dependencies.
INCLUDE(CMakeFindDependencyMacro)

# spdlog is always required for logging.
FIND_DEPENDENCY(spdlog CONFIG)

IF(LITEFX_HAS_DIRECTX12_BACKEND)
  LIST(APPEND LITEFX_DEPENDENCIES LiteFX.Backends.DirectX12)

  FIND_DEPENDENCY(directx-headers CONFIG)
  FIND_DEPENDENCY(directx12-agility CONFIG)
  FIND_DEPENDENCY(directx-dxc CONFIG)
  FIND_PACKAGE(D3D12MemoryAllocator CONFIG)
ENDIF(LITEFX_HAS_DIRECTX12_BACKEND)

IF(LITEFX_HAS_VULKAN_BACKEND)
  LIST(APPEND LITEFX_DEPENDENCIES LiteFX.Backends.Vulkan)

  FIND_DEPENDENCY(Vulkan)
  FIND_PACKAGE(VulkanMemoryAllocator CONFIG)
  FIND_PACKAGE(unofficial-spirv-reflect CONFIG)
ENDIF(LITEFX_HAS_VULKAN_BACKEND)

IF(LITEFX_HAS_PIX_RUNTIME)
    LIST(APPEND LITEFX_DEPENDENCIES Microsoft::WinPixEventRuntime)

    FIND_DEPENDENCY(winpixevent CONFIG)
ENDIF(LITEFX_HAS_PIX_RUNTIME)

IF(LITEFX_HAS_DIRECTX_MATH)
  FIND_DEPENDENCY(directxmath CONFIG)
ENDIF(LITEFX_HAS_DIRECTX_MATH)

IF(LITEFX_HAS_GLM)
  FIND_DEPENDENCY(glm CONFIG)
ENDIF(LITEFX_HAS_GLM)

FIND_PROGRAM(LITEFX_BUILD_GLSLC_COMPILER glslc HINTS ENV VULKAN_SDK PATH_SUFFIXES bin DOC "The full path to the `glslc.exe` shader compiler binary.")
FIND_PROGRAM(LITEFX_BUILD_DXC_COMPILER dxc HINTS ENV VULKAN_SDK PATH_SUFFIXES bin DOC "The full path to the `dxc.exe` shader compiler binary.")

# Include target export configuration.
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/LiteFX.cmake")

# Also include helper scripts here. Simply define LITEFX_WITHOUT_HELPERS, to prevent those scripts from being imported.
IF(NOT DEFINED LITEFX_WITHOUT_HELPERS)
  INCLUDE("${CMAKE_CURRENT_LIST_DIR}/Assets.cmake")
  INCLUDE("${CMAKE_CURRENT_LIST_DIR}/Shaders.cmake")
ENDIF(NOT DEFINED LITEFX_WITHOUT_HELPERS)

CHECK_REQUIRED_COMPONENTS("@PROJECT_NAME@")