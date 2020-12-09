#include <p2p.h>

<<<<<<< HEAD
// 0: no error
// 1: socket creation failed
// 2: socket binding error
int startCentralService(unsigned short port) {
    struct sockaddr_in local_addr, src_addr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "<Error>\tSocket creation failed\n");
=======
int startP2PServer(unsigned short port) {
    struct sockaddr_in local_addr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
>>>>>>> b47df0b036b97b646dda795573191f4078206bdb
        return 1;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);

<<<<<<< HEAD
    if(bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        fprintf(stderr, "<Error>\tSocket binding error\n");
        close(sockfd);
        return 2;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    socklen_t addr_len = sizeof(src_addr);

    char buf[1024];
    int rv;
    while(true) {
        if((rv = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&src_addr, &addr_len)) < 0) {
            fprintf(stdlog2, "<Info>\tReceiver error. RET: %d\n", rv);
        }
    }
=======
    if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        fprintf(stdlog1, "<Error>\tCannot bind socket\n");
        return 2;
    }

    int rv;
    char recv_buf[1024];
    while(true) {
        if(recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, NULL, NULL) < 0) {
            fprintf(stdlog2, "<Info>\tReceiving error. RET: %d\n", rv);
        }
        
    }

    close(sockfd);
    return 0;
>>>>>>> b47df0b036b97b646dda795573191f4078206bdb
}