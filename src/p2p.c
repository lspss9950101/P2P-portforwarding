#include <p2p.h>

// 0: no error
// 1: socket creation failed
// 2: socket binding error
int startCentralService(unsigned short port) {
    struct sockaddr_in local_addr, src_addr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "<Error>\tSocket creation failed\n");
        return 1;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(local_port);

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
}