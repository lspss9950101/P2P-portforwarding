#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <types.h>

typedef enum {
    BINDING_REQUEST,
    BONDING_ECHO,
} TaskType;

typedef struct {
    char *buf;
    ip_address client_ip;
} Task;

struct TaskList {
    Task *task;
    TaskList *next, *prev;
}

typedef struct {
    pthread_t *pool;
    int pool_size;
    sem_t free_thread_count, task_count, mutex_task;
    Task *task_list_front, *task_list_back;
} thread_pool;

void* _worker_func(void*);

int createThreadPool(thread_pool*, int, void*(void*));

bool isAvailable(thread_pool*);

int destroyThreadPool(thread_pool*);

void destroyTask(Task*);

void pushBackTask(thread_pool*, Task*);

Task* popFrontTask(thread_pool*);