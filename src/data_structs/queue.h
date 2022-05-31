
#ifndef BROWN_BOT_QUEUE_H
#define BROWN_BOT_QUEUE_H

#include <stddef.h>
#include <sys/types.h>
#include <stdbool.h>

typedef struct Queue {
    long long* nums;
    size_t capacity;

    ssize_t front; // откуда берем
    ssize_t rear; // куда добавляем
    bool is_empty;
} Queue;

int make_queue(size_t capacity, Queue* dst);

size_t get_free_count(Queue* queue);

bool try_enqueue(Queue* queue, long long n);

bool try_dequeue(Queue* queue, long long* dst);

bool is_full(Queue* queue);

#endif //BROWN_BOT_QUEUE_H
