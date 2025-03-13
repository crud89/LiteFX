###################################################################################################
#####                                                                                         #####
#####                          Contains LiteFX build configurations.                          #####
#####                                                                                         #####
###################################################################################################

# C++ standard version.
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define C++ compile flags.
IF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Enable "just-my-code" for debug builds.
    ADD_COMPILE_OPTIONS($<$<CONFIG:Debug,RelWithDebInfo>:/JMC>)

    # Be more pedantic with warnings and treat them as errors for release builds.
    ADD_COMPILE_OPTIONS(/W4 $<$<CONFIG:Release,RelWithDebInfo>:/WX>)
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # For clang-cl we need a different warning level, as `/W4` is mapped to `/Wall` which then gets mapped to `-Weverything`, which warns about C++98 compatibility and other obscure flavors irrelevant to modern C++.
    ADD_COMPILE_OPTIONS(-W4 -Wno-missing-field-initializers $<$<CONFIG:Release,RelWithDebInfo>:-Werror> $<$<CONFIG:Debug,RelWithDebInfo>:/JMC> $<$<CONFIG:Debug,RelWithDebInfo>:/Zi> /EHsc)
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Be more pedantic with warnings and treat them as errors for release builds.
    # Explicitly disable the following warnings:
    # - Wextra-semi: Complains a lot about code in `generator.hpp`. Can be disabled after VS 17.13 release.
    # - Wlanguage-extension-token: Complains about usage of `IID_PPV_ARGS()`.
    ADD_COMPILE_OPTIONS(-Wall -Wpedantic -Wno-extra-semi -Wno-language-extension-token -Wno-unknown-pragmas -Wno-gnu-zero-variadic-macro-arguments $<$<CONFIG:Release,RelWithDebInfo>:-Werror>)
ENDIF(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

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
