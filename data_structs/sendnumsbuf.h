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
#include "../config_read.h" // its bad...
#include "../socket_help.h"
#include "queue.h"


#ifndef BROWN_BOT_SENDNUMSBUF_H
#define BROWN_BOT_SENDNUMSBUF_H


typedef struct SendNumsBuf {
    Queue queue;

    char* unsent_num;
    size_t unsent_pos;
    size_t unsent_bytes;
} SendNumsBuf;

int make_sendbuf(size_t size, int max_sent_num_len, SendNumsBuf* dst);

int send_nums(int client_fd, SendNumsBuf* buf);

#endif //BROWN_BOT_SENDNUMSBUF_H
