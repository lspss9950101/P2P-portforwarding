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

    unsigned char upacket[1];
	
	if(sendto(sockfd, upacket, 1, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
		std::cerr << "<Error>\tSending failed" << std::endl;
        close(sockfd);
		return 0x0008;
	}

    close(sockfd);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "<Error>\tSocket creation failed" << std::endl;
        return 0x0001;
    }

    if(is_server) {
        if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
		    std::cerr << "<Error>\tBinding failed" << std::endl;
            close(sockfd);
		    return 0x0004;
	    }

        if(listen(sockfd, 8) < 0) return 0x0001;

        sockaddr_in client;
        int addr_len = sizeof(sockaddr);
        int clientfd;
        char buf[1024];
        while(1) {
            clientfd = accept(sockfd, (sockaddr *)&client, (socklen_t *)&addr_len);
            recv(clientfd, buf, sizeof(buf), 0);
            printf("Get %s\n", buf);
        }
    } else {
        if(connect(sockfd, (sockaddr *)&peer_addr, sizeof(sockaddr)) < 0) return 0x0001;
        while(1) {
            char buf[1024];
            scanf("%s", buf);
            send(sockfd, buf, strlen(buf), 0);
        }
    }

    close(sockfd);
}