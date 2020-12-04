#include <utils.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <constants.h>
#include <stun.h>

int main(int argc, char **argv) {
    ip_address addr1, addr2;
    getIPFromHost("stun.ekiga.net", DNS_QUERY_IPV4 | DNS_QUERY_RET_FIRST, &addr1, NULL);
    addr1.port = 3478;
    getIPFromHost("stun.l.google.com", DNS_QUERY_IPV4 | DNS_QUERY_RET_FIRST, &addr2, NULL);
    addr2.port = 19302;

    ip_address global_ip_addr;
    char type[32];
    int type_code = examineNetworkEnvironment(&addr1, &addr2, argc > 1 ? atoi(argv[1]) : 2222, &global_ip_addr);
    printf("\n\n");
    if(type_code >= 3) {
        printf("Global IP Addr: %s:%d\n", global_ip_addr.addr, global_ip_addr.port);
    }
    if(!translateNetworkType(type_code, type)) {
        printf("Network Environment: %s\n", type);
    }
    return 0;
}