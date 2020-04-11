###################################################################################################
#####                                                                                         #####
#####                        Contains LiteFX build option definitions.                        #####
#####                                                                                         #####
###################################################################################################

#OPTION(BUILD_OPEN_GL_BACKEND "Builds the OpenGL backend." OFF)
OPTION(BUILD_VULKAN_BACKEND "Builds the Vulkan backend." ON)
#OPTION(BUILD_DIRECTX_11_BACKEND "Builds the DirectX 11 backend." OFF)
#OPTION(BUILD_DIRECTX_12_BACKEND "Builds the DirectX 12 backend." OFF)

OPTION(BUILD_SAMPLE_BASIC_RENDERING "Builds the basic rendering sample." ON)

OPTION(BUILD_ENABLE_GLM "Enables glm converters for math types." ON)
OPTION(BUILD_ENABLE_DIRECTX_MATH "Enables DirectXMath converters for math types." ON)

OPTION(BUILD_PRECOMPILED_HEADERS "Use pre-compiled headers during build." OFF)

SET(BUILD_ENGINE_IDENTIFIER "LiteFX" CACHE STRING "Defines the engine identifier string.")

IF(NOT MSVC OR (MSVC AND MSVC_VERSION LESS 1910))
	# TODO: Also check for DirectX backends.
	IF(BUILD_ENABLE_DIRECTX_MATH)
		MESSAGE(WARNING "DirectX features may only be working with Visual Studio 2017 or newer.")
	ENDIF(BUILD_ENABLE_DIRECTX_MATH)
ENDIF(NOT MSVC OR (MSVC AND MSVC_VERSION LESS 1910))