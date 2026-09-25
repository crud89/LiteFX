###################################################################################################
#####                                                                                         #####
#####               Contains the build script for glslc/dxc shader compilers.                 #####
#####                                                                                         #####
###################################################################################################

# The script exports the following functions:
#
# - TARGET_ADD_SHADER_PROGRAM: Defines all shader modules of a shader program for one or more backends and links them to a target (recommended).
# - ADD_SHADER_MODULE: Creates a single shader module target.
# - TARGET_LINK_SHADERS: Links a set of shader module targets to another target.
# - ADD_SHADER_LIBRARY: Creates a shader library, i.e., a header that embeds the binaries of a set of shader modules.
# - TARGET_LINK_SHADER_LIBRARIES: Links a set of shader libraries to another target.
#
# In most cases, TARGET_ADD_SHADER_PROGRAM is sufficient. It defines a shader program, i.e., a set of shader modules for different stages, compiles it for each
# backend and links the resulting modules to a target:
#
# TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
#   VERTEX "shaders/basic_vs.hlsl"
#   PIXEL "shaders/basic_fs.hlsl"
#   SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
#   FOLDER "Samples/Shaders"
#   INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/${SHADER_DEFAULT_SUBDIR}"
# )
#
# Shader programs can also be authored in a single file. In this case, SOURCE provides the file and each stage keyword optionally names the entry point of
# the stage (default: main). The binaries are named `<source name>_<stage>`, e.g. `basic_vs` and `basic_fs`:
#
# TARGET_ADD_SHADER_PROGRAM(${PROJECT_NAME}
#   SOURCE "shaders/basic.hlsl"
#   VERTEX VS_Main
#   PIXEL PS_Main
#   SHADER_MODEL ${LITEFX_BUILD_HLSL_SHADER_MODEL}
# )
#
# The stage suffixes are: vs (VERTEX), gs (GEOMETRY), hs (HULL/TESSELLATION_CONTROL), ds (DOMAIN/TESSELLATION_EVALUATION), fs (FRAGMENT/PIXEL), cs (COMPUTE),
# ts (TASK/AMPLIFICATION), ms (MESH) and rt (RAYTRACING). GLSL does not support entry points other than main. Instead, each module is compiled with a define
# that identifies its stage (e.g. SHADER_STAGE_VERTEX, see ADD_SHADER_MODULE), so that the stages can be separated using the preprocessor. In this case, the
# stage keywords are specified without values, e.g. `SOURCE "shaders/basic.glsl" VERTEX FRAGMENT`. Only one entry point per stage is supported with SOURCE.
#
# Without SOURCE, each stage keyword (VERTEX, GEOMETRY, HULL/TESSELLATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, FRAGMENT/PIXEL, COMPUTE, TASK/AMPLIFICATION,
# MESH and RAYTRACING) accepts one or more source files. Each source file becomes one shader module per backend. Shader modules with identical settings are shared, so
# multiple programs (also of different targets) can use the same source file, e.g. a common vertex shader. A source file can, however, only be compiled with
# one set of settings (stage, compile options, includes, ...) per backend, since the binary is named after the source file.
#
# The SPIRV and DXIL flags select the backends to compile for. If none is set, the program is compiled for all backends LiteFX has been built with (SPIR-V
# only for GLSL). LANGUAGE defaults to HLSL and COMPILER to DXC (HLSL) or GLSLC (GLSL). SHADER_MODEL, INCLUDES and LIBRARY are forwarded to each shader
# module (see ADD_SHADER_MODULE). COMPILE_OPTIONS are passed for all backends, while SPIRV_COMPILE_OPTIONS and DXIL_COMPILE_OPTIONS are only passed for
# the respective backend.
#
# The shader module targets are named `<target>.<Vk|Dx>.Shaders.<binary name>`. If FOLDER is provided, they are placed in the IDE folders
# `<folder>/Vulkan` and `<folder>/DirectX 12`. If INSTALL_DESTINATION is provided, the shader binaries are installed to it.
#
# For shader modules that need individual settings (e.g. multiple entry points of the same stage), use ADD_SHADER_MODULE and TARGET_LINK_SHADERS directly.
#
# Shader modules are built from one source file and (optionally) multiple includes. Those includes may be used by multiple shader module targets simultaneously.
# The output of a shader module target is a single binary file. You have to call ADD_SHADER_MODULE for each shader you want to compile and then link all shaders
# to your project by calling TARGET_LINK_SHADERS. The following example shows how to define a shader module target:
#
# ADD_SHADER_MODULE(${PROJECT_NAME}.VertexShader
#   SOURCE "vs.hlsl"
#   LANGUAGE HLSL
#   TYPE VERTEX
#   COMPILE_AS DXIL
#   SHADER_MODEL 6_3
#   COMPILER DXC
#   ENTRY_POINT "main"
#   COMPILE_OPTIONS "-HV 2021"
#   INCLUDES "a.hlsli" "b.hlsli"
#   LIBRARY ${PROJECT_NAME}.Shaders
#   FOLDER "Shaders"
# )
#
# The first parameter is the unique name of the shader module target. The SOURCE parameter provides a file name relative to CMAKE_CURRENT_SOURCE_DIR, that
# contains the main shader source.
#
# The LANGUAGE parameter specifies the shader language. Possible values are GLSL and HLSL. Note that the language influences which compiler you can choose and
# which intermediate language (COMPILE_AS) can be selected. See below for more details.
#
# The TYPE parameter dictates the shader module type. Valid values are VERTEX, GEOMETRY, HULL/TESSELLATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION,
# FRAGMENT/PIXEL, COMPUTE, TASK/AMPLIFICATION, MESH and RAYTRACING. Values separated by '/' are synonymous (the misspelled TESSELATION_* variants are accepted
# as well). Note that for the GLSLC compiler, RAYTRACING and COMPUTE are treated as synonymous, too.
#
# The COMPILE_AS parameter specifies the intermediate language. Valid values are DXIL (for DirectX 12) and SPIRV (for Vulkan). DXIL can only be built using DXC.
#
# The SHADER_MODEL parameter is optional for the GLSLC compiler, but mandatory for the DXC compiler. It specifies the shader profile used to build HLSL shaders.
# Not all combinations are valid. For example, it is not valid to use legacy shader model (5_x) for RAYTRACING shaders. Please consult the DXC help
# (`dxc --help`) for more information.
#
# The COMPILER parameter specifies which compiler to use. GLSL shaders can only be compiled with GLSLC, while HLSL shaders can be also compiled with DXC. On the
# other hand, DXC can target both, DXIL and SPIRV, whilst GLSLC can only target SPIRV.
#
# The ENTRY_POINT parameter specifies the shader entry point, i.e., the name of the function at which the shader executes. If it is not set, the entry point will
# default to "main". Entry points are supported for HLSL (with DXC and GLSLC). GLSL shaders always use "main".
#
# Each shader module is compiled with a define that identifies its stage: SHADER_STAGE_VERTEX, SHADER_STAGE_GEOMETRY, SHADER_STAGE_TESSELLATION_CONTROL,
# SHADER_STAGE_TESSELLATION_EVALUATION, SHADER_STAGE_FRAGMENT, SHADER_STAGE_COMPUTE, SHADER_STAGE_TASK, SHADER_STAGE_MESH or SHADER_STAGE_RAYTRACING. This allows
# putting multiple stages into one source file, which is the only way to do so for GLSL.
#
# The OUTPUT_NAME parameter optionally sets the name of the binary (without extension). By default, the binary is named after the source file.
#
# The COMPILE_OPTIONS parameter provides additional compiler options as a single (space-separated) string.
#
# The INCLUDES parameter lists additional include sources. They are added to the target, so that they are accessible from the IDE, and the shader is rebuilt
# if one of them changes. Note that their names are also relative to the CMAKE_CURRENT_SOURCE_DIR. For GLSLC, includes are also tracked automatically.
#
# The LIBRARY parameter optionally adds the shader module to a shader library (see ADD_SHADER_LIBRARY).
#
# The FOLDER parameter optionally sets the IDE folder of the shader module target.
#
# Shader modules are built into a subdirectory of the runtime output directory (see LITEFX_GET_RUNTIME_DIRECTORY in Runtime.cmake). The subdirectory name can be
# set using SHADER_DEFAULT_SUBDIR. Shaders are only rebuilt, if their source or one of their includes changes. The following properties are set for a shader
# module target and can be read after calling ADD_SHADER_MODULE (changing them does not affect the output):
#
# - RUNTIME_OUTPUT_DIRECTORY: The directory the shader module binary is built into (may contain generator expressions).
# - OUTPUT_NAME: The shader module name (without the file extension), which defaults to the input file name without its extension.
# - SUFFIX: The file extension (including the dot) of the shader module binary.
#
# The suffix is configured by the DXIL_DEFAULT_SUFFIX and SPIRV_DEFAULT_SUFFIX variables, which can be overwritten before calling ADD_SHADER_MODULE. The Vulkan
# target environment for SPIR-V is configured by SPIRV_DEFAULT_TARGET_ENV.
#
# A shader module target can defined as a dependency by using TARGET_LINK_SHADERS:
#
# TARGET_LINK_SHADERS(${PROJECT_NAME}
#   SHADERS ${PROJECT_NAME}.VertexShader ${PROJECT_NAME}.PixelShader
#   INSTALL_DESTINATION "${CMAKE_INSTALL_BINDIR}/shaders/"
# )
#
# This will define a dependency for the specified target for all shader module targets. If INSTALL_DESTINATION (relative to the install prefix) is provided, an
# install command is created for the shader module binaries.
#
# A shader library generates a header, which embeds the binaries of all shader modules added to it (using the LIBRARY parameter of ADD_SHADER_MODULE):
#
# ADD_SHADER_LIBRARY(${PROJECT_NAME}.Shaders SOURCE_FILE "shader_resources.hpp" NAMESPACE "MyProject::Shaders")
# TARGET_LINK_SHADER_LIBRARIES(${PROJECT_NAME} LIBRARIES ${PROJECT_NAME}.Shaders)
#
# The header is only replaced if its content changes, so that sources including it are not rebuilt unnecessarily. Shader modules must be added to a library
# within the same directory (CMakeLists.txt) that defines the library.

INCLUDE_GUARD(GLOBAL)

SET(SHADER_DEFAULT_SUBDIR "shaders" CACHE STRING "Default subdirectory for shader module binaries within the runtime output directory.")
SET(DXIL_DEFAULT_SUFFIX ".dxi" CACHE STRING "Default file extension for DXIL shaders.")
SET(SPIRV_DEFAULT_SUFFIX ".spv" CACHE STRING "Default file extension for SPIR-V shaders.")
SET(SPIRV_DEFAULT_TARGET_ENV "vulkan1.3" CACHE STRING "Default Vulkan target environment for SPIR-V shaders.")

# _LITEFX_SHADER_STAGE(<compiler> <type> <out-var>)
#
# Maps a shader type to the stage name (GLSLC) or profile prefix (DXC).
FUNCTION(_LITEFX_SHADER_STAGE compiler type out_var)
  IF(compiler STREQUAL "GLSLC")
    SET(stage_map
      VERTEX vert GEOMETRY geom FRAGMENT frag PIXEL frag
      HULL tesc TESSELLATION_CONTROL tesc TESSELATION_CONTROL tesc
      DOMAIN tese TESSELLATION_EVALUATION tese TESSELATION_EVALUATION tese
      COMPUTE comp RAYTRACING comp TASK task AMPLIFICATION task MESH mesh)
  ELSE()
    SET(stage_map
      VERTEX vs GEOMETRY gs FRAGMENT ps PIXEL ps
      HULL hs TESSELLATION_CONTROL hs TESSELATION_CONTROL hs
      DOMAIN ds TESSELLATION_EVALUATION ds TESSELATION_EVALUATION ds
      COMPUTE cs RAYTRACING lib TASK as AMPLIFICATION as MESH ms)
  ENDIF()

  LIST(FIND stage_map "${type}" index)

  IF(index EQUAL -1)
    MESSAGE(FATAL_ERROR "Unsupported shader type: '${type}'. Valid shader types are: VERTEX, GEOMETRY, HULL/TESSELLATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, FRAGMENT/PIXEL, COMPUTE, TASK/AMPLIFICATION, MESH and RAYTRACING.")
  ENDIF()

  MATH(EXPR index "${index} + 1")
  LIST(GET stage_map ${index} stage)
  SET(${out_var} ${stage} PARENT_SCOPE)
ENDFUNCTION()


# _LITEFX_SHADER_STAGE_INFO(<type> <suffix-var> <define-var>)
#
# Returns the file name suffix (e.g. "vs") and the stage define (e.g. "SHADER_STAGE_VERTEX") for a shader type. Synonymous types share both values.
FUNCTION(_LITEFX_SHADER_STAGE_INFO type suffix_var define_var)
  SET(types    VERTEX GEOMETRY HULL TESSELLATION_CONTROL TESSELATION_CONTROL DOMAIN TESSELLATION_EVALUATION TESSELATION_EVALUATION FRAGMENT PIXEL COMPUTE TASK AMPLIFICATION MESH RAYTRACING)
  SET(suffixes vs     gs       hs   hs                   hs                  ds     ds                      ds                     fs       fs    cs      ts   ts            ms   rt)
  SET(stages   VERTEX GEOMETRY TESSELLATION_CONTROL TESSELLATION_CONTROL TESSELLATION_CONTROL TESSELLATION_EVALUATION TESSELLATION_EVALUATION TESSELLATION_EVALUATION FRAGMENT FRAGMENT COMPUTE TASK TASK MESH RAYTRACING)

  LIST(FIND types "${type}" index)

  IF(index EQUAL -1)
    MESSAGE(FATAL_ERROR "Unsupported shader type: '${type}'.")
  ENDIF()

  LIST(GET suffixes ${index} suffix)
  LIST(GET stages ${index} stage)
  SET(${suffix_var} ${suffix} PARENT_SCOPE)
  SET(${define_var} SHADER_STAGE_${stage} PARENT_SCOPE)
ENDFUNCTION()


FUNCTION(ADD_SHADER_MODULE module_name)
  CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 SHADER "" "SOURCE;LANGUAGE;COMPILE_AS;SHADER_MODEL;TYPE;COMPILER;LIBRARY;ENTRY_POINT;COMPILE_OPTIONS;FOLDER;OUTPUT_NAME" "INCLUDES")

  # Validate arguments.
  IF(SHADER_UNPARSED_ARGUMENTS)
    MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): Unknown arguments: ${SHADER_UNPARSED_ARGUMENTS}")
  ENDIF()

  FOREACH(required IN ITEMS SOURCE LANGUAGE COMPILE_AS TYPE COMPILER)
    IF(NOT SHADER_${required})
      MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): ${required} is required.")
    ENDIF()
  ENDFOREACH()

  IF(NOT SHADER_ENTRY_POINT)
    SET(SHADER_ENTRY_POINT "main")
  ENDIF()

  IF(NOT SHADER_LANGUAGE MATCHES "^(GLSL|HLSL)$")
    MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): Unsupported shader language: '${SHADER_LANGUAGE}'. Only GLSL and HLSL are supported.")
  ELSEIF(NOT SHADER_COMPILER MATCHES "^(GLSLC|DXC)$")
    MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): Unrecognized compiler: '${SHADER_COMPILER}'. Only DXC and GLSLC are allowed.")
  ELSEIF(NOT SHADER_COMPILE_AS MATCHES "^(SPIRV|DXIL)$")
    MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): Unrecognized intermediate language: '${SHADER_COMPILE_AS}'. Only SPIRV and DXIL are supported.")
  ELSEIF(SHADER_LANGUAGE STREQUAL "GLSL" AND NOT SHADER_COMPILER STREQUAL "GLSLC")
    MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): GLSL shaders can only be built with GLSLC.")
  ELSEIF(SHADER_COMPILER STREQUAL "GLSLC" AND NOT SHADER_COMPILE_AS STREQUAL "SPIRV")
    MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): GLSLC can only compile to SPIR-V.")
  ELSEIF(SHADER_COMPILER STREQUAL "DXC" AND NOT SHADER_SHADER_MODEL)
    MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): SHADER_MODEL is required when compiling with DXC.")
  ENDIF()

  IF(SHADER_LANGUAGE STREQUAL "GLSL" AND NOT SHADER_ENTRY_POINT STREQUAL "main")
    MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): GLSL shaders always use 'main' as entry point. To put multiple stages into one GLSL file, use the SHADER_STAGE_* defines instead.")
  ENDIF()

  _LITEFX_SHADER_STAGE(${SHADER_COMPILER} ${SHADER_TYPE} stage)
  _LITEFX_SHADER_STAGE_INFO(${SHADER_TYPE} stage_suffix stage_define)

  # Resolve input and output files.
  IF(SHADER_OUTPUT_NAME)
    SET(out_name "${SHADER_OUTPUT_NAME}")
  ELSE()
    GET_FILENAME_COMPONENT(out_name "${SHADER_SOURCE}" NAME_WE)
  ENDIF()

  CMAKE_PATH(ABSOLUTE_PATH SHADER_SOURCE BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" NORMALIZE OUTPUT_VARIABLE source_file)

  SET(include_files "")

  FOREACH(include_file ${SHADER_INCLUDES})
    CMAKE_PATH(ABSOLUTE_PATH include_file BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" NORMALIZE OUTPUT_VARIABLE include_path)
    LIST(APPEND include_files "${include_path}")
  ENDFOREACH()

  IF(SHADER_COMPILE_AS STREQUAL "DXIL")
    SET(suffix "${DXIL_DEFAULT_SUFFIX}")
  ELSE()
    SET(suffix "${SPIRV_DEFAULT_SUFFIX}")
  ENDIF()

  LITEFX_GET_RUNTIME_DIRECTORY(output_dir)
  SET(output_dir "${output_dir}/${SHADER_DEFAULT_SUBDIR}")
  SET(output_file "${output_dir}/${out_name}${suffix}")

  # Two modules building the same binary would overwrite each other, so this is reported as an error.
  STRING(MD5 output_key "${output_file}")
  GET_PROPERTY(output_owner GLOBAL PROPERTY _LITEFX_SHADER_OUTPUT_${output_key})

  IF(output_owner)
    MESSAGE(FATAL_ERROR "ADD_SHADER_MODULE(${module_name}): The shader binary '${out_name}${suffix}' is already built by the shader module '${output_owner}'. Use OUTPUT_NAME to build the modules into different binaries.")
  ENDIF()

  SET_PROPERTY(GLOBAL PROPERTY _LITEFX_SHADER_OUTPUT_${output_key} ${module_name})

  SEPARATE_ARGUMENTS(compile_options UNIX_COMMAND "${SHADER_COMPILE_OPTIONS}")

  # Vulkan uses a flipped y-axis compared to DirectX. The stages that output positions flip it, so that the same shaders work for both backends.
  SET(invert_y OFF)

  IF(SHADER_COMPILE_AS STREQUAL "SPIRV" AND SHADER_TYPE MATCHES "^(VERTEX|GEOMETRY|DOMAIN|TESSELL?ATION_EVALUATION)$")
    SET(invert_y ON)
  ENDIF()

  # Build the compiler command.
  SET(depfile_args "")

  IF(SHADER_COMPILER STREQUAL "GLSLC")
    STRING(TOLOWER "${SHADER_LANGUAGE}" language)
    SET(command "${LITEFX_BUILD_GLSLC_COMPILER}" --target-env=${SPIRV_DEFAULT_TARGET_ENV} -mfmt=bin -fshader-stage=${stage} -DSPIRV -D${stage_define} -x ${language})

    # GLSLC only supports selecting the entry point for HLSL sources. The option applies to all subsequent source files, so it must precede the source.
    IF(SHADER_LANGUAGE STREQUAL "HLSL" AND NOT SHADER_ENTRY_POINT STREQUAL "main")
      LIST(APPEND command -fentry-point=${SHADER_ENTRY_POINT})
    ENDIF()

    IF(invert_y)
      LIST(APPEND command -finvert-y)
    ENDIF()

    LIST(APPEND command ${compile_options} -c "${source_file}" -o "${output_file}" -MD -MF "${output_file}.d")

    # GLSLC reports all included files, so that changes to them trigger a rebuild even if they are not listed as INCLUDES.
    SET(depfile_args DEPFILE "${output_file}.d")
    SET(comment "glslc: compiling ${language} shader '${SHADER_SOURCE}'...")
  ELSE()
    SET(command "${LITEFX_BUILD_DXC_COMPILER}" -T ${stage}_${SHADER_SHADER_MODEL} -E ${SHADER_ENTRY_POINT} -D ${stage_define} -Fo "${output_file}" "$<$<CONFIG:Debug,RelWithDebInfo>:-Zi>")

    IF(SHADER_COMPILE_AS STREQUAL "SPIRV")
      LIST(APPEND command -spirv -fspv-target-env=${SPIRV_DEFAULT_TARGET_ENV} -D SPIRV)

      IF(invert_y)
        LIST(APPEND command -fvk-invert-y)
      ENDIF()
    ELSE()
      LIST(APPEND command -D DXIL "$<IF:$<CONFIG:Debug,RelWithDebInfo>,-Qembed_debug,-Qstrip_debug>" -Wno-ignored-attributes)
    ENDIF()

    LIST(APPEND command ${compile_options} "${source_file}")
    SET(comment "dxc: compiling hlsl shader '${SHADER_SOURCE}' (profile: ${stage}_${SHADER_SHADER_MODEL}) to ${SHADER_COMPILE_AS}...")
  ENDIF()

  # The shader is only rebuilt, if the source, one of the includes or the compiler options change.
  ADD_CUSTOM_COMMAND(OUTPUT "${output_file}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${output_dir}"
    COMMAND ${command}
    DEPENDS "${source_file}" ${include_files}
    ${depfile_args}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "${comment}"
    VERBATIM
    COMMAND_EXPAND_LISTS
  )

  ADD_CUSTOM_TARGET(${module_name}
    DEPENDS "${output_file}"
    SOURCES "${source_file}" ${include_files}
  )

  SET_TARGET_PROPERTIES(${module_name} PROPERTIES
    OUTPUT_NAME "${out_name}"
    SUFFIX "${suffix}"
    RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
  )

  IF(SHADER_FOLDER)
    SET_TARGET_PROPERTIES(${module_name} PROPERTIES FOLDER "${SHADER_FOLDER}")
  ENDIF()

  # If a library is specified, add the shader module to it.
  IF(SHADER_LIBRARY)
    ADD_DEPENDENCIES(${SHADER_LIBRARY} ${module_name})
    GET_TARGET_PROPERTY(library_namespace ${SHADER_LIBRARY} NAMESPACE)
    GET_TARGET_PROPERTY(library_file ${SHADER_LIBRARY} LITEFX_SHADER_LIBRARY_TEMP_FILE)

    ADD_CUSTOM_COMMAND(TARGET ${SHADER_LIBRARY} POST_BUILD
      COMMAND pcksl pack "${library_file}" "${output_file}" "${library_namespace}" "${out_name}${suffix}"
      VERBATIM
    )
  ENDIF()
ENDFUNCTION(ADD_SHADER_MODULE module_name)


FUNCTION(TARGET_LINK_SHADERS target_name)
  CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 SHADER "" "INSTALL_DESTINATION" "SHADERS")

  ADD_DEPENDENCIES(${target_name} ${SHADER_SHADERS})

  # Shaders are only installed, if a destination (relative to the install prefix) is provided.
  IF(SHADER_INSTALL_DESTINATION)
    FOREACH(shader_module ${SHADER_SHADERS})
      GET_TARGET_PROPERTY(shader_dir ${shader_module} RUNTIME_OUTPUT_DIRECTORY)
      GET_TARGET_PROPERTY(shader_name ${shader_module} OUTPUT_NAME)
      GET_TARGET_PROPERTY(shader_suffix ${shader_module} SUFFIX)
      INSTALL(FILES "${shader_dir}/${shader_name}${shader_suffix}" DESTINATION "${SHADER_INSTALL_DESTINATION}")
    ENDFOREACH(shader_module ${SHADER_SHADERS})
  ENDIF(SHADER_INSTALL_DESTINATION)
ENDFUNCTION(TARGET_LINK_SHADERS target_name)


FUNCTION(TARGET_ADD_SHADER_PROGRAM target_name)
  SET(stage_keywords VERTEX GEOMETRY HULL TESSELLATION_CONTROL DOMAIN TESSELLATION_EVALUATION FRAGMENT PIXEL COMPUTE TASK AMPLIFICATION MESH RAYTRACING)
  CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 PROGRAM
    "SPIRV;DXIL"
    "SOURCE;LANGUAGE;COMPILER;SHADER_MODEL;FOLDER;INSTALL_DESTINATION;LIBRARY;COMPILE_OPTIONS;SPIRV_COMPILE_OPTIONS;DXIL_COMPILE_OPTIONS"
    "INCLUDES;${stage_keywords}")

  IF(PROGRAM_UNPARSED_ARGUMENTS)
    MESSAGE(FATAL_ERROR "TARGET_ADD_SHADER_PROGRAM(${target_name}): Unknown arguments: ${PROGRAM_UNPARSED_ARGUMENTS}")
  ENDIF()

  # Apply defaults.
  IF(NOT PROGRAM_LANGUAGE)
    SET(PROGRAM_LANGUAGE HLSL)
  ENDIF()

  IF(NOT PROGRAM_COMPILER)
    IF(PROGRAM_LANGUAGE STREQUAL "GLSL")
      SET(PROGRAM_COMPILER GLSLC)
    ELSE()
      SET(PROGRAM_COMPILER DXC)
    ENDIF()
  ENDIF()

  # Select backends: explicitly requested ones, or all backends LiteFX has been built with (LITEFX_BUILD_* within the project, LITEFX_HAS_* for applications).
  SET(backends "")

  IF(PROGRAM_SPIRV OR PROGRAM_DXIL)
    IF(PROGRAM_SPIRV)
      LIST(APPEND backends SPIRV)
    ENDIF()

    IF(PROGRAM_DXIL)
      LIST(APPEND backends DXIL)
    ENDIF()
  ELSE()
    IF(LITEFX_BUILD_VULKAN_BACKEND OR LITEFX_HAS_VULKAN_BACKEND)
      LIST(APPEND backends SPIRV)
    ENDIF()

    IF((LITEFX_BUILD_DIRECTX_12_BACKEND OR LITEFX_HAS_DIRECTX12_BACKEND) AND NOT PROGRAM_LANGUAGE STREQUAL "GLSL")
      LIST(APPEND backends DXIL)
    ENDIF()
  ENDIF()

  IF(NOT backends)
    MESSAGE(FATAL_ERROR "TARGET_ADD_SHADER_PROGRAM(${target_name}): No backend to compile for. Set SPIRV and/or DXIL explicitly.")
  ENDIF()

  # Resolve includes relative to the calling directory, so that they identify the same files when modules are shared between directories.
  SET(include_paths "")

  FOREACH(include_file IN LISTS PROGRAM_INCLUDES)
    CMAKE_PATH(ABSOLUTE_PATH include_file BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" NORMALIZE OUTPUT_VARIABLE include_path)
    LIST(APPEND include_paths "${include_path}")
  ENDFOREACH()

  # Collect the shader modules to build as parallel lists of source file, stage, entry point and output name.
  SET(item_sources "")
  SET(item_stages "")
  SET(item_entries "")
  SET(item_outputs "")

  IF(PROGRAM_SOURCE)
    # Single-file program: each stage keyword optionally names its entry point (default: main), and binaries are named "<source name>_<stage>".
    GET_FILENAME_COMPONENT(source_name "${PROGRAM_SOURCE}" NAME_WE)

    FOREACH(stage IN LISTS stage_keywords)
      IF(NOT DEFINED PROGRAM_${stage} AND NOT stage IN_LIST PROGRAM_KEYWORDS_MISSING_VALUES)
        CONTINUE()
      ENDIF()

      SET(entry_points ${PROGRAM_${stage}})
      LIST(LENGTH entry_points entry_point_count)

      IF(entry_point_count GREATER 1)
        MESSAGE(FATAL_ERROR "TARGET_ADD_SHADER_PROGRAM(${target_name}): Only one entry point per stage is supported with SOURCE (${stage}: ${entry_points}). Use ADD_SHADER_MODULE with OUTPUT_NAME for additional entry points.")
      ELSEIF(entry_point_count EQUAL 0)
        SET(entry_points "main")
      ENDIF()

      _LITEFX_SHADER_STAGE_INFO(${stage} stage_suffix stage_define)
      LIST(APPEND item_sources "${PROGRAM_SOURCE}")
      LIST(APPEND item_stages ${stage})
      LIST(APPEND item_entries ${entry_points})
      LIST(APPEND item_outputs "${source_name}_${stage_suffix}")
    ENDFOREACH()
  ELSE()
    # One source file per module: stage keywords name the source files, and binaries are named after them.
    FOREACH(stage IN LISTS stage_keywords)
      IF(stage IN_LIST PROGRAM_KEYWORDS_MISSING_VALUES)
        MESSAGE(FATAL_ERROR "TARGET_ADD_SHADER_PROGRAM(${target_name}): ${stage} requires source files, unless SOURCE is set.")
      ENDIF()

      FOREACH(source IN LISTS PROGRAM_${stage})
        GET_FILENAME_COMPONENT(source_name "${source}" NAME_WE)
        LIST(APPEND item_sources "${source}")
        LIST(APPEND item_stages ${stage})
        LIST(APPEND item_entries "main")
        LIST(APPEND item_outputs "${source_name}")
      ENDFOREACH()
    ENDFOREACH()
  ENDIF()

  LIST(LENGTH item_sources item_count)

  IF(item_count EQUAL 0)
    MESSAGE(FATAL_ERROR "TARGET_ADD_SHADER_PROGRAM(${target_name}): No shader stages provided.")
  ENDIF()

  MATH(EXPR last_item "${item_count} - 1")
  SET(modules "")

  FOREACH(backend IN LISTS backends)
    IF(backend STREQUAL "SPIRV")
      SET(prefix "Vk")
      SET(folder_suffix "Vulkan")
    ELSE()
      SET(prefix "Dx")
      SET(folder_suffix "DirectX 12")
    ENDIF()

    # Arguments shared by all shader modules of this backend. Empty values are omitted, so that ADD_SHADER_MODULE applies its defaults.
    STRING(STRIP "${PROGRAM_COMPILE_OPTIONS} ${PROGRAM_${backend}_COMPILE_OPTIONS}" compile_options)
    SET(module_args LANGUAGE ${PROGRAM_LANGUAGE} COMPILER ${PROGRAM_COMPILER} COMPILE_AS ${backend})

    IF(PROGRAM_SHADER_MODEL)
      LIST(APPEND module_args SHADER_MODEL ${PROGRAM_SHADER_MODEL})
    ENDIF()

    IF(compile_options)
      LIST(APPEND module_args COMPILE_OPTIONS "${compile_options}")
    ENDIF()

    IF(PROGRAM_LIBRARY)
      LIST(APPEND module_args LIBRARY ${PROGRAM_LIBRARY})
    ENDIF()

    IF(include_paths)
      LIST(APPEND module_args INCLUDES ${include_paths})
    ENDIF()

    FOREACH(index RANGE ${last_item})
      LIST(GET item_sources ${index} source)
      LIST(GET item_stages ${index} stage)
      LIST(GET item_entries ${index} entry_point)
      LIST(GET item_outputs ${index} output_name)

      # Shader modules with identical settings are shared between programs and targets (e.g. a common vertex shader). The IDE folder is not part of the
      # settings, and synonymous stages (e.g. FRAGMENT and PIXEL) are treated as equal.
      CMAKE_PATH(ABSOLUTE_PATH source BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" NORMALIZE OUTPUT_VARIABLE source_path)
      _LITEFX_SHADER_STAGE(${PROGRAM_COMPILER} ${stage} stage_name)
      STRING(MD5 module_key "${source_path}|${stage_name}|${entry_point}|${output_name}|${module_args}")
      GET_PROPERTY(shared_module GLOBAL PROPERTY _LITEFX_SHADER_MODULE_${module_key})

      IF(shared_module)
        LIST(APPEND modules ${shared_module})
        CONTINUE()
      ENDIF()

      SET(module_name "${target_name}.${prefix}.Shaders.${output_name}")

      IF(TARGET ${module_name})
        MESSAGE(FATAL_ERROR "TARGET_ADD_SHADER_PROGRAM(${target_name}): The shader module '${module_name}' already exists with different settings (e.g. another stage, entry point or other compile options). Each binary can only be built with one set of settings per intermediate language.")
      ENDIF()

      SET(module_folder_args "")

      IF(PROGRAM_FOLDER)
        SET(module_folder_args FOLDER "${PROGRAM_FOLDER}/${folder_suffix}")
      ENDIF()

      ADD_SHADER_MODULE(${module_name} SOURCE "${source}" TYPE ${stage} ENTRY_POINT ${entry_point} OUTPUT_NAME ${output_name} ${module_args} ${module_folder_args})
      SET_PROPERTY(GLOBAL PROPERTY _LITEFX_SHADER_MODULE_${module_key} ${module_name})
      LIST(APPEND modules ${module_name})
    ENDFOREACH()
  ENDFOREACH()

  LIST(REMOVE_DUPLICATES modules)

  IF(PROGRAM_INSTALL_DESTINATION)
    TARGET_LINK_SHADERS(${target_name} SHADERS ${modules} INSTALL_DESTINATION "${PROGRAM_INSTALL_DESTINATION}")
  ELSE()
    TARGET_LINK_SHADERS(${target_name} SHADERS ${modules})
  ENDIF()
ENDFUNCTION(TARGET_ADD_SHADER_PROGRAM target_name)


FUNCTION(ADD_SHADER_LIBRARY library_name)
  CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 SHADER_LIBRARY "" "SOURCE_FILE;NAMESPACE" "")

  IF(NOT SHADER_LIBRARY_SOURCE_FILE OR NOT SHADER_LIBRARY_NAMESPACE)
    MESSAGE(FATAL_ERROR "ADD_SHADER_LIBRARY(${library_name}): SOURCE_FILE and NAMESPACE are required.")
  ENDIF()

  SET(library_dir "${CMAKE_CURRENT_BINARY_DIR}/shaders")
  SET(library_file "${library_dir}/${SHADER_LIBRARY_SOURCE_FILE}")

  # The header is assembled in a temporary file and only copied, if its content changed (see _LITEFX_FINALIZE_SHADER_LIBRARY).
  ADD_CUSTOM_TARGET(${library_name}
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${library_dir}"
    COMMAND pcksl init "${library_file}.tmp"
    COMMENT "Packing shader library ${library_name} to '${library_file}'..."
    VERBATIM
  )

  ADD_DEPENDENCIES(${library_name} pcksl)

  SET_TARGET_PROPERTIES(${library_name} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${library_dir}"
    NAMESPACE "${SHADER_LIBRARY_NAMESPACE}"
    OUTPUT_NAME "${SHADER_LIBRARY_SOURCE_FILE}"
    LITEFX_SHADER_LIBRARY_TEMP_FILE "${library_file}.tmp"
  )

  # The final copy must run after all shader modules have been packed. Their pack commands are added by ADD_SHADER_MODULE later on, so the copy step is
  # deferred until the end of the current directory.
  CMAKE_LANGUAGE(EVAL CODE "CMAKE_LANGUAGE(DEFER CALL _LITEFX_FINALIZE_SHADER_LIBRARY [[${library_name}]] [[${library_file}]])")
ENDFUNCTION(ADD_SHADER_LIBRARY library_name)


FUNCTION(_LITEFX_FINALIZE_SHADER_LIBRARY library_name library_file)
  ADD_CUSTOM_COMMAND(TARGET ${library_name} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${library_file}.tmp" "${library_file}"
    BYPRODUCTS "${library_file}"
    VERBATIM
  )
ENDFUNCTION(_LITEFX_FINALIZE_SHADER_LIBRARY library_name library_file)


FUNCTION(TARGET_LINK_SHADER_LIBRARIES target_name)
  CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 SHADER "" "" "LIBRARIES")

  ADD_DEPENDENCIES(${target_name} ${SHADER_LIBRARIES})

  FOREACH(shader_library ${SHADER_LIBRARIES})
    GET_TARGET_PROPERTY(SHADER_LIBRARY_INCLUDE_DIR ${shader_library} INTERFACE_INCLUDE_DIRECTORIES)
    TARGET_INCLUDE_DIRECTORIES(${target_name} PRIVATE ${SHADER_LIBRARY_INCLUDE_DIR})
  ENDFOREACH(shader_library ${SHADER_LIBRARIES})
ENDFUNCTION(TARGET_LINK_SHADER_LIBRARIES target_name)


###################################################################################################
#####                                                                                         #####
##### Define auxiliary targets.                                                               #####
#####                                                                                         #####
###################################################################################################

FILE(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/Auxiliary/pcksl.cxx" CONTENT [==[
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <cstdint>
#include <cctype>
#include <cstddef>

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTBEGIN(performance-avoid-endl)

namespace {

    constexpr int INIT_ARGUMENT_COUNT = 3;
    constexpr int PACK_ARGUMENT_COUNT = 6;
    constexpr std::size_t BYTES_PER_LINE = 32;
    constexpr int PACK_HEADER = 2;
    constexpr int PACK_BINARY = 3;
    constexpr int PACK_NAMESPACE = 4;
    constexpr int PACK_NAME = 5;

}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return -1;
    }

    try
    {
        std::string command(argv[1]);

        if (command == "init")
        { 
            if (argc != INIT_ARGUMENT_COUNT) {
                return -1;
            }

            std::string sourceFile(argv[2]);
            std::ofstream file(sourceFile);
            file << "#pragma once // NOLINT(portability-avoid-pragma-once,llvm-header-guard)" << std::endl <<
                "#include <array>" << std::endl <<
                "#include <cstddef>" << std::endl <<
                "#include <istream>" << std::endl <<
                "#include <span>" << std::endl <<
                "#include <streambuf>" << std::endl <<
                "#include <string>" << std::endl << std::endl;
            
            file << "#ifndef LITEFX_PCKSL_MEMBUF_DEFINED" << std::endl;
            file << "#define LITEFX_PCKSL_MEMBUF_DEFINED" << std::endl << std::endl;
            file << "// Read-only stream buffer over constant data." << std::endl;
            file << "struct _pcksl_mem_buf : public std::streambuf { // NOLINT(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)" << std::endl;
            file << "    explicit _pcksl_mem_buf(std::span<const char> data) {" << std::endl;
            file << "        auto* begin = const_cast<char*>(data.data()); // NOLINT(cppcoreguidelines-pro-type-const-cast)" << std::endl;
            file << "        this->setg(begin, begin, begin + data.size()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)" << std::endl;
            file << "    }" << std::endl;
            file << "};" << std::endl << std::endl;
            file << "struct _pcksl_mem_istream : private virtual _pcksl_mem_buf, public std::istream { // NOLINT(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp,misc-multiple-inheritance)" << std::endl;
            file << "    explicit _pcksl_mem_istream(std::span<const char> data) :" << std::endl;
            file << "        _pcksl_mem_buf(data), std::istream(static_cast<std::streambuf*>(this)) { }" << std::endl;
            file << "};" << std::endl << std::endl;
            file << "#endif // !LITEFX_PCKSL_MEMBUF_DEFINED" << std::endl << std::endl;

            file.close();
        }
        else if (command == "pack")
        {
            if (argc != PACK_ARGUMENT_COUNT) {
                return -1;
            }

            std::string sourceFile(argv[PACK_HEADER]);
            std::string resourceFile(argv[PACK_BINARY]);
            std::string ns(argv[PACK_NAMESPACE]);
            std::string name(argv[PACK_NAME]);
            std::string resourceName(name);
            std::ranges::replace_if(resourceName, [](unsigned char c) { return std::isalnum(c) == 0; }, '_');
            
            // Replace backslashes to prevent uninteded escape sequences.
            std::string embedPath(resourceFile);
            std::ranges::replace(embedPath, '\\', '/');

            std::ifstream resource(resourceFile, std::ios::binary);

            if (!resource) {
                return -3;
            }

            std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(resource), { });

            std::ofstream file(sourceFile, std::ios::app);
            file << "namespace " << ns << " {" << std::endl;
            file << "    // Shader source: " << resourceFile << "." << std::endl;
            file << "    class " << resourceName << " { // NOLINT(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)" << std::endl;
            file << "    public:" << std::endl;
            file << "        " << resourceName << "() = delete;" << std::endl;
            file << "        ~" << resourceName << "() = delete;" << std::endl << std::endl;
            file << "        static std::string name() { return \"" << name << "\"; } // NOLINT(modernize-use-trailing-return-type)" << std::endl << std::endl;
            file << "        static constexpr std::span<const unsigned char> data() noexcept { return _data; } // NOLINT(modernize-use-trailing-return-type)" << std::endl << std::endl;
            file << "        static _pcksl_mem_istream open() { // NOLINT(modernize-use-trailing-return-type)" << std::endl;
            file << "            return _pcksl_mem_istream({ reinterpret_cast<const char*>(_data), sizeof(_data) }); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)" << std::endl;
            file << "        }" << std::endl << std::endl;
            file << "    private:" << std::endl;
            
            // Prefer #embed if available (requires C++26).
            file << "#ifdef __has_embed" << std::endl;
            file << "#  if __has_embed(\"" << embedPath << "\")" << std::endl;
            file << "#    define LITEFX_PCKSL_USE_EMBED" << std::endl;
            file << "#  endif" << std::endl;
            file << "#endif" << std::endl << std::endl;
            file << "#ifdef LITEFX_PCKSL_USE_EMBED" << std::endl;
            file << "#  ifdef __clang__" << std::endl;
            file << "#    pragma clang diagnostic push" << std::endl;
            file << "#    pragma clang diagnostic ignored \"-Wc23-extensions\"" << std::endl;
            file << "#  endif" << std::endl;
            file << "        static constexpr unsigned char _data[] = { // NOLINT(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)" << std::endl;
            file << "#  embed \"" << embedPath << "\"" << std::endl;
            file << "        };" << std::endl;
            file << "#  ifdef __clang__" << std::endl;
            file << "#    pragma clang diagnostic pop" << std::endl;
            file << "#  endif" << std::endl;
            file << "#  undef LITEFX_PCKSL_USE_EMBED" << std::endl;
            file << "#else" << std::endl;
            file << "        static constexpr unsigned char _data[" << buffer.size() << "] = { // NOLINT(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)";

            std::size_t column = 0;

            for (const auto byte : buffer)
            {
                if (column++ % BYTES_PER_LINE == 0) {
                    file << std::endl << "            ";
                }

                file << "0x" << std::setfill('0') << std::setw(2) << std::hex << +byte << ", ";
            }

            file << std::dec << std::endl;
            file << "        };" << std::endl;
            file << "#endif" << std::endl;
            file << "    };" << std::endl;
            file << "} // namespace " << ns << std::endl << std::endl;

            file.close();
        }
        else
        {
            return -1;
        }
    }
    catch(...)
    {
        return -2;
    }

    return 0;
}

// NOLINTEND(performance-avoid-endl)
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
]==])

IF(NOT TARGET pcksl)
  ADD_EXECUTABLE(pcksl "${CMAKE_BINARY_DIR}/Auxiliary/pcksl.cxx")
  SET_PROPERTY(TARGET pcksl PROPERTY FOLDER "Auxiliary")

  # The tool is compiled with the settings of the project including this script, so it requests the standard it needs itself.
  TARGET_COMPILE_FEATURES(pcksl PRIVATE cxx_std_20)
ENDIF(NOT TARGET pcksl)