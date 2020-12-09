#include <utils.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <constants.h>
#include <stun.h>
#include <p2p.h>

FILE* stdlog1;
FILE* stdlog2;

void printUsage();

int main(int argc, char **argv) {
    bool verbose1 = false, verbose2 = false, subcmd_set = false;
    int subcmd = SUBCMD_DETECT;
    unsigned short port = 2222;
    char peer_ip[MAX_IP_ADDR_STR_LEN];

    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "-v")) {
            verbose1 = true;
        } else if(!strcmp(argv[i], "-vv")) {
            verbose1 = verbose2 = true;
        } else if(!strcmp(argv[i], "-p")) {
            if(i == argc-1) {
                printUsage();
                return -1;
            }
            port = atoi(argv[i+1]);
            i++;
        } else if(!strcmp(argv[i], "-h")) {
            printUsage();
            return 0;
        } else if(!strcmp(argv[i], "detect")) {
            if(subcmd_set) {
                printUsage();
                return -1;
            }
            subcmd_set = true;
            subcmd = SUBCMD_DETECT;
        } else if(!strcmp(argv[i], "server")) {
            if(subcmd_set) {
                printUsage();
                return -1;
            }
            subcmd_set = true;
            subcmd = SUBCMD_SERVER;
        } else if(!strcmp(argv[i], "connect")) {
            if(subcmd_set) {
                printUsage();
                return -1;
            }
            strcpy(peer_ip, argv[i+1]);
            i++;
            subcmd_set = true;
            subcmd = SUBCMD_CONNECT;
        } else if(!strcmp(argv[i], "shutdown")) {
            if(subcmd_set) {
                printUsage();
                return -1;
            }
            subcmd_set = true;
            subcmd = SUBCMD_SHUTDOWN;
        }
        else {
            if(!subcmd_set) {
                subcmd = SUBCMD_UNKNOWN;
            }
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
            if(type_code == 0) {
                printf("Port in use\n");
            } else if(type_code >= 3) {
                printf("Global IP Addr: %s:%d\n", global_ip_addr.addr, global_ip_addr.port);
            }
            if(!translateNetworkType(type_code, type)) {
                printf("Network Environment: %s\n", type);
            }
            return 0;
        }
        case SUBCMD_SERVER:
        {
            printf("Starting server on port: %hu\n", port);
            int rv = startCentralService(port);
            printf("Server shut down. RET: %d\n", rv);
            return 0;
        }
        case SUBCMD_SHUTDOWN:
        {
            printf("Sending shut down request.\n");
            sendImmediateCommand(MSG_SHUT_DOWN, port, NULL);
            return 0;
        }
        case SUBCMD_CONNECT:
        {
            printf("Sending local echo request.\n");
            ip_address arg;
            strcpy(arg.addr, peer_ip);
            sendImmediateCommand(MSG_LOCAL_BIND, port, &arg);
            return 0;
        }
        default:
            printUsage();
    }

    return 0;
}

void printUsage() {
    printf( "NAT tunnelling tool developed by tragicDilemma\n"
            "Usage: ./main [OPTION] SUBCMD\n"
            "\n"
            "OPTION:\n"
            "\t-p\t\tspecify port\n"
            "\t-v\t\tlevel 1 verbose\n"
            "\t-vv\t\tlevel 2 verbose\n"
            "SUBCMD:\n"
            "\tdetect\t\tdetect current internet environment\n"
            "\tserver\t\topen a tunnelling server\n"
            "\tclient\t\topen a tunnelling client\n"
            "\tshutdown\tsend shutdown request\n"
    );
}