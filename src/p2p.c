#include <p2p.h>

int startP2PServer(unsigned short port) {
    struct sockaddr_in local_addr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return 1;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);

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
}