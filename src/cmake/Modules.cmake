###################################################################################################
#####                                                                                         #####
#####                            Contains LiteFX module settings.                             #####
#####                                                                                         #####
###################################################################################################

# Setup default module path.
IF("${CMAKE_MODULE_PATH}" STREQUAL "")
    SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" CACHE PATH "Module directory." FORCE)
ENDIF("${CMAKE_MODULE_PATH}" STREQUAL "")

# Bootstrap vcpkg
MESSAGE(STATUS "Looking for vcpkg at '${VCPKG_MODULE_DIRECTORY}'...")
FIND_PROGRAM(VCPKG_EXECUTABLE vcpkg PATHS ${VCPKG_MODULE_DIRECTORY} NO_DEFAULT_PATH)

IF(NOT VCPKG_EXECUTABLE)
    MESSAGE(STATUS "Compiling vcpkg...")

    IF(WIN32)
        EXECUTE_PROCESS(COMMAND "${VCPKG_MODULE_DIRECTORY}/bootstrap-vcpkg.bat" -disableMetrics WORKING_DIRECTORY "${VCPKG_MODULE_DIRECTORY}")
    ELSE(WIN32)
        EXECUTE_PROCESS(COMMAND "${VCPKG_MODULE_DIRECTORY}/bootstrap-vcpkg.sh" -disableMetrics WORKING_DIRECTORY "${VCPKG_MODULE_DIRECTORY}")
    ENDIF(WIN32)
ELSE(NOT VCPKG_EXECUTABLE)
    MESSAGE(STATUS "Found vcpkg at '${VCPKG_EXECUTABLE}'.")
ENDIF(NOT VCPKG_EXECUTABLE)

SET(AUTO_VCPKG_ROOT ${VCPKG_MODULE_DIRECTORY})
INCLUDE(AutoVcpkg)