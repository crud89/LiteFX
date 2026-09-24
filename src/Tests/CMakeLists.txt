###################################################################################################
#####                                                                                         #####
#####                     LiteFX.Tests - Contains tests for the project.                      #####
#####                                                                                         #####
###################################################################################################

INCLUDE("${CMAKE_CURRENT_LIST_DIR}/TestHelpers.cmake")

# Include individual tests.
ADD_SUBDIRECTORY(Core.Enumerable)

IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
    ADD_SUBDIRECTORY(Backends.D3D12)
ENDIF(LITEFX_BUILD_DIRECTX_12_BACKEND)

IF(LITEFX_BUILD_VULKAN_BACKEND)
    ADD_SUBDIRECTORY(Backends.Vk)
ENDIF(LITEFX_BUILD_VULKAN_BACKEND)
