###################################################################################################
#####                                                                                         #####
#####            LiteFX Package version file. Installed alongside target export.              #####
#####                                                                                         #####
###################################################################################################

SET(PACKAGE_VERSION @LITEFX_VERSION@)

IF("${PACKAGE_FIND_VERSION_MAJOR}" EQUAL "@LITEFX_YEAR@")
  IF("${PACKAGE_FIND_VERSION_MINOR}" EQUAL "@LITEFX_RELEASE@")
    SET(PACKAGE_VERSION_EXACT TRUE)
  ELSEIF("${PACKAGE_FIND_VERSION_MINOR}" LESS "@LITEFX_PATCH@")
    SET(PACKAGE_VERSION_COMPATIBLE TRUE)
  ELSEIF()
    SET(PACKAGE_VERSION_UNSUITABLE TRUE)
  ENDIF()
ELSE()
  SET(PACKAGE_VERSION_UNSUITABLE TRUE)
ENDIF()