###################################################################################################
#####                                                                                         #####
#####                        Contains LiteFX build option definitions.                        #####
#####                                                                                         #####
###################################################################################################

OPTION(BUILD_VULKAN_BACKEND "Builds the Vulkan backend." ON)
OPTION(BUILD_DIRECTX_12_BACKEND "Builds the DirectX 12 backend." ON)

OPTION(BUILD_EXAMPLES "When set to OFF, no samples will be built, regardless of their individual option." ON)
OPTION(BUILD_EXAMPLES_DX12_PIX_LOADER "Add code to DirectX12 samples to load PIX GPU capture library when starting with --load-pix=1 command line argument." ON)

OPTION(BUILD_WITH_GLM "Enables glm converters for math types." ON)
OPTION(BUILD_WITH_DIRECTX_MATH "Enables DirectXMath converters for math types." ON)

FIND_PROGRAM(BUILD_GLSLC_COMPILER glslc HINTS ENV VULKAN_SDK PATH_SUFFIXES bin DOC "The full path to the `glslc.exe` shader compiler binary.")
FIND_PROGRAM(BUILD_DXC_COMPILER dxc HINTS ENV VULKAN_SDK PATH_SUFFIXES bin DOC "The full path to the `dxc.exe` shader compiler binary.")

SET(BUILD_HLSL_SHADER_MODEL "6_3" CACHE STRING "Shader model version used to compile HLSL shaders.")

OPTION(BUILD_PRECOMPILED_HEADERS "Use pre-compiled headers during build." OFF)

SET(BUILD_ENGINE_IDENTIFIER "LiteFX" CACHE STRING "Defines the engine identifier string.")

IF(NOT MSVC OR (MSVC AND MSVC_VERSION LESS 1910))
  IF(BUILD_DIRECTX_12_BACKEND OR BUILD_WITH_DIRECTX_MATH)
    MESSAGE(WARNING "DirectX features may only be working with Visual Studio 2017 or newer.")
  ENDIF(BUILD_DIRECTX_12_BACKEND OR BUILD_WITH_DIRECTX_MATH)
ENDIF(NOT MSVC OR (MSVC AND MSVC_VERSION LESS 1910))