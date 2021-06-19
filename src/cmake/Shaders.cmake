###################################################################################################
#####                                                                                         #####
#####               Contains the build script for glslc/dxc shader compilers.                 #####
#####                                                                                         #####
###################################################################################################

# The script exports two main functions:
#
# - ADD_SHADER_MODULE: Creates a shader module target.
# - TARGET_LINK_SHADERS: Links a set of shader module targets to another target.
#
# TODO: Document interface for ADD_SHADER_MODULE
#
# If CMAKE_RUNTIME_OUTPUT_DIRECTORY is set, the shader modules will be built into a subdirectory within the directory pointed by this variable. If it is not set,
# the subdirectory will be created in CMAKE_CURRENT_BINARY_DIR. The subdirectory name can be set using SHADER_DEFAULT_SUBDIR. The build target location will be 
# stored in the BINARY_DIR property of the shader module target. Furthermore, there are two more properties that are set for a shader module target:
#
# - OUTPUT_NAME: Stores the shader module name (without the target file extension). By default it is set to equal the input file name without the extension.
# - SUFFIX: Stores the file extension (including the dot) for a shader module binary file.
#
# The suffix is configured by the DXIL_DEFAULT_SUFFIX and SPIRV_DEFAULT_SUFFIX variables, which can be overwritten before calling ADD_SHADER_MODULE.
#
# TODO: Document interface for TARGET_LINK_SHADERS
# 
# Linking shader modules automatically creates an install command for the shader module binary. The source file is build from the BINARY_DIR, OUTPUT_NAME and 
# SUFFIX properties of each shader module target. The install destination can be provided by the INSTALL_DESTINATION parameter. Note that it is always prepended
# with the CMAKE_INSTALL_PREFIX.

SET(SHADER_DEFAULT_SUBDIR "shaders" CACHE STRING "Default subdirectory for shader module binaries within the current binary directory (CMAKE_CURRENT_BINARY_DIR).")
SET(DXIL_DEFAULT_SUFFIX ".dxi" CACHE STRING "Default file extension for DXIL shaders.")
SET(SPIRV_DEFAULT_SUFFIX ".spv" CACHE STRING "Default file extension for SPIR-V shaders.")

FUNCTION(TARGET_HLSL_SHADERS target_name source_file shader_model compile_as compile_with shader_type includes)
  GET_FILENAME_COMPONENT(source_file ${shader_source} NAME)
  GET_FILENAME_COMPONENT(out_name ${shader_source} NAME_WE)
  GET_FILENAME_COMPONENT(file_in ${shader_file} ABSOLUTE)
    
  SET (SHADER_STAGE "")
  SET (OUTPUT_DIR "")

  IF(${compile_with} STREQUAL "GLSLC")
    IF(NOT ${compile_as} STREQUAL "SPIR-V")
      MESSAGE(SEND_ERROR "Glsl shaders can only be compiled to SPIR-V.")
    ENDIF(NOT ${compile_as} STREQUAL "SPIR-V")

    IF(${shader_type} STREQUAL "VERTEX")
      SET(SHADER_STAGE "vert")
    ELSEIF(${shader_type} STREQUAL "GEOMETRY")
      SET(SHADER_STAGE "geom")
    ELSEIF(${shader_type} STREQUAL "FRAGMENT" OR ${shader_type} STREQUAL "PIXEL")
      SET(SHADER_STAGE "frag")
    ELSEIF(${shader_type} STREQUAL "HULL" OR ${shader_type} STREQUAL "TESSELATION_CONTROL")
      SET(SHADER_STAGE "tesc")
    ELSEIF(${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")
      SET(SHADER_STAGE "tese")
    ELSEIF(${shader_type} STREQUAL "COMPUTE" OR ${shader_type} STREQUAL "RAYTRACING")
      SET(SHADER_STAGE "comp")
    ELSE()
      MESSAGE(SEND_ERROR "Unsupported shader type: ${shader_type}. Valid shader types are: VERTEX, GEOMETRY, HULL/TESSELATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, FRAGMENT/PIXEL, COMPUTE and RAYTRACING.")
    ENDIF(${shader_type} STREQUAL "VERTEX")
    
    IF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")
      SET(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR})
    ELSE()
      SET(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER_DEFAULT_SUBDIR})
    ENDIF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")

    # TODO: Check if we can use a generator expression to build the output directory and file names, so it is possible to set the target properties to control the result file name.
    ADD_CUSTOM_TARGET(${target_name} 
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND ${BUILD_GLSLC_COMPILER} -mfmt=c -DSPIRV -x hlsl -fshader_stage=${SHADER_STAGE} -c ${file_in} -o "${OUTPUT_DIR}/${out_name}${SPIRV_DEFAULT_SUFFIX}" -MD
      COMMENT "glslc: compiling hlsl shader '${shader_file}'..."
      DEPENDS ${source_file} ${includes}
    )

    SET_TARGET_PROPERTIES(${target_name} PROPERTIES 
      SOURCES ${source_file} ${includes}
      NAME ${target_name}
      OUTPUT_NAME ${out_name}
      SUFFIX ${SPIRV_DEFAULT_SUFFIX}
      BINARY_DIR ${OUTPUT_DIR}
    )
  ELSEIF(${compile_with} STREQUAL "DXC")
    IF(${shader_type} STREQUAL "VERTEX")
      SET(SHADER_STAGE "vs")
    ELSEIF(${shader_type} STREQUAL "GEOMETRY")
      SET(SHADER_STAGE "gs")
    ELSEIF(${shader_type} STREQUAL "FRAGMENT" OR ${shader_type} STREQUAL "PIXEL")
      SET(SHADER_STAGE "ps")
    ELSEIF(${shader_type} STREQUAL "HULL" OR ${shader_type} STREQUAL "TESSELATION_CONTROL")
      SET(SHADER_STAGE "hs")
    ELSEIF(${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")
      SET(SHADER_STAGE "ds")
    ELSEIF(${shader_type} STREQUAL "COMPUTE")
      SET(SHADER_STAGE "cs")
    ELSEIF(${shader_type} STREQUAL "RAYTRACING")
      SET(SHADER_STAGE "lib")
    ELSE()
      MESSAGE(SEND_ERROR "Unsupported shader type: ${shader_type}. Valid shader types are: VERTEX, GEOMETRY, HULL/TESSELATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, FRAGMENT/PIXEL, COMPUTE and RAYTRACING.")
    ENDIF(${shader_type} STREQUAL "VERTEX")

    SET(SHADER_PROFILE "${SHADER_STAGE}_${shader_model}")
    
    IF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")
      SET(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR})
    ELSE()
      SET(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER_DEFAULT_SUBDIR})
    ENDIF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")

    SET(compiler_options "")

    IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      LIST(APPEND compiler_options -Zi)
    ELSE()
      LIST(APPEND compiler_options -Vd)
    ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    
    IF(${compile_as} STREQUAL "SPIR-V")
      LIST(APPEND compiler_options -D SPIRV)
      ADD_CUSTOM_TARGET(${target_name} 
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${BUILD_DXC_COMPILER} -spirv -T ${shader_profile} -E main -Fo ${file_out} ${compiler_options} ${file_in}
        COMMENT "dxc: compiling hlsl shader '${shader_file}' (profile: ${shader_profile}) to SPIR-V..."
        DEPENDS ${source_file} ${includes}
      )
    
      SET_TARGET_PROPERTIES(${target_name} PROPERTIES 
        SOURCES ${source_file} ${includes}
        NAME ${target_name}
        OUTPUT_NAME ${out_name}
        SUFFIX ${SPIRV_DEFAULT_SUFFIX}
        BINARY_DIR ${OUTPUT_DIR}
      )
    ELSEIF(${compile_as} STREQUAL "DXIL")
      LIST(APPEND compiler_options -D DXIL)
      
      IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        LIST(APPEND compiler_options -Qembed_debug)
      ELSE()
        LIST(APPEND compiler_options -Qstrip_debug)
      ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

      ADD_CUSTOM_TARGET(${target_name} 
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${BUILD_DXC_COMPILER} -dxil -T ${shader_profile} -E main -Fo ${file_out} ${compiler_options} ${file_in}
        COMMENT "dxc: compiling hlsl shader '${shader_file}' (profile: ${shader_profile}) to DXIL..."
        DEPENDS ${source_file} ${includes}
      )
    
      SET_TARGET_PROPERTIES(${target_name} PROPERTIES 
        SOURCES ${source_file} ${includes}
        NAME ${target_name}
        OUTPUT_NAME ${out_name}
        SUFFIX ${DXIL_DEFAULT_SUFFIX}
        BINARY_DIR ${OUTPUT_DIR}
      )
    ELSE()
      MESSAGE(SEND_ERROR "Unrecognized intermediate language ${compile_as}. Only SPIR-V and DXIL are supported.")
    ENDIF(${compile_as} STREQUAL "SPIR-V")
  ELSE()
    MESSAGE(SEND_ERROR "Unrecognized compiler: ${compile_with}. Only DXC and GLSLC are allowed.")
  ENDIF(${compile_with} STREQUAL "GLSLC")
ENDFUNCTION(TARGET_HLSL_SHADERS target_name)

FUNCTION(TARGET_GLSL_SHADERS target_name source_file compile_as compile_with shader_type includes)
  GET_FILENAME_COMPONENT(source_file ${shader_source} NAME)
  GET_FILENAME_COMPONENT(out_name ${shader_source} NAME_WE)
  GET_FILENAME_COMPONENT(file_in ${shader_file} ABSOLUTE)
    
  SET (SHADER_STAGE "")
  SET (OUTPUT_DIR "")

  IF(NOT ${compile_with} STREQUAL "GLSLC")
    MESSAGE(SEND_ERROR "Glsl shaders can only be built with glslc.")
  ELSEIF(NOT ${compile_as} STREQUAL "SPIR-V")
    MESSAGE(SEND_ERROR "Glsl shaders can only be compiled to SPIR-V.")
  ELSE()
    IF(${shader_type} STREQUAL "VERTEX")
      SET(SHADER_STAGE "vert")
    ELSEIF(${shader_type} STREQUAL "GEOMETRY")
      SET(SHADER_STAGE "geom")
    ELSEIF(${shader_type} STREQUAL "FRAGMENT" OR ${shader_type} STREQUAL "PIXEL")
      SET(SHADER_STAGE "frag")
    ELSEIF(${shader_type} STREQUAL "HULL" OR ${shader_type} STREQUAL "TESSELATION_CONTROL")
      SET(SHADER_STAGE "tesc")
    ELSEIF(${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")
      SET(SHADER_STAGE "tese")
    ELSEIF(${shader_type} STREQUAL "COMPUTE" OR ${shader_type} STREQUAL "RAYTRACING")
      SET(SHADER_STAGE "comp")
    ELSE()
      MESSAGE(SEND_ERROR "Unsupported shader type: ${shader_type}. Valid shader types are: VERTEX, GEOMETRY, HULL/TESSELATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, FRAGMENT/PIXEL, COMPUTE and RAYTRACING.")
    ENDIF(${shader_type} STREQUAL "VERTEX")
    
    IF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")
      SET(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR})
    ELSE()
      SET(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER_DEFAULT_SUBDIR})
    ENDIF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")

    # TODO: Check if we can use a generator expression to build the output directory and file names, so it is possible to set the target properties to control the result file name.
    ADD_CUSTOM_TARGET(${target_name} 
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND ${BUILD_GLSLC_COMPILER} -mfmt=c -DSPIRV -x glsl -fshader_stage=${SHADER_STAGE} -c ${file_in} -o "${OUTPUT_DIR}/${out_name}${SPIRV_DEFAULT_SUFFIX}" -MD
      COMMENT "glslc: compiling glsl shader '${shader_file}'..."
      DEPENDS ${source_file} ${includes}
    )

    SET_TARGET_PROPERTIES(${target_name} PROPERTIES 
      SOURCES ${source_file} ${includes}
      NAME ${target_name}
      OUTPUT_NAME ${out_name}
      SUFFIX ${SPIRV_DEFAULT_SUFFIX}
      BINARY_DIR ${OUTPUT_DIR}
    )
  ENDIF(NOT ${compile_with} STREQUAL "GLSLC")
ENDFUNCTION(TARGET_GLSL_SHADERS target_name source_file compile_to shader_type includes)

FUNCTION(ADD_SHADER_MODULE module_name)
  CMAKE_PARSE_ARGUMENTS(SHADER "" "SOURCE;LANGUAGE;COMPILE_AS;SHADER_MODEL;TYPE;COMPILER" "INCLUDES" ${ARGN})
  
  # TODO: There's also the shader type ms (mesh shader) and as (amplification shader; used as a second ms stage) since shader model 6.5.
  #       see: https://microsoft.github.io/DirectX-Specs/d3d/HLSL_ShaderModel6_5.html

  IF(${SHADER_LANGUAGE} STREQUAL "GLSL")
    TARGET_GLSL_SHADER(${module_name} ${SOURCE} ${COMPILE_AS} ${COMPILER} ${TYPE} ${INCLUDES})
  ELSEIF(${SHADER_LANGUAGE} STREQUAL "HLSL")
    TARGET_HLSL_SHADERS(${module_name} ${SOURCE} ${SHADER_MODEL} ${COMPILE_AS} ${COMPILER} ${TYPE} ${INCLUDES})
  ELSE()
    MESSAGE(SEND_ERROR "Unsupported shader language: ${SHADER_LANGUAGE}.")
  ENDIF(${SHADER_LANGUAGE} STREQUAL "GLSL")
ENDFUNCTION(ADD_SHADER_MODULE module_name)

FUNCTION(TARGET_LINK_SHADERS target_name)
  CMAKE_PARSE_ARGUMENTS(SHADER "" "INSTALL_DESTINATION" "SHADERS" ${ARGN})
  
  ADD_DEPENDENCIES(${target_name} ${SHADERS})

  FOREACH(shader_module ${SHADERS})
    GET_TARGET_PROPERTY(SHADER_PROGRAM_NAME ${shader_module} OUTPUT_NAME)
    GET_TARGET_PROPERTY(SHADER_PROGRAM_SUFFIX ${shader_module} SUFFIX)
    GET_TARGET_PROPERTY(SHADER_PROGRAM_BINARY_DIR ${shader_module} BINARY_DIR)
    INSTALL(FILES "${SHADER_PROGRAM_BINARY_DIR}/${SHADER_PROGRAM_NAME}${SHADER_PROGRAM_SUFFIX}" ${CMAKE_INSTALL_PREFIX}/${INSTALL_DESTINATION})
  ENDFOREACH(shader_module ${SHADERS})
ENDFUNCTION(TARGET_LINK_SHADERS target_name)