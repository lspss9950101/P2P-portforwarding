#include <StunUtils.h>

int StunUtils::getAddrFromHost(std::string host, std::string &addr) {
    addrinfo *addrinfo, *res;
    int rv;
    char address[INET_ADDRSTRLEN];

    if((rv = getaddrinfo(host.c_str(), NULL, NULL, &addrinfo)) != 0) {
        std::cerr << "<Error>\t" << gai_strerror(rv) << std::endl;
        return 0x0001;
    }

    for(res = addrinfo; res != NULL; res = res->ai_next) {
        if(res->ai_family == AF_INET) {
            if(NULL == inet_ntop(AF_INET, &((sockaddr_in *)res->ai_addr)->sin_addr, address, sizeof(address))) {
                std::cerr << "<Error>\tDNS resolution failed" << std::endl;
                return 0x0002;
            }
            addr = address;
            return 0x0000;
        }
    }

    std::cerr << "<Error>\tDNS resolution failed, no record found" << std::endl;
    return 0x0004;
}

int StunUtils::sendStunPacket(std::string stun_server_ip, short stun_server_port, short local_port, StunMsg& msg, uvector &buf) {
    sockaddr_in serv_addr, local_addr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "<Error>\tSocket creation failed" << std::endl;
        return 0x0001;
    }

    bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	if(inet_pton(AF_INET, stun_server_ip.c_str(), &serv_addr.sin_addr) < 0) {
		std::cerr << "<Error>\tInvalid stun server ip" << std::endl;
        close(sockfd);
		return 0x0002;
	}
	serv_addr.sin_port = htons(stun_server_port);

	bzero(&local_addr, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(local_port);

	if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
		std::cerr << "<Error>\tBinding failed" << std::endl;
        close(sockfd);
		return 0x0004;
	}

	uvector packet = msg.toPacket(false, false);
    unsigned char upacket[packet.size()];
    std::copy(packet.begin(), packet.end(), upacket);
	
	if(sendto(sockfd, upacket, packet.size(), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		std::cerr << "<Error>\tSending failed" << std::endl;
        close(sockfd);
		return 0x0008;
	}
	
	sleep(1);

    unsigned char char_buf[1024];

    sockaddr_in src_addr;
    int addr_len = sizeof(sockaddr_in);
	if(recvfrom(sockfd, char_buf, 1024, 0, (sockaddr *)&src_addr, (socklen_t *)&addr_len) < 0) {
		std::cerr << "<Error>\tReceiving failed" << std::endl;
        close(sockfd);
		return 0x0010;
	}
    printf("%d\n", ntohs(src_addr.sin_port));

	close(sockfd);

    buf = uvector(char_buf, char_buf + 1024);
    return 0x0000;
}

void StunUtils::dumpPacket(StunMsg& msg, bool msg_integrity) {
    uvector buf = msg.toPacket(msg_integrity, false);
    int size = msg.size();
    short cnt = 0;
    for(int i = 0; i < size; i++) {
        printf("%02X ", (unsigned char)buf[i]);
        if(cnt == 3) printf("\n");
        cnt = (cnt + 1) % 4;
    }
    printf("\n");
}

void StunUtils::dumpBuffer(uvector &buf, short size) {
    short cnt = 0;
    for(int i = 0; i < size; i++) {
        printf("%02X ", (unsigned char)buf[i]);
        if(cnt == 3) printf("\n");
        cnt = (cnt + 1) % 4;
    }
    printf("\n");
}

std::string StunUtils::translateXORAddress(uvector &buf) {
    std::stringstream ss;
    ss << (buf[3]^0x21) << '.' << (buf[2]^0x12) << '.' << (buf[1]^0xA4) << '.' << (buf[0]^0x42) << ':' << ((*(short *)&buf[5])^0x2112);
    return ss.str();
}

int StunUtils::detectNAT(std::string stun_server1_host, short stun_server1_port, std::string stun_server2_host, short stun_server2_port, short local_port) {
    std::string stun_server1_ip, stun_server2_ip;
	StunUtils::getAddrFromHost(stun_server1_host, stun_server1_ip);
	StunUtils::getAddrFromHost(stun_server2_host, stun_server2_ip);

	unsigned char transaction_id[12];
	*(int *)(&transaction_id[0]) = 0x63c7117e;
	*(int *)(&transaction_id[4]) = 0x0714278f;
	*(int *)(&transaction_id[8]) = 0x5ded3221;

    uvector change_req_flags = {0x00, 0x00, 0x00, 0x00};
    uvector finger_print = {0x53, 0x54, 0x55, 0x4e};

	StunMsg msg(STUN_MSG_TYPE::STUN_MSG_TYPE_BINDING_REQ, transaction_id);
    msg.setAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_CHANGE_REQ, change_req_flags);

    // Test 1
    uvector buf;
	if(StunUtils::sendStunPacket(stun_server1_ip, stun_server1_port, local_port, msg, buf) == 0x0010) {
        // UDP blocked
        return 0x0001;
    }

    StunMsg response(buf);
    std::string global_ip_same_ip_port;
	if(response.getType() == STUN_MSG_TYPE::STUN_MSG_TYPE_BINDING_RES) {
		uvector xor_ip = response.getAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS);
		global_ip_same_ip_port = StunUtils::translateXORAddress(xor_ip);
        std::cout << "Global IP Port: " << global_ip_same_ip_port << std::endl;
	}

    // TODO: check link ip and global ip

    // Test 2
    
    change_req_flags = {0xFF, 0xFF, 0xFF, 0xFF};
    msg.setAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_CHANGE_REQ, change_req_flags);
    if(StunUtils::sendStunPacket(stun_server2_ip, stun_server2_port, local_port, msg, buf) == 0x0000) {
        response.parseBuffer(buf);
        // Full Cone NAT
        return 0x0002;
    }

    response.parseBuffer(buf);
    if(response.getType() == STUN_MSG_TYPE::STUN_MSG_TYPE_BINDING_RES) {
		uvector xor_ip = response.getAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS);
		if(global_ip_same_ip_port != StunUtils::translateXORAddress(xor_ip)) {
            // Symmetric NAT
            return 0x0004;
        }
	}

    // Test 3
    change_req_flags[0] = 0x02;
    msg.setAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_CHANGE_REQ, change_req_flags);
    if(StunUtils::sendStunPacket(stun_server1_ip, stun_server1_port, local_port, msg, buf) == 0x0000) {
        // Restricted NAT
        return 0x0008;
    }

    // Port Restricted NAT
    return 0x0010;
}

std::string StunUtils::translateNATType(int type) {
    switch(type) {
        case 0x0001:
            return "UDP blocked";
        case 0x0002:
            return "Full Cone NAT";
        case 0x0004:
            return "Symmetric NAT";
        case 0x0008:
            return "Restricted NAT";
        case 0x0010:
            return "Port Restricted NAT";
    }
    return "Unknown type";
}