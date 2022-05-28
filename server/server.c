#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <poll.h>
#include "../config_read.h" // its bad...
#include "../socket_help.h"

const int MAX_CONN_COUNT = 1000;
const int CONN_QUEUE = 100;

const int POLL_TIMEOUT = -1;
const int MAX_NUM_LEN = 10;

long sum = 0;

typedef struct ReadNumsBuf {
    char* buf;
    int buf_len;

    int buf_pos;
    int last_num_start;
} ReadNumsBuf;

int recv_nums(int client_fd, ReadNumsBuf buf) {
    while (true) {
        int recvd = recv(client_fd, buf.buf, buf.buf_len - buf.buf_pos, MSG_DONTWAIT);
        if (recvd < 0) {
            perror("Error recieving data from client");
            return -1;
        }
        if (recvd == 0) {
            return 0;
        }

        buf.buf_pos += recvd;
    }
}

int process_conns(int server_fd, struct pollfd poll_set[], int* sockets_count) {
    printf("Waiting for client (%d total)...\n", *sockets_count);
    poll(poll_set, *sockets_count, POLL_TIMEOUT);

    for(int pollfd_idx = 0; pollfd_idx < *sockets_count; pollfd_idx++)
    {
        if (poll_set[pollfd_idx].revents & POLLIN) {
            if (poll_set[pollfd_idx].fd == server_fd) {
                if (*sockets_count < MAX_CONN_COUNT) {
                    continue;
                }

                struct sockaddr_un client_address;
                int client_len = sizeof(client_address);
                int client_fd = accept(
                        server_fd, (struct sockaddr*)&client_address, (socklen_t *)&client_len);
                if (client_fd < 0) {
                    perror("Error accepting connection");
                    return -1;
                }

                poll_set[*sockets_count].fd = client_fd;
                poll_set[*sockets_count].events = POLLIN; // POLLOUT?
                *sockets_count++;

                printf("Adding client on fd %d\n", client_fd);
            }
            else {
                // read

                if (poll_set[*sockets_count].events & POLLHUP) {
                    printf("Removing client on fd %d\n", poll_set[*sockets_count].fd);
                    if (close(poll_set[*sockets_count].fd) < 0) {
                        perror("Couldn't close socket");
                        return -1;
                    }

                    poll_set[pollfd_idx] = poll_set[*sockets_count];
                    sockets_count--;
                }
            }
        }
    }
    return 0;
}

int main() { // todo atexit
    bool ok = true;
    printf("I'm server!\n");

    const char* server_full_path = get_server_full_path();
    if (server_full_path == NULL) {
        fprintf(stderr, "Couldn't read server path\n");
        ok = false;
    }

    int server_fd;
    if (ok) {
        server_fd = socket(PF_UNIX, SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("Couldn't create server socket");
            ok = false;
        }
    }

    struct sockaddr_un sockaddr;
    if (ok) {
        sockaddr.sun_family = AF_UNIX;
        strcpy(sockaddr.sun_path, server_full_path);

        if (bind_assuring_dirs(server_fd, &sockaddr, server_full_path) < 0) {
            fprintf(stderr, "Couldn't bind server socket");
            ok = false;
        }
    }

    if (ok) {
        if (listen(server_fd, SERVER_BACKLOG) < 0) {
            perror("Couldn't make server socket listen");
            ok = false;
        }
    }

    if (ok) {
        struct pollfd poll_set[MAX_CONN_COUNT];
        memset(poll_set, '\0', sizeof(poll_set));
        poll_set[0].fd = server_fd;
        poll_set[0].events = POLLIN;
        int sockets_count = 1;

        while (true) {
            if (process_conns(server_fd, poll_set, &sockets_count) < 0) {
                fprintf(stderr, "Error while processing connections");
                ok = false;
                break;
            }
        }
    }

    unlink(server_full_path); // ?
    return 0;
}
