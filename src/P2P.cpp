#include <P2P.h>

int P2PConnection::connectP2P(bool is_server, std::string peer_ip, short peer_port, short local_port) {
    sockaddr_in peer_addr, local_addr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "<Error>\tSocket creation failed" << std::endl;
        return 0x0001;
    }

    bzero(&peer_addr, sizeof(peer_addr));
	peer_addr.sin_family = AF_INET;
	if(inet_pton(AF_INET, peer_ip.c_str(), &peer_addr.sin_addr) < 0) {
		std::cerr << "<Error>\tInvalid stun server ip" << std::endl;
        close(sockfd);
		return 0x0002;
	}
	peer_addr.sin_port = htons(peer_port);

	bzero(&local_addr, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(local_port);

	if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
		std::cerr << "<Error>\tBinding failed" << std::endl;
        close(sockfd);
		return 0x0004;
	}

    unsigned char upacket[8] = {0x01, 0x02, 0x03, 0x00};
	
	if(sendto(sockfd, upacket, 8, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
		std::cerr << "<Error>\tSending failed" << std::endl;
        close(sockfd);
		return 0x0008;
	}

    if(is_server) {
        char buf[1024];
        while(1) {
            recvfrom(sockfd, buf, 1024, 0, NULL, NULL);
            printf("Get %s\n", buf);
        }
    } else {
        char buf[1024];
        while(1) {
            scanf("%s", buf);
            if(sendto(sockfd, buf, strlen(buf)+1, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
		        std::cerr << "<Error>\tSending failed" << std::endl;
                close(sockfd);
		        return 0x0008;
	        }
        }
    }

    close(sockfd);

    return 0;
}