cmake_minimum_required(VERSION 3.9)
project(streams)

set( CMAKE_CXX_FLAGS "-lpthread -O3" )
file(GLOB SOURCES "src/*.cpp")

enable_testing()

add_executable(clustreetest src/clustreetest.cpp)
add_executable(subgraphtest src/subgraphtest.cpp)

add_executable(subgraphmain src/subgraphmain.cpp)

add_test(NAME clustreeut COMMAND clustreetest)
add_test(NAME subgraphut COMMAND subgraphtest)

target_link_libraries(subgraphmain pthread)