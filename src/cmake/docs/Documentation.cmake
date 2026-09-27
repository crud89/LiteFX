###################################################################################################
#####                                                                                         #####
#####                 Generates the API reference as Markdown pages using MrDocs.            #####
#####                                                                                         #####
###################################################################################################

INCLUDE_GUARD(GLOBAL)
INCLUDE(FetchContent)

IF(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    MESSAGE(FATAL_ERROR "LITEFX_BUILD_DOCUMENTATION requires CMAKE_EXPORT_COMPILE_COMMANDS, which must be enabled before the targets are defined.")
ENDIF()

# C++ module scanning adds response files (`@<object>.modmap`) to the compile commands, which only exist during a build and cannot be resolved
# by MrDocs. Disable module scanning for documentation.
IF(NOT DEFINED CMAKE_CXX_SCAN_FOR_MODULES OR CMAKE_CXX_SCAN_FOR_MODULES)
    MESSAGE(FATAL_ERROR "LITEFX_BUILD_DOCUMENTATION requires CMAKE_CXX_SCAN_FOR_MODULES to be OFF, which must be set before the targets are defined.")
ENDIF()

IF(NOT CMAKE_GENERATOR MATCHES "Ninja|Makefiles")
    MESSAGE(FATAL_ERROR "LITEFX_BUILD_DOCUMENTATION requires a Ninja or Makefile generator, which export a compilation database (current generator: ${CMAKE_GENERATOR}).")
ENDIF()

SET(LITEFX_DOCUMENTATION_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/../docs/site/content/api" CACHE PATH "Directory to write the generated API reference to. Its content is replaced on every build of the documentation target.")
SET(LITEFX_DOCUMENTATION_SOURCE_REF "main" CACHE STRING "Git branch, tag or commit that links to the source code on GitHub refer to (e.g. a release tag).")
SET(LITEFX_MRDOCS_EXECUTABLE "" CACHE FILEPATH "Path to an installed MrDocs executable. If empty, the pinned release is downloaded.")
SET(LITEFX_MRDOCS_VERSION "2026.9.4")
SET(LITEFX_MRDOCS_MARKDOWN_COMMIT "bbf20c832028d8f7e49ac4940a26a86e293630a8")
SET(LITEFX_MRDOCS_MARKDOWN_SHA256 "04a1f49477ca3eb1aecc7f0d13836f3f16dcb558290cc5d4d13ac5ed2f11de91")

IF(NOT LITEFX_MRDOCS_EXECUTABLE)
    IF(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        SET(_mrdocs_archive "MrDocs-${LITEFX_MRDOCS_VERSION}-win64.zip")
        SET(_mrdocs_sha256 "4a570d433d449b230d9672eab7bb59f062bb04cbb71a2a3bf1d81ab6a7aeb688")
    ELSEIF(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        SET(_mrdocs_archive "MrDocs-${LITEFX_MRDOCS_VERSION}-Linux.tar.xz")
        SET(_mrdocs_sha256 "1e0a455d3e68cb0bacc0884b10fbdf9865b260ae76ce46003c5542e2489af769")
    ELSEIF(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        SET(_mrdocs_archive "MrDocs-${LITEFX_MRDOCS_VERSION}-Darwin.tar.xz")
        SET(_mrdocs_sha256 "a1cfa12f2a980772f91610091978b2bb40302cbbc0740500026ea870c6893e2e")
    ELSE()
        MESSAGE(FATAL_ERROR "No MrDocs release is available for ${CMAKE_HOST_SYSTEM_NAME}. Install MrDocs and set LITEFX_MRDOCS_EXECUTABLE.")
    ENDIF()

    MESSAGE(STATUS "Fetching MrDocs ${LITEFX_MRDOCS_VERSION} (${_mrdocs_archive})...")

    FetchContent_Declare(litefx_mrdocs
        URL "https://github.com/cppalliance/mrdocs/releases/download/${LITEFX_MRDOCS_VERSION}/${_mrdocs_archive}"
        URL_HASH SHA256=${_mrdocs_sha256}
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )

    FetchContent_MakeAvailable(litefx_mrdocs)
    FIND_PROGRAM(_mrdocs_executable mrdocs PATHS "${litefx_mrdocs_SOURCE_DIR}/bin" NO_DEFAULT_PATH NO_CACHE REQUIRED)
ELSE()
    SET(_mrdocs_executable "${LITEFX_MRDOCS_EXECUTABLE}")
ENDIF()

MESSAGE(STATUS "Fetching the MrDocs Markdown generator (mp-units@${LITEFX_MRDOCS_MARKDOWN_COMMIT})...")

# Only the add-on is used from the archive.
FetchContent_Declare(litefx_mrdocs_markdown
    URL "https://github.com/mpusz/mp-units/archive/${LITEFX_MRDOCS_MARKDOWN_COMMIT}.tar.gz"
    URL_HASH SHA256=${LITEFX_MRDOCS_MARKDOWN_SHA256}
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    SOURCE_SUBDIR scripts/mrdocs
)

FetchContent_MakeAvailable(litefx_mrdocs_markdown)
SET(LITEFX_MRDOCS_ADDONS_DIR "${litefx_mrdocs_markdown_SOURCE_DIR}/scripts/mrdocs/addons")

IF(NOT EXISTS "${LITEFX_MRDOCS_ADDONS_DIR}/generator/md/mrdocs-generator.yml")
    MESSAGE(FATAL_ERROR "The MrDocs Markdown generator was not found in ${LITEFX_MRDOCS_ADDONS_DIR}.")
ENDIF()

# Define the documentation target.
CMAKE_PATH(CONVERT "${CMAKE_SOURCE_DIR}" TO_CMAKE_PATH_LIST LITEFX_DOCUMENTATION_SOURCE_ROOT NORMALIZE)
CMAKE_PATH(CONVERT "${LITEFX_DOCUMENTATION_OUTPUT_DIR}" TO_CMAKE_PATH_LIST LITEFX_DOCUMENTATION_OUTPUT NORMALIZE)
CMAKE_PATH(CONVERT "${CMAKE_BINARY_DIR}/compile_commands.json" TO_CMAKE_PATH_LIST LITEFX_DOCUMENTATION_COMPILATION_DATABASE NORMALIZE)
CMAKE_PATH(CONVERT "${LITEFX_MRDOCS_ADDONS_DIR}" TO_CMAKE_PATH_LIST LITEFX_DOCUMENTATION_ADDONS NORMALIZE)

CONFIGURE_FILE("${CMAKE_CURRENT_LIST_DIR}/mrdocs.yml.in" "${CMAKE_BINARY_DIR}/mrdocs.yml" @ONLY)

# The output directory is cleared first, so that pages of removed symbols do not remain.
ADD_CUSTOM_TARGET(LiteFX.Documentation
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${LITEFX_DOCUMENTATION_OUTPUT}"
    COMMAND "${_mrdocs_executable}" "--config=${CMAKE_BINARY_DIR}/mrdocs.yml"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating the API reference with MrDocs to '${LITEFX_DOCUMENTATION_OUTPUT}'..."
    VERBATIM
)

SET_TARGET_PROPERTIES(LiteFX.Documentation PROPERTIES FOLDER "Docs")

# MrDocs parses the translation units of the compilation database, so headers generated during the build must exist first. Export headers and
# configuration headers are generated at configure time, but the shader library of LiteFX.Graphics (included by its sources) is generated
# during the build. Add further targets here, if more headers are generated at build time.
FOREACH(_generated_headers_target IN ITEMS LiteFX.Graphics.Shaders)
    IF(TARGET ${_generated_headers_target})
        ADD_DEPENDENCIES(LiteFX.Documentation ${_generated_headers_target})
    ENDIF()
ENDFOREACH()
