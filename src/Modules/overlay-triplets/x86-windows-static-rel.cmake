set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_BUILD_TYPE release)

if(${PORT} MATCHES "winpixeventruntime|directx-agility-sdk")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()