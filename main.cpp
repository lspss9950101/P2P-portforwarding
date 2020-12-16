#include <constants.h>
#include <p2p.h>
#include <stun.h>
#include <profile.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *stdlog1;
FILE *stdlog2;

Profile self_profile;

int main(int argc, char **argv) {
    int verbose_level = 0;
    int subcmd = SUBCMD_UNKNOWN;
    unsigned short port = 2222;
    char target_ip[MAX_IP_ADDR_STR_LEN];
    unsigned short target_port;

    for(int i = 1; i < argc; i++) {
        if(!strcmp("-v", argv[i])) {
            verbose_level = 1;
        } else if(!strcmp("-vv", argv[i])) {
            verbose_level = 2;
        } else if(!strcmp("-p", argv[i]) && i+1 != argc) {
            port = atoi(argv[++i]);
        } else if(!strcmp("-tp", argv[i]) && i+1 != argc) {
            target_port = atoi(argv[++i]);
        } else if(!strcmp("detect", argv[i]) && subcmd == SUBCMD_UNKNOWN) {
            subcmd = SUBCMD_DETECT;
        } else if(!strcmp("server", argv[i]) && subcmd == SUBCMD_UNKNOWN) {
            subcmd = SUBCMD_SERVER;
        } else if(!strcmp("connect", argv[i]) && subcmd == SUBCMD_UNKNOWN) {
            subcmd = SUBCMD_CONNECT;
        } else if(!strcmp("shutdown", argv[i]) && subcmd == SUBCMD_UNKNOWN) {
            subcmd = SUBCMD_SHUTDOWN;
        } else {
            strcpy(target_ip, argv[i]);
        }
    }

    if(verbose_level == 1) {
        stdlog1 = stdout;
        stdlog2 = fopen("/dev/null", "w");
    } else if(verbose_level == 2) {
        stdlog1 = stdout;
        stdlog2 = stdout;
    } else {
        stdlog1 = fopen("/dev/null", "w");
        stdlog2 = fopen("/dev/null", "w");
    }

    switch(subcmd) {
        case SUBCMD_DETECT:{
            printf("Detecting network environment\n");
            sockaddr_in addr1, addr2, global_ip;
            getIPFromHost("stun.ekiga.net", DNS_QUERY_IPV4 | DNS_QUERY_RET_FIRST, &addr1, NULL);
            getIPFromHost("stun1.l.google.com", DNS_QUERY_IPV4 | DNS_QUERY_RET_FIRST, &addr2, NULL);
            addr1.sin_port = htons(3478);
            addr2.sin_port = htons(19302);
            NetworkEnvironmentType type = examineNetworkEnvironment(&addr1, &addr2, port, &global_ip);
            printf("Network environment: %s\n", translateNetworkType(type).c_str());
            char ip[MAX_IP_ADDR_STR_LEN];
            inet_ntop(global_ip.sin_family, &global_ip.sin_addr, ip, sizeof(sockaddr_in));
            printf("Global ip: %s:%u\n", ip, ntohs(global_ip.sin_port));
            return 0;
        }
        case SUBCMD_SERVER:{
            /*inet_pton(AF_INET, target_ip, &self_profile.global_ip.sin_addr);
            self_profile.global_ip.sin_family = AF_INET;
            self_profile.global_ip.sin_port = htons(port);*/
            sockaddr_in addr;
            getIPFromHost("stun1.l.google.com", DNS_QUERY_IPV4 | DNS_QUERY_RET_FIRST, &addr, NULL);
            addr.sin_port = htons(19302);
            if(getGlobalIp(&addr, port, &self_profile.global_ip) < 0) return 0;
            printf("Starting server on port: %hu\n", port);
            printf("Global IP: %s\n", getIpString(&self_profile.global_ip, true).c_str());
            int rv = startCentralService(port, 8);
            printf("Server shut down. RET: %d\n", rv);
            return 0;
        }
        case SUBCMD_SHUTDOWN:{
            printf("Sending shut down request.\n");
            sendImmediateCommand(MSG_TYPE::SHUT_DOWN, port, NULL);
            return 0;
        }
        case SUBCMD_CONNECT:{
            printf("Sending local echo request.\n");
            sockaddr_in arg;
            inet_pton(AF_INET, target_ip, &arg.sin_addr);
            arg.sin_family = AF_INET;
            arg.sin_port = htons(target_port);
            sendImmediateCommand(MSG_TYPE::ACTIVE_BIND, port, &arg);
            return 0;
        }
    }

    return 0;
}