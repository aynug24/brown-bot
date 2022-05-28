//
// Created by sergei on 29.05.22.
//

#include <stdio.h>
#include <time.h>
#include "logs.h"

const char* INC_STRING = "INC_STRING";
const char* SND_ANSWR = "SND_ANSWR";
const char* RCV_ANSWR = "RCV_ANSWR";
const char* TTL_SLP = "TOTAL_SLP";

int f_escape_newline(FILE* file, char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] != '\n') {
            if (fputc(str[i], file) == EOF) {
                return -1;
            }
        } else {
            if (fputs("\n", file) == EOF) {
                return -1;
            }
        }
    }
    return 0;
}

int log_incoming_string(FILE* file, char* str) {
    int print_res = fprintf(file, "%ld %s", time(NULL), INC_STRING);
    if (print_res < 0) {
        return print_res;
    }

    int escape_res = f_escape_newline(file, str);
    if (escape_res < 0) {
        return escape_res;
    }

    if (fputc('\n', file) == EOF) {
        return -1;
    }
    return 0;
}

int log_sending_answer(FILE* file, long long answer) {
    return fprintf(file, "%ld %s %lld\n", time(NULL), SND_ANSWR, answer);
}

int log_receiving_answer(FILE* file, long long answer) {
    return fprintf(file, "%ld %s %lld\n", time(NULL), RCV_ANSWR, answer);
}

int log_total_sleep(FILE* file, int total_sleep) {
    return fprintf(file, "%ld %s %d\n", time(NULL), TTL_SLP, total_sleep);
}