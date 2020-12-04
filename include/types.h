#ifndef __TYPES_H__
#define __TYPES_H__

#include <constants.h>

typedef char bool;
#define true 1
#define false 0

typedef struct {
    char addr[MAX_IP_ADDR_STR_LEN];
    unsigned short port, ip_family;
} ip_address;

#endif