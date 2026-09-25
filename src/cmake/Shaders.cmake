###################################################################################################
#####                                                                                         #####
#####               Contains the build script for glslc/dxc shader compilers.                 #####
#####                                                                                         #####
###################################################################################################

# The script exports the following functions:
#
# - ADD_SHADER_MODULE: Creates a shader module target.
# - TARGET_LINK_SHADERS: Links a set of shader module targets to another target.
# - ADD_SHADER_LIBRARY: Creates a shader library, i.e., a header that embeds the binaries of a set of shader modules.
# - TARGET_LINK_SHADER_LIBRARIES: Links a set of shader libraries to another target.
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
# default to "main". Note that setting an entry point is only supported when using the DXC compiler.
#
# The COMPILE_OPTIONS parameter provides additional compiler options as a single (space-separated) string.
#
# The INCLUDES parameter lists additional include sources. They are added to the target, so that they are accessible from the IDE, and the shader is rebuilt
# if one of them changes. Note that their names are also relative to the CMAKE_CURRENT_SOURCE_DIR. For GLSLC, includes are also tracked automatically.
#
# The LIBRARY parameter optionally adds the shader module to a shader library (see ADD_SHADER_LIBRARY).
#
# Shader modules are built into a subdirectory of the runtime output directory (see LITEFX_GET_RUNTIME_DIRECTORY in Runtime.cmake). The subdirectory name can be
# set using SHADER_DEFAULT_SUBDIR. Shaders are only rebuilt, if their source or one of their includes changes. The following properties are set for a shader
# module target and can be read after calling ADD_SHADER_MODULE (changing them does not affect the output):
#
# - RUNTIME_OUTPUT_DIRECTORY: The directory the shader module binary is built into (may contain generator expressions).
# - OUTPUT_NAME: The shader module name (without the file extension), which equals the input file name without its extension.
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

SET(SHADER_DEFAULT_SUBDIR       "shaders"   CACHE STRING "Default subdirectory for shader module binaries within the runtime output directory.")
SET(DXIL_DEFAULT_SUFFIX         ".dxi"      CACHE STRING "Default file extension for DXIL shaders.")
SET(SPIRV_DEFAULT_SUFFIX        ".spv"      CACHE STRING "Default file extension for SPIR-V shaders.")
SET(SPIRV_DEFAULT_TARGET_ENV    "vulkan1.3" CACHE STRING "Default Vulkan target environment for SPIR-V shaders.")

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


FUNCTION(ADD_SHADER_MODULE module_name)
  CMAKE_PARSE_ARGUMENTS(PARSE_ARGV 1 SHADER "" "SOURCE;LANGUAGE;COMPILE_AS;SHADER_MODEL;TYPE;COMPILER;LIBRARY;ENTRY_POINT;COMPILE_OPTIONS" "INCLUDES")

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

  IF(SHADER_COMPILER STREQUAL "GLSLC" AND NOT SHADER_ENTRY_POINT STREQUAL "main")
    MESSAGE(WARNING "ADD_SHADER_MODULE(${module_name}): Setting the entry point is only supported when compiling using DXC. The entry point will default to 'main'.")
  ENDIF()

  _LITEFX_SHADER_STAGE(${SHADER_COMPILER} ${SHADER_TYPE} stage)

  # Resolve input and output files.
  GET_FILENAME_COMPONENT(out_name "${SHADER_SOURCE}" NAME_WE)
  CMAKE_PATH(ABSOLUTE_PATH SHADER_SOURCE BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE source_file)

  SET(include_files "")

  FOREACH(include_file ${SHADER_INCLUDES})
    CMAKE_PATH(ABSOLUTE_PATH include_file BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE include_path)
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
    SET(command "${LITEFX_BUILD_GLSLC_COMPILER}" --target-env=${SPIRV_DEFAULT_TARGET_ENV} -mfmt=bin -fshader-stage=${stage} -DSPIRV -x ${language})

    IF(invert_y)
      LIST(APPEND command -finvert-y)
    ENDIF()

    LIST(APPEND command ${compile_options} -c "${source_file}" -o "${output_file}" -MD -MF "${output_file}.d")

    # GLSLC reports all included files, so that changes to them trigger a rebuild even if they are not listed as INCLUDES.
    SET(depfile_args DEPFILE "${output_file}.d")
    SET(comment "glslc: compiling ${language} shader '${SHADER_SOURCE}'...")
  ELSE()
    SET(command "${LITEFX_BUILD_DXC_COMPILER}" -T ${stage}_${SHADER_SHADER_MODEL} -E ${SHADER_ENTRY_POINT} -Fo "${output_file}" "$<$<CONFIG:Debug,RelWithDebInfo>:-Zi>")

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

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
// NOLINTBEGIN(performance-avoid-endl)

int main(int argc, char* argv[]) {
    if (argc < 2)
        return -1;

    try
    {
        std::string command(argv[1]);

        if (command == "init")
        { 
            if (argc != 3)
                return -1;

            std::string sourceFile(argv[2]);
            std::ofstream file(sourceFile);
            file << "#pragma once" << std::endl << 
                "#include <iostream>" << std::endl <<
                "#include <array>" << std::endl <<
                "#include <cstdint>" << std::endl <<
                "#include <istream>" << std::endl <<
                "#include <string>" << std::endl <<
                "#include <streambuf>" << std::endl << std::endl;
            
            file << "// NOLINTBEGIN" << std::endl;
            file << "#ifndef _LITEFX_PCKSL_MEMBUF_DEFINED" << std::endl;
            file << "struct _pcksl_mem_buf : public std::streambuf {" << std::endl;
            file << "    _pcksl_mem_buf(char* begin, size_t size) { this->setg(begin, begin, begin + size); }" << std::endl;
            file << "};" << std::endl << std::endl;
            file << "struct _pcksl_mem_istream : private virtual _pcksl_mem_buf, public std::istream {" << std::endl;
            file << "    explicit _pcksl_mem_istream(char* begin, size_t size) :" << std::endl;
            file << "        _pcksl_mem_buf(begin, size), std::istream(static_cast<std::streambuf*>(this)) { }" << std::endl;
            file << "};" << std::endl;
            file << "#define _LITEFX_PCKSL_MEMBUF_DEFINED" << std::endl;
            file << "#endif // !_LITEFX_PCKSL_MEMBUF_DEFINED" << std::endl << std::endl;
            file << "// NOLINTEND" << std::endl;

            file.close();
        }
        else if (command == "pack")
        {
            if (argc != 6)
                return -1;

            std::string sourceFile(argv[2]);
            std::string resourceFile(argv[3]);
            std::string ns(argv[4]);
            std::string resourceName(argv[5]);
            std::string name(argv[5]);
            std::replace_if(resourceName.begin(), resourceName.end(), [](unsigned char c) { return !std::isalnum(c); }, '_');

            std::ifstream resource(resourceFile, std::ios::binary);

            if (!resource)
                return -3;
            std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(resource), { });

            std::ofstream file(sourceFile, std::ios::app);
            file << "namespace " << ns << " {" << std::endl;
            file << "    // Shader source: " << resourceFile << "." << std::endl;
            file << "    // NOLINTBEGIN" << std::endl;
            file << "    class " << resourceName << " {" << std::endl;
            file << "    public:" << std::endl;
            file << "        " << resourceName << "() = delete;" << std::endl;
            file << "        ~" << resourceName << "() = delete;" << std::endl;
            file << "" << std::endl;
            file << "        static std::string name() { return \"" << name << "\"; }" << std::endl << std::endl;
            file << "        static _pcksl_mem_istream open() {" << std::endl;
            file << "            static std::array<std::uint8_t, " << buffer.size() << "> _data = {" << std::endl;
            file << "                ";

            for (const auto& v : buffer) 
                file << "0x" << std::setfill('0') << std::setw(sizeof(v) * 2) << std::hex << +v << ", ";

            file << std::endl;
            file << "            };" << std::endl << std::endl;
            file << "            // NOTE: reinterpret_cast should be safe here: https://eel.is/c++draft/basic.lval#11.3." << std::endl;
            file << "            return ::_pcksl_mem_istream(reinterpret_cast<char*>(_data.data()), _data.size());" << std::endl; // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            file << "        }" << std::endl;
            file << "    };" << std::endl;
            file << "    // NOLINTEND" << std::endl;
            file << "}" << std::endl << std::endl;

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
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
]==])

IF(NOT TARGET pcksl)
  ADD_EXECUTABLE(pcksl "${CMAKE_BINARY_DIR}/Auxiliary/pcksl.cxx")
  SET_PROPERTY(TARGET pcksl PROPERTY FOLDER "Auxiliary")
ENDIF(NOT TARGET pcksl)