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
#include "../readnumsbuf.h"
#include "../sendnumsbuf.h"

const int MAX_CONN_COUNT = 1000;
const int CONN_QUEUE = 100;

const int POLL_TIMEOUT = -1;
const int MAX_NUM_LEN = 10;

const int BUFFER_LEN = 1024;

long sum = 0;

int free_recv(ReadNumsBuf* recv_bufs, int sock_count) {
    for (int i = 0; i < sock_count; ++i) {
        free(recv_bufs[i].buf);
    }
}

int free_send(SendNumsBuf* send_bufs, int sock_count) {
    for (int i = 0; i < sock_count; ++i) {
        free(send_bufs[i].buf);
    }
}

int process_conns(
        int server_fd, struct pollfd poll_set[], int* sockets_count,
                ReadNumsBuf* recv_bufs, SendNumsBuf* send_bufs) {
    bool ok = true;
    printf("Waiting for client (%d total)...\n", *sockets_count);
    poll(poll_set, *sockets_count, POLL_TIMEOUT);

    for (int pollfd_idx = 0; pollfd_idx < *sockets_count; pollfd_idx++)
    {
        if (poll_set[pollfd_idx].revents & POLLIN) {
            if (poll_set[pollfd_idx].fd == server_fd) {
                if (*sockets_count < MAX_CONN_COUNT) {
                    continue;
                }

                struct sockaddr_un client_address;
                int client_len = sizeof(client_address);
                int client_fd = accept(
                        server_fd, (struct sockaddr*) &client_address, (socklen_t*) &client_len);
                printf("Adding client on fd %d\n", client_fd);
                if (client_fd < 0) {
                    perror("Error accepting connection");
                    ok = false;
                    break;
                }

                if (make_readbuf(BUFFER_LEN, recv_bufs + *sockets_count) < 0) {
                    fprintf(stderr, "Error creating readbuf");
                    ok = false;
                    break;
                }
                if (make_sendbuf(BUFFER_LEN, send_bufs + *sockets_count) < 0) {
                    fprintf(stderr, "Error creating sendbuf");
                    ok = false;
                    break;
                }

                poll_set[*sockets_count].fd = client_fd;
                poll_set[*sockets_count].events = POLLIN & POLLOUT & POLLHUP;
                *sockets_count++;
            } else {
                // read
            }
        }
        if (poll_set[pollfd_idx].events & POLLOUT) {
            if (pollfd_idx == 0) {
                continue;
            }

            // send
        }
        if (poll_set[pollfd_idx].events & POLLHUP) {
            printf("Removing client on fd %d\n", poll_set[*sockets_count].fd);
            if (close(poll_set[pollfd_idx].fd) < 0) {
                perror("Couldn't close socket");
                ok = false;
                break;
            }

            free(recv_bufs[pollfd_idx].buf);
            recv_bufs[pollfd_idx] = recv_bufs[*sockets_count];
            free(send_bufs[pollfd_idx].buf);
            send_bufs[pollfd_idx] = send_bufs[*sockets_count];

            poll_set[pollfd_idx] = poll_set[*sockets_count];
            sockets_count--;
        }
    }

    return ok ? 0 : -1;
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

        ReadNumsBuf recv_bufs[MAX_CONN_COUNT];
        SendNumsBuf send_bufs[MAX_CONN_COUNT];
        memset(recv_bufs, 0, sizeof(recv_bufs));
        memset(send_bufs, 0, sizeof(send_bufs));

        while (true) {
            if (process_conns(server_fd, poll_set, &sockets_count, recv_bufs, send_bufs) < 0) {
                fprintf(stderr, "Error while processing connections");
                ok = false;
                break;
            }
        }
        free_recv(recv_bufs, sockets_count);
        free_send(send_bufs, sockets_count);
    }

    unlink(server_full_path); // ?
    return 0;
}
