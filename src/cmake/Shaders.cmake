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
# Shader modules are build from one source file and (optionally) multiple includes. Those includes may be used by multiple shader module targets simultaneously.
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
#   INCLUDES "a.hlsli" "b.hlsli"
# )
#
# The first parameter is the unique name of the shader module target. The SOURCE parameter provides a file name relative to CMAKE_CURRENT_SOURCE_DIR, that 
# contains the main shader source. 
#
# The LANGUAGE parameter specifies the shader language. Possible values are GLSL and HLSL. Note that the language influences which compiler you can choose and 
# which intermediate language (COMPILE_AS) can be selected. See below for more details.
#
# The TYPE parameter dictates the shader module type. Valid values are VERTEX, GEOMETRY, HULL/TESSELATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, 
# FRAGMENT/PIXEL, COMPUTE and RAYTRACING. Values separated by '/' are synonymous. Note that for the GLSLC compiler, RAYTRACING and COMPUTE are treated as 
# synonymous, too.
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
# Finally the INCLUDES parameter lists additional include sources. Those are not required and only used to be added to the target, so that they are accessible 
# from the IDE. Note that their names are also relative to the CMAKE_CURRENT_SOURCE_DIR.
#
# Finally, note that it is currently not supported to change the name of the entry point. It is always set to `main`.
#
# If CMAKE_RUNTIME_OUTPUT_DIRECTORY is set, the shader modules will be built into a subdirectory within the directory pointed by this variable. If it is not set,
# the subdirectory will be created in CMAKE_CURRENT_BINARY_DIR. The subdirectory name can be set using SHADER_DEFAULT_SUBDIR. The build target location will be 
# stored in the RUNTIME_OUTPUT_DIRECTORY property of the shader module target. Furthermore, there are two more properties that are set for a shader module target:
#
# - OUTPUT_NAME: Stores the shader module name (without the target file extension). By default it is set to equal the input file name without the extension.
# - SUFFIX: Stores the file extension (including the dot) for a shader module binary file.
#
# The suffix is configured by the DXIL_DEFAULT_SUFFIX and SPIRV_DEFAULT_SUFFIX variables, which can be overwritten before calling ADD_SHADER_MODULE.
#
# A shader module target can defined as a dependency by using TARGET_LINK_SHADERS:
#
# TARGET_LINK_SHADERS(${PROJECT_NAME}
#   SHADERS ${PROJECT_NAME}.VertexShader ${PROJECT_NAME}.PixelShader
#   INSTALL_DESTINATION "${INSTALL_BINARY_DIR}/shaders/"
# )
#
# This will define a dependency for the specified target for all shader module targets. Furthermore, it automatically creates an install command for the shader module 
# binaries. The source file is build from the RUNTIME_OUTPUT_DIRECTORY, OUTPUT_NAME and SUFFIX properties of each shader module target. The install destination can be 
# provided by the INSTALL_DESTINATION parameter. Note that it is always prepended with the CMAKE_INSTALL_PREFIX.

SET(SHADER_DEFAULT_SUBDIR "shaders" CACHE STRING "Default subdirectory for shader module binaries within the current binary directory (CMAKE_CURRENT_BINARY_DIR).")
SET(DXIL_DEFAULT_SUFFIX ".dxi" CACHE STRING "Default file extension for DXIL shaders.")
SET(SPIRV_DEFAULT_SUFFIX ".spv" CACHE STRING "Default file extension for SPIR-V shaders.")


FUNCTION(TARGET_HLSL_SHADERS target_name shader_source shader_model compile_as compile_with shader_type compile_options)
  GET_FILENAME_COMPONENT(out_name ${shader_source} NAME_WE)

  SET(SHADER_SOURCES ${shader_source})
  
  FOREACH(SHADER_INCLUDE ${ARGN})
    LIST(APPEND SHADER_SOURCES ${SHADER_INCLUDE})
  ENDFOREACH(SHADER_INCLUDE ${ARGN})
    
  SET (SHADER_STAGE "")
  SET (OUTPUT_DIR "")

  IF(${compile_with} STREQUAL "GLSLC")
    IF(NOT ${compile_as} STREQUAL "SPIRV")
      MESSAGE(SEND_ERROR "Glsl shaders can only be compiled to SPIR-V.")
    ENDIF(NOT ${compile_as} STREQUAL "SPIRV")

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
    ELSEIF(${shader_type} STREQUAL "TASK" OR ${shader_type} STREQUAL "AMPLIFICATION")
      SET(SHADER_STAGE "task")
    ELSEIF(${shader_type} STREQUAL "MESH")
      SET(SHADER_STAGE "mesh")
    ELSE()
      MESSAGE(SEND_ERROR "Unsupported shader type: ${shader_type}. Valid shader types are: VERTEX, GEOMETRY, HULL/TESSELATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, FRAGMENT/PIXEL, COMPUTE, TASK/AMPLIFICATION, MESH and RAYTRACING.")
    ENDIF(${shader_type} STREQUAL "VERTEX")
    
    IF(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
      SET(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR})
    ELSE()
      SET(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER_DEFAULT_SUBDIR})
    ENDIF(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    
    SET(compiler_options ${compile_options})
    SEPARATE_ARGUMENTS(compiler_options)

    IF(${shader_type} STREQUAL "VERTEX" OR ${shader_type} STREQUAL "GEOMETRY" OR ${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")
      LIST(APPEND compiler_options -finvert-y)
    ENDIF(${shader_type} STREQUAL "VERTEX" OR ${shader_type} STREQUAL "GEOMETRY" OR ${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")

    # TODO: Check if we can use a generator expression to build the output directory and file names, so it is possible to set the target properties to control the result file name.
    ADD_CUSTOM_TARGET(${target_name} 
      COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
      COMMENT "glslc: compiling hlsl shader '${shader_source}'..."
      SOURCES ${SHADER_SOURCES}
    )

    ADD_CUSTOM_COMMAND(TARGET ${target_name} POST_BUILD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND ${LITEFX_BUILD_GLSLC_COMPILER} --target-env=vulkan1.3 -mfmt=bin -fshader-stage=${SHADER_STAGE} -DSPIRV -x hlsl ${compiler_options} -c ${shader_source} -o "${OUTPUT_DIR}/${out_name}${SPIRV_DEFAULT_SUFFIX}" -MD
    )

    SET_TARGET_PROPERTIES(${target_name} PROPERTIES 
      SOURCES "${SHADER_SOURCES}"
      OUTPUT_NAME ${out_name}
      SUFFIX ${SPIRV_DEFAULT_SUFFIX}
      RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
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
    ELSEIF(${shader_type} STREQUAL "TASK" OR ${shader_type} STREQUAL "AMPLIFICATION")
      SET(SHADER_STAGE "as")
    ELSEIF(${shader_type} STREQUAL "MESH")
      SET(SHADER_STAGE "ms")
    ELSEIF(${shader_type} STREQUAL "RAYTRACING")
      SET(SHADER_STAGE "lib")
    ELSE()
      MESSAGE(SEND_ERROR "Unsupported shader type: ${shader_type}. Valid shader types are: VERTEX, GEOMETRY, HULL/TESSELATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, FRAGMENT/PIXEL, COMPUTE, TASK/AMPLIFICATION, MESH and RAYTRACING.")
    ENDIF(${shader_type} STREQUAL "VERTEX")

    SET(SHADER_PROFILE "${SHADER_STAGE}_${shader_model}")
    
    IF(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
      SET(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR})
    ELSE()
      SET(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER_DEFAULT_SUBDIR})
    ENDIF(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    
    SET(compiler_options ${compile_options})
    SEPARATE_ARGUMENTS(compiler_options)
    
    IF(${compile_as} STREQUAL "SPIRV")
      LIST(APPEND compiler_options -D SPIRV)
      
      IF(${shader_type} STREQUAL "VERTEX" OR ${shader_type} STREQUAL "GEOMETRY" OR ${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")
        LIST(APPEND compiler_options -fvk-invert-y)
      ENDIF(${shader_type} STREQUAL "VERTEX" OR ${shader_type} STREQUAL "GEOMETRY" OR ${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")
      
      ADD_CUSTOM_TARGET(${target_name} 
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
        COMMENT "dxc: compiling hlsl shader '${shader_source}' (profile: ${SHADER_PROFILE}) to SPIR-V..."
        SOURCES ${SHADER_SOURCES}
      )

      ADD_CUSTOM_COMMAND(TARGET ${target_name} POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${LITEFX_BUILD_DXC_COMPILER} -spirv -T ${SHADER_PROFILE} -E main -Fo "${OUTPUT_DIR}/${out_name}${SPIRV_DEFAULT_SUFFIX}" $<$<CONFIG:Debug,RelWithDebInfo>:-Zi> ${compiler_options} -fspv-target-env=vulkan1.3 ${shader_source}
      )
    
      SET_TARGET_PROPERTIES(${target_name} PROPERTIES 
        SOURCES "${SHADER_SOURCES}"
        OUTPUT_NAME ${out_name}
        SUFFIX ${SPIRV_DEFAULT_SUFFIX}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
      )
    ELSEIF(${compile_as} STREQUAL "DXIL")
      LIST(APPEND compiler_options -D DXIL)
      
      ADD_CUSTOM_TARGET(${target_name} 
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
        COMMENT "dxc: compiling hlsl shader '${shader_source}' (profile: ${SHADER_PROFILE}) to DXIL..."
        SOURCES ${SHADER_SOURCES}
      )
      
      ADD_CUSTOM_COMMAND(TARGET ${target_name} POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${LITEFX_BUILD_DXC_COMPILER} -T ${SHADER_PROFILE} -E main -Fo "${OUTPUT_DIR}/${out_name}${DXIL_DEFAULT_SUFFIX}" $<$<CONFIG:Debug,RelWithDebInfo>:-Zi> $<IF:$<CONFIG:Debug,RelWithDebInfo>,-Qembed_debug,-Qstrip_debug> ${compiler_options} ${shader_source} -Wno-ignored-attributes
      )
    
      SET_TARGET_PROPERTIES(${target_name} PROPERTIES 
        SOURCES "${SHADER_SOURCES}"
        OUTPUT_NAME ${out_name}
        SUFFIX ${DXIL_DEFAULT_SUFFIX}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
      )
    ELSE()
      MESSAGE(SEND_ERROR "Unrecognized intermediate language ${compile_as}. Only SPIR-V and DXIL are supported.")
    ENDIF(${compile_as} STREQUAL "SPIRV")
  ELSE()
    MESSAGE(SEND_ERROR "Unrecognized compiler: ${compile_with}. Only DXC and GLSLC are allowed.")
  ENDIF(${compile_with} STREQUAL "GLSLC")
ENDFUNCTION(TARGET_HLSL_SHADERS target_name shader_source shader_model compile_as compile_with shader_type compile_options)


FUNCTION(TARGET_GLSL_SHADERS target_name shader_source compile_as compile_with shader_type compile_options)
  GET_FILENAME_COMPONENT(out_name ${shader_source} NAME_WE)

  SET(SHADER_SOURCES ${shader_source})
  
  FOREACH(SHADER_INCLUDE ${ARGN})
    LIST(APPEND SHADER_SOURCES ${SHADER_INCLUDE})
  ENDFOREACH(SHADER_INCLUDE ${ARGN})
    
  SET (SHADER_STAGE "")
  SET (OUTPUT_DIR "")

  IF(NOT ${compile_with} STREQUAL "GLSLC")
    MESSAGE(SEND_ERROR "Glsl shaders can only be built with glslc.")
  ELSEIF(NOT ${compile_as} STREQUAL "SPIRV")
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
    ELSEIF(${shader_type} STREQUAL "TASK" OR ${shader_type} STREQUAL "AMPLIFICATION")
      SET(SHADER_STAGE "task")
    ELSEIF(${shader_type} STREQUAL "MESH")
      SET(SHADER_STAGE "mesh")
    ELSE()
      MESSAGE(SEND_ERROR "Unsupported shader type: ${shader_type}. Valid shader types are: VERTEX, GEOMETRY, HULL/TESSELATION_CONTROL, DOMAIN/TESSELLATION_EVALUATION, FRAGMENT/PIXEL, COMPUTE and RAYTRACING.")
    ENDIF(${shader_type} STREQUAL "VERTEX")
    
    IF(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
      SET(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DEFAULT_SUBDIR})
    ELSE()
      SET(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER_DEFAULT_SUBDIR})
    ENDIF(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    
    SET(compiler_options ${compile_options})
    SEPARATE_ARGUMENTS(compiler_options)

    IF(${shader_type} STREQUAL "VERTEX" OR ${shader_type} STREQUAL "GEOMETRY" OR ${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")
      LIST(APPEND compiler_options -finvert-y)
    ENDIF(${shader_type} STREQUAL "VERTEX" OR ${shader_type} STREQUAL "GEOMETRY" OR ${shader_type} STREQUAL "DOMAIN" OR ${shader_type} STREQUAL "TESSELATION_EVALUATION")
    
    # TODO: Check if we can use a generator expression to build the output directory and file names, so it is possible to set the target properties to control the result file name.
    ADD_CUSTOM_TARGET(${target_name} 
      COMMENT "glslc: compiling glsl shader '${shader_source}'..."
      COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
      SOURCES ${SHADER_SOURCES}
    )

    ADD_CUSTOM_COMMAND(TARGET ${target_name} POST_BUILD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND ${LITEFX_BUILD_GLSLC_COMPILER} --target-env=vulkan1.3 -mfmt=bin -fshader-stage=${SHADER_STAGE} -DSPIRV -x glsl ${compiler_options} -c ${shader_source} -o "${OUTPUT_DIR}/${out_name}${SPIRV_DEFAULT_SUFFIX}" -MD
    )
    
    SET_TARGET_PROPERTIES(${target_name} PROPERTIES 
      SOURCES "${SHADER_SOURCES}"
      OUTPUT_NAME ${out_name}
      SUFFIX ${SPIRV_DEFAULT_SUFFIX}
      RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    )
  ENDIF(NOT ${compile_with} STREQUAL "GLSLC")
ENDFUNCTION(TARGET_GLSL_SHADERS target_name shader_source compile_as compile_with shader_type compile_options)


FUNCTION(ADD_SHADER_MODULE module_name)
  CMAKE_PARSE_ARGUMENTS(SHADER "" "SOURCE;LANGUAGE;COMPILE_AS;SHADER_MODEL;TYPE;COMPILER;LIBRARY;COMPILE_OPTIONS" "INCLUDES" ${ARGN})

  IF(NOT SHADER_COMPILE_OPTIONS)
    SET(SHADER_COMPILE_OPTIONS " ")  # This must be set to some valid value, since all variable arguments are reserved for shader includes. A whitespace does not hurt.
  ENDIF(NOT SHADER_COMPILE_OPTIONS)

  IF(${SHADER_LANGUAGE} STREQUAL "GLSL")
    TARGET_GLSL_SHADERS(${module_name} ${SHADER_SOURCE} ${SHADER_COMPILE_AS} ${SHADER_COMPILER} ${SHADER_TYPE} ${SHADER_COMPILE_OPTIONS} ${SHADER_INCLUDES})
  ELSEIF(${SHADER_LANGUAGE} STREQUAL "HLSL")
    TARGET_HLSL_SHADERS(${module_name} ${SHADER_SOURCE} ${SHADER_SHADER_MODEL} ${SHADER_COMPILE_AS} ${SHADER_COMPILER} ${SHADER_TYPE} ${SHADER_COMPILE_OPTIONS} ${SHADER_INCLUDES})
  ELSE()
    MESSAGE(SEND_ERROR "Unsupported shader language: ${SHADER_LANGUAGE}.")
  ENDIF(${SHADER_LANGUAGE} STREQUAL "GLSL")

  # If a library is specified, append the shader target it to the library target.
  IF(SHADER_LIBRARY)
    ADD_DEPENDENCIES(${SHADER_LIBRARY} pcksl ${module_name})
    GET_TARGET_PROPERTY(SHADER_LIBRARY_NAMESPACE ${SHADER_LIBRARY} NAMESPACE)
    GET_TARGET_PROPERTY(SHADER_LIBRARY_DIR ${SHADER_LIBRARY} INTERFACE_INCLUDE_DIRECTORIES)
    GET_TARGET_PROPERTY(SHADER_LIBRARY_SOURCE ${SHADER_LIBRARY} OUTPUT_NAME)
    GET_TARGET_PROPERTY(SHADER_PROGRAM_NAME ${module_name} OUTPUT_NAME)
    GET_TARGET_PROPERTY(SHADER_PROGRAM_SUFFIX ${module_name} SUFFIX)
    GET_TARGET_PROPERTY(SHADER_PROGRAM_BINARY_DIR ${module_name} RUNTIME_OUTPUT_DIRECTORY)

    ADD_CUSTOM_COMMAND(TARGET ${SHADER_LIBRARY} POST_BUILD
      COMMAND pcksl pack "\"${SHADER_LIBRARY_DIR}/${SHADER_LIBRARY_SOURCE}\"" "\"${SHADER_PROGRAM_BINARY_DIR}/${SHADER_PROGRAM_NAME}${SHADER_PROGRAM_SUFFIX}\"" ${SHADER_LIBRARY_NAMESPACE} "${SHADER_PROGRAM_NAME}${SHADER_PROGRAM_SUFFIX}"
    )
  ENDIF(SHADER_LIBRARY)
ENDFUNCTION(ADD_SHADER_MODULE module_name)

FUNCTION(TARGET_LINK_SHADERS target_name)
  CMAKE_PARSE_ARGUMENTS(SHADER "" "INSTALL_DESTINATION" "SHADERS" ${ARGN})
  
  ADD_DEPENDENCIES(${target_name} ${SHADER_SHADERS})

  FOREACH(shader_module ${SHADER_SHADERS})
    GET_TARGET_PROPERTY(SHADER_PROGRAM_NAME ${shader_module} OUTPUT_NAME)
    GET_TARGET_PROPERTY(SHADER_PROGRAM_SUFFIX ${shader_module} SUFFIX)
    GET_TARGET_PROPERTY(SHADER_PROGRAM_BINARY_DIR ${shader_module} RUNTIME_OUTPUT_DIRECTORY)
    
    CMAKE_PATH(SET SHADER_INSTALL_DEST NORMALIZE ${CMAKE_INSTALL_PREFIX}/${SHADER_INSTALL_DESTINATION})
    INSTALL(FILES "${SHADER_PROGRAM_BINARY_DIR}/${SHADER_PROGRAM_NAME}${SHADER_PROGRAM_SUFFIX}" DESTINATION ${SHADER_INSTALL_DEST})
  ENDFOREACH(shader_module ${SHADER_SHADERS})
ENDFUNCTION(TARGET_LINK_SHADERS target_name)


FUNCTION(ADD_SHADER_LIBRARY library_name)
  CMAKE_PARSE_ARGUMENTS(SHADER_LIBRARY "" "SOURCE_FILE;NAMESPACE" "" ${ARGN})

  # Define a target for the shader library.
  ADD_CUSTOM_TARGET(${library_name}
    COMMENT "Packing shader library ${library_name} to '${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_LIBRARY_SOURCE_FILE}'..."
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
  )

  ADD_CUSTOM_COMMAND(TARGET ${library_name} POST_BUILD
    COMMAND pcksl init "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_LIBRARY_SOURCE_FILE}"
  )

  SET_TARGET_PROPERTIES(${library_name} PROPERTIES 
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
    NAMESPACE "${SHADER_LIBRARY_NAMESPACE}"
    OUTPUT_NAME "${SHADER_LIBRARY_SOURCE_FILE}"
  )
ENDFUNCTION(ADD_SHADER_LIBRARY library_name)


FUNCTION(TARGET_LINK_SHADER_LIBRARIES target_name)
  CMAKE_PARSE_ARGUMENTS(SHADER "" "" "LIBRARIES" ${ARGN})
  
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
            std::string sourceFile(argv[2]);
            std::ofstream file(sourceFile);
            file << "#pragma once" << std::endl << 
                "#include <iostream>" << std::endl <<
                "#include <array>" << std::endl <<
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
            std::replace(resourceName.begin(), resourceName.end(), '.', '_');
            std::replace(resourceName.begin(), resourceName.end(), ':', '_');

            std::ifstream resource(resourceFile, std::ios::binary);
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
            file << "        static const std::string name() { return \"" << name << "\"; }" << std::endl << std::endl;
            file << "        static _pcksl_mem_istream open() {" << std::endl;
            file << "            static std::array<uint8_t, " << buffer.size() << "> _data = {" << std::endl;
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

ADD_EXECUTABLE(pcksl "${CMAKE_BINARY_DIR}/Auxiliary/pcksl.cxx")
SET_PROPERTY(TARGET pcksl PROPERTY FOLDER "Auxiliary")