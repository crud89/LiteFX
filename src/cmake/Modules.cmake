###################################################################################################
#####                                                                                         #####
#####                            Contains LiteFX module settings.                             #####
#####                                                                                         #####
###################################################################################################

# Setup default module path.
IF("${CMAKE_MODULE_PATH}" STREQUAL "")
    SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/" CACHE PATH "Module directory." FORCE)
ENDIF("${CMAKE_MODULE_PATH}" STREQUAL "")

INCLUDE(GenerateExportHeader)

# Library definition helper. 
#
# Usage:
# LITEFX_ADD_LIBRARY(<name> EXPORT_NAME <export-name> [STATIC | SHARED]
#                    [SOURCES <file>...]
#                    [HEADERS <file>...] [HEADER_BASE_DIRS <dir>...]
#                    [EXPORT_MACRO <macro> EXPORT_HEADER <path>]
#                    [NATVIS <file>])
FUNCTION(LITEFX_ADD_LIBRARY name)
    CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 arg "STATIC;SHARED" "EXPORT_NAME;NATVIS;EXPORT_MACRO;EXPORT_HEADER" "SOURCES;HEADERS;HEADER_BASE_DIRS")

    IF(arg_UNPARSED_ARGUMENTS)
        MESSAGE(FATAL_ERROR "LITEFX_ADD_LIBRARY: Unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    ENDIF()

    IF(NOT arg_EXPORT_NAME)
        MESSAGE(FATAL_ERROR "LITEFX_ADD_LIBRARY: EXPORT_NAME is required.")
    ENDIF()

    IF(arg_STATIC AND arg_SHARED)
        MESSAGE(FATAL_ERROR "LITEFX_ADD_LIBRARY: STATIC and SHARED are mutually exclusive.")
    ELSEIF(arg_STATIC)
        SET(_type STATIC)
    ELSEIF(arg_SHARED)
        SET(_type SHARED)
    ELSE()
        SET(_type "")
    ENDIF()

    ADD_LIBRARY(${name} ${_type} ${arg_SOURCES})
    ADD_LIBRARY(LiteFX::${arg_EXPORT_NAME} ALIAS ${name})
    SET_TARGET_PROPERTIES(${name} PROPERTIES EXPORT_NAME ${arg_EXPORT_NAME})

    IF(arg_HEADERS)
        IF(NOT arg_HEADER_BASE_DIRS)
            SET(arg_HEADER_BASE_DIRS "include")
        ENDIF()

        TARGET_SOURCES(${name} PUBLIC
            FILE_SET HEADERS
            BASE_DIRS ${arg_HEADER_BASE_DIRS}
            FILES ${arg_HEADERS}
        )
    ENDIF()

    IF(arg_EXPORT_MACRO)
        IF(NOT arg_EXPORT_HEADER)
            MESSAGE(FATAL_ERROR "LITEFX_ADD_LIBRARY: EXPORT_MACRO requires EXPORT_HEADER.")
        ENDIF()

        STRING(REGEX REPLACE "_API$" "" _base_name "${arg_EXPORT_MACRO}")
        SET(_export_header "${CMAKE_CURRENT_BINARY_DIR}/include/${arg_EXPORT_HEADER}")

        GENERATE_EXPORT_HEADER(${name}
            BASE_NAME ${_base_name}
            EXPORT_MACRO_NAME ${arg_EXPORT_MACRO}
            EXPORT_FILE_NAME "${_export_header}"
        )

        TARGET_SOURCES(${name} PUBLIC
            FILE_SET HEADERS
            BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/include"
            FILES "${_export_header}"
        )

        # Symbols are only exported if explicitly marked (this is the default on Windows anyway).
        SET_TARGET_PROPERTIES(${name} PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
        )

        GET_TARGET_PROPERTY(_actual_type ${name} TYPE)

        IF(_actual_type STREQUAL "STATIC_LIBRARY")
            TARGET_COMPILE_DEFINITIONS(${name} PUBLIC ${_base_name}_STATIC_DEFINE)
        ENDIF()
    ENDIF()

    IF(arg_NATVIS)
        CMAKE_PATH(ABSOLUTE_PATH arg_NATVIS BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE _natvis)
        TARGET_SOURCES(${name} PRIVATE "${_natvis}")

        GET_TARGET_PROPERTY(_actual_type ${name} TYPE)

        IF(_actual_type STREQUAL "STATIC_LIBRARY")
            TARGET_SOURCES(${name} INTERFACE "$<BUILD_INTERFACE:${_natvis}>")
        ENDIF()
    ENDIF()
ENDFUNCTION()
