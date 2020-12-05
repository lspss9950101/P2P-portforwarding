#include <p2p.h>
/*
int UDPHolePunching(ip_address* addr, unsigned short local_port, int timeout) {
    struct sockaddr_in serv_addr, local_addr, src_addr;
    int sockfd;

    if((sockfd = socket(addr->ip_family, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "<Error>\tSocket creation failed\n");
        return 1;
    }

    struct timeval timeout = {1, 0};
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "<Error>\tTimeout setting error.\n");
        close(sockfd);
        return 2;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = addr->ip_family;
    
    if(inet_pton(addr->ip_family, addr->addr, &serv_addr.sin_addr) < 0) {
        fprintf(stderr, "<Error>\tInvalid ip\n");
        close(sockfd);
        return 3;
    }
    serv_addr.sin_port = htons(addr->port);

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(local_port);

    if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        fprintf(stderr, "<Error>\tSocket binding error\n");
        close(sockfd);
        return 4;
    }

    unsigned char req[4] = {0x01, 0x01, 0x01, 0x01};
    unsigned char ack[4] = {0x02, 0x02, 0x02, 0x02};

    unsigned char recv_buf[4];
    int phase = 0;
    bool punched = false;
    struct timeval begin, now;
    socklen_t addr_len = sizeof(src_addr);
    gettimeofday(&begin, NULL);
    while(true) {
        if(phase == 0) {
            if(sendto(sockfd, req, sizeof(req), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                fprintf(stderr, "<Error>\tSending failed\n");
                close(sockfd);
                return 5;
            }
        
            memset(&src_addr, 0, sizeof(src_addr));
            if(recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, &src_addr, &addr_len) == 0) {
                if(src_addr.sin_addr.s_addr == serv_addr.sin_addr.s_addr && src_addr.sin_port == serv_addr.sin_port) {
                    // Got req

                }
            }   
        }

        if(recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, NULL, NULL) == 0 && ((recv_buf[0] | recv_buf[1] | recv_buf[2] | recv_buf[3]) & 0x03)) {
            // Got req
        }

        if(punched) break;
        gettimeofday(&now, NULL);
        if(timeout > 0 && now.tv_sec - begin.tv_sec > timeout) break;
    }

    close(sockfd);

    return 1 - punched;
}*/