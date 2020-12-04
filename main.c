#include <utils.h>
#include <types.h>
#include <stdio.h>
#include <constants.h>

int main() {
    int count;
    ip_address addr[16];
    getIPFromHost("www.google.com", DNS_QUERY_IPV4 | DNS_QUERY_IPV6, addr, &count);
    for(int i = 0; i < count; i++) {
        printf("%s\n", addr[i].ip_ver == IPV4 ? addr[i].addr.ipv4 : addr[i].addr.ipv6);
    }
    getIPFromHost("www.google.com", DNS_QUERY_IPV4, addr, &count);
    for(int i = 0; i < count; i++) {
        printf("%s\n", addr[i].ip_ver == IPV4 ? addr[i].addr.ipv4 : addr[i].addr.ipv6);
    }
    return 0;
}