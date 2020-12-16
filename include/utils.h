#ifndef __UTILS_H__
#define __UTILS_H__

#define MIN(X,Y) ((X)<(Y)?(X):(Y))
#define MAX(X,Y) ((X)>(Y)?(X):(Y))

#include <constants.h>

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <unistd.h>
#include <string>

extern FILE* stdlog1;
extern FILE* stdlog2;



int initSocket(sockaddr_in* addr, timeval* timeout, int socktype, int protocol);

int getIPFromHost(std::string, int, sockaddr_in*, int*);

bool isSameAsLinkIP(sockaddr_in*);

int checkPort(unsigned short);

int sendPacket(int, sockaddr_in*, unsigned char*, int);

void dumpBuffer(unsigned char *buf, short buf_size);

std::string getIpString(sockaddr_in *addr, bool show_port);

#endif