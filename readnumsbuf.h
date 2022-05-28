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

#ifndef BROWN_BOT_READNUMSBUF_H
#define BROWN_BOT_READNUMSBUF_H


typedef struct ReadNumsBuf {
    char* buf;
    size_t buf_len; // sizeof(buf) - 1

    ssize_t buf_pos;
    ssize_t last_num_start;
} ReadNumsBuf;

int make_readbuf(size_t size, ReadNumsBuf* dst) {
    char* buf = malloc(size * sizeof(char));
    if (buf == NULL) {
        perror("Couldn't allocate buf for ReadNumsBuf");
        return -1;
    }

    dst->buf = buf;
    dst->buf_len = size;
    dst->buf_pos = 0;
    dst->last_num_start = 0;
    return 0;
}

int parse_buf_num(ReadNumsBuf* buf, char* next_newline_pos, int max_num_len, long long* res);

// BUFFER_LEN > 2 * MAX_NUM_LEN
// -2 if error, -1 if invalid data, 0 if ok
int recv_nums(int client_fd, ReadNumsBuf* buf, int max_num_len, long long* dest);

// -1 if error, 0 if no num, 1 if got num
int try_get_last_num(ReadNumsBuf* buf, int max_num_len, long long* dst);

#endif //BROWN_BOT_READNUMSBUF_H
