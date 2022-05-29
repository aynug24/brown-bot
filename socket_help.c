#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "socket_help.h"
#include "config_read.h"

const char* TMP_DIR_TEMPLATE = "client_XXXXXX";
const char* TMP_SOCKET_NAME = "brown-bot-client.so";


int make_temp_socket(int domain, int type, int protocol, char* full_sock_path) {
    bool ok = true;

    char* full_template = NULL;
    if (ok) {
        full_template = malloc(strlen(SOCKETS_DIR) + strlen(TMP_DIR_TEMPLATE) + 1);
        if (full_template == NULL) {
            perror("Couldn't allocate temp socket dir name");
            ok = false;
        } else if (sprintf(full_template, "%s%s", SOCKETS_DIR, TMP_DIR_TEMPLATE) < 0) {
            perror("Couldn't concat temp socket dir name");
            ok = false;
        }
    }

    const char* temp_dir = NULL;
    if (ok) {
        temp_dir = mkdtemp(full_template);
        if (temp_dir == NULL) {
            perror("Couldn't create directory for temp socket");
            ok = false;
        }
    }

    if (ok) {
        if (sprintf(full_sock_path, "%s/%s", temp_dir, TMP_SOCKET_NAME) < 0) {
            perror("Couldn't concat temp socket path");
            ok = false;
        }
    }

    int temp_fd = 0;
    if (ok) {
        temp_fd = socket(domain, type, protocol);
        if (temp_fd == -1) {
            perror("Couldn't create temp socket");
            ok = false;
        }
    }

    if (ok) {
        struct sockaddr_un sockaddr;
        sockaddr.sun_family = PF_UNIX;
        strcpy(sockaddr.sun_path, full_sock_path);

        if (bind(temp_fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
            perror("Couldn't bind temp socket");
            ok = false;
        }
    }

    free(full_template);

    return temp_fd;
}

int close_temp_socket(const char* full_socket_path) {
    if (unlink(full_socket_path) < 0) {
        perror("Couldn't delete client socket");
        return -1;
    }

    char path_cpy[strlen(full_socket_path)];
    strcpy(path_cpy, full_socket_path);

    char* dir_end = strrchr(path_cpy, '/');
    *dir_end = '\0';

    if (rmdir(path_cpy) < 0) {
        perror("Couldn't remove client socket directory");
        return -1;
    }
    return 0;
}

//struct sockaddr_un* make_server_sockaddr() {
//    const char* server_full_path = get_server_full_path();
//    if (server_full_path == NULL) {
//        fprintf(stderr, "Couldn't read server path");
//        return NULL;
//    }
//
//
//}

struct sockaddr_un make_sockaddr(const char* full_path) {
    struct sockaddr_un sockaddr;
    sockaddr.sun_family = AF_UNIX;
    strcpy(sockaddr.sun_path, full_path);
    return sockaddr;
}

int mkdirs(const char* path) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, S_IRWXU) < 0 && errno != EEXIST) {
                perror("Couldn't create dir");
                return -1;
            };
            *p = '/';
        }
//    if (mkdir(tmp, S_IRWXU) < 0) { // ?
//        perror("Couldn't create dir");
//        return -1;
//    };
}

int bind_assuring_dirs(int fd, struct sockaddr_un* addr, const char* full_path) {
    if (mkdirs(full_path) < 0) {
        fprintf(stderr, "Couldn't create dirs");
        return -1;
    }

    unlink(full_path);
    if (bind(fd, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        perror("Couldn't bind server socket");
        return -1;
    }
    return 0;
}