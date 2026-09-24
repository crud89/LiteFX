###################################################################################################
#####                                                                                         #####
#####                        Contains helper functions to create tests.                       #####
#####                                                                                         #####
###################################################################################################

FUNCTION(DEFINE_TEST test_name)
    CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 _TEST "WITH_WARP" "FOLDER;EXECUTABLE_NAME" "SOURCES;DEPENDENCIES")

    IF(_TEST_UNPARSED_ARGUMENTS)
        MESSAGE(FATAL_ERROR "DEFINE_TEST: Unknown arguments: ${_TEST_UNPARSED_ARGUMENTS}")
    ENDIF()

    IF(NOT _TEST_EXECUTABLE_NAME OR NOT _TEST_SOURCES)
        MESSAGE(FATAL_ERROR "DEFINE_TEST: EXECUTABLE_NAME and SOURCES are required for test \"${test_name}\".")
    ENDIF()

    # Add the executable.
    ADD_EXECUTABLE(${_TEST_EXECUTABLE_NAME} ${_TEST_SOURCES})
    TARGET_LINK_LIBRARIES(${_TEST_EXECUTABLE_NAME} PRIVATE ${_TEST_DEPENDENCIES})

    IF(_TEST_FOLDER)
        SET_TARGET_PROPERTIES(${_TEST_EXECUTABLE_NAME} PROPERTIES FOLDER ${_TEST_FOLDER})
    ENDIF()

    # Deploy the engine runtime.
    IF(_TEST_WITH_WARP)
        LITEFX_DEPLOY_RUNTIME(${_TEST_EXECUTABLE_NAME} WITH_WARP)
    ELSE()
        LITEFX_DEPLOY_RUNTIME(${_TEST_EXECUTABLE_NAME})
    ENDIF()

    # Add the test.
    ADD_TEST(NAME ${test_name} COMMAND ${_TEST_EXECUTABLE_NAME})
ENDFUNCTION()
