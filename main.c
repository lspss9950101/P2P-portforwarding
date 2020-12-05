#include <utils.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <constants.h>
#include <stun.h>

FILE* stdlog1;
FILE* stdlog2;

int main(int argc, char **argv) {
    bool verbose1 = false, verbose2 = false, subcmd_set = false;
    int subcmd = SUBCMD_DETECT;
    unsigned short port = 2222;

    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "-v")) {
            verbose1 = true;
        } else if(!strcmp(argv[i], "-vv")) {
            verbose1 = verbose2 = true;
        } else if(!strcmp(argv[i], "-p")) {
            if(i == argc-1) return -1;
            port = atoi(argv[i+1]);
            i++;
        } else if(!strcmp(argv[i], "detect")) {
            if(subcmd_set) return -1;
            subcmd_set = true;
            subcmd = SUBCMD_DETECT;
        } else if(!strcmp(argv[i], "server")) {
            if(subcmd_set) return -1;
            subcmd_set = true;
            subcmd = SUBCMD_SERVER;
        } else if(!strcmp(argv[i], "client")) {
            if(subcmd_set) return -1;
            subcmd_set = true;
            subcmd = SUBCMD_CLIENT;
        }
    }

    if(verbose1) stdlog1 = stdout;
    else stdlog1 = fopen("/dev/null", "w");
    if(verbose2) stdlog2 = stdout;
    else stdlog2 = fopen("/dev/null", "w");

    switch(subcmd) {
        case SUBCMD_DETECT:
        {
            printf("Examining Port: %hu\n", port);
            ip_address addr1, addr2;
            getIPFromHost("stun.ekiga.net", DNS_QUERY_IPV4 | DNS_QUERY_RET_FIRST, &addr1, NULL);
            addr1.port = 3478;
            getIPFromHost("stun1.l.google.com", DNS_QUERY_IPV4 | DNS_QUERY_RET_FIRST, &addr2, NULL);
            addr2.port = 19302;

            ip_address global_ip_addr;
            char type[32];
            int type_code = examineNetworkEnvironment(&addr1, &addr2, port, &global_ip_addr);
            printf("\n\n");
            if(type_code >= 3) {
                printf("Global IP Addr: %s:%d\n", global_ip_addr.addr, global_ip_addr.port);
            }
            if(!translateNetworkType(type_code, type)) {
                printf("Network Environment: %s\n", type);
            }
            return 0;
        }
    }

    return 0;
}