#ifndef STUN_UTILS
#define STUN_UTILS

#include <StunMsg.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#include <Type.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

namespace StunUtils {
    int getAddrFromHost(std::string, std::string&);

    int sendStunPacket(std::string, short, short, StunMsg&, uvector&);

    void dumpPacket(StunMsg&, bool);

    void dumpBuffer(uvector&, short);

    std::string translateXORAddress(uvector&);

    int detectNAT(std::string, short, std::string, short, short);

    std::string translateNATType(int);
};
#endif