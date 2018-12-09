cmake_minimum_required(VERSION 3.9)
project(streams)

set( CMAKE_CXX_FLAGS "-lpthread -g" )
file(GLOB SOURCES "src/*.cpp")

enable_testing()

add_executable(clustreetest src/clustreetest.cpp)
add_executable(subgraphtest src/subgraphtest.cpp)

add_executable(subgraphmain src/subgraphmain.cpp)
add_executable(clustreemain src/clustreemain.cpp)

target_link_libraries(subgraphmain pthread)
target_link_libraries(clustreemain pthread)

add_test(NAME clustree_ut COMMAND clustreetest)
add_test(NAME subgraph_ut COMMAND subgraphtest)