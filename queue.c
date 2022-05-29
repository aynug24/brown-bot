//
// Created by sergei on 29.05.22.
//

#include "queue.h"

int make_queue(size_t capacity, Queue* dst) {
    long long* nums = malloc(capacity * sizeof(long long));
    if (nums == NULL) {
        perror("Couldn't allocate queue buffer");
        return -1;
    }
    dst->nums = nums;
    dst->capacity = capacity;
    dst->rear = 0;
    dst->front = 0;
    dst->is_empty = true;
    return 0;
}

bool try_enqueue(Queue* queue, long long n) {
    if ((queue->rear == queue->front) && !queue->is_empty) {
        return false;
    }

    queue->nums[queue->rear] = n;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->is_empty = false;
    return true;
}

bool try_dequeue(Queue* queue, long long* dst) {
    if (queue->is_empty) {
        return false;
    }

    *dst = queue->nums[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->is_empty = (queue->front == queue->rear);
    return true;
}
