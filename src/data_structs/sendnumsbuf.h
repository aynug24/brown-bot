#ifndef BROWN_BOT_SENDNUMSBUF_H
#define BROWN_BOT_SENDNUMSBUF_H

#include <bits/types/FILE.h>
#include "queue.h"

typedef struct SendNumsBuf {
    Queue queue;

    char* unsent_num;
    long long unsent_ll;
    size_t unsent_pos;
    size_t unsent_bytes;
} SendNumsBuf;

int make_sendbuf(size_t size, int max_sent_num_len, SendNumsBuf* dst);

int send_nums(int client_fd, SendNumsBuf* buf, FILE* log_file);

#endif //BROWN_BOT_SENDNUMSBUF_H
