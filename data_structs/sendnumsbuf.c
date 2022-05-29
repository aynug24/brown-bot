//
// Created by sergei on 29.05.22.
//

#include "sendnumsbuf.h"

int make_sendbuf(size_t size, int max_sent_num_len, SendNumsBuf* dst) {
    Queue queue;
    if (make_queue(size, &queue) < 0) {
        fprintf(stderr, "Error creating queue\n");
        return -1;
    }

    char* last_num = malloc((max_sent_num_len + 1) * sizeof(long long));
    if (last_num == NULL) {
        perror("Couldn't create buffer for sending number");
        return -1;
    }

    dst->queue = queue;
    dst->unsent_num = last_num;
    dst->unsent_bytes = 0;
    dst->unsent_pos = 0;
    return 0;
}

/*
 * -1 if err, 0 if client closed, 1 if can't send yet, 2 if sent everything
 */
int send_nums(int client_fd, SendNumsBuf* buf) {
    while (!buf->queue.is_empty || buf->unsent_bytes > 0) {
        while (buf->unsent_bytes > 0) {
            ssize_t sent = send(client_fd, buf->unsent_num + buf->unsent_pos, buf->unsent_bytes, MSG_DONTWAIT | MSG_NOSIGNAL); // ?
            if (sent < 0 && errno == EPIPE) {
                return 0;
            }
            if (sent < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK)) {
                perror("Error sending number");
                return -1;
            }
            if (sent < 0) {
                return 1;
            }
            if (sent == 0) {
                return 0; // can this be?
            }
            buf->unsent_bytes -= sent;
            buf->unsent_pos += sent;
        }

        long long next_num;
        if (try_dequeue(&buf->queue, &next_num)) {
            int num_len = sprintf(buf->unsent_num, "%lld", next_num);
            if (num_len < 0) {
                perror("Couldn't fomat numer");
                return -1;
            }
            buf->unsent_num[num_len] = '\n';
            buf->unsent_num[num_len + 1] = '\0';

            buf->unsent_bytes = num_len + 1;
            buf->unsent_pos = 0;
        }
    }
    return 2;
}