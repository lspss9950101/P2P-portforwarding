#ifndef P2P
#define P2P

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>

namespace P2PConnection {
    int connectP2P(bool, std::string, short, short);
};

#endif