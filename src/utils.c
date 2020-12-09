#include <utils.h>

// ret
// 0: No error
// 1: getaddrinfo error
// query_type
// 1: ipv4
// 2: ipv6
// 3: both
// 4: return first
int getIPFromHost(char *host, int query_type, ip_address *addr, int *count) {
    struct addrinfo *addr_info, *res;
    int rv;
    int len = 0;

    if((rv = getaddrinfo(host, NULL, NULL, &addr_info)) != 0) {
        fprintf(stderr, "<Error>\t%s\n", gai_strerror(rv));
        return 1;
    }

    for(res = addr_info; res != NULL; res = res->ai_next) {
        if((query_type & 0x01) && res->ai_family == AF_INET) {
            if(NULL == inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, addr[len].addr, MAX_IP_ADDR_STR_LEN)) continue;
            addr[len].ip_family = AF_INET;
            len++;
        } else if((query_type & 0x02) && res->ai_family == AF_INET6) {
            if(NULL == inet_ntop(AF_INET6, &((struct sockaddr_in *)res->ai_addr)->sin_addr, addr[len].addr, MAX_IP_ADDR_STR_LEN)) continue;
            addr[len].ip_family = AF_INET6;
            len++;
        }
        if(len != 0 && (query_type & 0x04)) break;
    }
    freeaddrinfo(addr_info);
    if(count != NULL) *count = len;
    return 0;
}

bool isSameAsLinkIP(ip_address *addr) {
    struct sockaddr_in tgt;
    inet_pton(addr->ip_family, addr->addr, (struct sockaddr *)&tgt);

    struct ifaddrs *ifaddr;
    if(getifaddrs(&ifaddr) == -1) {
        return false;
    }

    bool result = false;
    int family;
    for(struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;
        struct sockaddr_in *ifa_addr = (struct sockaddr_in *)ifa->ifa_addr;
        if((family == addr->ip_family) && (tgt.sin_addr.s_addr == ifa_addr->sin_addr.s_addr)) {
            result = true;
            break;
        }
    }
    freeifaddrs(ifaddr);
    return result;
}

// ret
// 0: port ok
// 1: socket creation failed
// 2: port in use
int checkPort(unsigned short port) {
    struct sockaddr_in local_addr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return 1;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        fprintf(stdlog2, "<Error>\tCannot bind socket\n");
        close(sockfd);
        return 2;
    }

    close(sockfd);
    return 0;
}