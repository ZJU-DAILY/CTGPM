cmake_minimum_required(VERSION 3.17)
project(Match)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -pthread -O3")

add_executable(run1 main.cpp  Basic.h PatternGraph.h DataGraph.h Baseline.h DependencyGraph.h SubGraph.h DisjointSetUnion.h SimRunner.h SimBalancer.h Fraction.h MatchGraph.h)

add_executable(run2 syn.cpp  Basic.h PatternGraph.h DataGraph.h Baseline.h DependencyGraph.h SubGraph.h DisjointSetUnion.h SimRunner.h SimBalancer.h Fraction.h MatchGraph.h)

add_executable(run3 smallcase.cpp  Basic.h PatternGraph.h DataGraph.h Baseline.h DependencyGraph.h SubGraph.h DisjointSetUnion.h SimRunner.h SimBalancer.h Fraction.h MatchGraph.h)