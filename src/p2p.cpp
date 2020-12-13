#include <p2p.h>

void* service_worker_func(void *args) {
    ThreadPool *thread_pool = (ThreadPool *) args;
    MSG_TYPE msg_type;
    unsigned short msg_len;
    while(true) {
        sem_wait(&thread_pool->task_count);
        sem_wait(&thread_pool->free_thread_count);

        Task *task = thread_pool->popTask();
        if(task == NULL) {
            sem_post(&thread_pool->free_thread_count);
            continue;
        }

        msg_type = (MSG_TYPE)ntohs(*(unsigned short *)&task->buf[0]);
        msg_len = ntohs(*(unsigned short *)&task->buf[2]);
        switch(msg_type) {
            case MSG_TYPE::ACTIVE_BIND:
                printf("Bind request\n");
                break;

            default:
                break;
        }

        delete task;
        sem_post(&thread_pool->free_thread_count);
    }
}

// 0: no error
// -1: socket initialization failed
// 3: thread pool creation error
// 4: thread pool destruction error
int startCentralService(unsigned short port, int thread_number) {
    sockaddr_in local_addr, src_addr;
    memset(&local_addr, 0, sizeof(sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    int sockfd = initSocket(&local_addr, NULL, SOCK_DGRAM, 0);
    if(sockfd < 0) return -1;

    ThreadPool thread_pool(thread_number);
    if(thread_pool.init(service_worker_func) < 0) {
        fprintf(stderr, "Thread pool init error\n");
        return -2;
    }

    int addrlen = sizeof(sockaddr_in);
    int rv;
    memset(&src_addr, 0, sizeof(sockaddr_in));
    unsigned char buf[4096];
    MSG_TYPE msg_type;
    Task *new_task;
    while(true) {
        if((rv = recvfrom(sockfd, buf, sizeof(buf), 0, (sockaddr *)&src_addr, (socklen_t *)&addrlen)) < 0) {
            fprintf(stdlog2, "<Error>\tReceiving error\n");
            continue;
        }

        if(buf[1] == 0x01) {
            // Local request
            if(src_addr.sin_addr.s_addr != 0x0100007F) {
                fprintf(stdlog1, "<Error>\tLocal request from remote.\n");
                continue;
            }
            msg_type = (MSG_TYPE)ntohs(*(unsigned short*)&buf[0]);
            if(msg_type == MSG_TYPE::SHUT_DOWN) {
                // Shutdown request
                fprintf(stdout, "<Info>\tReceived shutdown request\nShutting down server\n");
                break;
            }
        }

        new_task = new Task(sockfd, buf, rv);
        thread_pool.pushTask(new_task);
    }
    return 0;
}

// 0 : no error
// -1: socket initialization error
// -2: msg type unkwown
// -3: sending failed
int sendImmediateCommand(MSG_TYPE msg_type, unsigned short port, void* args) {
    struct sockaddr_in target_addr;

    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = 0x0100007F;
    target_addr.sin_port = htons(port);

    int sockfd = initSocket(NULL, NULL, SOCK_DGRAM, 0);
    if(sockfd < 0) return -1;

    unsigned char packet[60];
    int packet_size;
    memset(packet, 0, sizeof(packet));
    *(unsigned short *)&packet[0] = htons((unsigned short)msg_type);

    switch(msg_type) {
        case MSG_TYPE::SHUT_DOWN:
            packet_size = 40;
            break;
        case MSG_TYPE::ACTIVE_BIND:{
            sockaddr_in *peer_addr = (sockaddr_in *)args;
            *(unsigned short *)&packet[16] = htons(0x0004);
            packet_size = 44;
            break;
        }
        default:
            close(sockfd);
            return -2;
    }

    if(sendto(sockfd, packet, packet_size, 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0) {
        fprintf(stderr, "<Error>\tSending failed\n");
        close(sockfd);
        return -3;
    }

    close(sockfd);
    return 0;
}
