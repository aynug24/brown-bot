//
// Created by sergei on 29.05.22.
//

#include "readnumsbuf.h"

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
// -2 if error, -1 if invalid data, 0 if ok
int recv_nums(int client_fd, ReadNumsBuf* buf, int max_num_len, long long* dest) {
    while (true) {
        ssize_t recvd = recv(client_fd, buf->buf + buf->buf_pos, buf->buf_len - buf->buf_pos, MSG_DONTWAIT);
        if (recvd < 0) {
            perror("Error recieving data from client");
            return -2;
        }
        if (recvd == 0) {
            return 0;
        }

        buf->buf_pos += recvd;
        buf->buf[buf->buf_pos] = '\0';

        char* next_newline_pos;
        while ((next_newline_pos = strchr(buf->buf + buf->last_num_start, '\n')) != NULL) {
            long long next_num;
            if (parse_buf_num(buf, next_newline_pos, &next_num) < 0) {
                fprintf(stderr, "Format error\n");
                return -1;
            }

            printf("%lld\n", next_num);
            *(dest++) = next_num;
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
    }
}

int try_get_last_num(ReadNumsBuf* buf, int max_num_len, long long* dst) {
    if (buf->buf_pos - buf->last_num_start == 0) {
        return 0;
    }

    long long last_num;
    if (parse_buf_num(buf, buf->buf + buf->buf_pos, &last_num) < 0) {
        fprintf(stderr, "Error parsing last line");
        return -1;
    }

    printf("%lld\n", last_num);
    *dst = last_num;
    return 1;
}