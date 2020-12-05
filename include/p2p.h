#ifndef __P2P_H__
#define __P2P_H__

#include <types.h>

#include <stdio.h>

#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

extern FILE* stdlog1;
extern FILE* stdlog2;

int UDPHolePunching(ip_address*, unsigned short, int);

#endif