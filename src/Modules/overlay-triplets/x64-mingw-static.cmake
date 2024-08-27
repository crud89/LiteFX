set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

if(${PORT} MATCHES "winpixeventruntime|directx-agility-sdk")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

set(VCPKG_ENV_PASSTHROUGH PATH)
set(VCPKG_CMAKE_SYSTEM_NAME MinGW)