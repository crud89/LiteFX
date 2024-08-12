###################################################################################################
#####                                                                                         #####
#####                    Test: Backends.Vk - Tests for the Vulkan backend.                    #####
#####                                                                                         #####
###################################################################################################

ADD_SHADER_MODULE(Tests.Vk.Shaders.VS SOURCE "shaders/test_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Vk.Shaders.FS SOURCE "shaders/test_fs.hlsl" LANGUAGE HLSL TYPE FRAGMENT COMPILE_AS SPIRV SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
SET_TARGET_PROPERTIES(Tests.Vk.Shaders.VS PROPERTIES FOLDER "Tests/Shaders/Vulkan")
SET_TARGET_PROPERTIES(Tests.Vk.Shaders.FS PROPERTIES FOLDER "Tests/Shaders/Vulkan")

DEFINE_TEST("app_should_accept_vk_backend" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_app_init_test" 
	SOURCES "common.h" "app_init.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan 
)

DEFINE_TEST("device_sets_up_vk_input_assembler" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_input_assembler_test" 
	SOURCES "common.h" "input_assembler.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan
)

DEFINE_TEST("device_sets_up_vk_rasterizer" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_irasterizer_test" 
	SOURCES "common.h" "rasterizer.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan
)

DEFINE_TEST("device_sets_up_vk_render_pass" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_create_render_pass_test" 
	SOURCES "common.h" "create_render_pass.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan
)

DEFINE_TEST("device_sets_up_vk_frame_buffer" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_create_frame_buffer_test" 
	SOURCES "common.h" "create_frame_buffer.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan
)

DEFINE_TEST("device_resizes_vk_frame_buffer" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_resize_frame_buffer_test" 
	SOURCES "common.h" "resize_frame_buffer.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan
)

DEFINE_TEST("render_target_maps_to_vk_frame_buffer" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_map_render_target_to_frame_buffer_test" 
	SOURCES "common.h" "map_frame_buffer_render_target.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan
)

DEFINE_TEST("device_sets_up_vk_shader_program" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_create_shader_program_test" 
	SOURCES "common.h" "create_shader_program.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan
)
    
TARGET_LINK_SHADERS(vk_create_shader_program_test
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS Tests.Vk.Shaders.VS Tests.Vk.Shaders.FS
)

DEFINE_TEST("device_sets_up_vk_render_pipeline" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_create_render_pipeline_test" 
	SOURCES "common.h" "create_render_pipeline.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan
)

TARGET_LINK_SHADERS(vk_create_render_pipeline_test
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS Tests.Vk.Shaders.VS Tests.Vk.Shaders.FS
)

DEFINE_TEST("shader_program_reflects_vk_render_pipeline" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_reflect_render_pipeline_test" 
	SOURCES "common.h" "reflect_render_pipeline.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan
)

TARGET_LINK_SHADERS(vk_reflect_render_pipeline_test
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS Tests.Vk.Shaders.VS Tests.Vk.Shaders.FS
)