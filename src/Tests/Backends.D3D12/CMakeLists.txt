###################################################################################################
#####                                                                                         #####
#####                Test: Backends.D3D12 - Tests for the DirectX 12 backend.                 #####
#####                                                                                         #####
###################################################################################################

FIND_PACKAGE(directx-warp CONFIG REQUIRED)
    
ADD_SHADER_MODULE(Tests.Dx.Shaders.VS    SOURCE "shaders/test_vs.hlsl"    LANGUAGE HLSL TYPE VERTEX   COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Dx.Shaders.PC.VS SOURCE "shaders/test_pc_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Dx.Shaders.PS    SOURCE "shaders/test_fs.hlsl"    LANGUAGE HLSL TYPE PIXEL    COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Dx.Shaders.CS    SOURCE "shaders/test_cs.hlsl"    LANGUAGE HLSL TYPE COMPUTE  COMPILE_AS DXIL SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL} COMPILER DXC)
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.VS PROPERTIES    FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.PC.VS PROPERTIES FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.PS PROPERTIES    FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.CS PROPERTIES    FOLDER "Tests/Shaders/DirectX 12")

DEFINE_TEST("app_should_accept_d3d12_backend" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_app_init_test" 
	SOURCES "common.h" "app_init.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12 
)

DEFINE_TEST("device_sets_up_d3d12_input_assembler" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_input_assembler_test" 
	SOURCES "common.h" "input_assembler.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12 
)

DEFINE_TEST("device_sets_up_d3d12_rasterizer" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_rasterizer_test" 
	SOURCES "common.h" "rasterizer.cpp"
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

DEFINE_TEST("device_resets_d3d12_swap_chain" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_reset_swap_chain_test" 
	SOURCES "common.h" "reset_swap_chain.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

DEFINE_TEST("render_target_maps_to_d3d12_frame_buffer" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_map_render_target_to_frame_buffer_test" 
	SOURCES "common.h" "map_frame_buffer_render_target.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

DEFINE_TEST("device_sets_up_d3d12_shader_program" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_create_shader_program_test" 
	SOURCES "common.h" "create_shader_program.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

TARGET_LINK_SHADERS(d3d12_create_shader_program_test
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS Tests.Dx.Shaders.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("device_sets_up_d3d12_render_pipeline" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_create_render_pipeline_test" 
	SOURCES "common.h" "create_render_pipeline.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

TARGET_LINK_SHADERS(d3d12_create_render_pipeline_test
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS Tests.Dx.Shaders.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("shader_program_reflects_d3d12_render_pipeline" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_reflect_render_pipeline_test" 
	SOURCES "common.h" "reflect_render_pipeline.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

TARGET_LINK_SHADERS(d3d12_reflect_render_pipeline_test
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS Tests.Dx.Shaders.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("device_sets_up_d3d12_render_pipeline_with_push_constants" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_push_constants_test" 
	SOURCES "common.h" "setup_push_constants.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

TARGET_LINK_SHADERS(d3d12_push_constants_test
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS Tests.Dx.Shaders.PC.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("device_sets_up_d3d12_compute_pipeline" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_compute_test" 
	SOURCES "common.h" "setup_compute_pipeline.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

TARGET_LINK_SHADERS(d3d12_compute_test
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS Tests.Dx.Shaders.CS
)

DEFINE_TEST("device_allocates_d3d12_descriptor_sets" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_alloc_descriptor_set_test" 
	SOURCES "common.h" "alloc_descriptor_set.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.DirectX12
)

TARGET_LINK_SHADERS(d3d12_alloc_descriptor_set_test
    INSTALL_DESTINATION "${CMAKE_INSTALL_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR}"
    SHADERS Tests.Dx.Shaders.VS Tests.Dx.Shaders.PS
)

ADD_DEPENDENCIES(d3d12_app_init_test dxwarp)
ADD_DEPENDENCIES(d3d12_input_assembler_test dxwarp)
ADD_DEPENDENCIES(d3d12_rasterizer_test dxwarp)
ADD_DEPENDENCIES(d3d12_create_render_pass_test dxwarp)
ADD_DEPENDENCIES(d3d12_create_frame_buffer_test dxwarp)
ADD_DEPENDENCIES(d3d12_resize_frame_buffer_test dxwarp)
ADD_DEPENDENCIES(d3d12_map_render_target_to_frame_buffer_test dxwarp)
ADD_DEPENDENCIES(d3d12_create_shader_program_test dxwarp)
ADD_DEPENDENCIES(d3d12_create_render_pipeline_test dxwarp)
ADD_DEPENDENCIES(d3d12_reflect_render_pipeline_test dxwarp)
ADD_DEPENDENCIES(d3d12_push_constants_test dxwarp)
ADD_DEPENDENCIES(d3d12_compute_test dxwarp)
ADD_DEPENDENCIES(d3d12_alloc_descriptor_set_test dxwarp)