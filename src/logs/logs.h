#ifndef BROWN_BOT_LOGS_H
#define BROWN_BOT_LOGS_H

const char* INC_STRING;
const char* SND_ANSWR;
const char* RCV_ANSWR;
const char* TTL_SLP;
const char* FMT_FAIL;

extern const int MAX_LOG_REL_PATH_LEN;
extern const int MAX_LOG_FULL_PATH_LEN;

int open_log(char* log_path, FILE** res);

int log_incoming_connection(FILE* file, int fd);
int log_incoming_string(FILE* file, char* str, ssize_t len);
int log_sending_answer(FILE* file, long long answer);
int log_receiving_answer(FILE* file, long long answer);
int log_total_sleep(FILE* file, long long total_sleep_ns);
int log_format_fail(FILE* file);

#endif //BROWN_BOT_LOGS_H
