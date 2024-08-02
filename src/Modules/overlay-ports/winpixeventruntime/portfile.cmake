set(VCPKG_POLICY_DLLS_IN_STATIC_LIBRARY enabled)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Microsoft/PixEvents
    REF 3a7e70dde7bf54f02f9d2e9dd6d3350c6cfb962f
    SHA512 91b7a2f8c83d8e848ec89866e8f30092a1e49f5d327fd5dd972bdb6f8848cef28fd135f6304df0811241a95f379089853edf98c451dc680fc6d7838a8be8edd1
    HEAD_REF main
    PATCHES win32compat.patch
)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt DESTINATION ${SOURCE_PATH})

vcpkg_cmake_configure(SOURCE_PATH ${SOURCE_PATH})
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH share/cmake/)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")