//
// Created by sergei on 28.05.22.
//

#ifndef BROWN_BOT_SOCKET_HELP_H
#define BROWN_BOT_SOCKET_HELP_H

int make_temp_socket(int domain, int type, int protocol, char* full_sock_path);
int close_temp_socket(const char* full_socket_path);

struct sockaddr_un make_sockaddr(const char* full_path);
int bind_assuring_dirs(int fd, struct sockaddr_un* addr, const char* full_path);
#endif //BROWN_BOT_SOCKET_HELP_H