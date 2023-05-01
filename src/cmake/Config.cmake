###################################################################################################
#####                                                                                         #####
#####                          Contains LiteFX build configurations.                          #####
#####                                                                                         #####
###################################################################################################

# C++ standard version.
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable C++ modules support.
SET(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
SET(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

# Define C++ compile flags.
IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")       # Use multi core compiling to speed up compile times.
ENDIF(MSVC)

# For debug builds, append the "d" suffix.
SET(CMAKE_DEBUG_POSTFIX "d")

# If not provided to be any different, initialize the install directory.
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "../install/${CMAKE_GENERATOR_PLATFORM}" CACHE PATH "Installation directory." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Setup build artifact directory.
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/binaries/>")

# Setup installation directories (ensure they are not empty).
IF("${CMAKE_INSTALL_LIBDIR}" STREQUAL "")
    SET(CMAKE_INSTALL_LIBRARY_DIR "lib")
ELSE()
    SET(CMAKE_INSTALL_LIBRARY_DIR "${CMAKE_INSTALL_LIBRARY_DIR}")
ENDIF("${CMAKE_INSTALL_LIBDIR}" STREQUAL "")

IF("${CMAKE_INSTALL_BINDIR}" STREQUAL "")
    SET(CMAKE_INSTALL_BINARY_DIR "bin")
ELSE()
    SET(CMAKE_INSTALL_BINARY_DIR "${CMAKE_INSTALL_BINDIR}")
ENDIF("${CMAKE_INSTALL_BINDIR}" STREQUAL "")

IF("${CMAKE_INSTALL_INCLUDEDIR}" STREQUAL "")
    SET(CMAKE_INSTALL_INCLUDE_DIR "include")
    SET(CMAKE_INSTALL_MODULE_DIR "include/modules/")
ELSE()
    SET(CMAKE_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
    SET(CMAKE_INSTALL_MODULE_DIR "${CMAKE_INSTALL_INCLUDEDIR}/modules/")
ENDIF("${CMAKE_INSTALL_INCLUDEDIR}" STREQUAL "")

SET(CMAKE_INSTALL_EXPORT_DIR "cmake")

# Allow hot reload in MSVC.
IF(POLICY CMP0141)
  CMAKE_POLICY(SET CMP0141 NEW)
  SET(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
ENDIF(POLICY CMP0141)

# Make sure the target export configuration proxy and helper scripts get installed.
INCLUDE(CMakePackageConfigHelpers)

CONFIGURE_FILE("cmake/LiteFXConfig-version.cmake" "${CMAKE_CURRENT_BINARY_DIR}/LiteFXConfig-version.cmake" @ONLY)
CONFIGURE_PACKAGE_CONFIG_FILE("cmake/LiteFXConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/LiteFXConfig.cmake" 
    INSTALL_DESTINATION "${CMAKE_INSTALL_EXPORT_DIR}"
)

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/LiteFXConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/LiteFXConfig-version.cmake" "cmake/Assets.cmake" "cmake/Shaders.cmake" 
    DESTINATION "${CMAKE_INSTALL_EXPORT_DIR}"
)
