cmake_minimum_required(VERSION 3.16)

project(dmilyushko_build_server)

set(CMAKE_C_COMPILER)

include_directories(.)
add_executable(client client.c)
add_executable(server server.c)
add_executable(param param.c)
 

add_executable(1c 1c.c)
add_executable(1s 1s.c)
add_executable(2c 2c.c)
add_executable(2s 2s.c)