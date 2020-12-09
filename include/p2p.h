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

//
//   00            07              15              23               31
//  |-----------------------------------------------------------------|
//  |                    Source IP Address(LAN)                       |
//  |-----------------------------------------------------------------|
//  |                    Target IP Address(LAN)                       |
//  |-----------------------------------------------------------------|
//  |          Source Port          |        Target Port              |
//  |-----------------------------------------------------------------|
//  |          Message Type         |        Message Length           |
//  |-----------------------------------------------------------------|
//  |                           Preserved                             |
//  |-----------------------------------------------------------------|
//  |                                                                 |
//  |                             Data                                |
//  |                               .                                 |
//  |                               .                                 |
//  |                               .                                 |
//  |                                                                 |
//  |-----------------------------------------------------------------|
//
//  Message Type:
//      0x0101:     Shut down server request. Both target IP and source IP are ignored.
//      0x0102:     Local binding request. Both target IP and source IP are ignored.
//                  Message length should always be 4.
//                  Data field should contain 4 bytes representing client's global IP.
//                  After recieving, server should send binding request immediately.
//      0x0103:     Refresh request.
//                  If a mapped IP doesn't ack the echo request for two time strip, the mapping should be removed.
//                  Send echo request to all mapped IP.
//      0x0001:     Ack.
//                  Message length and data field are ignored.
//                  When recieving, the tunnel is created.
//      0x0002:     Echo request.
//                  Message length and data field are ignored.
//                  After recieving, server should send ack message immediately.
//      0x0003:     DHCP request.
//                  Ask for the mapped IP. If no mapping exists, server should assign a new one.
//      0x0004:     DHCP response.
//                  Message length should be 4.
//                  Data field should contain 4 btyes representing the mapped IP.
//      0x0005:     IP allocation request.
//      0x0006:     IP allocation response.
//

typedef enum {
    MSG_SHUT_DOWN=0x0101,
    MSG_LOCAL_BIND=0x0102,
    MSG_ACK=0x0001,
    MSG_ECHO=0x0002,
} MSG_TYPE;

int startCentralService(unsigned short);

int sendImmediateCommand(MSG_TYPE, unsigned short, void*);

#endif