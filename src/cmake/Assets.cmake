###################################################################################################
#####                                                                                         #####
#####               Contains a helper function to define asset dependencies.                  #####
#####                                                                                         #####
###################################################################################################

# The script exports one functions:
#
# - TARGET_ADD_ASSETS: Defines a dependency to a list of static asset files, that are copied to the runtime directory and creates an install command for each.

SET(ASSET_DEFAULT_SUBDIR "assets" CACHE STRING "Default subdirectory for static assets within the current binary directory (CMAKE_CURRENT_BINARY_DIR).")

FUNCTION(TARGET_ADD_ASSETS target_name)
  CMAKE_PARSE_ARGUMENTS(ASSET "" "INSTALL_DESTINATION" "ASSETS" ${ARGN})
  
  IF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")
    SET(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ASSET_DEFAULT_SUBDIR})
  ELSE()
    SET(OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSET_DEFAULT_SUBDIR})
  ENDIF(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} STREQUAL "")

  ADD_CUSTOM_TARGET(${target_name}-assets
    COMMENT "Copying assets to runtime directory '${OUTPUT_DIR}'..."
  )

  ADD_CUSTOM_COMMAND(TARGET ${target_name}-assets 
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
  )
    
  SET_TARGET_PROPERTIES(${target_name}-assets PROPERTIES 
    SOURCES ${ASSET_ASSETS}
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
  )

  FOREACH(ASSET_FILE ${ASSET_ASSETS})
    GET_FILENAME_COMPONENT(ASSET_NAME ${ASSET_FILE} NAME)
    ADD_CUSTOM_COMMAND(TARGET ${target_name}-assets 
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${ASSET_FILE} ${OUTPUT_DIR}/${ASSET_NAME}
      DEPENDS ${ASSET_FILE}
    )

    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${ASSET_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/${ASSET_INSTALL_DESTINATION})
  ENDFOREACH(ASSET_FILE ${ASSET_ASSETS})

  ADD_DEPENDENCIES(${target_name} ${target_name}-assets)
ENDFUNCTION(TARGET_ADD_ASSETS target_name)