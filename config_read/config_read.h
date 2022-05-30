//
// Created by sergei on 28.05.22.
//

#ifndef BROWN_BOT_CONFIG_READ_H
#define BROWN_BOT_CONFIG_READ_H

char* get_param(const char* param_name);
char* get_server_full_path();

extern const char* CONFIG_NAME;
extern const char* SOCKETS_DIR;
extern const char* CONFIG_SERVER_PATH_PARAM;
extern const int CONFIG_LINE_MAXLEN;
extern const int SERVER_BACKLOG;

#endif //BROWN_BOT_CONFIG_READ_H