###################################################################################################
#####                                                                                         #####
#####                          Contains LiteFX build configurations.                          #####
#####                                                                                         #####
###################################################################################################

# C++ standard version.
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define C++ compile flags.
IF(MSVC)
    IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17.10")
        MESSAGE(FATAL_ERROR "The minimum required MSVC version is 17.10, but only ${CMAKE_CXX_COMPILER_VERSION} has been found.")
    ENDIF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17.10")

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")         # Use multi core compiling to speed up compile times.
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.2")
        MESSAGE(FATAL_ERROR "The minimum required GCC version is 14.2, but only ${CMAKE_CXX_COMPILER_VERSION} has been found.")
    ENDIF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.2")
ENDIF(MSVC)

# For debug builds, append the "d" suffix.
SET(CMAKE_DEBUG_POSTFIX "d")

# If not provided to be any different, initialize the install directory.
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "../install/${CMAKE_GENERATOR_PLATFORM}" CACHE PATH "Installation directory." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Setup build artifact directory.
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/binaries/>")

# Setup installation directories.
IF(CMAKE_INSTALL_LIBDIR STREQUAL "")
    SET(CMAKE_INSTALL_LIBRARY_DIR "${CMAKE_INSTALL_LIBRARY_DIR}")
ELSE()
    SET(CMAKE_INSTALL_LIBRARY_DIR "lib")
ENDIF(CMAKE_INSTALL_LIBDIR STREQUAL "")

IF(CMAKE_INSTALL_BINDIR STREQUAL "")
    SET(CMAKE_INSTALL_BINARY_DIR "${CMAKE_INSTALL_BINDIR}")
ELSE()
    SET(CMAKE_INSTALL_BINARY_DIR "bin")
ENDIF(CMAKE_INSTALL_BINDIR STREQUAL "")

IF(CMAKE_INSTALL_INCLUDEDIR STREQUAL "")
    SET(CMAKE_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
ELSE()
    SET(CMAKE_INSTALL_INCLUDE_DIR "include")
ENDIF(CMAKE_INSTALL_INCLUDEDIR STREQUAL "")

SET(CMAKE_INSTALL_EXPORT_DIR  "cmake")

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
