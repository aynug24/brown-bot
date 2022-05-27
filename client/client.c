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

const int CLIENT_PATH_MAXLEN = 1024;

int main() {
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
            perror("Couldn't connect to server\n");
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

    free((char*)server_full_path);
    if (close_temp_socket(server_full_path) < 0) {
        fprintf(stderr, "Can't close client socket\n");
    }
    return ok ? 0 : -1;
}
