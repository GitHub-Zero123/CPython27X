cmake_minimum_required(VERSION 3.15)

project(CPYTHON27_X_LIB)

add_library(cpython27x_lib STATIC IMPORTED GLOBAL)

if(MSVC)
    set_target_properties(cpython27x_lib PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/PCbuild/amd64/python27.lib"
        IMPORTED_LOCATION_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/PCbuild/amd64/python27_d.lib"
    )
    target_include_directories(cpython27x_lib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Include")
    target_include_directories(cpython27x_lib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/PC")
    # 添加Py_NO_ENABLE_SHARED定义
    target_compile_definitions(cpython27x_lib INTERFACE Py_NO_ENABLE_SHARED)
else()
    # linux环境 链接.a文件
    if(UNIX)
        set_target_properties(cpython27x_lib PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/libpython2.7.a"
        )
        target_include_directories(cpython27x_lib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Include")
        target_include_directories(cpython27x_lib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
        # 添加Py_NO_ENABLE_SHARED定义
        target_compile_definitions(cpython27x_lib INTERFACE Py_NO_ENABLE_SHARED)
    else()
        message(FATAL_ERROR "Only MSVC and UNIX are supported for now.")
    endif()
endif()