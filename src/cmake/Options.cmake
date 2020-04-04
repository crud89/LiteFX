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

SET(BUILD_ENGINE_IDENTIFIER "LiteFX" CACHE STRING "Defines the engine identifier string.")