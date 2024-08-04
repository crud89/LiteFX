###################################################################################################
#####                                                                                         #####
#####                Test: Backends.D3D12 - Tests for the DirectX 12 backend.                 #####
#####                                                                                         #####
###################################################################################################

FIND_PACKAGE(directx-warp CONFIG REQUIRED)

DEFINE_TEST("app_should_accept_d3d12_backend" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_app_init_test" 
	SOURCES "common.h" "app_init.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12 
)

DEFINE_TEST("device_sets_up_d3d12_input_assembler" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_input_assembler_test" 
	SOURCES "common.h" "input_assembler.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12 
)

DEFINE_TEST("device_sets_up_d3d12_render_pass" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_create_render_pass_test" 
	SOURCES "common.h" "create_render_pass.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

DEFINE_TEST("device_sets_up_d3d12_frame_buffer" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_create_frame_buffer_test" 
	SOURCES "common.h" "create_frame_buffer.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

DEFINE_TEST("device_resizes_d3d12_frame_buffer" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_resize_frame_buffer_test" 
	SOURCES "common.h" "resize_frame_buffer.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

DEFINE_TEST("render_target_maps_to_d3d12_frame_buffer" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_map_render_target_to_frame_buffer_test" 
	SOURCES "common.h" "map_frame_buffer_render_target.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

ADD_DEPENDENCIES(d3d12_app_init_test dxwarp)
ADD_DEPENDENCIES(d3d12_input_assembler_test dxwarp)
ADD_DEPENDENCIES(d3d12_create_render_pass_test dxwarp)
ADD_DEPENDENCIES(d3d12_create_frame_buffer_test dxwarp)
ADD_DEPENDENCIES(d3d12_resize_frame_buffer_test dxwarp)
ADD_DEPENDENCIES(d3d12_map_render_target_to_frame_buffer_test dxwarp)