#include <utils.h>

// ret
// 0: No error
// 1: getaddrinfo error
// query_type
// 1: ipv4
// 2: ipv6
// 3: both
int getIPFromHost(char *host, int query_type, ip_address *addr, int *count) {
    struct addrinfo *addr_info, *res;
    int rv;
    *count = 0;

    if((rv = getaddrinfo("www.google.com", NULL, NULL, &addr_info)) != 0) {
        fprintf(stderr, "<Error>\t%s\n", gai_strerror(rv));
        return 1;
    }

    for(res = addr_info; res != NULL; res = res->ai_next) {
        if((query_type & 0x01) && res->ai_family == AF_INET) {
            if(NULL == inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, addr[*count].addr, MAX_IP_ADDR_STR_LEN)) continue;
            addr[*count].ip_family = AF_INET;
            (*count)++;
        } else if((query_type & 0x02) && res->ai_family == AF_INET6) {
            if(NULL == inet_ntop(AF_INET6, &((struct sockaddr_in *)res->ai_addr)->sin_addr, addr[*count].addr, MAX_IP_ADDR_STR_LEN)) continue;
            addr[*count].ip_family = AF_INET6;
            (*count)++;
        }
    }
    freeaddrinfo(addr_info);
    return 0;
}