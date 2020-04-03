cmake_minimum_required(VERSION 2.6)

add_subdirectory(OMPEval/omp)



add_executable(holdem-eval holdem-eval.cpp PercentageToRange.cpp)


set(link_libs "ompeval")
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
list(APPEND link_libs "pthread")
if(BUILD_SSE4)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
endif(BUILD_SSE4)
else()
if(BUILD_SSE4)
message("SSE4 Not Support")
endif(BUILD_SSE4)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(holdem-eval
        ${link_libs}
)




