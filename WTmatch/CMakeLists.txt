cmake_minimum_required(VERSION 3.5)
project(WTmatch)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -O3")

add_executable(run main.cpp Basic.hpp PatternGraph.hpp DataGraph.hpp Fraction.hpp DependencyGraph.hpp SimRunner.hpp MatchGraph.hpp Baseline.hpp DisjointSetUnion.hpp)

add_executable(run2 main2.cpp Basic.hpp PatternGraph.hpp DataGraph.hpp Fraction.hpp DependencyGraph.hpp SimRunner.hpp MatchGraph.hpp Baseline.hpp DisjointSetUnion.hpp)