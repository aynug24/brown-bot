all: bb_server bb_client bb_zerosum config scripts test_files

bb_server: config
	gcc src/data_structs/queue.c src/data_structs/queue.h \
		src/server/server.c \
		src/config_read/config_read.c src/config_read/config_read.h \
		src/socket_help/socket_help.c src/socket_help/socket_help.h \
		src/logs/logs.c src/logs/logs.h \
		src/data_structs/readnumsbuf.c src/data_structs/readnumsbuf.h \
		src/data_structs/sendnumsbuf.c src/data_structs/sendnumsbuf.h \
		-o bb_server

bb_client: config
	gcc src/client/client.c \
		src/config_read/config_read.c src/config_read/config_read.h \
		src/socket_help/socket_help.c src/socket_help/socket_help.h \
		src/logs/logs.c src/logs/logs.h \
		src/data_structs/readnumsbuf.c src/data_structs/readnumsbuf.h \
		src/client/client_help.c src/client/client_help.h \
		src/data_structs/queue.h src/data_structs/queue.c \
		src/data_structs/sendnumsbuf.c src/data_structs/sendnumsbuf.h \
		-o bb_client

bb_zerosum: config
	gcc src/client/get_state_client.c \
		src/config_read/config_read.c src/config_read/config_read.h \
		src/socket_help/socket_help.c src/socket_help/socket_help.h \
		src/logs/logs.c src/logs/logs.h \
		src/data_structs/readnumsbuf.c src/data_structs/readnumsbuf.h \
		src/client/client_help.c src/client/client_help.h \
		src/data_structs/sendnumsbuf.c src/data_structs/sendnumsbuf.h \
		src/data_structs/queue.h src/data_structs/queue.c \
		-o bb_zerosum

config:
	cp src/config .

scripts:
	cp -R src/test_scripts/* .
	chmod u+x *.sh

test_files:
	cp src/test_zerosum.txt .
	chmod u+x *.sh

clear:
	ls | grep -v runme.sh | grep .sh | xargs rm -f
	rm -f bb_*
	rm -f result.txt config