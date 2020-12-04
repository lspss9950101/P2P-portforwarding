#include <stun.h>

// ret
// 0: No error
// 1: socket creation error
// 2: timeout setting error
int sendSTUNPacket(ip_address* addr, bool change_addr, bool change_port, unsigned short local_port) {
    sockaddr_in serv_addr, local_addr;
    int sockfd;

    if((sockfd = socket(addr->ip_ver == IPV4 ? AF_INET : AF_INET6,
        SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "<Error>\tSocket creation failed\n");
        return 1;
    }

    timeval timeout = {3, 0};
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) != 0) {
        fprintf(stderr, "<Error>\tTimeout setting error.\n");
        close(sockfd);
        return 2;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = addr.ip_family;
    if(inet_pton(addr_ip_family, addr.addr, ))
}