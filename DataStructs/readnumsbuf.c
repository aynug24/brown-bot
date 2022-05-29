//
// Created by sergei on 29.05.22.
//

#include "readnumsbuf.h"

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

ssize_t min(ssize_t x, ssize_t y) {
    return x < y ? x : y;
}

int parse_buf_num(ReadNumsBuf* buf, char* next_newline_pos, int max_num_len, long long* res) {
    const int next_num_len = next_newline_pos - buf->buf - buf->last_num_start;
    if (next_num_len > max_num_len) {
        fprintf(stderr, "Received line exceeding max length\n");
        return -1;
    } else if (next_num_len == 0) {
        fprintf(stderr, "Received empty line\n");
        return -1;
    }

    char next_num_str[max_num_len + 1];
    memcpy(next_num_str, next_newline_pos - next_num_len, next_num_len);
    next_num_str[next_num_len] = '\0';

    char* str_end = NULL;
    const long long next_num = strtol(next_num_str, &str_end, 10);
    if (*str_end != '\0') {
        fprintf(stderr, "Line format error: couldn't parse long long\n");
        return -1;
    }

    buf->last_num_start += next_num_len + 1; // if buf_pos then strchr gets ""

    *res = next_num;
    return 0;
}

// BUFFER_LEN > 2 * MAX_NUM_LEN
// QUEUE_LEN > 2 * BUFFER_LEN
// -2 if error, -1 if invalid data, 0 if eof, 1 if nothing to read yet, 2 if not enough queue space, 3 if ok
int recv_nums(int client_fd, ReadNumsBuf* buf, int max_num_len, ssize_t stop_recv, Queue* dest, int min_queue_freespace) {
    ssize_t total_recvd = 0;
    while (true) {
        size_t queue_free = get_free_count(dest);
        ssize_t bytes_to_read = buf->buf_len - buf->buf_pos;
        bytes_to_read = min(bytes_to_read, stop_recv - total_recvd);
        bytes_to_read = min(bytes_to_read, sizeof(long long) * queue_free);

        ssize_t recvd = recv(client_fd, buf->buf + buf->buf_pos, bytes_to_read, MSG_DONTWAIT);
        if (recvd < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK)) {
            perror("Error recieving data from client");
            return -2;
        }
        if (recvd < 0) {
            return 1;
        }
        if (recvd == 0) {
            if (try_get_last_num(buf, max_num_len, dest) < 0) {
                fprintf(stderr, "Couldn't parse last number");
                return -2;
            }
            return 0;
        }

        if (recvd >= 0) {
            buf->buf_pos += recvd;
        }
        buf->buf[buf->buf_pos] = '\0';

        char* next_newline_pos;
        while ((next_newline_pos = strchr(buf->buf + buf->last_num_start, '\n')) != NULL) {
            long long next_num;
            if (parse_buf_num(buf, next_newline_pos, max_num_len, &next_num) < 0) {
                fprintf(stderr, "Format error\n");
                return -1;
            }

            printf("Parsed %lld\n", next_num);
            if (!try_enqueue(dest, next_num)) {
                fprintf(stderr, "Number queue overflow\n");
                return -1;
            }
        }
        if (buf->buf_pos - buf->last_num_start > max_num_len) {
            fprintf(stderr, "Received line start exceeding line max length\n");
            return -1;
        }

        if (buf->buf_pos == buf->buf_len) {
            size_t last_num_part_len = buf->buf_len - buf->last_num_start;
            char* copy_from = buf->buf + buf->last_num_start;
            memcpy(buf->buf, copy_from, last_num_part_len * sizeof(*(buf->buf)));

            buf->buf_pos = last_num_part_len;
            buf->last_num_start = 0;
        }

        total_recvd += recvd;
        if (total_recvd >= stop_recv) {
            return 3;
        }
        if (get_free_count(dest) < min_queue_freespace) {
            return 2;
        }
    }
}

int try_get_last_num(ReadNumsBuf* buf, int max_num_len, Queue* dest) {
    if (buf->buf_pos - buf->last_num_start == 0) {
        return 0;
    }

    if (get_free_count(dest) == 0) {
        fprintf(stderr, "No space in queue for last number\n");
        return -1;
    }

    long long last_num;
    if (parse_buf_num(buf, buf->buf + buf->buf_pos, max_num_len, &last_num) < 0) {
        fprintf(stderr, "Error parsing last line");
        return -1;
    }

    printf("Parsed last %lld\n", last_num);
    try_enqueue(dest, last_num);
    return 1;
}