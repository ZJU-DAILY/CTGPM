cmake_minimum_required(VERSION 3.15)
project(CTGPM)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g2 -ggdb")

include_directories(${CMAKE_CURRENT_LIST_DIR}/src)

#include_directories(${CMAKE_CURRENT_LIST_DIR}/3rdpart/boost/include)

add_executable(main test/main.cpp)
target_link_libraries(main pthread)