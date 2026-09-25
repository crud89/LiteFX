###################################################################################################
#####                                                                                         #####
#####                       Contains helper function to define samples.                       #####
#####                                                                                         #####
###################################################################################################

FUNCTION(DEFINE_SAMPLE name)
    CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 arg "" "" "SOURCES;HEADERS;LIBRARIES")

    IF(arg_UNPARSED_ARGUMENTS)
        MESSAGE(FATAL_ERROR "DEFINE_SAMPLE: Unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    ENDIF()

    IF(NOT LITEFX_BUILD_WITH_GLM)
        MESSAGE(FATAL_ERROR "The sample \"${name}\" requires the glm converters for the math module. Set the LITEFX_BUILD_WITH_GLM option to ON and retry.")
    ENDIF()

    # Resolve package dependencies.
    FIND_PACKAGE(glfw3 CONFIG REQUIRED)
    FIND_PACKAGE(CLI11 CONFIG REQUIRED)

    # Create sample configuration header.
    CONFIGURE_FILE("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/config.tmpl" "${CMAKE_CURRENT_BINARY_DIR}/src/config.h")

    # Add executable project.
    ADD_EXECUTABLE(${name}
        ${arg_HEADERS}
        ${arg_SOURCES}
        "${CMAKE_CURRENT_BINARY_DIR}/src/config.h"
    )

    # Create source groups for better code organization.
    SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${arg_HEADERS} ${arg_SOURCES})

    # Setup project properties.
    SET_TARGET_PROPERTIES(${name} PROPERTIES
        FOLDER "Samples"
        VERSION ${LITEFX_VERSION}
    )

    # Setup target include directories.
    TARGET_INCLUDE_DIRECTORIES(${name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src/")

    IF(LITEFX_BUILD_EXAMPLES_RENDERDOC_LOADER)
        TARGET_INCLUDE_DIRECTORIES(${name} PRIVATE ${RENDERDOC_INCLUDE_DIR})
    ENDIF()

    # Link engine, backends and additional dependencies.
    TARGET_LINK_LIBRARIES(${name} PRIVATE
        LiteFX::Core LiteFX::Math LiteFX::AppModel LiteFX::Rendering LiteFX::Graphics
        glfw CLI11::CLI11
        ${arg_LIBRARIES}
    )

    IF(LITEFX_BUILD_VULKAN_BACKEND)
        TARGET_LINK_LIBRARIES(${name} PRIVATE LiteFX::Vulkan)
    ENDIF()

    IF(LITEFX_BUILD_DIRECTX_12_BACKEND)
        TARGET_LINK_LIBRARIES(${name} PRIVATE LiteFX::DirectX12)
    ENDIF()

    # Deploy engine runtime and setup installer.
    LITEFX_DEPLOY_RUNTIME(${name} INSTALL_DESTINATION ${CMAKE_INSTALL_BINDIR})
    INSTALL(TARGETS ${name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
ENDFUNCTION()