#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <queue>

class Task {
    public:
        int sockfd;
        int size;
        char *buf;
        Task(int sockfd, unsigned char *buf, int size);
        ~Task();
};

class ThreadPool {
    private:
        pthread_t *pool;
        int size;
        std::queue<Task*> task_queue;
    public:
        sem_t free_thread_count, task_count, mutex_task;

        ThreadPool(int size);
        ~ThreadPool();
        int init(void*(*worker_func)(void*));
        bool isAvailable();
        void pushTask(Task *new_task);
        Task* popTask();
};

#endif