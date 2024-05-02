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

ADD_DEPENDENCIES(d3d12_app_init_test dxwarp)