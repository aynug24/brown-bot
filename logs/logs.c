#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include "logs.h"
#include "../config_read/config_read.h" // terrible but ok
#include "../socket_help.h"

// idk how logs are usually implemented)
// for the taks's purposes this will suffice

const char* LOGS_DIR = "logs/"; // ${BOT_DIR}${LOGS_DIR} = /tmp/brown-bot/logs/
const char* DEFAULT_LOG_PATH = "/dev/null"; // ))))

const char* INC_STRING = "INC_STRING";
const char* SND_ANSWR = "SND_ANSWR";
const char* RCV_ANSWR = "RCV_ANSWR";
const char* TTL_SLP = "TOTAL_SLP";
const char* FMT_FAIL = "FMT_FAIL";

const int MAX_LOG_REL_PATH_LEN = 256;
const int MAX_LOG_FULL_PATH_LEN = 256;

const int CLOCKS_PER_MS = CLOCKS_PER_SEC / 1000;

int _f_escape_newline(FILE* file, char* str, ssize_t len) {
    for (int i = 0; i != len; ++i) {
        if (str[i] != '\n') {
            if (fputc(str[i], file) == EOF) {
                return -1;
            }
        } else {
            if (fputs("\\n", file) == EOF) {
                return -1;
            }
        }
    }
    return 0;
}

int open_log(char* log_path, FILE** res) { // only one parameter in both scripts, so won't need to parse again
    //printf("Opening log %s\n", log_path == NULL ? "NULL" : log_path);
    char* full_log_path = malloc(MAX_LOG_FULL_PATH_LEN);
    if (full_log_path == NULL) {
        perror("Couldn't allocate memory for full log path");
        return -1;
    }

    if (log_path == NULL) {
        memcpy(full_log_path, DEFAULT_LOG_PATH, strlen(DEFAULT_LOG_PATH));
    } else {
        int copied = sprintf(full_log_path, "%s%s%s", BOT_DIR, LOGS_DIR, log_path);
        if (copied < 0) {
            perror("Error concating path");
            free(full_log_path);
            return -1;
        }
    }

    if (mkdirs(full_log_path) < 0) {
        fprintf(stderr, "Couldn't create directory for logs");
        free(full_log_path);
        return -1;
    }

    *res = fopen(full_log_path, "w");
    if (*res == NULL) {
        //printf("%s\n", full_log_path);
        perror("Couldn't open log file");
        free(full_log_path);
        return -1;
    }
    free(full_log_path);
    return 0;
}

int log_incoming_string(FILE* file, char* str, ssize_t len) {
    int print_res = fprintf(file, "%ld %s ", clock() / CLOCKS_PER_MS /*time(NULL)*/, INC_STRING);
    if (print_res < 0) {
        return print_res;
    }

    int escape_res = _f_escape_newline(file, str, len);
    if (escape_res < 0) {
        return escape_res;
    }

    if (fputc('\n', file) == EOF) {
        return -1;
    }
    return 0;
}

int log_sending_answer(FILE* file, long long answer) {
    return fprintf(file, "%ld %s %lld\n", clock() / CLOCKS_PER_MS /*time(NULL)*/, SND_ANSWR, answer);
}

int log_receiving_answer(FILE* file, long long answer) {
    return fprintf(file, "%ld %s %lld\n", clock() / CLOCKS_PER_MS /*time(NULL)*/, RCV_ANSWR, answer);
}

int log_total_sleep(FILE* file, long long total_sleep_ns) {
    long long total_sleep_ms = total_sleep_ns / 1000000;
    return fprintf(file, "%ld %s %lld\n", clock() / CLOCKS_PER_MS /*time(NULL)*/, TTL_SLP, total_sleep_ms);
}

int log_format_fail(FILE* file) {
    return fprintf(file, "%ld %s\n", clock() / CLOCKS_PER_MS /*time(NULL)*/, FMT_FAIL);
}