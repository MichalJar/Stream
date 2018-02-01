project(streams)
include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_executable(streams ${SOURCES})
