//
// Created by sergei on 29.05.22.
//

#ifndef BROWN_BOT_LOGS_H
#define BROWN_BOT_LOGS_H

const char* INC_STRING;
const char* SND_ANSWR;
const char* RCV_ANSWR;
const char* TTL_SLP;
const char* FMT_FAIL;

int log_incoming_string(FILE* file, char* str);
int log_sending_answer(FILE* file, long long answer);
int log_receiving_answer(FILE* file, long long answer);
int log_total_sleep(FILE* file, int total_sleep);
int log_format_fail(FILE* file);

#endif //BROWN_BOT_LOGS_H
