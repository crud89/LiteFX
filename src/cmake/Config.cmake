###################################################################################################
#####                                                                                         #####
#####                          Contains LiteFX build configurations.                          #####
#####                                                                                         #####
###################################################################################################

# C++ standard version.
SET(CMAKE_CXX_STANDARD 17)

# For debug builds, append the "d" suffix.
SET(CMAKE_DEBUG_POSTFIX "d")

# If not provided to be any different, initialize the install directory.
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "../install/${CMAKE_GENERATOR_PLATFORM}" CACHE PATH "Installation directory." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Setup installation directories.
SET (CMAKE_INSTALL_LIBRARY_DIR "lib")
SET (CMAKE_INSTALL_BINARY_DIR  "bin")
SET (CMAKE_INSTALL_EXPORT_DIR  "cmake")
SET (CMAKE_INSTALL_INCLUDE_DIR "include")
SET (CMAKE_INSTALL_INCLUDE_DIR "include")

# Export all symbols automatically on Windows.
IF (WIN32)
    SET (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
ENDIF()
