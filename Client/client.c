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

int send_stdin_recv_sums(long wait_ms, int client_fd, char* recv_buf, ssize_t* total_recvd) {
    bool ok = true;
    *total_recvd = 0;
    srand(time(NULL));

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
            *total_recvd += recvd;
        }

        if (msleep(wait_ms) < 0) {
            fprintf(stderr, "Couldn't sleep after input");
            ok = false;
            break;
        }
    }

    free(read_buf);
    return ok ? 0 : -1;
}

int main(int argc, char* argv[]) {
    bool ok = true;
    char* full_server_path = NULL;
    char* full_client_addr = NULL;

    char* recv_buf = NULL;
    if ((recv_buf = malloc(RECV_BUF_SIZE)) == NULL) {
        perror("Couldn't allocate memory for result");
        ok = false;
    }

    int client_fd;
    long wait_ms;
    if (ok) {
        client_fd = get_connected_client_sock(argc, argv, &full_server_path, &full_client_addr, &wait_ms);
        if (client_fd < 0) {
            ok = false;
        }
    }

    ssize_t total_recvd;
    if (ok) {
        if (send_stdin_recv_sums(wait_ms, client_fd, recv_buf, &total_recvd) < 0) {
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
        printf("%s", recv_buf);
    }

    if (close_temp_socket(full_client_addr) < 0) {
        fprintf(stderr, "Can't close client socket\n");
    }
    free((char*)full_server_path);
    free(recv_buf);
    return ok ? 0 : -1;
}
