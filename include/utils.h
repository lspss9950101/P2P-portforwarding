#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <types.h>
#include <constants.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

int getIPFromHost(char*, int, ip_address*, int*);

#endif