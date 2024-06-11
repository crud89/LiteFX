###################################################################################################
#####                                                                                         #####
#####                            Contains LiteFX module settings.                             #####
#####                                                                                         #####
###################################################################################################

# Setup default module path.
IF("${CMAKE_MODULE_PATH}" STREQUAL "")
    SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" CACHE PATH "Module directory." FORCE)
ENDIF("${CMAKE_MODULE_PATH}" STREQUAL "")

# Define overlay ports (separated by `;`).
SET(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/Modules/overlay-ports/winpixeventruntime/;${CMAKE_SOURCE_DIR}/Modules/overlay-ports/dx-agility-sdk/;${CMAKE_SOURCE_DIR}/Modules/overlay-ports/d3d12-memory-allocator/;${CMAKE_SOURCE_DIR}/Modules/overlay-ports/spdlog/")