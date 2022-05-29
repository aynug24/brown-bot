//
// Created by sergei on 29.05.22.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../config_read.h"
#include "../socket_help.h"
#include "../data_structs/readnumsbuf.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include "client_help.h"

const int CLIENT_PATH_MAXLEN = 1024; // todo разнести константы туда где должны быть (или избавиться)
const int CLIENT_WAIT_TIME_MAXLEN = 10;
const int DEFAULT_WAIT_TIME_MS = 10;

const int MAX_BATCH_SIZE = 255;
const int RECV_BUF_SIZE = 65536; // can print one at a time if needed
const int RES_QUEUE_SIZE = 2048;

bool _try_get_wait_arg(int argc, char* argv[], char** dest) {
    opterr = 0;
    int argname;
    while ((argname = getopt(argc, argv, "w:")) != -1) {
        switch (argname) {
            case 'w':
                strcpy(*dest, optarg);
                return true;
            case '?':
                if (optopt == 'w')
                    fprintf(stderr, "Option -%c requires an argument.\n", (char) optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", (char) optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return false;
            default:
                fprintf(stderr, "Unknown getopt return");
                return false;
        }
    }

    *dest = NULL;
    return true;
}

long get_wait_time(int argc, char* argv[]) {
    char* wait_time_str = malloc(CLIENT_WAIT_TIME_MAXLEN * sizeof(*wait_time_str));
    if (wait_time_str == NULL) {
        perror("Couldn't allocate memory for wait time");
    }

    char* old_wait_time = wait_time_str;
    if (!_try_get_wait_arg(argc, argv, &wait_time_str)) {
        fprintf(stderr, "Error while parsing arguments\n");
    }
    if (wait_time_str == NULL) {
        free(old_wait_time);
        return DEFAULT_WAIT_TIME_MS;
    }

    bool ok = true;
    char* arg_end = NULL;
    const long wait_time = strtol(wait_time_str, &arg_end, 10);
    if (*wait_time_str == '\0' || *arg_end != '\0') {
        fprintf(stderr, "Invalid wait time argument format: \"%s\"\n", wait_time_str);
        ok = false;
    } else if (wait_time < 0) {
        fprintf(stderr, "Argument value error: negative wait time %ld", wait_time);
        ok = false;
    }

    free(wait_time_str);
    return ok ? wait_time : -1;
}

int rand_range(int min, int max) { // not really uniform buuut
    int r = rand();
    return min + (r % (max - min + 1));
}

int msleep(long ms) {
    struct timespec ts;
    int res;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    if (res < 0 && errno != EINTR) {
        perror("Error sleeping");
    }
    return res;
}

int get_connected_client_sock(int argc, char* argv[], char** server_full_path, char** client_full_path, long* wait_ms) {
    printf("I'm client!\n"); // todo удалить отладочные принты

    *wait_ms = get_wait_time(argc, argv);
    if (*wait_ms < 0) {
        fprintf(stderr, "Couldn't determine wait value\n");
        return -1;
    }

    *client_full_path = malloc(CLIENT_PATH_MAXLEN);
    if (*client_full_path == NULL) {
        perror("Couldn't allocate socket path string");
        return -1;
    }

    *server_full_path = get_server_full_path();
    if (*server_full_path == NULL) {
        fprintf(stderr, "Couldn't read server path\n");
        return -1;
    }

    int client_fd = make_temp_socket(AF_UNIX, SOCK_STREAM, 0, *client_full_path);
    if (client_fd == -1) {
        fprintf(stderr, "Couldn't make client socket\n");
        return -1;
    }

    struct sockaddr_un server_addr = make_sockaddr(*server_full_path);
    if (connect(client_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("Couldn't connect to server");
        return -1;
    }

    return client_fd;
}

