cmake_minimum_required(VERSION 3.8)

project(client_stream2)

add_executable(client_stream2 main.cpp)

target_compile_features(client_stream2 PRIVATE cxx_std_17)

install(TARGETS client_stream2 RUNTIME DESTINATION bin)
