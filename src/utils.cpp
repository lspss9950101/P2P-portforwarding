#include <utils.h>

int initSocket(sockaddr_in* addr, timeval* timeout, int socktype, int protocol) {
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "<Error>\tSocket creation failed\n");
        return -1;
    }

    if(addr) {
        if(bind(sockfd, (sockaddr *)addr, sizeof(sockaddr_in)) < 0) {
            fprintf(stderr, "<Error>\tSocket binding error\n");
            close(sockfd);
            return -2;
        }
    }

    if(timeout) {
        if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)timeout, sizeof(timeval)) < 0) {
            fprintf(stderr, "<Error>\tsetsockopt error\n");
            close(sockfd);
            return -3;
        }
    }

    return sockfd;
}

// ret
// 0: No error
// 1: getaddrinfo error
// query_type
// 1: ipv4
// 2: ipv6
// 3: both
// 4: return first
int getIPFromHost(std::string host, int query_type, sockaddr_in *addr, int *count) {
    struct addrinfo *addr_info, *res;
    int rv;
    int len = 0;

    if((rv = getaddrinfo(host.c_str(), NULL, NULL, &addr_info)) != 0) {
        fprintf(stderr, "<Error>\t%s\n", gai_strerror(rv));
        return 1;
    }

    for(res = addr_info; res != NULL; res = res->ai_next) {
        if((query_type & 0x01) && res->ai_family == AF_INET) {
            memset(addr + len, 0, sizeof(sockaddr));
            addr[len].sin_addr.s_addr = ((sockaddr_in*)(res->ai_addr))->sin_addr.s_addr;
            addr[len].sin_family = AF_INET;
            len++;
        } else if((query_type & 0x02) && res->ai_family == AF_INET6) {
            memset(addr + len, 0, sizeof(sockaddr_in));
            memcpy(&((sockaddr_in6 *)&addr[len])->sin6_addr.__in6_u.__u6_addr8, &((sockaddr_in6*)(res->ai_addr))->sin6_addr.__in6_u.__u6_addr8, 16);
            addr[len].sin_family = AF_INET6;
            len++;
        }
        if(len != 0 && (query_type & 0x04)) break;
    }
    freeaddrinfo(addr_info);
    if(count != NULL) *count = len;
    return 0;
}

bool isSameAsLinkIP(sockaddr_in *addr) {
    struct ifaddrs *ifaddr;
    if(getifaddrs(&ifaddr) == -1) {
        return false;
    }

    bool result = false;
    int family;
    for(struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;
        struct sockaddr_in *ifa_addr = (struct sockaddr_in *)ifa->ifa_addr;
        if((family == addr->sin_family) && (addr->sin_addr.s_addr == ifa_addr->sin_addr.s_addr)) {
            result = true;
            break;
        }
    }
    freeifaddrs(ifaddr);
    return result;
}

// ret
// 0 : port ok
// -1: port in use
int checkPort(unsigned short port) {
    struct sockaddr_in local_addr;
    
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);

    int sockfd = initSocket(&local_addr, NULL, SOCK_DGRAM, 0);
    if(sockfd < 0) return -1;

    close(sockfd);
    return 0;
}

void dumpBuffer(unsigned char* buf, short len) {
    for(int i = 0, col_count = 0; i < len; i++) {
        printf("%02X ", buf[i]);
        if(col_count == 3) printf("\n");
        col_count = (col_count+1) % 4;
    }
    printf("\n");
}