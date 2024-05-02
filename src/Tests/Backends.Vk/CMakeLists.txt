###################################################################################################
#####                                                                                         #####
#####                    Test: Backends.Vk - Tests for the Vulkan backend.                    #####
#####                                                                                         #####
###################################################################################################

DEFINE_TEST("app_should_accept_vk_backend" FOLDER "Tests/Backends/Vk" EXECUTABLE_NAME "vk_app_init_test" 
	SOURCES "common.h" "app_init.cpp"
	DEPENDENCIES LiteFX.AppModel LiteFX.Rendering LiteFX.Logging LiteFX.Backends.Vulkan 
)