#include <p2p.h>

// 0 : no error
// -1: sending error
int sendP2PPacket(int sockfd, sockaddr_in *target_addr, MSG_TYPE msg_type, UUID source_uuid, UUID target_uuid, void *args) {
    unsigned char packet[1024];
    unsigned short packet_size;
    *(unsigned short *)&packet[0] = htons((unsigned short)msg_type);
    source_uuid.toBytes(&packet[4]);
    target_uuid.toBytes(&packet[20]);
    
    switch(msg_type) {
        case MSG_TYPE::SHUT_DOWN:
            packet_size = 36;
            *(unsigned short *)&packet[2] = ntohs(0x0000);
            break;
        case MSG_TYPE::ACTIVE_BIND:
        case MSG_TYPE::ECHO:
            packet_size = 42;
            *(unsigned short *)&packet[2] = ntohs(0x0006);
            *(unsigned int *)&packet[36] = ((sockaddr_in *)args)->sin_addr.s_addr;
            *(unsigned short *)&packet[40] = ((sockaddr_in *)args)->sin_port;
            break;
    }

    if(sendto(sockfd, packet, packet_size, 0, (sockaddr *)target_addr, sizeof(sockaddr)) < 0) {
        fprintf(stderr, "<Error>\tSending error\n");
        return -1;
    }
    return 0;
}


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
            case MSG_TYPE::ACTIVE_BIND:{
                fprintf(stdlog1, "<Info>Active bind request\n");
                
                break;
            }
            case MSG_TYPE::ECHO:

            default:
                break;
        }

        delete task;
        sem_post(&thread_pool->free_thread_count);
    }
}

// 0 : no error
// -1: socket initialization failed
// -2: thread pool creation error
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

    sendP2PPacket(sockfd, &target_addr, msg_type, UUID::zero(), UUID::zero(), args);

    close(sockfd);
    return 0;
}
