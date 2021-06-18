###################################################################################################
#####                                                                                         #####
#####               Contains the build script for glslc/dxc shader compilers.                 #####
#####                                                                                         #####
###################################################################################################

SET(DXIL_DEFAULT_SUFFIX ".dxi" CACHE STRING "Default file extension for DXIL shaders.")
SET(SPIRV_DEFAULT_SUFFIX ".spv" CACHE STRING "Default file extension for SPIR-V shaders.")

FUNCTION(DXC_COMPILE_DXIL shader_file file_out)
  GET_FILENAME_COMPONENT(file_in ${shader_file} ABSOLUTE)
  GET_FILENAME_COMPONENT(shader_type ${file_in} LAST_EXT)
  
  SET(shader_profile "")

  IF(${shader_type} STREQUAL ".comp")
    SET(shader_profile "cs_${BUILD_HLSL_SHADER_MODEL}")
  
  ELSEIF(${shader_type} STREQUAL ".vert")
    SET(shader_profile "vs_${BUILD_HLSL_SHADER_MODEL}")

  #ELSEIF(${shader_type} STREQUAL ".geom")
  #  SET(shader_profile "gs_${BUILD_HLSL_SHADER_MODEL}")
  
  #ELSEIF(${shader_type} STREQUAL ".tesc")
  #  SET(shader_profile "hs_${BUILD_HLSL_SHADER_MODEL}")

  #ELSEIF(${shader_type} STREQUAL ".tese")
  #  SET(shader_profile "ds_${BUILD_HLSL_SHADER_MODEL}")
  
  ELSEIF(${shader_type} STREQUAL ".frag")
    SET(shader_profile "ps_${BUILD_HLSL_SHADER_MODEL}")

  #ELSEIF(${shader_type} STREQUAL ".slib")
  #  SET(shader_profile "lib_${BUILD_HLSL_SHADER_MODEL}")
  ENDIF(${shader_type} STREQUAL ".comp")
  
  SET(compiler_options "")

  IF (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    LIST(APPEND compiler_options -Zi -Qembed_debug)
  ELSE()
    LIST(APPEND compiler_options -Vd -Qstrip_debug)
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

  LIST(APPEND compiler_options -D DXIL)

  ADD_CUSTOM_COMMAND(OUTPUT ${file_out} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "dxc: compiling hlsl shader '${shader_file}' (profile: ${shader_profile}) to DXIL..."
    DEPENDS ${shader_file} 
    COMMAND ${BUILD_DXC_COMPILER} -T ${shader_profile} -E main -Fo ${file_out} ${compiler_options} ${file_in}
  )
ENDFUNCTION(DXC_COMPILE_DXIL)

FUNCTION(DXC_COMPILE_SPIRV shader_file file_out)
  GET_FILENAME_COMPONENT(file_in ${shader_file} ABSOLUTE)
  GET_FILENAME_COMPONENT(shader_type ${file_in} LAST_EXT)

  IF(${shader_type} STREQUAL ".comp")
    SET(shader_profile "cs_${BUILD_HLSL_SHADER_MODEL}")
  
  ELSEIF(${shader_type} STREQUAL ".vert")
    SET(shader_profile "vs_${BUILD_HLSL_SHADER_MODEL}")

  #ELSEIF(${shader_type} STREQUAL ".geom")
  #  SET(shader_profile "gs_${BUILD_HLSL_SHADER_MODEL}")
  
  #ELSEIF(${shader_type} STREQUAL ".tesc")
  #  SET(shader_profile "hs_${BUILD_HLSL_SHADER_MODEL}")

  #ELSEIF(${shader_type} STREQUAL ".tese")
  #  SET(shader_profile "ds_${BUILD_HLSL_SHADER_MODEL}")
  
  ELSEIF(${shader_type} STREQUAL ".frag")
    SET(shader_profile "ps_${BUILD_HLSL_SHADER_MODEL}")

  #ELSEIF(${shader_type} STREQUAL ".slib")
  #  SET(shader_profile "lib_${BUILD_HLSL_SHADER_MODEL}")
  ENDIF(${shader_type} STREQUAL ".comp")

  SET(compiler_options "")

  IF (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    #LIST(APPEND compiler_options -Fd ${file_out}.pdb -Zi)
    LIST(APPEND compiler_options -Zi)
  ELSE()
    LIST(APPEND compiler_options -Vd)
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

  LIST(APPEND compiler_options -D SPIRV)

  ADD_CUSTOM_COMMAND(OUTPUT ${file_out} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "dxc: compiling hlsl shader '${shader_file}' (profile: ${shader_profile}) to SPIR-V..."
    DEPENDS ${shader_file} 
    COMMAND ${BUILD_DXC_COMPILER} -spirv -T ${shader_profile} -E main -Fo ${file_out} ${compiler_options} ${file_in}
  )
ENDFUNCTION(DXC_COMPILE_SPIRV)

FUNCTION(TARGET_HLSL_SHADERS target_name)
  SET(compiled_shaders "")

  IF(BUILD_USE_GLSLC)
    FOREACH(shader_source ${SHADER_SOURCES})
      GET_FILENAME_COMPONENT(shader_name ${shader_source} NAME)
	  SET(output_file ${CMAKE_CURRENT_BINARY_DIR}/shaders/${shader_name}.spv)
      GLSLC_COMPILE_HLSL(${shader_source} ${output_file})
      LIST(APPEND compiled_shaders ${output_file})
    ENDFOREACH(shader_source ${SHADER_SOURCES})
  ELSEIF(BUILD_USE_DXC)
    FOREACH(shader_source ${SHADER_SOURCES})
      GET_FILENAME_COMPONENT(shader_name ${shader_source} NAME)

      IF(BUILD_DXC_DXIL)
	    SET(output_file ${CMAKE_CURRENT_BINARY_DIR}/shaders/${shader_name}.dxi)
        DXC_COMPILE_DXIL(${shader_source} ${output_file})
        LIST(APPEND compiled_shaders ${output_file})
      ENDIF(BUILD_DXC_DXIL)

      IF(BUILD_DXC_SPIRV)
	    SET(output_file ${CMAKE_CURRENT_BINARY_DIR}/shaders/${shader_name}.spv)
        DXC_COMPILE_SPIRV(${shader_source} ${output_file})
        LIST(APPEND compiled_shaders ${output_file})
      ENDIF(BUILD_DXC_SPIRV)
    ENDFOREACH(shader_source ${SHADER_SOURCES})
  ENDIF(BUILD_USE_GLSLC)
  
  ADD_CUSTOM_TARGET(${target_name}.Shaders ALL DEPENDS ${compiled_shaders} SOURCES ${SHADER_SOURCES})
  ADD_DEPENDENCIES(${target_name} ${target_name}.Shaders)
  SET_TARGET_PROPERTIES(${target_name}.Shaders PROPERTIES FOLDER "Shaders" VERSION ${LITEFX_VERSION} SOVERSION ${LITEFX_YEAR})

  INSTALL(FILES ${compiled_shaders} DESTINATION ${CMAKE_INSTALL_BINARY_DIR}/shaders)
ENDFUNCTION(TARGET_HLSL_SHADERS target_name)

FUNCTION(GLSLC_COMPILE_GLSL shader_file file_out)
  GET_FILENAME_COMPONENT(file_in ${shader_file} ABSOLUTE)
  ADD_CUSTOM_COMMAND(OUTPUT ${file_out} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "glslc: compiling glsl shader '${shader_file}'..."
    DEPENDS ${shader_file} 
    COMMAND ${BUILD_GLSLC_COMPILER} -mfmt=c -DSPIRV -x glsl -c ${file_in} -o ${file_out} -MD
  )
ENDFUNCTION(GLSLC_COMPILE_GLSL)

FUNCTION(GLSLC_COMPILE_HLSL shader_file file_out)
  GET_FILENAME_COMPONENT(file_in ${shader_file} ABSOLUTE)
  ADD_CUSTOM_COMMAND(OUTPUT ${file_out} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "glslc: compiling hlsl shader '${shader_file}'..."
    DEPENDS ${shader_file} 
    COMMAND ${BUILD_GLSLC_COMPILER} -mfmt=c -DSPIRV -x hlsl -c ${file_in} -o ${file_out} -MD
  )
ENDFUNCTION(GLSLC_COMPILE_HLSL)

FUNCTION(TARGET_GLSL_SHADERS target_name source_file compile_to shader_type includes)
  IF(BUILD_USE_DXC)
    MESSAGE(SEND_ERROR "Glsl shader can not be build using DXC.")
  ELSEIF(NOT ${compile_to} STREQUAL "SPIR-V")
    MESSAGE(SEND_ERROR "Glsl shaders can only be compiled to SPIR-V.")
  ELSE()
    GET_FILENAME_COMPONENT(source_file ${shader_source} NAME)
    GET_FILENAME_COMPONENT(file_in ${shader_file} ABSOLUTE)

    IF(${shader_type} STREQUAL "VERTEX")
      SET(SHADER_STAGE "vert")
    IF(${shader_type} STREQUAL "GEOMETRY")
      SET(SHADER_STAGE "geom")
    IF(${shader_type} STREQUAL "FRAGMENT" OR ${shader_type} STREQUAL "PIXEL")
      SET(SHADER_STAGE "frag")
    IF(${shader_type} STREQUAL "HULL" OR ${shader_type} STREQUAL "TESSELATION_CONTROL")
      SET(SHADER_STAGE "tesc")
    IF(${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")
      SET(SHADER_STAGE "tese")
    IF(${shader_type} STREQUAL "COMPUTE" OR ${shader_type} STREQUAL "RAYTRACING")
      SET(SHADER_STAGE "comp")
    ELSE()
      MESSAGE(SEND_ERROR "Unsupported shader type: ${shader_type}. Valid shader types are: VERTEX, GEOMETRY, HULL/TESSELATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, FRAGMENT/PIXEL, COMPUTE and RAYTRACING.")
    ENDIF(${shader_type} STREQUAL "VERTEX")

    ADD_CUSTOM_TARGET(${target_name} 
      COMMAND ${BUILD_GLSLC_COMPILER} -mfmt=c -DSPIRV -x glsl -fshader_stage=${SHADER_STAGE} -c ${file_in} -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/$<TARGET_FILE:${target_name}>" -MD
      COMMENT "glslc: compiling glsl shader '${shader_file}'..."
      DEPENDS ${source_file} ${includes}
    )

    SET_TARGET_PROPERTIES(${target_name} PROPERTIES 
      SOURCES ${source_file} ${includes}
      OUTPUT_NAME ${target_name}
      NAME ${target_name}
      SUFFIX ${SPIRV_DEFAULT_SUFFIX}
    )
  
    #INSTALL(FILES ${compiled_shaders} DESTINATION ${CMAKE_INSTALL_BINARY_DIR}/shaders)
  ENDIF(BUILD_USE_DXC)
ENDFUNCTION(TARGET_GLSL_SHADERS target_name)

FUNCTION(ADD_SHADER_PROGRAM program_target)
  # TODO: We can add a property for the compiler here, too.
  CMAKE_PARSE_ARGUMENTS(SHADER "" "SOURCE;LANGUAGE;COMPILE_TO;SHADER_MODEL;TYPE" "INCLUDES" ${ARGN})
  
  IF(${SHADER_LANGUAGE} STREQUAL "GLSL")
    TARGET_GLSL_SHADER(${target_name} ${SOURCE} ${COMPILE_TO} ${TYPE} ${INCLUDES})
  ELSEIF(${SHADER_LANGUAGE} STREQUAL "HLSL")
    TARGET_HLSL_SHADERS(${target_name} ${SOURCE} ${SHADER_MODEL} ${COMPILE_TO} ${TYPE} ${INCLUDES})
  ELSE()
    MESSAGE(SEND_ERROR "Unsupported shader language: ${SHADER_LANGUAGE}.")
  ENDIF(${SHADER_LANGUAGE} STREQUAL "GLSL")
ENDFUNCTION(TARGET_SHADER_SOURCES program_target)

FUNCTION(TARGET_SHADER_PROGRAMS target_name)
  CMAKE_PARSE_ARGUMENTS(SHADER "" "INSTALL_DESTINATION" "SHADER_PROGRAMS" ${ARGN})
  
  ADD_DEPENDENCIES(${target_name} ${SHADER_PROGRAMS})
  GET_TARGET_PROPERTY(SHADER_PROGRAM_NAME ${target_name} OUTPUT_NAME)
  GET_TARGET_PROPERTY(SHADER_PROGRAM_SUFFIX ${target_name} SUFFIX)
  INSTALL(FILES "${SHADER_PROGRAM_NAME}${SHADER_PROGRAM_SUFFIX}" ${INSTALL_DESTINATION})
ENDFUNCTION(TARGET_SHADER_PROGRAMS target_name)