#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <types.h>

sem_t free_thread, mutex_sock;

typedef enum {
    BINDING_REQUEST,
    BONDING_ECHO,
} TaskType;

typedef struct {
    pthread_t **pool;
    int pool_size;
} thread_pool;

typedef struct {
    TaskType type;
    union {
        struct {
            ip_address client_ip;
            float timeout;
            int retry_limit;
        } binding_request_attr;
        struct {
            ip_address client_ip;
        } binding_echo_attr;
    };
} Task;

void createThreadPool(thread_pool*, int);

void destroyThreadPool(thread_pool*);

int pushTask(thread_pool*, Task);