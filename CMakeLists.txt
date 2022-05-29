cmake_minimum_required(VERSION 3.22)
project(brown-bot C)

set(CMAKE_C_STANDARD 17)

add_executable(bb_server data_structs/queue.c data_structs/queue.h server/server.c config_read.c config_read.h socket_help.c socket_help.h logs/logs.c logs/logs.h data_structs/readnumsbuf.c data_structs/readnumsbuf.h data_structs/sendnumsbuf.c data_structs/sendnumsbuf.h)
add_executable(bb_client client/client.c config_read.c config_read.h socket_help.c socket_help.h logs/logs.c logs/logs.h data_structs/readnumsbuf.c data_structs/readnumsbuf.h client/client_help.c client/client_help.h data_structs/queue.h data_structs/queue.c data_structs/sendnumsbuf.c data_structs/sendnumsbuf.h)
# add_executable(bb_test test.c config_read.c config_read.h socket_help.c socket_help.h readnumsbuf.c readnumsbuf.h)
# add_executable(bb_test2 test2.c queue.h queue.c config_read.c config_read.h socket_help.c socket_help.h readnumsbuf.c readnumsbuf.h)
add_executable(bb_zerotest client/get_state_client.c config_read.c config_read.h socket_help.c socket_help.h logs/logs.c logs/logs.h data_structs/readnumsbuf.c data_structs/readnumsbuf.h client/client_help.c client/client_help.h data_structs/sendnumsbuf.c data_structs/sendnumsbuf.h data_structs/queue.h data_structs/queue.c)

file(COPY config test_zerosum.txt runme.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})