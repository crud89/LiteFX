cmake_minimum_required(VERSION 3.20)
project(WinPixEventRuntime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Run message compiler for ETW manifest.
# Note: this naturally requries msvc toolset.
add_custom_target(PixEtw 
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/intermediates/"
    COMMAND mc "${CMAKE_SOURCE_DIR}/runtime/lib/PixEtw.man" -um -P ETW_EVENT -r "${CMAKE_SOURCE_DIR}/intermediates/" -h "${CMAKE_SOURCE_DIR}/intermediates/"
)

# Define library project
add_library(WinPixEventRuntimeLib STATIC
    "runtime/lib/BlockAllocator.h"
    "runtime/lib/pch.h"
    "runtime/lib/ThreadData.h"
    "runtime/lib/ThreadedWorker.h"
    "runtime/lib/Threads.h"
    "runtime/lib/WinPixEventRuntime.h"
    "runtime/lib/Worker.h"

    "runtime/lib/BlockAllocator.cpp"
    "runtime/lib/ThreadData.cpp"
    "runtime/lib/ThreadedWorker.cpp"
    "runtime/lib/Threads.cpp"
    "runtime/lib/WinPixEventRuntime.cpp"
    "runtime/lib/pch.cpp"

    "runtime/lib/PixEtw.man"
)

# TODO: Use vcpkg to resolve WIL.
target_include_directories(WinPixEventRuntimeLib 
    PUBLIC "include/" 
    PRIVATE "intermediates/" "third_party/wil/include/" "${CMAKE_SOURCE_DIR}/" 
)
target_precompile_headers(WinPixEventRuntimeLib PRIVATE "runtime/lib/pch.h")
target_compile_definitions(WinPixEventRuntimeLib PUBLIC "USE_PIX" "USE_PIX_ON_ALL_ARCHITECTURES") # TODO: For DEBUG builds, define DBG macro.
add_dependencies(WinPixEventRuntimeLib PixEtw)

# Define dynamic runtime library.
add_library(WinPixEventRuntime SHARED
    "runtime/dll/desktop/WinPixEventRuntime.def"

    "runtime/dll/shared/WinPixEventRuntime.shared.cpp"
    "runtime/dll/shared/WinDllMain.cpp"
)

target_link_libraries(WinPixEventRuntime PRIVATE WinPixEventRuntimeLib)
target_include_directories(WinPixEventRuntime 
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>" "$<INSTALL_INTERFACE:include/>"
    PRIVATE "runtime/" "intermediates/" "third_party/wil/include/" "${CMAKE_SOURCE_DIR}/" 
)

set_target_properties(WinPixEventRuntime PROPERTIES
    PUBLIC_HEADER "include/PIXEvents.h;include/PIXEventsCommon.h;include/PIXEventsLegacy.h;include/pix3.h;include/pix3_win.h"
)

# Define install targets.
install(TARGETS WinPixEventRuntime EXPORT WinPixEventRuntime)
export(TARGETS WinPixEventRuntime NAMESPACE Microsoft:: FILE WinPixEventRuntimeConfig.cmake)
install(EXPORT WinPixEventRuntime DESTINATION "${CMAKE_INSTALL_PREFIX}/share/cmake" NAMESPACE Microsoft:: FILE WinPixEventRuntimeConfig.cmake)