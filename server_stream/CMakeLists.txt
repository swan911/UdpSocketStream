cmake_minimum_required(VERSION 3.8)

project(server_stream)

add_executable(server_stream main.cpp)

target_compile_features(server_stream PRIVATE cxx_std_17)

set(CMAKE_EXE_LINKER_FLAGS "-pthread")

install(TARGETS server_stream RUNTIME DESTINATION bin)
