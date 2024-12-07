vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    REF "5a53a198945ba8260fbc58fadb788745ce6aa263"
    SHA512 bf1847f0a33473340dd3e886360527a4d7d76644a151862304bdeb372f7a0dc03c54c333d5ed331a8b321706703b68c5a51eed0ab548f7218d8291aa7847def7
    HEAD_REF master
)

set(opts "")
if(VCPKG_TARGET_IS_WINDOWS)
  set(opts "-DCMAKE_INSTALL_INCLUDEDIR=include/vma") # Vulkan SDK layout!
endif()

set(VCPKG_BUILD_TYPE release) # header-only port
vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS ${opts}

)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME VulkanMemoryAllocator CONFIG_PATH "share/cmake/VulkanMemoryAllocator")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
