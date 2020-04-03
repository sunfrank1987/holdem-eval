cmake_minimum_required(VERSION 2.8)

file(GLOB lib_ompeval *.cpp)

add_library(ompeval ${lib_ompeval})
