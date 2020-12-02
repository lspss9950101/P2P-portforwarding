#include <stdio.h>
#include <iostream>
#include <StunMsg.h>
#include <StunUtils.h>
#include <Crypto.h>

#define STUN_1_HOST "stun1.l.google.com"
#define STUN_1_PORT 19302
#define STUN_2_HOST "stun2.l.google.com"
#define STUN_2_PORT 19305

int main() {
	int type = StunUtils::detectNAT(STUN_1_HOST, STUN_1_PORT, STUN_2_HOST, STUN_2_PORT, 2222);
	std::cout << StunUtils::translateNATType(type) << std::endl;
}
