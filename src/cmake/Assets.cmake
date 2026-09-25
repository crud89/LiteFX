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
# The INSTALL_DESTINATION specifies the destination, the directory will be installed to by the created install command. The destination is
# relative to the install prefix.
#
# The ASSETS parameter defines a list of the files that are copied to the directory during the build. Note that you have to ensure, that no two 
# file names are the same, since it would result in one of the files being overwritten.

FUNCTION(TARGET_ADD_ASSET_DIRECTORY target_name)
  CMAKE_PARSE_ARGUMENTS(ASSET_DIRECTORY "" "NAME;INSTALL_DESTINATION" "ASSETS" ${ARGN})
  STRING(REPLACE "/" "-" directory_target_name ${ASSET_DIRECTORY_NAME})
  SET(directory_target_name "${target_name}-${directory_target_name}")

  LITEFX_GET_RUNTIME_DIRECTORY(OUTPUT_DIR)
  SET(OUTPUT_DIR "${OUTPUT_DIR}/${ASSET_DIRECTORY_NAME}")

  ADD_CUSTOM_TARGET(${directory_target_name}
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${OUTPUT_DIR}"
    COMMENT "Copying assets to runtime directory '${ASSET_DIRECTORY_NAME}'..."
    SOURCES ${ASSET_DIRECTORY_ASSETS}
    VERBATIM
  )

  SET_TARGET_PROPERTIES(${directory_target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
  GET_TARGET_PROPERTY(folder_name ${target_name} FOLDER)

  IF(folder_name)
    SET_TARGET_PROPERTIES(${directory_target_name} PROPERTIES FOLDER "${folder_name}")
  ENDIF(folder_name)

  FOREACH(ASSET_FILE ${ASSET_DIRECTORY_ASSETS})
    CMAKE_PATH(ABSOLUTE_PATH ASSET_FILE BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE ASSET_PATH)
    CMAKE_PATH(GET ASSET_FILE FILENAME ASSET_NAME)

    ADD_CUSTOM_COMMAND(TARGET ${directory_target_name} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${ASSET_PATH}" "${OUTPUT_DIR}/${ASSET_NAME}"
      VERBATIM
    )
  ENDFOREACH(ASSET_FILE ${ASSET_DIRECTORY_ASSETS})

  IF(ASSET_DIRECTORY_INSTALL_DESTINATION)
    CMAKE_PATH(GET ASSET_DIRECTORY_NAME PARENT_PATH ASSET_PARENT_DIR)
    CMAKE_PATH(APPEND ASSET_DIRECTORY_INSTALL_DESTINATION "${ASSET_PARENT_DIR}" OUTPUT_VARIABLE ASSET_INSTALL_DIR)
    INSTALL(DIRECTORY "${OUTPUT_DIR}" DESTINATION "${ASSET_INSTALL_DIR}")
  ENDIF(ASSET_DIRECTORY_INSTALL_DESTINATION)

  ADD_DEPENDENCIES(${target_name} ${directory_target_name})
ENDFUNCTION(TARGET_ADD_ASSET_DIRECTORY target_name)