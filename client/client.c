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
#include <time.h>
#include "client_help.h"

ClientArguments args;

long long total_sleep_ns = 0LL;

int send_stdin_recv_sums(int client_fd, char* recv_buf, ssize_t* total_recvd) {
    bool ok = true;
    *total_recvd = 0;
    srand(time(NULL) ^ getpid());

    char* read_buf = malloc(MAX_BATCH_SIZE * sizeof(*read_buf));
    if (read_buf == NULL) {
        perror("Couldn't allocate read_buf for input");
        return -1;
    }

    while (true) {
        int bytes_to_read = rand_range(1, MAX_BATCH_SIZE);
        ssize_t bytes_read = read(STDIN_FILENO, read_buf, bytes_to_read);
        if (bytes_read == -1) {
            perror("Error reading input");
            ok = false;
            break;
        }
        if (bytes_read == 0) { // its eof if using read
            break;
        }

        if (send(client_fd, read_buf, bytes_read, MSG_NOSIGNAL) < 0) {
            perror("Error sending input to server");
            ok = false;
            break;
        }

        int recvd = recv(client_fd, recv_buf + *total_recvd, RECV_BUF_SIZE - *total_recvd, MSG_DONTWAIT);
        if (recvd < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK)) {
            perror("Error receiving from server");
            ok = false;
            break;
        }
        if (recvd >= 0) {
            //printf("%.*s\n", recvd, recv_buf + *total_recvd);
            *total_recvd += recvd;
        }

        struct timespec sleep_start, sleep_end;
        clock_gettime(CLOCK_REALTIME, &sleep_start);
        if (msleep(args.wait_time) < 0) {
            fprintf(stderr, "Couldn't sleep after input\n");
            ok = false;
            break;
        }
        clock_gettime(CLOCK_REALTIME, &sleep_end);
        total_sleep_ns += 1000000000LL * (sleep_end.tv_sec - sleep_start.tv_sec) + (sleep_end.tv_nsec - sleep_start.tv_nsec);
    }

    free(read_buf);
    return ok ? 0 : -1;
}

int main(int argc, char* argv[]) {
    bool ok = true;
    char* full_client_addr = NULL;

    char* recv_buf = NULL;
    if (ok) {
        if ((recv_buf = malloc(RECV_BUF_SIZE)) == NULL) {
            perror("Couldn't allocate memory for result");
            ok = false;
        }
    }

    int client_fd;
    if (ok) {
        client_fd = get_connected_client_sock(argc, argv, &args, &full_client_addr);
        if (client_fd < 0) {
            ok = false;
        }
        //printf("!%ld!", args.wait_time);
    }

    ssize_t total_recvd;
    if (ok) {
        if (send_stdin_recv_sums(client_fd, recv_buf, &total_recvd) < 0) {
            fprintf(stderr, "Error while communicating with server\n");
            ok = false;
        }
    }

    if (ok) {
        if (shutdown(client_fd, SHUT_WR) < 0) {
            perror("Error while shutting down socket write");
            ok = false;
        }
    }

    if (ok) {
        ssize_t last_recv = -1;
        while (last_recv != 0) {
            last_recv = recv(client_fd, recv_buf + total_recvd, RECV_BUF_SIZE - total_recvd, 0);
            if (last_recv < 0) {
                perror("Error receiving last answer from server");
                ok = false;
                break;
            }
            total_recvd += last_recv;
        }
        recv_buf[total_recvd] = '\0';
        //printf("%s", recv_buf);
    }

    if (ok) {
        if (log_total_sleep(args.log_file, total_sleep_ns) < 0) {
            fprintf(stderr, "Error writing total sleep to log\n");
            ok = false;
        }
    }

    if (close_temp_socket(full_client_addr) < 0) {
        fprintf(stderr, "Can't close client socket\n");
    }
    free(recv_buf);
    return ok ? 0 : -1;
}
