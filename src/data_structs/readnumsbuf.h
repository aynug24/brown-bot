#ifndef BROWN_BOT_READNUMSBUF_H
#define BROWN_BOT_READNUMSBUF_H

#include <bits/types/FILE.h>
#include "queue.h"

typedef struct ReadNumsBuf {
    char* buf;
    size_t buf_len; // sizeof(buf) - 1

    ssize_t buf_pos;
    ssize_t last_num_start;
} ReadNumsBuf;

int make_readbuf(size_t size, ReadNumsBuf* dst);
int parse_buf_num(ReadNumsBuf* buf, char* next_newline_pos, int max_num_len, long long* res);

// BUFFER_LEN > 2 * MAX_NUM_LEN
// -2 if error, -1 if invalid data, 0 if ok
int recv_nums(int client_fd, ReadNumsBuf* buf, int max_num_len, ssize_t stop_recv, Queue* dest, int min_queue_freespace, FILE* log_file);

// -1 if error, 0 if no num, 1 if got num
int try_get_last_num(ReadNumsBuf* buf, int max_num_len, Queue* dest);

#endif //BROWN_BOT_READNUMSBUF_H
