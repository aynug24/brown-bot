//
// Created by sergei on 29.05.22.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../config_read/config_read.h"
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
#include "../logs/logs.h"

const int CLIENT_PATH_MAXLEN = 1024; // todo разнести константы туда где должны быть (или избавиться)
const int CLIENT_WAIT_TIME_MAXLEN = 10;
const int DEFAULT_WAIT_TIME_MS = 10;

const int MAX_BATCH_SIZE = 255;
const int RECV_BUF_SIZE = 65536; // can print one at a time if needed
const int RES_QUEUE_SIZE = 2048;

int get_wait_and_log(int argc, char* argv[], char** wait, char** log) {
    bool found_wait = false;
    bool found_log = false;

    opterr = 0;
    int argname;
    while ((argname = getopt(argc, argv, "w:l:")) != -1) {
        switch (argname) {
            case 'w':
                strcpy(*wait, optarg);
                found_wait = true;
                break;
            case 'l':
                strcpy(*log, optarg);
                found_log = true;
                break;
            case '?':
                if (optopt == 'w')
                    fprintf(stderr, "Option -%c requires an argument.\n", (char) optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", (char) optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return -1;
            default:
                fprintf(stderr, "Unknown getopt return");
                return -1;
        }
    }

    if (!found_wait) {
        *wait = NULL;
    }
    if (!found_log) {
        *log = NULL;
    }
    return 1;
}

int get_client_args(int argc, char* argv[], ClientArguments* arguments) {
    bool ok = true;

    char* wait_time_str = malloc(CLIENT_WAIT_TIME_MAXLEN * sizeof(*wait_time_str));
    if (wait_time_str == NULL) {
        perror("Couldn't allocate memory for wait time");
        ok = false;
    }

    char* log_file_path = malloc(MAX_LOG_REL_PATH_LEN);
    if (log_file_path == NULL) {
        perror("Couldn't allocate memory for log path");
        ok = false;
    }

    char* old_wait_time = wait_time_str;
    char* old_log_path = log_file_path;
    if (get_wait_and_log(argc, argv, &wait_time_str, &log_file_path) < 0) {
        fprintf(stderr, "Couldn't parse client arguments");
        ok = false;
    }

    if (wait_time_str == NULL) {
        arguments->wait_time = DEFAULT_WAIT_TIME_MS;
    } else {
        char* arg_end = NULL;
        const long wait_time = strtol(wait_time_str, &arg_end, 10);
        if (*wait_time_str == '\0' || *arg_end != '\0') {
            fprintf(stderr, "Invalid wait time argument format: \"%s\"\n", wait_time_str);
            ok = false;
        } else if (wait_time < 0) {
            fprintf(stderr, "Argument value error: negative wait time %ld", wait_time);
            ok = false;
        } else {
            arguments->wait_time = wait_time;
        }
    }

    if (ok) {
        if (open_log(log_file_path, &arguments->log_file) < 0) {
            fprintf(stderr, "Error opening client log\n");
            ok = false;
        }
    }

    free(old_wait_time);
    free(old_log_path);
    return ok ? 0 : -1;
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
        return -1;
    }
    return res;
}

int get_connected_client_sock(int argc, char* argv[], ClientArguments* arguments, char** client_full_path) {
    bool ok = true;
    //printf("I'm client!\n"); // todo удалить отладочные принты

    if (get_client_args(argc, argv, arguments) < 0) {
        fprintf(stderr, "Couldn't parse client args\n");
        return -1;
    }

    *client_full_path = malloc(CLIENT_PATH_MAXLEN);
    if (*client_full_path == NULL) {
        perror("Couldn't allocate socket path string");
        return -1;
    }

    char* server_full_path = get_server_full_path();
    if (server_full_path == NULL) {
        fprintf(stderr, "Couldn't read server path\n");
        ok = false;
    }

    int client_fd;
    if (ok) {
        client_fd = make_temp_socket(AF_UNIX, SOCK_STREAM, 0, *client_full_path);
        if (client_fd < 0) {
            fprintf(stderr, "Couldn't make client socket\n");
            ok = false;
        }
    }

    if (ok) {
        struct sockaddr_un server_addr = make_sockaddr(server_full_path);
        if (connect(client_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
            perror("Couldn't connect to server");
            ok = false;
        }
    }

    free(server_full_path);
    return ok ? client_fd : -1;
}

