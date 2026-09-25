###################################################################################################
#####                                                                                         #####
#####                Test: Backends.D3D12 - Tests for the DirectX 12 backend.                 #####
#####                                                                                         #####
###################################################################################################

SET(LITEFX_TEST_HLSL_SHADER_MODEL "6_6")

ADD_SHADER_MODULE(Tests.Dx.Shaders.VS    SOURCE "shaders/test_vs.hlsl"    LANGUAGE HLSL TYPE VERTEX   COMPILE_AS DXIL SHADER_MODEL ${LITEFX_TEST_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Dx.Shaders.PC.VS SOURCE "shaders/test_pc_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS DXIL SHADER_MODEL ${LITEFX_TEST_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Dx.Shaders.DD.VS SOURCE "shaders/test_dd_vs.hlsl" LANGUAGE HLSL TYPE VERTEX   COMPILE_AS DXIL SHADER_MODEL ${LITEFX_TEST_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Dx.Shaders.PS    SOURCE "shaders/test_fs.hlsl"    LANGUAGE HLSL TYPE PIXEL    COMPILE_AS DXIL SHADER_MODEL ${LITEFX_TEST_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Dx.Shaders.CR.PS SOURCE "shaders/test_cr_fs.hlsl" LANGUAGE HLSL TYPE PIXEL    COMPILE_AS DXIL SHADER_MODEL ${LITEFX_TEST_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Dx.Shaders.CS    SOURCE "shaders/test_cs.hlsl"    LANGUAGE HLSL TYPE COMPUTE  COMPILE_AS DXIL SHADER_MODEL ${LITEFX_TEST_HLSL_SHADER_MODEL} COMPILER DXC)
ADD_SHADER_MODULE(Tests.Dx.Shaders.RG    SOURCE "shaders/raytracing_gen.hlsl"  LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS DXIL SHADER_MODEL ${LITEFX_TEST_HLSL_SHADER_MODEL} COMPILER DXC INCLUDES "shaders/raytracing_common.hlsli" COMPILE_OPTIONS "-disable-payload-qualifiers")
ADD_SHADER_MODULE(Tests.Dx.Shaders.RH    SOURCE "shaders/raytracing_hit.hlsl"  LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS DXIL SHADER_MODEL ${LITEFX_TEST_HLSL_SHADER_MODEL} COMPILER DXC INCLUDES "shaders/raytracing_common.hlsli" COMPILE_OPTIONS "-disable-payload-qualifiers")
ADD_SHADER_MODULE(Tests.Dx.Shaders.RM    SOURCE "shaders/raytracing_miss.hlsl" LANGUAGE HLSL TYPE RAYTRACING COMPILE_AS DXIL SHADER_MODEL ${LITEFX_TEST_HLSL_SHADER_MODEL} COMPILER DXC INCLUDES "shaders/raytracing_common.hlsli" COMPILE_OPTIONS "-disable-payload-qualifiers")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.VS PROPERTIES    FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.PC.VS PROPERTIES FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.DD.VS PROPERTIES FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.PS PROPERTIES    FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.CR.PS PROPERTIES FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.CS PROPERTIES    FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.RG PROPERTIES    FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.RH PROPERTIES    FOLDER "Tests/Shaders/DirectX 12")
SET_TARGET_PROPERTIES(Tests.Dx.Shaders.RM PROPERTIES    FOLDER "Tests/Shaders/DirectX 12")

DEFINE_TEST("app_should_accept_d3d12_backend" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_app_init_test" WITH_WARP
	SOURCES "common.h" "app_init.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_sets_up_d3d12_input_assembler" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_input_assembler_test" WITH_WARP
	SOURCES "common.h" "input_assembler.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_sets_up_d3d12_rasterizer" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_rasterizer_test" WITH_WARP
	SOURCES "common.h" "rasterizer.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_sets_up_d3d12_render_pass" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_create_render_pass_test" WITH_WARP
	SOURCES "common.h" "create_render_pass.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_sets_up_d3d12_frame_buffer" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_create_frame_buffer_test" WITH_WARP
	SOURCES "common.h" "create_frame_buffer.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_resizes_d3d12_frame_buffer" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_resize_frame_buffer_test" WITH_WARP
	SOURCES "common.h" "resize_frame_buffer.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_resets_d3d12_swap_chain" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_reset_swap_chain_test" WITH_WARP
	SOURCES "common.h" "reset_swap_chain.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("render_target_maps_to_d3d12_frame_buffer" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_map_render_target_to_frame_buffer_test" WITH_WARP
	SOURCES "common.h" "map_frame_buffer_render_target.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_sets_up_d3d12_shader_program" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_create_shader_program_test" WITH_WARP
	SOURCES "common.h" "create_shader_program.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

TARGET_LINK_SHADERS(d3d12_create_shader_program_test
    SHADERS Tests.Dx.Shaders.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("device_sets_up_d3d12_render_pipeline" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_create_render_pipeline_test" WITH_WARP
	SOURCES "common.h" "create_render_pipeline.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

TARGET_LINK_SHADERS(d3d12_create_render_pipeline_test
    SHADERS Tests.Dx.Shaders.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("shader_program_reflects_d3d12_render_pipeline" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_reflect_render_pipeline_test" WITH_WARP
	SOURCES "common.h" "reflect_render_pipeline.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

TARGET_LINK_SHADERS(d3d12_reflect_render_pipeline_test
    SHADERS Tests.Dx.Shaders.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("device_sets_up_d3d12_render_pipeline_with_push_constants" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_push_constants_test" WITH_WARP
	SOURCES "common.h" "setup_push_constants.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

TARGET_LINK_SHADERS(d3d12_push_constants_test
    SHADERS Tests.Dx.Shaders.PC.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("device_sets_up_d3d12_compute_pipeline" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_compute_test" WITH_WARP
	SOURCES "common.h" "setup_compute_pipeline.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

TARGET_LINK_SHADERS(d3d12_compute_test
    SHADERS Tests.Dx.Shaders.CS
)

DEFINE_TEST("device_allocates_d3d12_descriptor_sets" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_alloc_descriptor_set_test" WITH_WARP
	SOURCES "common.h" "alloc_descriptor_set.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

TARGET_LINK_SHADERS(d3d12_alloc_descriptor_set_test
    SHADERS Tests.Dx.Shaders.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("device_sets_up_d3d12_ray_tracing_pipeline" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_ray_tracing_test" WITH_WARP
	SOURCES "common.h" "setup_raytracing_pipeline.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

TARGET_LINK_SHADERS(d3d12_ray_tracing_test
    SHADERS Tests.Dx.Shaders.RG Tests.Dx.Shaders.RH Tests.Dx.Shaders.RM
)

DEFINE_TEST("device_sets_up_d3d12_acceleration_structures" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_acceleration_structure_test" WITH_WARP
	SOURCES "common.h" "setup_acceleration_structure.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("d3d12_partial_scissor_rects_dont_fail" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_partial_scissor_test" WITH_WARP
	SOURCES "common.h" "partial_scissor_rect.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("shader_program_reflects_d3d12_render_pipeline_with_dynamic_descriptors" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_reflect_dynamic_descriptors_pipeline_test" WITH_WARP
	SOURCES "common.h" "reflect_dynamic_descriptors.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

TARGET_LINK_SHADERS(d3d12_reflect_dynamic_descriptors_pipeline_test
    SHADERS Tests.Dx.Shaders.DD.VS Tests.Dx.Shaders.PS
)

DEFINE_TEST("device_allocates_d3d12_buffers" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_alloc_buffer_test" WITH_WARP
	SOURCES "common.h" "alloc_buffer.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_allocates_d3d12_vertex_buffers" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_alloc_vertex_buffer_test" WITH_WARP
	SOURCES "common.h" "alloc_vertex_buffer.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_allocates_d3d12_index_buffers" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_alloc_index_buffer_test" WITH_WARP
	SOURCES "common.h" "alloc_index_buffer.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("device_allocates_d3d12_images" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_alloc_image_test" WITH_WARP
	SOURCES "common.h" "alloc_image.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

DEFINE_TEST("d3d12_backend_sets_up_conservative_rasterization" FOLDER "Tests/Backends/D3D12" EXECUTABLE_NAME "d3d12_conservative_rasterization_test" WITH_WARP
	SOURCES "common.h" "conservative_rasterization.cpp"
	DEPENDENCIES LiteFX::AppModel LiteFX::Rendering LiteFX::Logging LiteFX::DirectX12
)

TARGET_LINK_SHADERS(d3d12_conservative_rasterization_test
    SHADERS Tests.Dx.Shaders.VS Tests.Dx.Shaders.CR.PS
)
