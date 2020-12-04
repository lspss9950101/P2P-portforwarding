#ifndef __UTILS_H__
#define __UTILS_H__

#define MIN(X,Y) ((X)<(Y)?(X):(Y))
#define MAX(X,Y) ((X)>(Y)?(X):(Y))

#include <stdio.h>
#include <types.h>
#include <constants.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>

int getIPFromHost(char*, int, ip_address*, int*);

bool isSameAsLinkIP(ip_address*);

#endif