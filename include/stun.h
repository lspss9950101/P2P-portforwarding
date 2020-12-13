#ifndef __STUN_H__
#define __STUN_H__

#include <utils.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string>

extern FILE* stdlog1;
extern FILE* stdlog2;

struct worker_ret {
    struct sockaddr_in addr;
    int rv;
};

struct worker_args {
    struct sockaddr_in *addr;
    unsigned short local_port;
    bool change_addr, change_port;
    short connection_try_limit;
    worker_ret ret;
};

enum class NetworkEnvironmentType {
    Port_in_use,
    Response_error,
    UDP_Blocked,
    Symmetric_Firewall,
    Open_Internet,
    Full_cone_NAT,
    Symmetric_NAT,
    Restricted_port_NAT,
    Restricted_NAT
};

int sendSTUNPacket(struct sockaddr_in* stun_server, bool change_addr, bool change_port, unsigned short local_port, short connection_tried_limit, unsigned char *buf, short buf_size);

void* _stun_test_worker(void *raw_args);

NetworkEnvironmentType examineNetworkEnvironment(struct sockaddr_in *stun_server1, struct sockaddr_in *stun_server2, unsigned short port, struct sockaddr_in *global_ip);

int getGlobalIPAddr(unsigned char *packet_buf, struct sockaddr_in *global_ip);

std::string translateNetworkType(NetworkEnvironmentType type);

void printVerbose(int test_no, worker_args *args);

#endif