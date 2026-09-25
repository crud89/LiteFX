vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)
set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

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

if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
    set(WARP_ARCH amd64)
else()
    set(WARP_ARCH ${VCPKG_TARGET_ARCHITECTURE})
endif()

set(WARP_DIR "${PACKAGE_PATH}/build/native/${WARP_ARCH}")

foreach(_dir IN ITEMS "bin" "debug/bin")
    file(COPY "${WARP_DIR}/d3d10warp.dll" "${WARP_DIR}/d3d10warp.pdb" DESTINATION "${CURRENT_PACKAGES_DIR}/${_dir}")
endforeach()

configure_file("${CMAKE_CURRENT_LIST_DIR}/directx-warp-config.cmake.in" "${CURRENT_PACKAGES_DIR}/share/${PORT}/${PORT}-config.cmake" @ONLY)
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${PACKAGE_PATH}/LICENSE.txt")
