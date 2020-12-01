#include <iostream>
#include <StunUtils.h>

#define STUN_HOST "stun.l.google.com"
#define STUN_PORT 19302

int main() {
	int type = StunUtils::detectNAT(STUN_HOST, STUN_PORT, 2222);
	std::cout << StunUtils::translateNATType(type) << std::endl;
}
