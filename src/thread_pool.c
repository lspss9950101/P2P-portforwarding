#include <thread_pool.h>

Task* popFrontTask(thread_pool* pool) {
    struct TaskList *popped = pool->task_list_front->next;
    if(popped->task == NULL) return NULL;
    
    popped->prev->next = popped->next;
    popped->next->prev = popped->prev;

    Task *result = popped->task;
    free(popped);

    return result;
}

void pushBackTask(thread_pool* pool, Task* task) {
    struct TaskList *new_entry = malloc(sizeof(struct TaskList));
    new_entry->task = task;
    new_entry->next = pool->task_list_back;
    new_entry->prev = pool->task_list_back->prev;
    pool->task_list_back->prev = new_entry;
    pool->task_list_back->prev->next = new_entry;

    sem_post(&pool->task_count);
}

void* _worker_func(void* args) {
    thread_pool *pool = args;
    while(true) {
        sem_wait(&pool->task_count);
        sem_wait(&pool->free_thread_count);
        sem_wait(&pool->mutex_task);
        Task *task = popFrontTask(pool);
        sem_post(&pool->mutex_task);
        if(task != NULL) {

            destroyTask(task);
        }
        sem_post(&pool->free_thread_count);
    }
}

bool isAvailable(thread_pool* pool) {
    int ret;
    sem_getvalue(&pool->free_thread_count, &ret);
    return ret > 0;
}

int createThreadPool(thread_pool* pool, int thread_number) {
    pool->pool_size = thread_number;
    pool->pool = malloc(sizeof(pthread_t) * thread_number);
    if(sem_init(&pool->free_thread_count, 0, thread_number)) return 1;
    if(sem_init(&pool->task_count, 0, 0)) return 1;
    if(sem_init(&pool->mutex_task, 0, 1)) return 1;

    for(int i = 0; i < thread_number; i++)
        pthread_create(pool->pool + i, NULL, _worker_func, pool);
    
    pool->task_list_front = malloc(sizeof(struct TaskList));
    pool->task_list_back = malloc(sizeof(struct TaskList));
    pool->task_list_front->prev = pool->task_list_back->next = NULL;
    pool->task_list_front->task = pool->task_list_back->task = NULL;
    pool->task_list_front->next = pool->task_list_back;
    pool->task_list_back->prev = pool->task_list_front;
    return 0;
}

int destroyThreadPool(thread_pool* pool) {
    struct TaskList *cur = pool->task_list_front;
    while(cur != NULL) {
        struct TaskList *tmp = cur->next;
        destroyTask(cur->task);
        free(cur);
        cur = tmp;
    }

    for(int i = 0; i < pool->pool_size; i++)
        pthread_cancel(pool->pool[i]);
    free(pool->pool);
    if(sem_destroy(&pool->free_thread_count)) return 1;
    if(sem_destroy(&pool->task_count)) return 1;
    if(sem_destroy(&pool->mutex_task)) return 1;

    return 0;
}

void destroyTask(Task* task) {
    free(task->buf);
    free(task);
}