#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../config_read/config_read.h"
#include "../socket_help.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

const int CLIENT_PATH_MAXLEN = 1024; // todo разнести константы туда где должны быть (или избавиться)
const int CLIENT_WAIT_TIME_MAXLEN = 10;
const int DEFAULT_WAIT_TIME_MS = 200;

const int MAX_BATCH_SIZE = 3;

bool try_get_wait_arg(int argc, char* argv[], char** dest) {
    opterr = 0;
    int argname;
    while ((argname = getopt(argc, argv, "w:")) != -1) {
        switch (argname) {
            case 'w':
                strcpy(*dest, optarg);
                return true;
            case '?':
                if (optopt == 'w')
                    fprintf(stderr, "Option -%c requires an argument.\n", (char)optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", (char)optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return false;
            default:
                fprintf(stderr, "Unknown getopt return");
                return false;
        }
    }

    *dest = NULL;
    return true;
}

long get_wait_time(int argc, char* argv[]) {
    char* wait_time_str = malloc(CLIENT_WAIT_TIME_MAXLEN * sizeof(*wait_time_str));
    if (wait_time_str == NULL) {
        perror("Couldn't allocate memory for wait time");
    }

    if (!try_get_wait_arg(argc, argv, &wait_time_str)) {
        fprintf(stderr, "Error while parsing arguments\n");
    }
    if (wait_time_str == NULL) {
        return DEFAULT_WAIT_TIME_MS;
    }

    char* arg_end = NULL;
    const long wait_time = strtol(wait_time_str, &arg_end, 10);
    if (*wait_time_str == '\0' || *arg_end != '\0') {
        fprintf(stderr, "Invalid wait time argument format: \"%s\"\n", wait_time_str);
        return -1;
    }
    if (wait_time < 0) {
        fprintf(stderr, "Argument value error: negative wait time %ld", wait_time);
        return -1;
    }

    return wait_time;
}

int rand_range(int min, int max) { // not really uniform buuut
    int r = rand();
    return min + (r % (max - min + 1));
}

void print_escaped(const char* src, ssize_t n) {
    for (int i = 0; i < n; ++i) {
        char c = *(src+i);
        switch (*(src + i)) {
            case '\n':
                printf("\\n");
                break;
            default:
                printf("%c", *(src + i));
                break;
        }
    }
    //fflush(stdout);
}

int msleep(long ms)
{
    struct timespec ts;
    int res;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    if (res < 0 && errno != EINTR) {
        perror("Error sleeping");
    }
    return res;
}

int main(int argc, char* argv[]) {
    bool ok = true;
    long wait = get_wait_time(argc, argv);
    if (wait < 0) {
        fprintf(stderr, "Error getting wait time");
        ok = false;
    }



    char* buf;
    if (ok) {
        srand(time(NULL));
        buf = malloc(MAX_BATCH_SIZE * sizeof(*buf));
        if (buf == NULL) {
            perror("Couldn't allocate buf for input");
            ok = false;
        }
    }

    if (ok) {
        while (true) {
            int bytes_to_read = rand_range(1, MAX_BATCH_SIZE);
            ssize_t bytes_read = read(STDIN_FILENO, buf, bytes_to_read);
            if (bytes_read == -1) {
                perror("Error reading input");
                ok = false;
                break;
            }
            if (bytes_read == 0) { // its eof if using read
                break;
            }

            print_escaped(buf, bytes_read);
            printf("\n");

            if (msleep(wait) < 0) {
                fprintf(stderr, "Couldn't sleep after input");
                ok = false;
                break;
            }
        }
    }

    printf("I ENDED");

    return 0;
}
