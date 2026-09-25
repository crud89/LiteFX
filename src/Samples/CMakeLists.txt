###################################################################################################
#####                                                                                         #####
#####                     LiteFX.Samples - Contains the sample projects.                      #####
#####                                                                                         #####
###################################################################################################

INCLUDE("${CMAKE_CURRENT_LIST_DIR}/SampleHelpers.cmake")

# Define individual samples.
ADD_SUBDIRECTORY(BasicRendering)
ADD_SUBDIRECTORY(RenderPasses)
ADD_SUBDIRECTORY(PushConstants)
ADD_SUBDIRECTORY(UniformArrays)
ADD_SUBDIRECTORY(Textures)
ADD_SUBDIRECTORY(Multisampling)
ADD_SUBDIRECTORY(Multithreading)
ADD_SUBDIRECTORY(Bindless)
ADD_SUBDIRECTORY(DynamicDescriptors)
ADD_SUBDIRECTORY(Compute)
ADD_SUBDIRECTORY(Indirect)
ADD_SUBDIRECTORY(MeshShader)
ADD_SUBDIRECTORY(RayTracing)
ADD_SUBDIRECTORY(RayQueries)
ADD_SUBDIRECTORY(Defragmentation)
ADD_SUBDIRECTORY(ResourceAliasing)

IF(LITEFX_BUILD_WITH_IMGUI_SAMPLE)
    ADD_SUBDIRECTORY(ImGui)
ENDIF(LITEFX_BUILD_WITH_IMGUI_SAMPLE)