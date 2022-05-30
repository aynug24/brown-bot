//
// Created by sergei on 28.05.22.
//
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "config_read.h"

const char* CONFIG_NAME = "config";
const char* BOT_DIR = "/tmp/brown-bot/";
const char* CONFIG_SERVER_PATH_PARAM = "server_path";
const int CONFIG_LINE_MAXLEN = 255;
const int SERVER_BACKLOG = 10;

char* get_param(const char* param_name) {
    FILE* config = fopen(CONFIG_NAME, "r");
    if (config == NULL) {
        perror("Couldn't open config");
        exit(-1);
    }

    char* config_line = malloc(CONFIG_LINE_MAXLEN * sizeof(*config_line));
    if (config_line == NULL) {
        perror("Couldn't create config line buffer");
        exit(-1);
    }

    while (true) {
        memset(config_line, 0, CONFIG_LINE_MAXLEN * sizeof(config_line[0]));
        if (fgets(config_line, CONFIG_LINE_MAXLEN, config) == NULL) {
            if (!feof(config)) {
                perror("Error reading config");
                exit(-1);
            }
            free(config_line);
            return NULL;
        }
        //printf("%s\n", config_line);

        //char* param_value = strtok(config_line, "=");
        char* param_name_end = strchr(config_line, '=');
        if (param_name_end == NULL) {
            fprintf(stderr, "Config line doesnt contain =: %s\n", config_line);
            exit(-1);
        }
        *param_name_end = '\0';
        const char* param_value = param_name_end + 1;

        //printf("%s %s\n", config_line, param_value);
        const long param_name_len = param_value - config_line - 1;
        if (param_name_len != strlen(param_name)) {
            continue;
        }

        if (strncmp(param_name, config_line, param_name_len) != 0) {
            continue;
        }

        if (fclose(config) == EOF) {
            perror("Couldn't close config");
            exit(-1);
        }

        const ulong param_value_len = strlen(param_value);
        char* param_value_str = malloc(param_value_len * sizeof(*param_value_str));
        if (param_value_str == NULL) {
            perror("Couldn't allocate result memory");
            exit(-1);
        }
        strcpy(param_value_str, param_value);

        free(config_line);
        return param_value_str;
    }
}

char* get_server_full_path() {
    bool no_err = true;

    const char* server_path_relative = get_param(CONFIG_SERVER_PATH_PARAM);
    if (server_path_relative == NULL) {
        fprintf(stderr, "No server_path in config\n");
        no_err = false;
    }

    char* server_path = NULL;
    if (no_err) {
        server_path = malloc(strlen(BOT_DIR) + strlen(server_path_relative) + 1);
        if (server_path == NULL) {
            perror("Couldn't allocate server path");
            no_err = false;
        } else if (sprintf(server_path, "%s%s", BOT_DIR, server_path_relative) < 0) {
            perror("Couldn't concat server path");
            no_err = false;
        }
    }

    free((char*)server_path_relative);
    return server_path;
}