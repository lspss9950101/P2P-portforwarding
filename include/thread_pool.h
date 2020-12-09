#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <types.h>
#include <p2p.h>

typedef struct {
    int sockfd;
    char *buf;
    ip_address client_ip;
} Task;

struct TaskList {
    Task *task;
    struct TaskList *next, *prev;
};

typedef struct {
    pthread_t *pool;
    int pool_size;
    sem_t free_thread_count, task_count, mutex_task;
    struct TaskList *task_list_front, *task_list_back;
} thread_pool;

void* _worker_func(void*);

int createThreadPool(thread_pool*, int);

bool isAvailable(thread_pool*);

int destroyThreadPool(thread_pool*);

void destroyTask(Task*);

void pushBackTask(thread_pool*, Task*);

Task* popFrontTask(thread_pool*);

#endif