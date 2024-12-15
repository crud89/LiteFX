vcpkg_minimum_required(VERSION 2022-10-12) # for ${VERSION}
vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
    URLS "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.WARP/${VERSION}"
    FILENAME "dxwarp.${VERSION}.zip"
    SHA512 43956bf44575082b14a37efb956fe1be4cbeb624acbe811e1dbb062308652e9bd08959b97e474530ad2d12427ef88032d4fd323baab102ba600262b0c38019c9
)

vcpkg_extract_source_archive(
    PACKAGE_PATH
    ARCHIVE ${ARCHIVE}
    NO_REMOVE_ONE_LEVEL
)

set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)
set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
    file(COPY "${PACKAGE_PATH}/build/native/amd64/d3d10warp.dll" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
    file(COPY "${PACKAGE_PATH}/build/native/amd64/d3d10warp.pdb" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
else()
    file(COPY "${PACKAGE_PATH}/build/native/${VCPKG_TARGET_ARCHITECTURE}/d3d10warp.dll" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
    file(COPY "${PACKAGE_PATH}/build/native/${VCPKG_TARGET_ARCHITECTURE}/d3d10warp.pdb" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
endif()

# Create debug binaries as a copy of the release binaries
file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/debug")
file(COPY "${CURRENT_PACKAGES_DIR}/bin" DESTINATION "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL "${PACKAGE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

configure_file("${CMAKE_CURRENT_LIST_DIR}/dxwarp-config.cmake.in" "${CURRENT_PACKAGES_DIR}/share/${PORT}/${PORT}-config.cmake" COPYONLY)