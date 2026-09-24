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

# Expose optional parts as components, so that applications can require them, e.g.:
# FIND_PACKAGE(LiteFX CONFIG REQUIRED COMPONENTS DirectX12)
SET(@PROJECT_NAME@_DirectX12_FOUND ${LITEFX_HAS_DIRECTX12_BACKEND})
SET(@PROJECT_NAME@_Vulkan_FOUND ${LITEFX_HAS_VULKAN_BACKEND})
SET(@PROJECT_NAME@_DirectXMath_FOUND ${LITEFX_HAS_DIRECTX_MATH})
SET(@PROJECT_NAME@_GLM_FOUND ${LITEFX_HAS_GLM})
SET(@PROJECT_NAME@_PIX_FOUND ${LITEFX_HAS_PIX_RUNTIME})

# Keep track of the imported libraries for convenience.
SET(LITEFX_DEPENDENCIES LiteFX::Core LiteFX::Logging LiteFX::AppModel LiteFX::Math LiteFX::Graphics LiteFX::Rendering)

# Lookup package dependencies.
INCLUDE(CMakeFindDependencyMacro)

# spdlog is always required for logging.
FIND_DEPENDENCY(spdlog CONFIG)

IF(LITEFX_HAS_DIRECTX12_BACKEND)
  LIST(APPEND LITEFX_DEPENDENCIES LiteFX::DirectX12)

  FIND_DEPENDENCY(directx-headers CONFIG)
  FIND_DEPENDENCY(directx12-agility CONFIG)
  FIND_DEPENDENCY(directx-dxc CONFIG)
  FIND_DEPENDENCY(D3D12MemoryAllocator CONFIG)
ENDIF(LITEFX_HAS_DIRECTX12_BACKEND)

IF(LITEFX_HAS_VULKAN_BACKEND)
  LIST(APPEND LITEFX_DEPENDENCIES LiteFX::Vulkan)

  FIND_DEPENDENCY(Vulkan)
  FIND_DEPENDENCY(VulkanMemoryAllocator CONFIG)
  FIND_DEPENDENCY(unofficial-spirv-reflect CONFIG)
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

# Resolve shader compilers. Prefer DXC from Vulkan SDK and fall back to the one 
# supplied by the directx-dxc port
IF(DIRECTX_DXC_TOOL)
  GET_FILENAME_COMPONENT(_litefx_dxc_dir "${DIRECTX_DXC_TOOL}" DIRECTORY)
ENDIF()

FIND_PROGRAM(LITEFX_BUILD_GLSLC_COMPILER glslc HINTS ENV VULKAN_SDK PATH_SUFFIXES bin DOC "The full path to the `glslc.exe` shader compiler binary.")
FIND_PROGRAM(LITEFX_BUILD_DXC_COMPILER dxc HINTS ENV VULKAN_SDK PATH_SUFFIXES bin PATHS ${_litefx_dxc_dir} DOC "The full path to the `dxc.exe` shader compiler binary.")
UNSET(_litefx_dxc_dir)

# Include target export configuration.
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/LiteFX.cmake")

# Also include helper scripts here. Set LITEFX_WITHOUT_HELPERS to ON, to prevent those scripts from being imported.
IF(NOT LITEFX_WITHOUT_HELPERS)
  INCLUDE("${CMAKE_CURRENT_LIST_DIR}/Runtime.cmake")
  INCLUDE("${CMAKE_CURRENT_LIST_DIR}/Assets.cmake")
  INCLUDE("${CMAKE_CURRENT_LIST_DIR}/Shaders.cmake")
ENDIF(NOT LITEFX_WITHOUT_HELPERS)

CHECK_REQUIRED_COMPONENTS("@PROJECT_NAME@")
