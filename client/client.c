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
#include <time.h>

const int CLIENT_PATH_MAXLEN = 1024; // todo разнести константы туда где должны быть (или избавиться)
const int CLIENT_WAIT_TIME_MAXLEN = 10;
const int DEFAULT_WAIT_TIME_MS = 10;

const int MAX_BATCH_SIZE = 255;

bool _try_get_wait_arg(int argc, char* argv[], char** dest) {
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

int msleep(long ms)
{
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

int send_stdin_recv_sum(long wait_ms, int client_fd) {
    bool ok = true;
    srand(time(NULL));

    char* read_buf = malloc(MAX_BATCH_SIZE * sizeof(*read_buf));
    if (read_buf == NULL) {
        perror("Couldn't allocate read_buf for input");
        return -1;
    }

    char* recv_buf = malloc(sizeof(long long));
    if (recv_buf == NULL) {
        perror("Couldn't allocate recv_buf");
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

        if (send(client_fd, read_buf, bytes_read, 0) < 0) {
            perror("Error sending input to server");
            ok = false;
            break;
        }

        int bytes_recvd;
        while ((bytes_recvd = recv(client_fd, recv_buf, sizeof(long long), MSG_DONTWAIT)) != 0) {
            if (bytes_recvd < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Error receiving server answer");
                ok = false;
            }
            if (bytes_recvd == 0) {
                continue;
            }
            if (bytes_recvd != sizeof(long long)) { // если надо, могу вфигачить совсем асинхронно, но ща мне это кажется перебором
                recv(client_fd, recv_buf + bytes_recvd, sizeof(long long) - bytes_recvd, MSG_WAITALL);
            }


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
    printf("I'm client!\n"); // todo удалить отладочные принты

    const long wait_ms = get_wait_time(argc, argv);
    if (wait_ms < 0) {
        fprintf(stderr, "Couldn't determine wait value\n");
        ok = false;
    }


    const char* server_full_path = NULL;
    if (ok) {
        server_full_path = get_server_full_path();
        if (server_full_path == NULL) {
            fprintf(stderr, "Couldn't read server path\n");
            ok = false;
        }
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
        if (send_stdin_recv_sum(wait_ms, client_fd) < 0) {
            fprintf(stderr, "Error while communicating with server\n");
            ok = false;
        }
    }

    if (close_temp_socket(full_client_addr) < 0) {
        fprintf(stderr, "Can't close client socket\n");
    }
    free((char*)server_full_path);
    return ok ? 0 : -1;
}
