cmake_minimum_required(VERSION 3.10)
project(d3d12-memory-allocator)

FIND_PACKAGE(directx-headers CONFIG REQUIRED)

add_library(${PROJECT_NAME} STATIC 
    "include/D3D12MemAlloc.h"
    "src/D3D12MemAlloc.cpp"
)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "include/D3D12MemAlloc.h")
target_include_directories(${PROJECT_NAME} PRIVATE "include/")
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Headers)
target_compile_definitions(${PROJECT_NAME} PUBLIC "USING_DIRECTX_HEADERS")

install(
    TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-config
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION include/
)
install(
    EXPORT ${PROJECT_NAME}-config DESTINATION cmake/
    NAMESPACE "unofficial::"
)