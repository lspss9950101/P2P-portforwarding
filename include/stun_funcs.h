#ifndef STUN_FUNCS
#define STUN_FUNCS

#include <stun_msgs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getAddrFromHost(char*, char*);

int send_stun_packet(char *, short, short, STUN_PACKET, unsigned char*);

int getGlobalIPFromPayload(unsigned char*, char *);

#endif