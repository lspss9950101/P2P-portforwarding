#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

int main() {
	int sockfd;
	struct sockaddr_in local_addr, src_addr;
	/*
	if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP | IPPROTO_UDP)) < 0) {
		printf("Fail creating\n");
		return -1;
	}
	*/
	
	if((sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP)) < 0) {
		printf("Fail creating\n");
		return -1;
	}

	char optval = 1;
	setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &optval, 1);
	

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr));

	unsigned char buf[1024 << 6];
	memset(&src_addr, 0, sizeof(src_addr));
	socklen_t len = sizeof(src_addr);
	while(1) {
		memset(buf, 0, 1024);
		int rv = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&src_addr, &len);
		//if(rv < 160) continue;
		//printf("%d %u\n", rv, ntohs(*(unsigned short *)&buf[22]));
		//if(*(unsigned short *)&buf[162] != htons(port)) continue;

		if(src_addr.sin_addr.s_addr != 0x0100007F) continue;
		printf("%u\n", ntohs(*(unsigned short *)&buf[22]));
		printf("%u\n", ntohs(*(unsigned short *)&buf[20]));
		int cnt = 0;
		for(int i = 0; i < 24; i++) {
			printf("%02X ", buf[i]);
			if(cnt == 3) printf("\n");
			cnt = (cnt + 1) % 4;
		}
		printf("\n");
	}
}
