###################################################################################################
#####                                                                                         #####
#####                            LiteFX runtime deployment helper.                            #####
#####                                                                                         #####
###################################################################################################

INCLUDE_GUARD(GLOBAL)

FUNCTION(_LITEFX_DEPLOY_FILES target group subdir)
  SET(_files ${ARGN})

  IF(NOT _files)
    RETURN()
  ENDIF()

  GET_TARGET_PROPERTY(_output_dir ${target} RUNTIME_OUTPUT_DIRECTORY)

  IF(NOT _output_dir)
    SET(_destination "$<TARGET_FILE_DIR:${target}>")

    IF(subdir)
      STRING(APPEND _destination "/${subdir}")
    ENDIF()

    ADD_CUSTOM_COMMAND(TARGET ${target} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${_destination}"
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${_files} "${_destination}"
      VERBATIM)

    RETURN()
  ENDIF()

  IF(NOT _output_dir MATCHES "\\$<")
    # Relative output directories are interpreted relative to the target's binary directory.
    IF(NOT IS_ABSOLUTE "${_output_dir}")
      GET_TARGET_PROPERTY(_binary_dir ${target} BINARY_DIR)
      SET(_output_dir "${_binary_dir}/${_output_dir}")
    ENDIF()

    GET_PROPERTY(_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

    IF(_multi_config)
      STRING(APPEND _output_dir "/$<CONFIG>")
    ENDIF()
  ENDIF()

  SET(_destination "${_output_dir}")

  IF(subdir)
    STRING(APPEND _destination "/${subdir}")
  ENDIF()

  STRING(MD5 _hash "${_destination}")
  STRING(SUBSTRING "${_hash}" 0 8 _hash)
  SET(_deploy_target "LiteFX.Deploy.${group}.${_hash}")

  IF(NOT TARGET ${_deploy_target})
    ADD_CUSTOM_TARGET(${_deploy_target}
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${_destination}"
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${_files} "${_destination}"
      COMMENT "Deploying ${group} runtime files to ${_destination}"
      VERBATIM)
  ENDIF()

  ADD_DEPENDENCIES(${target} ${_deploy_target})
ENDFUNCTION()

# LITEFX_DEPLOY_RUNTIME(<target> [WITH_WARP] [INSTALL_DESTINATION <dir>])
#
# Sets up an executable's runtime for both, the build and install tree.
FUNCTION(LITEFX_DEPLOY_RUNTIME target)
  CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 arg "WITH_WARP" "INSTALL_DESTINATION" "")

  IF(arg_UNPARSED_ARGUMENTS)
    MESSAGE(FATAL_ERROR "LITEFX_DEPLOY_RUNTIME: Unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
  ENDIF()

  IF(NOT TARGET ${target})
    MESSAGE(FATAL_ERROR "LITEFX_DEPLOY_RUNTIME: \"${target}\" is not a target.")
  ENDIF()

  # Copy DirectX 12 Agility SDK.
  IF(TARGET LiteFX.Backends.DirectX12)
    IF(NOT TARGET Microsoft::DirectX12-Core)
      FIND_PACKAGE(directx12-agility CONFIG REQUIRED)
    ENDIF()

    GET_TARGET_PROPERTY(_d3d12_core   Microsoft::DirectX12-Core   IMPORTED_LOCATION_RELEASE)
    GET_TARGET_PROPERTY(_d3d12_layers Microsoft::DirectX12-Layers IMPORTED_LOCATION_DEBUG)

    _LITEFX_DEPLOY_FILES(${target} Agility "d3d12" "${_d3d12_core}" "${_d3d12_layers}")

    IF(arg_INSTALL_DESTINATION)
      INSTALL(FILES "${_d3d12_core}" "${_d3d12_layers}" DESTINATION "${arg_INSTALL_DESTINATION}/d3d12")
    ENDIF()
  ENDIF()

  # WARP.
  IF(arg_WITH_WARP)
    IF(NOT TARGET Microsoft::DirectX-WARP)
      FIND_PACKAGE(directx-warp CONFIG REQUIRED)
    ENDIF()

    GET_TARGET_PROPERTY(_warp Microsoft::DirectX-WARP IMPORTED_LOCATION)

    _LITEFX_DEPLOY_FILES(${target} WARP "" "${_warp}")

    IF(arg_INSTALL_DESTINATION)
      INSTALL(FILES "${_warp}" DESTINATION "${arg_INSTALL_DESTINATION}")
    ENDIF()
  ENDIF()

  # DirectX Shader Compiler runtime.
  IF(TARGET LiteFX.Backends.DirectX12)
    IF(NOT TARGET Microsoft::DirectXShaderCompiler)
      FIND_PACKAGE(directx-dxc CONFIG REQUIRED)
    ENDIF()

    GET_TARGET_PROPERTY(_dxc Microsoft::DirectXShaderCompiler IMPORTED_LOCATION_RELEASE)
    GET_FILENAME_COMPONENT(_dxc_dir "${_dxc}" DIRECTORY)

    # Not taken from Microsoft::DXIL, as the port misspells its IMPORTED_LOCATION_RELEASE property.
    SET(_dxil "${_dxc_dir}/dxil.dll")

    _LITEFX_DEPLOY_FILES(${target} DXC "" "${_dxc}" "${_dxil}")

    IF(arg_INSTALL_DESTINATION)
      INSTALL(FILES "${_dxc}" "${_dxil}" DESTINATION "${arg_INSTALL_DESTINATION}")
    ENDIF()
  ENDIF()
ENDFUNCTION()
