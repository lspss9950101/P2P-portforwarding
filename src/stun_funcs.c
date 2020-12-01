#include <stun_funcs.h>

int getAddrFromHost(char* host, char* addr) {
	int sockfd = 0;
	struct addrinfo *addrinfo, *res;
	int rv;
	char address[INET_ADDRSTRLEN];

	if((rv = getaddrinfo(host, NULL, NULL, &addrinfo)) != 0) {
		fprintf(stderr, "<Error>\t%s\n", gai_strerror(rv));
		return 0x0001;
	}

	for(res = addrinfo; res != NULL; res = res->ai_next) {
		if(res->ai_family == AF_INET) {
			if(NULL == inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, address, sizeof(address))) {
				fprintf(stderr, "<Error>\tDNS resolution failed\n");
				return 0x0002;
			}
			strcpy(addr, address);
			return 0x0000;;
		}
	}
	
	fprintf(stderr, "<Error>\tDNS resolution failed, no valid address found\n");
	return 0x0004;
}

int send_stun_packet(char *stun_server_ip, short stun_server_port, short local_port, STUN_PACKET packet, unsigned char *buf) {
	struct sockaddr_in serv_addr, local_addr;
	int sockfd;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "<Error>\tSocket creation failed\n");
		return 0x0001;
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	if(inet_pton(AF_INET, stun_server_ip, &serv_addr.sin_addr) < 0) {
		fprintf(stderr, "<Error>\tInvalid stun server ip\n");
		return 0x0002;
	}
	serv_addr.sin_port = htons(stun_server_port);

	bzero(&local_addr, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(local_port);

	if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
		fprintf(stderr, "<Error>\tBinding failed\n");
		return 0x0004;
	}

	fprintf(stdout, "<Info>\tSocket opened to %s:%d at local port %d\n", stun_server_ip, stun_server_port, local_port);

	int packet_size = packet_len(&packet);
	unsigned char req[packet_size];
	stun_packet_to_str(&packet, req);
	
	if(sendto(sockfd, req, packet_size, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "<Error>\tSending failed\n");
		return 0x0008;
	}
	
	sleep(1);

	if(recvfrom(sockfd, buf, 1024, 0, NULL, 0) < 0) {
		fprintf(stderr, "<Error>\tReceiving failed");
		return 0x0010;
	}

	close(sockfd);
}

int getGlobalIPFromPayload(unsigned char* buf, char *return_ip_port) {
	short attr_type, attr_len, port;
	if(*(short *)(&buf[0]) == htons(0x0101)) {
		htons(*(short *)(&buf[2]));
		for(int i = 20; i < 1024;) {
			attr_type = htons(*(short *)(&buf[i]));
			attr_len = htons(*(short *)(&buf[i+2]));
			if(attr_type == 0x0020) {
				port = ntohs(*(short *)(&buf[i+6]));
				port ^= 0x2112;
				sprintf(return_ip_port, "%d.%d.%d.%d:%d", buf[i+8]^0x21, buf[i+9]^0x12, buf[i+10]^0xA4, buf[i+11]^0x42, port);

				break;
			}
			i += (4 + attr_len);
		}
	} else {
		fprintf(stderr, "<Error>\tSTUN binding failed\n");
		return 0x0020;
	}

	return 0x0000;
}