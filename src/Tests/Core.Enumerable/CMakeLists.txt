###################################################################################################
#####                                                                                         #####
#####            Test: Core.Enumerable - Tests for the core enumerable container.             #####
#####                                                                                         #####
###################################################################################################

DEFINE_TEST("enumerable_should_store_trivial_types" FOLDER "Tests/Core" EXECUTABLE_NAME "core_trivial_elements" 
	SOURCES "common.h" "trivial.cpp"
	DEPENDENCIES LiteFX.Core
)

DEFINE_TEST("enumerable_should_store_reference_wrappers" FOLDER "Tests/Core" EXECUTABLE_NAME "core_references" 
	SOURCES "common.h" "references.cpp"
	DEPENDENCIES LiteFX.Core
)

DEFINE_TEST("enumerable_should_store_shared_pointers" FOLDER "Tests/Core" EXECUTABLE_NAME "core_shared_ptrs" 
	SOURCES "common.h" "shared_ptrs.cpp"
	DEPENDENCIES LiteFX.Core
)

DEFINE_TEST("enumerable_should_store_unique_pointers" FOLDER "Tests/Core" EXECUTABLE_NAME "core_unique_ptrs" 
	SOURCES "common.h" "unique_ptrs.cpp"
	DEPENDENCIES LiteFX.Core
)