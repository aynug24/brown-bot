#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../config_read.h"
#include "../socket_help.h"
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

const int CLIENT_PATH_MAXLEN = 1024; // todo разнести константы туда где должны быть (или избавиться)
const int CLIENT_WAIT_TIME_MAXLEN = 10;
const int DEFAULT_WAIT_TIME_MS = 10;

bool try_get_wait_arg(int argc, char* argv[], char* dest) {
    opterr = 0;
    int argname;
    while ((argname = getopt(argc, argv, "w:")) != -1) {
        switch (argname) {
            case 'w':
                strcpy(dest, optarg);
                return true;
            case '?':
                if (optopt == 'w')
                    fprintf(stderr, "Option -%c requires an argument.\n", (char)optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", (char)optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return false;
            default:
                fprintf(stderr, "Unknown getopt return");
                return false;
        }
    }

    dest = NULL;
    return true;
}

long get_wait_time(int argc, char* argv[]) {
    char* wait_time_str = malloc(CLIENT_WAIT_TIME_MAXLEN * sizeof(*wait_time_str));
    if (wait_time_str == NULL) {
        perror("Couldn't allocate memory for wait time");
    }

    if (!try_get_wait_arg(argc, argv, wait_time_str)) {
        fprintf(stderr, "Error while parsing arguments\n");
    }
    if (wait_time_str == NULL) {
        return DEFAULT_WAIT_TIME_MS;
    }

    char** arg_end = &wait_time_str;
    const long wait_time = strtol(wait_time_str, arg_end, 10);
    if (*wait_time_str == '\0' || **arg_end != '\0') {
        fprintf(stderr, "Invalid wait time argument format: \"%s\"\n", wait_time_str);
        return -1;
    }
    if (wait_time < 0) {
        fprintf(stderr, "Argument value error: negative wait time %ld", wait_time);
        return -1;
    }
    if (wait_time > INT_MAX) {
        fprintf(stderr, "Argument value too big for int: %lds", wait_time);
        return -1;
    }

    return wait_time;
}

int main(int argc, char* argv[]) {
    bool ok = true;
    printf("I'm client!\n");

    const char* server_full_path = get_server_full_path();
    if (server_full_path == NULL) {
        fprintf(stderr, "Couldn't read server path\n");
        ok = false;
    }

    int client_fd;
    char full_client_addr[CLIENT_PATH_MAXLEN];
    if (ok) {
        client_fd = make_temp_socket(AF_UNIX, SOCK_STREAM, 0, full_client_addr);
        if (client_fd == -1) {
            fprintf(stderr, "Couldn't make client socket\n");
            ok = false;
        }
    }

    if (ok) {
        struct sockaddr_un server_addr = make_sockaddr(server_full_path);
        if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("Couldn't connect to server");
            ok = false;
        }
    }

    if (ok) {
        char* buf = malloc(1024);
        size_t len = 1024;
        ssize_t r = getline(&buf, &len, stdin);

        send(client_fd, buf, r, 0);
        printf("Sent!");
    }

    if (close_temp_socket(full_client_addr) < 0) {
        fprintf(stderr, "Can't close client socket\n");
    }
    free((char*)server_full_path);
    return ok ? 0 : -1;
}
