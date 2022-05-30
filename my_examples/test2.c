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
#include "../config_read/config_read.h" // its bad...
#include "../socket_help.h"

const int BUFFER_LEN = 100;
const int MAX_NUM_LEN = 10;

typedef struct ReadNumsBuf {
    char* buf;
    size_t buf_len; // sizeof(buf) - 1

    ssize_t buf_pos;
    ssize_t last_num_start;
} ReadNumsBuf;

long long sum = 0LL;

int parse_buf_num(ReadNumsBuf* buf, char* next_newline_pos, long long* res) {
    const int next_num_len = next_newline_pos - buf->buf - buf->last_num_start;
    if (next_num_len > MAX_NUM_LEN) {
        fprintf(stderr, "Received line exceeding max length\n");
        return -1;
    } else if (next_num_len == 0) {
        fprintf(stderr, "Received empty line\n");
        return -1;
    }

    char next_num_str[MAX_NUM_LEN + 1];
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
int recv_nums(int client_fd, ReadNumsBuf* buf) {
    while (true) {
        ssize_t recvd = read(client_fd, buf->buf + buf->buf_pos, buf->buf_len - buf->buf_pos);
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
            long long next_int;
            if (parse_buf_num(buf, next_newline_pos, &next_int) < 0) {
                fprintf(stderr, "Format error\n");
                return -1;
            }

            printf("%lld\n", next_int);
            sum += next_int;
        }
        if (buf->buf_pos - buf->last_num_start > MAX_NUM_LEN) {
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

int main() {
    int fd = open("test_zerosum.txt", O_RDONLY);

    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    char buff[BUFFER_LEN + 1];
    buff[0] = '\0';
    ReadNumsBuf buf = { buff, 100, 0, 0 };

    bool ok = true;
    if (recv_nums(fd, &buf) < 0) {
        fprintf(stderr, "Error receiving numbers\n");
        ok = false;
    }
    if (ok && buf.buf_pos - buf.last_num_start > 0) {
        long long last_num;
        if (parse_buf_num(&buf, buf.buf + buf.buf_pos, &last_num) < 0) {
            fprintf(stderr, "Error parsing last line\n");
            bool ok = false;
        }

        printf("%lld\n", last_num);
        sum += last_num;
    }

    if (ok) printf("Sum = %lld", sum);
}