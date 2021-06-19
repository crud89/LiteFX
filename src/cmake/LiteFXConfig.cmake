###################################################################################################
#####                                                                                         #####
#####         LiteFX Package configuration file. Installed alongside target export.           #####
#####                                                                                         #####
###################################################################################################

GET_FILENAME_COMPONENT(LITEFX_EXPORT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Include target export configuration.
INCLUDE("${LITEFX_EXPORT_DIR}/LiteFX.cmake")

# Also include helper scripts here. Simply define LITEFX_WITHOUT_HELPERS, to prevent those scripts from being imported.
IF(NOT DEFINED LITEFX_WITHOUT_HELPERS)
  INCLUDE("${LITEFX_EXPORT_DIR}/Assets.cmake")
  INCLUDE("${LITEFX_EXPORT_DIR}/Shaders.cmake")
ENDIF(NOT DEFINED LITEFX_WITHOUT_HELPERS)