#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "../config_read.h" // its bad...
#include "../socket_help.h"

long sum = 0;

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
        while (true) {
            const int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd == -1) {
                perror("Error accepting client socket");
                ok = false;
                break;
            }

            // tmp
            char* buf = malloc(1000);
            const int r = read(client_fd, buf, 1000);
            buf[r] = '\0';
            printf("%s", buf);
            free(buf);
        }
    }

    unlink(server_full_path); // ?
    return 0;
}
