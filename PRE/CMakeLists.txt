cmake_minimum_required(VERSION 3.17)
project(PRE)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -O3")

add_executable(run1 main.cpp)

add_executable(run2 relabel.cpp)

add_executable(run3 test.cpp)

add_executable(run4 sort_edge.cpp)