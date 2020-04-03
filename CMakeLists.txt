# test: test.cpp benchmark.cpp lib/ompeval.a
cmake_minimum_required(VERSION 2.6)

project(holdem-eval)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# TODO: Remove the depricated register keywords
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register")

option(BUILD_TEST "build for test" OFF)
option(BUILD_SSE4 "build for SSE4" ON)


find_package(Threads)

set(root_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${root_src_dir})

add_subdirectory(src)





