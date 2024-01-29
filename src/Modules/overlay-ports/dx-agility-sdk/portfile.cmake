vcpkg_minimum_required(VERSION 2022-10-12) # for ${VERSION}
vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

vcpkg_download_distfile(ARCHIVE
    URLS "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/${VERSION}"
    FILENAME "dxagilitysdk.${VERSION}.zip"
    SHA512 22dba68f5911588a6261fdc0192e8dca57edd25bdd3bc72b610ad468a0a6dbae51db1a9cdeadf4b57f6017136af78b07ef1357b1d93bedf4e6922076bd57efb5
)

vcpkg_extract_source_archive(
    PACKAGE_PATH
    ARCHIVE ${ARCHIVE}
    NO_REMOVE_ONE_LEVEL
)

set(_d3d12_implib "$ENV{WindowsSdkDir}/Lib/$ENV{WindowsSDKLibVersion}/um/$ENV{VSCMD_ARG_TGT_ARCH}/d3d12.lib")
string(REPLACE "\\" "/" _d3d12_implib "${_d3d12_implib}")
string(REPLACE "//" "/" _d3d12_implib "${_d3d12_implib}")

if (NOT EXISTS "${_d3d12_implib}")
    message(FATAL_ERROR "Could not find d3d12.lib. Make sure a recent Windows SDK version is properly installed on the system.")
endif()

#set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)

file(INSTALL "${PACKAGE_PATH}/build/native/include/d3d12.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3d12sdklayers.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3d12shader.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3d12video.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dcommon.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/dxgiformat.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3d12.idl" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3d12sdklayers.idl" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3d12video.idl" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dcommon.idl" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${PACKAGE_PATH}/build/native/include/dxgiformat.idl" DESTINATION "${CURRENT_PACKAGES_DIR}/include")

file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_barriers.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_check_feature_support.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_core.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_default.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_pipeline_state_stream.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_property_format_table.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_render_pass.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_resource_helpers.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_root_signature.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")
file(INSTALL "${PACKAGE_PATH}/build/native/include/d3dx12/d3dx12_state_object.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/d3dx12")

file(INSTALL "${PACKAGE_PATH}/build/native/src/d3dx12/d3dx12_property_format_table.cpp" DESTINATION "${CURRENT_PACKAGES_DIR}/src/d3dx12")

file(INSTALL "${_d3d12_implib}" DESTINATION "${CURRENT_PACKAGES_DIR}/lib")

if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
    file(COPY "${PACKAGE_PATH}/build/native/bin/win32/D3D12Core.dll" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
    file(COPY "${PACKAGE_PATH}/build/native/bin/win32/d3d12SDKLayers.dll" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
    #file(COPY "${PACKAGE_PATH}/build/native/bin/win32/d3dconfig.exe" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
else()
    file(COPY "${PACKAGE_PATH}/build/native/bin/${VCPKG_TARGET_ARCHITECTURE}/D3D12Core.dll" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
    file(COPY "${PACKAGE_PATH}/build/native/bin/${VCPKG_TARGET_ARCHITECTURE}/d3d12SDKLayers.dll" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
    #file(COPY "${PACKAGE_PATH}/build/native/bin/${VCPKG_TARGET_ARCHITECTURE}/d3dconfig.exe" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
endif()

# Agility SDK requires apps to export symbols, which are defined in this file.
string(REPLACE "." ";" VERSION_LIST ${VERSION})
list(GET VERSION_LIST 1 D3D12SDK_VERSION)
configure_file("${CMAKE_CURRENT_LIST_DIR}/d3d12agility.hpp.in" "${CURRENT_PACKAGES_DIR}/include/d3d12agility.hpp")
unset(D3D12SDK_VERSION)
unset(VERSION_LIST)

# Create debug binaries as a copy of the release binaries
file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/debug")
file(COPY "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/lib" DESTINATION "${CURRENT_PACKAGES_DIR}/debug")

file(INSTALL "${PACKAGE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

configure_file("${CMAKE_CURRENT_LIST_DIR}/dxagilitysdk-config.cmake.in" "${CURRENT_PACKAGES_DIR}/share/${PORT}/${PORT}-config.cmake" COPYONLY)