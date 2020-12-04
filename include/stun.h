#ifndef __STUN_H__
#define __STUN_H__

#include <types.h>
#include <utils.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

typedef struct {
    ip_address *addr;
    unsigned short local_port;
    bool change_addr, change_port;
    short connection_try_limit;
} worker_args;

typedef struct {
    ip_address addr;
    int rv;
} worker_ret;

int sendSTUNPacket(ip_address*, bool, bool, unsigned short, short, unsigned char *, short);

void dumpBuffer(unsigned char*, short);

void* _stun_test_worker(void *);

int checkPort(unsigned short);

int examineNetworkEnvironment(ip_address *, ip_address *, unsigned short, ip_address *);

int getGlobalIPAddr(unsigned char*, ip_address*);

int translateNetworkType(int, char*);

void printVerbose(int, worker_args*, worker_ret*);

#endif