#include <p2p.h>

// 0: no error
// 1: socket creation failed
// 2: socket binding error
// 3: thread pool creation error
// 4: thread pool destruction error
int startCentralService(unsigned short port) {
    thread_pool pool;
    if(createThreadPool(&pool, 8)) return 3;

    struct sockaddr_in local_addr, src_addr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "<Error>\tSocket creation failed\n");
        return 1;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        fprintf(stderr, "<Error>\tSocket binding error\n");
        close(sockfd);
        return 2;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    socklen_t addr_len = sizeof(src_addr);

    char buf[1024];
    unsigned short msg_type, msg_length;
    int rv;
    Task *new_task;
    while(true) {
        if((rv = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&src_addr, &addr_len)) < 0) {
            fprintf(stdlog2, "<Info>\tReceiver error. RET: %d\n", rv);
        }

        msg_type = ntohs(*(unsigned short*)&buf[12]);
        if(msg_type == 0x0000) continue;
        else if(msg_type & 0x0100) {
            if(src_addr.sin_addr.s_addr != 0x0100007F) continue;
            if(msg_type == MSG_SHUT_DOWN) break;
        }
        msg_length = ntohs(*(unsigned short *)&buf[14]) + 20;

        new_task = malloc(sizeof(Task));
        new_task->sockfd = sockfd;
        new_task->client_ip.ip = src_addr.sin_addr.s_addr;
        new_task->client_ip.port = src_addr.sin_port;
        new_task->buf = malloc(msg_length);
        memcpy(new_task->buf, buf, msg_length);
        pushBackTask(&pool, new_task);
    }

    fprintf(stdlog1, "Shutting down server...\n");
    if(destroyThreadPool(&pool)) return 4;
    return 0;
}

// 0: no error
// 1: socket creation error
// 2: msg type unkwown
// 3: sending failed
int sendImmediateCommand(MSG_TYPE msg_type, unsigned short port, void* args) {
    struct sockaddr_in target_addr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "<Error>\tSocket creation failed\n");
        return 1;
    }

    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = 0x0100007F;
    target_addr.sin_port = htons(port);

    unsigned char packet[128];
    int packet_size;
    memset(packet, 0, sizeof(packet));
    *(unsigned short *)&packet[12] = htons(msg_type);

    switch(msg_type) {
        case MSG_SHUT_DOWN:
            packet_size = 20;
            break;
        case MSG_ACTIVE_BIND:{
            ip_address *peer_ip = args;
            *(unsigned short *)&packet[14] = htons(0x0004);
            inet_pton(AF_INET, peer_ip->addr, (unsigned int *)&packet[16]);
            packet_size = 24;
            break;
        }
        default:
            close(sockfd);
            return 2;
    }

    if(sendto(sockfd, packet, packet_size, 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0) {
        fprintf(stderr, "<Error>\tSending failed\n");
        close(sockfd);
        return 3;
    }

    close(sockfd);
    return 0;
}