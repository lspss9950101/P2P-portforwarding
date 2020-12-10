#ifndef __P2P_H__
#define __P2P_H__

#include <types.h>

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread_pool.h>

extern FILE* stdlog1;
extern FILE* stdlog2;

typedef enum {
    MSG_SHUT_DOWN=0x0101,
    MSG_ACTIVE_BIND=0x0102,
    MSG_CHECK_ALIVE=0x0103,
    MSG_SET_UUID=0x0104,
    MSG_LOCAL_SET_IP=0x0105,

    MSG_ECHO=0x0201,
    MSG_JOIN_REQ=0x0202,
    MSG_SET_IP_REQ=0x0203,

    MSG_ACK=0x0301,
    MSG_JOIN_RESP=0x0302,
    MSG_SET_IP_RESP=0X0303
} MSG_TYPE;

int startCentralService(unsigned short);

int sendImmediateCommand(MSG_TYPE, unsigned short, void*);

#endif