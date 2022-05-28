cmake_minimum_required(VERSION 3.22)
project(brown-bot C)

set(CMAKE_C_STANDARD 17)

add_executable(bb_server server/server.c config_read.c config_read.h socket_help.c socket_help.h)
add_executable(bb_client client/client.c config_read.c config_read.h socket_help.c socket_help.h)
add_executable(bb_test test.c config_read.c config_read.h socket_help.c socket_help.h)

file(COPY config test_zerosum.txt runme.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})