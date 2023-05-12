###################################################################################################
#####                                                                                         #####
#####                        Contains helper macros to create tests.                          #####
#####                                                                                         #####
###################################################################################################

MACRO(DEFINE_TEST test_name)
	CMAKE_PARSE_ARGUMENTS(_TEST "" "FOLDER;EXECUTABLE_NAME" "SOURCES;DEPENDENCIES" ${ARGN})
	
	# Add the executable
	ADD_EXECUTABLE(${_TEST_EXECUTABLE_NAME} ${_TEST_SOURCES})
	SET_TARGET_PROPERTIES(${_TEST_EXECUTABLE_NAME} PROPERTIES FOLDER ${_TEST_FOLDER})
	TARGET_LINK_LIBRARIES(${_TEST_EXECUTABLE_NAME} PRIVATE ${_TEST_DEPENDENCIES})

	# Add the test.
	ADD_TEST(NAME ${test_name} COMMAND ${_TEST_EXECUTABLE_NAME})
ENDMACRO()