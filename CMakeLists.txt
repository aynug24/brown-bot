cmake_minimum_required(VERSION 3.22)
project(brown-bot C)

set(CMAKE_C_STANDARD 17)

add_executable(src/data_structs/queue.c src/data_structs/queue.h src/server/server.c src/config_read/config_read.c src/config_read/config_read.h src/socket_help/socket_help.c src/socket_help/socket_help.h src/logs/logs.c src/logs/logs.h src/data_structs/readnumsbuf.c src/data_structs/readnumsbuf.h src/data_structs/sendnumsbuf.c src/data_structs/sendnumsbuf.h)
add_executable(src/client/client.c src/config_read/config_read.c src/config_read/config_read.h src/socket_help/socket_help.c src/socket_help/socket_help.h src/logs/logs.c src/logs/logs.h src/data_structs/readnumsbuf.c src/data_structs/readnumsbuf.h src/client/client_help.c src/client/client_help.h src/data_structs/queue.h src/data_structs/queue.c src/data_structs/sendnumsbuf.c src/data_structs/sendnumsbuf.h)
# add_executable(bb_test test.c config_read.c config_read.h socket_help.c socket_help.h readnumsbuf.c readnumsbuf.h)
# add_executable(bb_test2 test2.c queue.h queue.c config_read.c config_read.h socket_help.c socket_help.h readnumsbuf.c readnumsbuf.h)
add_executable(bb_zerosum src/client/get_state_client.c src/config_read/config_read.c src/config_read/config_read.h src/socket_help/socket_help.c src/socket_help/socket_help.h src/logs/logs.c src/logs/logs.h src/data_structs/readnumsbuf.c src/data_structs/readnumsbuf.h src/client/client_help.c src/client/client_help.h src/data_structs/sendnumsbuf.c src/data_structs/sendnumsbuf.h src/data_structs/queue.h src/data_structs/queue.c)

file(
        COPY
        src/config src/test_zerosum.txt runme.sh src/test_scripts/test_n_clients.sh src/test_scripts/clear_server_log.sh
        src/test_scripts/remove_client_logs.sh src/test_scripts/test_performance.sh src/test_scripts/test_memory.sh
        DESTINATION
        ${CMAKE_CURRENT_BINARY_DIR}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ)