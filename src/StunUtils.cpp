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

    timeval timeout = {3, 0};
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) != 0) {
        std::cerr << "<Error>\tCannot set timeout" << std::endl;
        close(sockfd);
        return 0x0020;
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

    unsigned char char_buf[1024];

	if(recvfrom(sockfd, char_buf, 1024, 0, NULL, NULL) < 0) {
		std::cerr << "<Error>\tReceiving failed" << std::endl;
        close(sockfd);
		return 0x0010;
	}

	close(sockfd);
    unsigned short size = (unsigned short)char_buf[2] << 8 | char_buf[3];
    buf = uvector(char_buf, char_buf + size + 20);
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

std::string StunUtils::translateXORAddress(uvector &buf, unsigned short &port) {
    // need fix
    std::stringstream ss;
    ss << (buf[4]^0x21) << '.' << (buf[5]^0x12) << '.' << (buf[6]^0xA4) << '.' << (buf[7]^0x42);
    port = ntohs(*(short*)&buf[2]) ^ 0x2112;
    return ss.str();
}

std::string StunUtils::translateAddress(uvector &buf, unsigned short &port) {
    std::stringstream ss;
    ss << (unsigned short)buf[4] << '.' << (unsigned short)buf[5] << '.' << (unsigned short)buf[6] << '.' << (unsigned short)buf[7];
    port = ntohs(*(short*)&buf[2]);
    return ss.str();
}

int StunUtils::detectNAT(std::string stun_server_host, short stun_server_port, short local_port, std::string *global_ip, short *global_port) {
    std::string stun_server_ip;
	StunUtils::getAddrFromHost(stun_server_host, stun_server_ip);

	unsigned char transaction_id[12];
	*(int *)(&transaction_id[0]) = 0x63c7117e;
	*(int *)(&transaction_id[4]) = 0x0714278f;
	*(int *)(&transaction_id[8]) = 0x5ded3221;

    uvector change_req_flags = {0x00, 0x00, 0x00, 0x00};
    uvector finger_print = {0x53, 0x54, 0x55, 0x4e};

	StunMsg msg(STUN_MSG_TYPE::STUN_MSG_TYPE_BINDING_REQ, transaction_id);

    // Test 1
    uvector buf;
	if(StunUtils::sendStunPacket(stun_server_ip, stun_server_port, local_port, msg, buf) != 0x0000) {
        // UDP blocked
        return 0x0001;
    }

    StunMsg response(buf);
    std::string global_ip_same_ip_port;
    unsigned short global_port_same_ip_port;
	if(response.getType() == STUN_MSG_TYPE::STUN_MSG_TYPE_BINDING_RES) {
        if(response.getAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS).size() != 0) {
		    uvector xor_ip = response.getAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS);
		    global_ip_same_ip_port = StunUtils::translateXORAddress(xor_ip, global_port_same_ip_port);
        } else if(response.getAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS_ALTER).size() != 0) {
            uvector xor_ip = response.getAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS_ALTER);
		    global_ip_same_ip_port = StunUtils::translateXORAddress(xor_ip, global_port_same_ip_port);
        } else {
            uvector ip = response.getAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_MAPPED_ADDR);
            global_ip_same_ip_port = StunUtils::translateAddress(ip, global_port_same_ip_port);
        }

        std::cout << "Global IP Port: " << global_ip_same_ip_port << ':' << global_port_same_ip_port << std::endl;
        uvector ip = response.getAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_MAPPED_ADDR);
        global_ip_same_ip_port = StunUtils::translateAddress(ip, global_port_same_ip_port);
        std::cout << "Global IP Port: " << global_ip_same_ip_port << ':' << global_port_same_ip_port << std::endl;
	}

    // TODO: check link ip and global ip
    std::vector<std::string> local_ip_list = StunUtils::getIpList();
    bool same_public_local = std::find(local_ip_list.begin(), local_ip_list.end(), global_ip_same_ip_port) != local_ip_list.end();

    // Test 2
    
    change_req_flags = {0x06, 0x00, 0x00, 0x00};
    msg.setAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_CHANGE_REQ, change_req_flags);
    if(StunUtils::sendStunPacket(stun_server_ip, stun_server_port, local_port, msg, buf) == 0x0000) {
        if(same_public_local) {
            // Open Internet
            return 0x0020;
        }
        // Full Cone NAT
        return 0x0002;
    }

    if(same_public_local) {
        // Symmetric Firewall
        return 0x0040;
    }

    response.parseBuffer(buf);
    if(response.getType() == STUN_MSG_TYPE::STUN_MSG_TYPE_BINDING_RES) {
		uvector xor_ip = response.getAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS);
        unsigned short port;
		if(global_ip_same_ip_port != StunUtils::translateXORAddress(xor_ip, port) && port == global_port_same_ip_port) {
            // Symmetric NAT
            return 0x0004;
        }
	}

    // Test 3
    change_req_flags = {0x02, 0x00, 0x00, 0x00};
    msg.setAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_CHANGE_REQ, change_req_flags);
    if(StunUtils::sendStunPacket(stun_server_ip, stun_server_port, local_port, msg, buf) == 0x0000) {
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
        case 0x0020:
            return "Open Internet";
        case 0x0040:
            return "Symmetric Firewall";
    }
    return "Unknown type";
}

std::vector<std::string> StunUtils::getIpList() {
    ifaddrs *ifaddr;
    if(getifaddrs(&ifaddr) == -1) {
        return std::vector<std::string>();
    }

    char host[NI_MAXHOST];
    int family;
    std::vector<std::string> result;
    for(ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;
        if(family == AF_INET || family == AF_INET6) {
            getnameinfo(ifa->ifa_addr, (family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6)), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            result.push_back(host);
        }
    }
    return result;
}