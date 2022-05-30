#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../socket_help.h"
#include "../data_structs/readnumsbuf.h"

#include <memory.h>
#include <sys/socket.h>
#include "client_help.h"


int main(int argc, char* argv[]) {
    bool ok = true;
    char* full_server_path = NULL;
    char* full_client_addr = NULL;

    char* state_buf = NULL;
    if ((state_buf = malloc(30)) == NULL) {
        perror("Couldn't allocate memory for result");
        ok = false;
    }

    int client_fd;
    ClientArguments _args;
    if (ok) {
        client_fd = get_connected_client_sock(argc, argv, &_args, &full_client_addr);
        if (client_fd < 0) {
            ok = false;
        }
    }

    if (ok) {
        char* zero = "0";
        if (send(client_fd, zero, 1, 0) < 0) {
//        if (dprintf(client_fd, "0") < 0) {
            perror("Couldn't send zero to server");
            ok = false;
        }
    }

    if (ok) {
        if (shutdown(client_fd, SHUT_WR) < 0) {
            perror("Couldn't shut socket's write down");
            ok = false;
        }
    }

    if (ok) {
        size_t total_recvd = 0;
        while (true) {
            ssize_t recvd = recv(client_fd, state_buf + total_recvd, 30, 0);
            if (recvd < 0) {
                perror("Error receiving");
                ok = false;
                break;
            }
            if (recvd == 0) {
                break;
            }
            total_recvd += recvd;
            if (total_recvd >= 1) {
                break;
            }
        }
        state_buf[total_recvd] = '\0';

    }

    if (ok) {
        char* end_ptr = NULL;
        long long answer = strtoll(state_buf, &end_ptr, 10);
        if (*state_buf != '\0' && *end_ptr == '\n') {
            fprintf(stdout, "%lld", answer);
        } else {
            fprintf(stderr, "Couldn't parse answer\n");
        }
    }

    if (close_temp_socket(full_client_addr) < 0) {
        fprintf(stderr, "Can't close client socket\n");
    }
    free((char*)full_server_path);
    return ok ? 0 : -1;
}
