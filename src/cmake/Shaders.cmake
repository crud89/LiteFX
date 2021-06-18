###################################################################################################
#####                                                                                         #####
#####               Contains the build script for glslc/dxc shader compilers.                 #####
#####                                                                                         #####
###################################################################################################

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

FUNCTION(TARGET_GLSL_SHADERS target_name)
  SET(compiled_shaders "")

  IF(BUILD_USE_DXC)
    MESSAGE(SEND_ERROR "Glsl shader can not be build using DXC.")
  ELSE()
    FOREACH(SHADER_SOURCE ${SHADER_SOURCES})
      GET_FILENAME_COMPONENT(shader_name ${shader_source} NAME)
      SET(output_file ${CMAKE_CURRENT_BINARY_DIR}/shaders/${shader_name}.spv)
      GLSLC_COMPILE_GLSL(${shader_source} ${output_file})
      LIST(APPEND compiled_shaders ${output_file})
    ENDFOREACH(SHADER_SOURCE ${SHADER_SOURCES})

    ADD_CUSTOM_TARGET(${target_name}.Shaders ALL DEPENDS ${compiled_shaders} SOURCES ${SHADER_SOURCES})
    ADD_DEPENDENCIES(${target_name} ${target_name}.Shaders)
    SET_TARGET_PROPERTIES(${target_name}.Shaders PROPERTIES FOLDER "Shaders" VERSION ${LITEFX_VERSION} SOVERSION ${LITEFX_YEAR})
  
    INSTALL(FILES ${compiled_shaders} DESTINATION ${CMAKE_INSTALL_BINARY_DIR}/shaders)
  ENDIF(BUILD_USE_DXC)
ENDFUNCTION(TARGET_GLSL_SHADERS target_name)

FUNCTION(TARGET_SHADER_SOURCES target_name)
  CMAKE_PARSE_ARGUMENTS(SHADER "" "LANGUAGE" "SOURCES" ${ARGN})
  
  IF(${SHADER_LANGUAGE} STREQUAL "GLSL")
    TARGET_GLSL_SHADERS(${target_name})
  ELSEIF(${SHADER_LANGUAGE} STREQUAL "HLSL")
    TARGET_HLSL_SHADERS(${target_name})
  ELSE()
    MESSAGE(SEND_ERROR "Unsupported shader language: ${SHADER_LANGUAGE}.")
  ENDIF(${SHADER_LANGUAGE} STREQUAL "GLSL")
ENDFUNCTION(TARGET_SHADER_SOURCES target_name)