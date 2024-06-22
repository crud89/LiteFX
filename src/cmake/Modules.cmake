###################################################################################################
#####                                                                                         #####
#####                            Contains LiteFX module settings.                             #####
#####                                                                                         #####
###################################################################################################

# Setup default module path.
IF("${CMAKE_MODULE_PATH}" STREQUAL "")
    SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" CACHE PATH "Module directory." FORCE)
ENDIF("${CMAKE_MODULE_PATH}" STREQUAL "")

# This must be defined before the top-level `PROJECT()` call.
OPTION(BUILD_SHARED_LIBS "Link libraries as shared objects." ON)

IF(NOT BUILD_SHARED_LIBS)
    SET(VCPKG_LIBRARY_LINKAGE "static")
ENDIF(NOT BUILD_SHARED_LIBS)

# Define overlay ports (separated by `;`).
SET(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/modules/overlay-ports/winpixeventruntime/;${CMAKE_SOURCE_DIR}/modules/overlay-ports/dx-agility-sdk/;${CMAKE_SOURCE_DIR}/modules/overlay-ports/d3d12-memory-allocator/;${CMAKE_SOURCE_DIR}/modules/overlay-ports/spdlog/")