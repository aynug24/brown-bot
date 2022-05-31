#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <ctype.h>
#include "../config_read/config_read.h" // its bad...
#include "../socket_help.h"
#include "../data_structs/readnumsbuf.h"
#include "../data_structs/sendnumsbuf.h"
#include "../logs/logs.h"

const int MAX_CONN_COUNT = 1000;
const int CONN_QUEUE = 100;

const int POLL_TIMEOUT = -1;
const int MAX_NUM_LEN = 10;
const int MAX_SENT_NUM_LEN = 22;

const int MAX_ONEPOLL_RECV = 2048;
const int MIN_QUEUE_FREESPACE = 10;

const int BUFFER_LEN = 1024;

long long sum = 0;

static char* server_full_path; // it was an attempt to unlink socket in atexit

FILE* log_file = NULL;

void process_number(long long n, long long* res) {
    //printf("Received %lld\n", n);
    *res += sum;
    sum += n;
}

void process_new_numbers(Queue* queue, ssize_t old_rear) {
    for (ssize_t pos = old_rear; pos < queue->rear; pos = (pos + 1) % queue->capacity) {
        process_number(queue->nums[pos], &queue->nums[pos]);
    }
}

int set_log_file(int argc, char* argv[]) {
    char* log_path_rel = malloc(MAX_LOG_REL_PATH_LEN);
    if (log_path_rel == NULL) {
        perror("Couldn't allocate memory for log path");
        return -1;
    }

    bool found_log;
    opterr = 0;
    int argname;
    while ((argname = getopt(argc, argv, "l:")) != -1) {
        switch (argname) {
            case 'l':
                strcpy(log_path_rel, optarg);
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

    if (!found_log) {
        free(log_path_rel);
        log_path_rel = NULL;
    }

    if (open_log(log_path_rel, &log_file) < 0) {
        fprintf(stderr, "Couldn't open log\n");
        free(log_path_rel);
        return -1;
    }

    free(log_path_rel);
    return 0;
}

void free_recv(ReadNumsBuf* recv_bufs, int sock_count) {
    for (int i = 0; i < sock_count; ++i) {
        free(recv_bufs[i].buf);
    }
}

void free_send(SendNumsBuf* send_bufs, int sock_count) {
    for (int i = 0; i < sock_count; ++i) {
        free(send_bufs[i].queue.nums);
        free(send_bufs[i].unsent_num);
    }
}

int accept_new_conn(int server_fd, struct pollfd poll_set[], int* sockets_count,
                    ReadNumsBuf* recv_bufs, SendNumsBuf* send_bufs) {
    if (*sockets_count >= MAX_CONN_COUNT) {
        return 0;
    }

    struct sockaddr_un client_address;
    int client_len = sizeof(client_address);
    int client_fd = accept(
            server_fd, (struct sockaddr*) &client_address, (socklen_t*) &client_len);
    //printf("Adding client on fd %d\n", client_fd);
    if (client_fd < 0) {
        perror("Error accepting connection");
        return -1;
    }

    if (make_readbuf(BUFFER_LEN, recv_bufs + *sockets_count) < 0) {
        fprintf(stderr, "Error creating readbuf\n");
        return -1;
    }
    if (make_sendbuf(BUFFER_LEN, MAX_SENT_NUM_LEN, send_bufs + *sockets_count) < 0) {
        fprintf(stderr, "Error creating sendbuf\n");
        return -1;
    }

    poll_set[*sockets_count].fd = client_fd;
    poll_set[*sockets_count].events = POLLIN | /*POLLOUT |*/ POLLHUP;
    (*sockets_count)++;

    if (log_incoming_connection(log_file, client_fd) < 0) {
        fprintf(stderr, "Error logging new connection\n");
        return -1;
    }

    return 0;
}

int close_socket(int fd_idx, struct pollfd poll_set[], int* sockets_count,
                         ReadNumsBuf* recv_bufs, SendNumsBuf* send_bufs) {
    //printf("Removing client on fd %d\n", poll_set[fd_idx].fd);
    if (close(poll_set[fd_idx].fd) < 0) {
        perror("Couldn't close socket");
        return -1;
    }

    free(recv_bufs[fd_idx].buf);
    recv_bufs[fd_idx] = recv_bufs[*sockets_count - 1];
    free(send_bufs[fd_idx].queue.nums);
    free(send_bufs[fd_idx].unsent_num);
    send_bufs[fd_idx] = send_bufs[*sockets_count - 1];

    poll_set[fd_idx] = poll_set[*sockets_count - 1];
    memset(&poll_set[*sockets_count - 1], 0, sizeof(*poll_set));
    (*sockets_count)--;
    return 0;
}

int process_conns(
        int server_fd, struct pollfd poll_set[], int* sockets_count,
                ReadNumsBuf* recv_bufs, SendNumsBuf* send_bufs) {
    bool ok = true;
    //printf("Waiting for client (%d total)...\n", *sockets_count);
    poll(poll_set, *sockets_count, POLL_TIMEOUT);

    for (int pollfd_idx = 0; pollfd_idx < *sockets_count; pollfd_idx++)
    {
        if (poll_set[pollfd_idx].revents == 0) { // just added this socket
            continue;
        }

        if (poll_set[pollfd_idx].revents & POLLIN && poll_set[pollfd_idx].fd == server_fd) {
            if (accept_new_conn(server_fd, poll_set, sockets_count, recv_bufs, send_bufs) < 0) {
                fprintf(stderr, "Couldn't accept new connection\n");
                ok = false;
                break;
            }
            continue;
        }
        if (poll_set[pollfd_idx].fd == server_fd) {
            continue;
        }

        if (poll_set[pollfd_idx].revents & POLLNVAL) {
            fprintf(stderr, "CANT HAPPEN?\n");
        }

        ssize_t old_numqueue_rear = send_bufs[pollfd_idx].queue.rear;
        bool nothing_to_recv = true;
        bool format_fail = false;
        if (poll_set[pollfd_idx].revents & POLLIN) {
            int recv_res = recv_nums(poll_set[pollfd_idx].fd, &recv_bufs[pollfd_idx],
                      MAX_NUM_LEN, MAX_ONEPOLL_RECV, &send_bufs[pollfd_idx].queue, MIN_QUEUE_FREESPACE, log_file);
            if (recv_res == -2) {
                fprintf(stderr, "Error receiving data\n");
                ok = false;
                break;
            } else if (recv_res == -1) {
                // format error
                format_fail = true;
                log_format_fail(stderr);
            } else if (recv_res == 0) {
                nothing_to_recv = true;
                poll_set[pollfd_idx].events |= POLLOUT;
                poll_set[pollfd_idx].events &= ~POLLIN;
            }

            process_new_numbers(&send_bufs[pollfd_idx].queue, old_numqueue_rear);
        }

        //if (poll_set[pollfd_idx].revents) { // & !POLLVAL?
        bool sending_is_over = false;
        int send_res = send_nums(poll_set[pollfd_idx].fd, &send_bufs[pollfd_idx], log_file);
        if (send_res < 0) {
            fprintf(stderr, "Error while sending to client\n");
            ok = false;
            break;
        } else if (send_res == 0) {
            sending_is_over = true;
        } else if (send_res == 2) {
            sending_is_over = (poll_set[pollfd_idx].revents & POLLOUT) > 0;
        }
        //}
        //if ((poll_set[pollfd_idx].revents & POLLNVAL) || format_fail || nothing_to_recv) { //(nothing_to_recv && poll_set[pollfd_idx].events & POLLHUP)) {
        if (format_fail || sending_is_over) { //(nothing_to_recv && poll_set[pollfd_idx].events & POLLHUP)) {
            if (close_socket(pollfd_idx, poll_set, sockets_count, recv_bufs, send_bufs) < 0) {
                fprintf(stderr, "Couldn't close socket\n");
                ok = false;
                break;
            }
            fflush(log_file); // in order to examine the logs
            --pollfd_idx; // iterate over (foremrly) last socket
        }
    }

    return ok ? 0 : -1;
}

int main(int argc, char* argv[]) {
    //atexit(clean);

    bool ok = true;
    //printf("I'm server!\n");

    if (set_log_file(argc, argv) < 0) {
        fprintf(stderr, "Error opening log\n");
        ok = false;
    }

    if (ok) {
        server_full_path = get_server_full_path();
        if (server_full_path == NULL) {
            fprintf(stderr, "Couldn't read server path\n");
            ok = false;
        }
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
            fprintf(stderr, "Couldn't bind server socket\n");
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
                fprintf(stderr, "Error while processing connections\n");
                ok = false;
                break;
            }
        }
        free_recv(recv_bufs, sockets_count);
        free_send(send_bufs, sockets_count);
    }

    return 0;
}
