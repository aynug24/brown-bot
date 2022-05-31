OPTIMIZATION := O3

COMMON_SOURCES := src/config_read/config_read.c \
	src/data_structs/readnumsbuf.c \
	src/data_structs/sendnumsbuf.c \
	src/data_structs/queue.c \
	src/logs/logs.c \
	src/socket_help/socket_help.c

COMMON_CLIENT_SOURCES := src/client/client_help.c

all: bb_server bb_client bb_zerosum config scripts test_files

bb_server: config
	gcc	-$(OPTIMIZATION) src/server/server.c \
		$(COMMON_SOURCES) \
		-o bb_server

bb_client: config
	gcc -$(OPTIMIZATION) src/client/client.c \
		$(COMMON_SOURCES) $(COMMON_CLIENT_SOURCES) \
		-o bb_client

bb_zerosum: config
	gcc -$(OPTIMIZATION) src/client/get_state_client.c \
		$(COMMON_SOURCES) $(COMMON_CLIENT_SOURCES) \
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
	rm -f result.txt config test_zerosum.txt