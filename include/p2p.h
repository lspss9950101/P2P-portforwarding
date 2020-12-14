#ifndef __P2P_H__
#define __P2P_H__

#include <utils.h>
#include <uuid.h>
#include <thread_pool.h>

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern FILE* stdlog1;
extern FILE* stdlog2;

enum class MSG_TYPE {
    SHUT_DOWN=0x0101,
    ACTIVE_BIND=0x0102,
    CHECK_ALIVE=0x0103,
    SET_UUID=0x0104,
    LOCAL_SET_IP=0x0105,

    ECHO=0x0201,
    JOIN_REQ=0x0202,
    SET_IP_REQ=0x0203,

    ACK=0x0301,
    JOIN_RESP=0x0302,
    SET_IP_RESP=0X0303
};

void* service_worker_func(void *args);

int sendP2PPacket(int sockfd, sockaddr_in *target_addr, MSG_TYPE msg_type, UUID source_uuid, UUID target_uuid, void *args);

int startCentralService(unsigned short port, int thread_number);

int sendImmediateCommand(MSG_TYPE, unsigned short, void*);

#endif