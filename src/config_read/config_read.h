#ifndef BROWN_BOT_CONFIG_READ_H
#define BROWN_BOT_CONFIG_READ_H

char* get_param(const char* param_name);
char* get_server_full_path();

extern const char* CONFIG_NAME;
extern const char* BOT_DIR;
extern const char* CONFIG_SERVER_PATH_PARAM;
extern const int CONFIG_LINE_MAXLEN;
extern const int SERVER_BACKLOG;

#endif //BROWN_BOT_CONFIG_READ_H
