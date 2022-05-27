#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "socket_help.h"
#include "config_read.h"

const char* TMP_DIR_TEMPLATE = "client_XXXXXXXXXXXXX";
const char* TMP_SOCKET_NAME = "brown-bot-client.so";


int make_temp_socket(int domain, int type, int protocol, char* full_sock_path) {
    bool ok = true;

    char* full_template = NULL;
    if (ok) {
        full_template = malloc(strlen(SOCKETS_DIR) + strlen(TMP_DIR_TEMPLATE) + 1);
        if (full_template == NULL) {
            perror("Couldn't allocate temp socket dir name\n");
            ok = false;
        } else if (sprintf(full_template, "%s%s", SOCKETS_DIR, TMP_DIR_TEMPLATE) < 0) {
            perror("Couldn't concat temp socket dir name\n");
            ok = false;
        }
    }

    const char* temp_dir = NULL;
    if (ok) {
        temp_dir = mkdtemp(full_template);
        if (temp_dir == NULL) {
            perror("Couldn't create directory for temp socket\n");
            ok = false;
        }
    }

    if (ok) {
        full_sock_path = malloc(strlen(full_template) + strlen(TMP_SOCKET_NAME) + 2);
        if (full_sock_path == NULL) {
            perror("Couldn't allocate temp socket path\n");
            ok = false;
        } else if (sprintf(full_sock_path, "%s/%s", temp_dir, TMP_SOCKET_NAME)) {
            perror("Couldn't concat temp socket path\n");
            ok = false;
        }
    }

    int temp_fd = 0;
    if (ok) {
        temp_fd = socket(domain, type, protocol);
        if (temp_fd == -1) {
            perror("Couldn't create temp socket\n");
            ok = false;
        }
    }

    if (ok) {
        struct sockaddr_un sockaddr;
        sockaddr.sun_family = PF_UNIX;
        strcpy(sockaddr.sun_path, full_sock_path);

        if (bind(temp_fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
            perror("Couldn't bind temp socket\n");
            ok = false;
        }
    }

    free(full_template);
    free((char*)temp_dir);

    return temp_fd;
}

int close_temp_socket(const char* full_socket_path) {
    if (unlink(full_socket_path) < 0) {
        perror("Couldn't delete client socket\n");
        return -1;
    }

    char path_cpy[strlen(full_socket_path)];
    strcpy(path_cpy, full_socket_path);

    char* dir_end = strrchr(path_cpy, '\\');
    *dir_end = '\0';

    if (rmdir(path_cpy) < 0) {
        perror("Couldn't remove client socket directory\n");
        return -1;
    };
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

int bind(int fd, struct sockaddr_un* sockaddr, char* full_path) {
    
}

int mkdir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp) < 0) {
                perror("Couldn't create dir");
                
            };
            *p = '/';
        }
    mkdir(tmp);
}