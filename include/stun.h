#ifndef __STUN_H__
#define __STUN_H__

#include <types.h>
#include <utils.h>

int sendSTUNPacket(ip_address*, bool, bool, unsigned short);

#endif