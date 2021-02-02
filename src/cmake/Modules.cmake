###################################################################################################
#####                                                                                         #####
#####                            Contains LiteFX module settings.                             #####
#####                                                                                         #####
###################################################################################################

# Setup default module path.
IF("${CMAKE_MODULE_PATH}" STREQUAL "")
    SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" CACHE PATH "Module directory." FORCE)
ENDIF("${CMAKE_MODULE_PATH}" STREQUAL "")

# Setup vcpkg
INCLUDE (AutoVcpkg)

IF (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	IF (NOT DEFINED ENV{VCPKG_ROOT})
		MESSAGE(FATAL_ERROR "Unable to find vcpkg.")
	ELSE (NOT DEFINED ENV{VCPKG_ROOT})
		SET (AUTO_VCPKG_ROOT $ENV{VCPKG_ROOT})
	ENDIF (NOT DEFINED ENV{VCPKG_ROOT})
ELSE (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	SET (AUTO_VCPKG_ROOT ${CMAKE_TOOLCHAIN_FILE})
ENDIF (NOT DEFINED CMAKE_TOOLCHAIN_FILE)

MESSAGE(STATUS "Found vcpkg: ${AUTO_VCPKG_ROOT}...")