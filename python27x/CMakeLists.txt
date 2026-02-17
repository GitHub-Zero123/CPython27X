cmake_minimum_required(VERSION 3.15)

project(CPYTHON27_X)

add_executable(python27x main.cpp patch.cpp)
target_compile_features(python27x PRIVATE cxx_std_20)
target_link_options(python27x PRIVATE /LTCG)
target_link_libraries(python27x PRIVATE cpython27x_lib)