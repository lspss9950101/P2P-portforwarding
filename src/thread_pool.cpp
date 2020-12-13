#include <thread_pool.h>

Task::Task(int sockfd, unsigned char *buf, int size) {
    this->buf = new char[size];
    this->size = size;
    this->sockfd = sockfd;
    memcpy(this->buf, buf, size);
}

Task::~Task() {
    delete [] this->buf;
}

ThreadPool::ThreadPool(int size) {
    this->size = size;
    this->pool = new pthread_t[size];
}

ThreadPool::~ThreadPool() {
    for(int i = 0; i < this->size; i++)
        pthread_cancel(this->pool[i]);
    delete [] this->pool;
}

int ThreadPool::init(void*(*worker_func)(void*)) {
    if(sem_init(&this->free_thread_count, 0, this->size)) return -1;
    if(sem_init(&this->task_count, 0, 0)) return -1;
    if(sem_init(&this->mutex_task, 0, 1)) return -1;
    for(int i = 0; i < size; i++)
        pthread_create(this->pool + i, NULL, worker_func, (void*)this);
    return 0;
}

bool ThreadPool::isAvailable() {
    int ret;
    sem_getvalue(&this->free_thread_count, &ret);
    return ret > 0;
}

void ThreadPool::pushTask(Task *new_task) {
    sem_wait(&this->mutex_task);
    this->task_queue.push(new_task);
    sem_post(&this->mutex_task);
    sem_post(&this->task_count);
}

Task* ThreadPool::popTask() {
    Task *ret = NULL;
    sem_wait(&this->mutex_task);
    if(this->task_queue.size()) {
        ret = this->task_queue.front();
        this->task_queue.pop();
    }
    sem_post(&this->mutex_task);
    return ret;
}