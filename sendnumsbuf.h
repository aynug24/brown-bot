//
// Created by sergei on 29.05.22.
//

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
#include <fcntl.h>
#include <assert.h>
#include "config_read.h" // its bad...
#include "socket_help.h"


#ifndef BROWN_BOT_SENDNUMSBUF_H
#define BROWN_BOT_SENDNUMSBUF_H


typedef struct SendNumsBuf {
    long long* buf;
    size_t buf_len; // sizeof(buf) - 1

    char* unsent_num;
    size_t unsent_num_len;
    size_t unsent_bytes;
} SendNumsBuf;

int make_sendbuf(size_t size, SendNumsBuf* dst);

int send_nums(int client_fd, struct SendNumsBuf* buf) {
    if (buf->unsent_bytes > 0) {
        while (true) {
            int bytes_sent = send(client_fd, buf->unsent_num + buf->u)
        }
    }

        int bytes_sent;
        while (bytes_sent) {
            int
        }
    }
}

#endif //BROWN_BOT_SENDNUMSBUF_H
