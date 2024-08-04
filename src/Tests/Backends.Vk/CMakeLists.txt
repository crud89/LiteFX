###################################################################################################
#####                                                                                         #####
#####                    Test: Backends.Vk - Tests for the Vulkan backend.                    #####
#####                                                                                         #####
###################################################################################################

DEFINE_TEST("app_should_accept_vk_backend" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_app_init_test" 
	SOURCES "common.h" "app_init.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan 
)

DEFINE_TEST("device_sets_up_vk_input_assembler" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_input_assembler_test" 
	SOURCES "common.h" "input_assembler.cpp"
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