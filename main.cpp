#include <stdio.h>
#include <iostream>
#include <StunMsg.h>
#include <StunUtils.h>
#include <Crypto.h>

#define STUN_1_HOST "stun.ekiga.net"
#define STUN_1_PORT 3478
int main() {
	int type = StunUtils::detectNAT(STUN_1_HOST, STUN_1_PORT, 2222);
	std::cout << StunUtils::translateNATType(type) << std::endl;
}
