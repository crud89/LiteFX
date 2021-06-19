###################################################################################################
#####                                                                                         #####
#####                    LiteFX version settings. Do not modify this file!                    #####
#####                                                                                         #####
###################################################################################################

# The year part of the version is actually ignored when it comes to compatibility. Different RELEASE versions are generally considered 
# incompatible, whilst PATCH versions are considered generally backwards compatible. Different BUILD versions don't affect compatibility,
# but instead only contain bug fixes.

SET(LITEFX_YEAR 2021)
SET(LITEFX_RELEASE 1)
SET(LITEFX_PATCH 0)
SET(LITEFX_BUILD 1)
SET(LITEFX_VERSION "${LITEFX_YEAR}.${LITEFX_RELEASE}.${LITEFX_PATCH}.${LITEFX_BUILD}")