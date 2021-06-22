###################################################################################################
#####                                                                                         #####
#####               Contains a helper function to define asset dependencies.                  #####
#####                                                                                         #####
###################################################################################################

# The script exports one function:
#
# - TARGET_ADD_ASSET_DIRECTORY: Defines a directory within the runtime output and a set of static assets that are copied to it. Also defines 
#                               an install command for each asset.
#
# The following sample demonstrates how to use this command:
#
# TARGET_ADD_ASSET_DIRECTORY(my_project
#   NAME "assets/textures"
#   INSTALL_DESTINATION "bin/"
#   ASSETS "images/a.tga" "textures/b.tga"
# )
#
# In the example above, one asset directory is defined for the target 'my_project'. The directory will be created in the runtime output 
# directory, if CMAKE_RUNTIME_OUTPUT_DIRECTORY is not empty, or in CMAKE_CURRENT_BINARY_DIR directory, if it is empty. The directory and 
# possible subdirectories are defined by the NAME parameter.
#
# The INSTALL_DESTINATION specifies the destination, the directory will be installed to by the created install command. Note that the parameter
# is always prepended by CMAKE_INSTALL_PREFIX.
#
# The ASSETS parameter defines a list of the files that are copied to the directory during the build. Note that you have to ensure, that no two 
# file names are the same, since it would result in one of the files being overwritten.

FUNCTION(TARGET_ADD_ASSET_DIRECTORY target_name)
  CMAKE_PARSE_ARGUMENTS(ASSET_DIRECTORY "" "NAME;INSTALL_DESTINATION" "ASSETS" ${ARGN})
  STRING(REPLACE "/" "-" directory_target_name ${ASSET_DIRECTORY_NAME})
  SET(directory_target_name "${target_name}-${directory_target_name}")

  GET_TARGET_PROPERTY(folder_name ${target_name} FOLDER)
  
  IF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")
    SET(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ASSET_DIRECTORY_NAME})
  ELSE()
    SET(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSET_DIRECTORY_NAME})
  ENDIF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")

  ADD_CUSTOM_TARGET(${directory_target_name}
    COMMENT "Copying assets to runtime directory '${OUTPUT_DIR}'..."
  )

  ADD_CUSTOM_COMMAND(TARGET ${directory_target_name}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
  )
    
  SET_TARGET_PROPERTIES(${directory_target_name} PROPERTIES 
    SOURCES ${ASSET_DIRECTORY_ASSETS}
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    FOLDER ${folder_name}
  )

  FOREACH(ASSET_FILE ${ASSET_DIRECTORY_ASSETS})
    GET_FILENAME_COMPONENT(ASSET_NAME ${ASSET_FILE} NAME)
    ADD_CUSTOM_COMMAND(TARGET ${directory_target_name} 
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${ASSET_FILE} ${OUTPUT_DIR}/${ASSET_NAME}
      DEPENDS ${ASSET_FILE}
    )
  ENDFOREACH(ASSET_FILE ${ASSET_DIRECTORY_ASSETS})

  INSTALL(DIRECTORY ${OUTPUT_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/${ASSET_DIRECTORY_INSTALL_DESTINATION})

  ADD_DEPENDENCIES(${target_name} ${directory_target_name})
ENDFUNCTION(TARGET_ADD_ASSET_DIRECTORY target_name)