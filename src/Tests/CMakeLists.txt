###################################################################################################
#####                                                                                         #####
#####                     LiteFX.Tests - Contains tests for the project.                      #####
#####                                                                                         #####
###################################################################################################

INCLUDE(TestHelpers.cmake)

# Include individual tests.
ADD_SUBDIRECTORY(Core.Enumerable)
ADD_SUBDIRECTORY(Backends.D3D12)
ADD_SUBDIRECTORY(Backends.Vk)