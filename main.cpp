#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <StunMsg.h>
#include <StunUtils.h>
#include <Crypto.h>
#include <P2P.h>
#include <vector>

#define STUN_1_HOST "stun.ekiga.net"
#define STUN_1_PORT 3478
#define STUN_2_HOST "stun.l.google.com"
#define STUN_2_PORT 19302

// ./main detect

int main(int argc, char **argv) {
	if(argc == 1 || (argc >= 2 && !strncmp(argv[1], "detect", 6))) {
		std::cout << "Detecting NAT..." << std::endl;
		int type = StunUtils::detectNAT(STUN_1_HOST, STUN_1_PORT, argc == 1 ? 2222 : atoi(argv[2]), NULL, NULL);
		std::cout << "Network Environment: " << StunUtils::translateNATType(type) << std::endl;
	} else if(!strncmp(argv[1], "server", 6)) {
		P2PConnection::connectP2P(true, argv[2], atoi(argv[3]), 2222);
	} else if(!strncmp(argv[1], "client", 6)) {
		P2PConnection::connectP2P(false, argv[2], atoi(argv[3]), 2223);
	} else {
		std::cout << "./main SUBCMD" << std::endl
			<< "----------------------" << std::endl
			<< "\tdetect" << std::endl;
	}
	return 0;
}
