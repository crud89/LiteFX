###################################################################################################
#####                                                                                         #####
#####                        Contains LiteFX build option definitions.                        #####
#####                                                                                         #####
###################################################################################################

OPTION(BUILD_VULKAN_BACKEND "Builds the Vulkan backend." ON)
OPTION(BUILD_DIRECTX_12_BACKEND "Builds the DirectX 12 backend." ON)

OPTION(BUILD_EXAMPLES "When set to OFF, no samples will be built, regardless of their individual option." ON)

OPTION(BUILD_WITH_GLM "Enables glm converters for math types." ON)
OPTION(BUILD_WITH_DIRECTX_MATH "Enables DirectXMath converters for math types." ON)

# GLSLC supports glsl but does not compile into DXIL.
OPTION(BUILD_USE_GLSLC "Uses `glslc` to build shaders. Mutual exclusive with the `BUILD_USE_DXC` option." OFF)

# DXC only supports hlsl but does compile into DXIL and SPIR-V.
OPTION(BUILD_USE_DXC "Uses `glslc` to build shaders. Mutual exclusive with the `BUILD_USE_GLSLC` option." ON)
OPTION(BUILD_DXC_DXIL "DXC is invoked to compile HLSL shaders into DXIL when this option is set to ON." ON)
OPTION(BUILD_DXC_SPIRV "DXC is invoked to compile HLSL shaders into SPIR-V when this option is set to ON." ON)

IF(BUILD_USE_GLSLC AND BUILD_USE_DXC)
	MESSAGE(FATAL_ERROR "The options `BUILD_USE_GLSLC` and `BUILD_USE_DXC` can not be both enabled at the same time.")
ENDIF(BUILD_USE_GLSLC AND BUILD_USE_DXC)

SET(BUILD_GLSLC_DIR $ENV{VULKAN_SDK}/bin CACHE STRING "A directory that contains the `glslc.exe` file.")
SET(BUILD_GLSLC_COMPILER ${BUILD_GLSLC_DIR}/glslc)
SET(BUILD_DXC_DIR $ENV{VULKAN_SDK}/bin CACHE STRING "A directory that contains the `dxc.exe` file.")
SET(BUILD_DXC_COMPILER ${BUILD_DXC_DIR}/dxc)

OPTION(BUILD_PRECOMPILED_HEADERS "Use pre-compiled headers during build." OFF)

SET(BUILD_ENGINE_IDENTIFIER "LiteFX" CACHE STRING "Defines the engine identifier string.")

IF(NOT MSVC OR (MSVC AND MSVC_VERSION LESS 1910))
	# TODO: Also check for DirectX backends.
	IF(BUILD_WITH_DIRECTX_MATH)
		MESSAGE(WARNING "DirectX features may only be working with Visual Studio 2017 or newer.")
	ENDIF(BUILD_WITH_DIRECTX_MATH)
ENDIF(NOT MSVC OR (MSVC AND MSVC_VERSION LESS 1910))