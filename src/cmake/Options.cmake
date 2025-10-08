###################################################################################################
#####                                                                                         #####
#####                        Contains LiteFX build option definitions.                        #####
#####                                                                                         #####
###################################################################################################

OPTION(BUILD_SHARED_LIBS "Link libraries as shared objects." ON)
SET(LITEFX_LINK_SHARED ${BUILD_SHARED_LIBS} CACHE INTERNAL "Internal variable to declare link support macro in config header.")

OPTION(LITEFX_BUILD_VULKAN_BACKEND "Builds the Vulkan backend." ON)
OPTION(LITEFX_BUILD_DIRECTX_12_BACKEND "Builds the DirectX 12 backend." ON)

OPTION(LITEFX_BUILD_DEFINE_BUILDERS "Defines builder types to allow to use builder syntax in applications." ON)
OPTION(LITEFX_BUILD_SUPPORT_DEBUG_MARKERS "Implements support for setting debug markers on device queues." OFF)

OPTION(LITEFX_BUILD_EXAMPLES "When set to OFF, no samples will be built, regardless of their individual option." ON)
OPTION(LITEFX_BUILD_EXAMPLES_DX12_PIX_LOADER "Add code to samples to load PIX GPU capture library when starting with --dx-load-pix=1 command line argument." ON)

OPTION(LITEFX_BUILD_TESTS "When set to ON, tests will be built for the project." OFF)

# Forcefully disable interop swap chain for test builds.
IF(LITEFX_BUILD_TESTS)
  SET(LITEFX_BUILD_VULKAN_INTEROP_SWAP_CHAIN OFF CACHE BOOL "" FORCE)
ELSE(LITEFX_BUILD_TESTS)
  OPTION(LITEFX_BUILD_VULKAN_INTEROP_SWAP_CHAIN "When set to ON, the Vulkan backend will use a DXIL interop swap chain, if the DirectX backend is also built. This option is not available when building tests." ON)
ENDIF(LITEFX_BUILD_TESTS)

# NOTE: In order for this to work, you should add the RenderDoc installation path to the PATH environment variable, so that the runtime can pick up the API dll.
OPTION(LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER "Adds code to the samples to create additional RenderDoc capture triggers for simplified debugging, when starting with --load-render-doc=1 command line argument." OFF)

OPTION(LITEFX_BUILD_WITH_GLM "Enables glm converters for math types." ON)
OPTION(LITEFX_BUILD_WITH_DIRECTX_MATH "Enables DirectXMath converters for math types." ON)

FIND_PROGRAM(LITEFX_BUILD_GLSLC_COMPILER glslc HINTS ENV VULKAN_SDK PATH_SUFFIXES bin DOC "The full path to the `glslc.exe` shader compiler binary.")
FIND_PROGRAM(LITEFX_BUILD_DXC_COMPILER dxc HINTS ${DIRECTX_DXC_TOOL} ENV VULKAN_SDK PATH_SUFFIXES bin DOC "The full path to the `dxc.exe` shader compiler binary.")

SET(LITEFX_BUILD_HLSL_SHADER_MODEL "6_5" CACHE STRING "Shader model version used to compile HLSL shaders.")

OPTION(LITEFX_BUILD_PRECOMPILED_HEADERS "Use pre-compiled headers during build." OFF)

SET(LITEFX_BUILD_ENGINE_IDENTIFIER "LiteFX" CACHE STRING "Defines the engine identifier string.")

IF("pix-support" IN_LIST VCPKG_MANIFEST_FEATURES)
  SET(LITEFX_BUILD_WITH_PIX_RUNTIME ON CACHE BOOL "Link DirectX 12 backend against PIX runtime (required for debug marker support).")
ELSEIF(NOT LITEFX_BUILD_WITH_PIX_RUNTIME) # Only force it to be off, if not otherwise defined. If setting this variable manually, make sure to also provide the proper dependency!
  SET(LITEFX_BUILD_WITH_PIX_RUNTIME OFF CACHE BOOL "Link DirectX 12 backend against PIX runtime (required for debug marker support).")
ENDIF("pix-support" IN_LIST VCPKG_MANIFEST_FEATURES)

IF("imgui" IN_LIST VCPKG_MANIFEST_FEATURES)
  SET(LITEFX_BUILD_WITH_IMGUI_SAMPLE ON CACHE BOOL "Enables building the ImGui sample.")
ELSEIF(NOT LITEFX_BUILD_WITH_IMGUI_SAMPLE)
  SET(LITEFX_BUILD_WITH_IMGUI_SAMPLE OFF CACHE BOOL "Enables building the ImGui sample.")
ENDIF("imgui" IN_LIST VCPKG_MANIFEST_FEATURES)