//
// Created by sergei on 29.05.22.
//

#ifndef BROWN_BOT_CLIENT_HELP_H
#define BROWN_BOT_CLIENT_HELP_H

extern const int CLIENT_PATH_MAXLEN; // todo разнести константы туда где должны быть (или избавиться)
extern const int CLIENT_WAIT_TIME_MAXLEN;
extern const int DEFAULT_WAIT_TIME_MS;

extern const int MAX_BATCH_SIZE;
extern const int RECV_BUF_SIZE;
extern const int RES_QUEUE_SIZE;


long get_wait_time(int argc, char* argv[]);
int rand_range(int min, int max);
int msleep(long ms);
int get_connected_client_sock(int argc, char* argv[], char** server_full_path, char** client_full_path, long* wait_ms);

#endif //BROWN_BOT_CLIENT_HELP_H
