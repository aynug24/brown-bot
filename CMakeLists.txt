cmake_minimum_required(VERSION 3.22)
project(brown-bot C)

set(CMAKE_C_STANDARD 17)

add_executable(bb_server DataStructs/queue.c DataStructs/queue.h Server/server.c ConfigRead/config_read.c ConfigRead/config_read.h socket_help.c socket_help.h Logs/logs.c Logs/logs.h DataStructs/readnumsbuf.c DataStructs/readnumsbuf.h DataStructs/sendnumsbuf.c DataStructs/sendnumsbuf.h)
add_executable(bb_client Client/client.c ConfigRead/config_read.c ConfigRead/config_read.h socket_help.c socket_help.h Logs/logs.c Logs/logs.h DataStructs/readnumsbuf.c DataStructs/readnumsbuf.h Client/client_help.c Client/client_help.h DataStructs/queue.h DataStructs/queue.c DataStructs/sendnumsbuf.c DataStructs/sendnumsbuf.h)
# add_executable(bb_test test.c ConfigRead.c ConfigRead.h socket_help.c socket_help.h readnumsbuf.c readnumsbuf.h)
# add_executable(bb_test2 test2.c queue.h queue.c ConfigRead.c ConfigRead.h socket_help.c socket_help.h readnumsbuf.c readnumsbuf.h)
add_executable(bb_zerosum Client/get_state_client.c ConfigRead/config_read.c ConfigRead/config_read.h socket_help.c socket_help.h Logs/logs.c Logs/logs.h DataStructs/readnumsbuf.c DataStructs/readnumsbuf.h Client/client_help.c Client/client_help.h DataStructs/sendnumsbuf.c DataStructs/sendnumsbuf.h DataStructs/queue.h DataStructs/queue.c)

file(COPY config test_zerosum.txt runme.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})