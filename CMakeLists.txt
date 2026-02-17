cmake_minimum_required(VERSION 3.15)

project(CPYTHON27_X_ROOT)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    if(MSVC)
        add_compile_options(/utf-8 /EHsc)
    endif()
endif()

add_subdirectory(cpython)
# cpython27x_lib
add_subdirectory(python27x)