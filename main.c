#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stun_msgs.h>
#include <stun_funcs.h>

#define STUN_HOST "stun1.l.google.com"
#define STUN_PORT 19302

int main() {
	char host[] = STUN_HOST;
	char addr[INET_ADDRSTRLEN];
	getAddrFromHost(host, addr);

	STUN_PACKET packet;
	unsigned char transaction_id[12];
	*(int *)(&transaction_id[0]) = 0x63c7117e;
	*(int *)(&transaction_id[4]) = 0x0714278f;
	*(int *)(&transaction_id[8]) = 0x5ded3221;
	create_stun_packet(&packet, STUN_MSG_TYPE_BINDING_REQ, transaction_id);
	unsigned char buf[1024];
	char global_ip[64];

	send_stun_packet(addr, STUN_PORT, 2224, packet, buf);
	getGlobalIPFromPayload(buf, global_ip);
	fprintf(stdout, "%s\n", global_ip);

	STUN_ATTR attr;
	unsigned char val[] = {0, 0, 0, 6};
	create_stun_attr(&attr, STUN_ATTR_TYPE_CHANGED_ADDR, 4, val);
	append_attr(&packet, &attr);
	delete_stun_attr(&attr);
	
	send_stun_packet(addr, STUN_PORT, 2224, packet, buf);
	getGlobalIPFromPayload(buf, global_ip);
	fprintf(stdout, "%s\n", global_ip);

	delete_stun_packet(&packet);
}
