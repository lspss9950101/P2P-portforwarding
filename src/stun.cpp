#include <stun.h>

// ret
// 0 : No error
// -1: socket init error
// -2: sending error
// -3: recv timeout
// -4: buffer overflows
int sendSTUNPacket(struct sockaddr_in* stun_server, bool change_addr, bool change_port, unsigned short local_port, short connection_tried_limit, unsigned char *buf, short buf_size) {
    struct sockaddr_in local_addr, src_addr;
    int addr_len = sizeof(sockaddr_in);
    memset(&local_addr, 0, sizeof(sockaddr_in));
    memset(&src_addr, 0, sizeof(sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(local_port);
    struct timeval timeout = {3, 0};
    int sockfd = initSocket(&local_addr, &timeout, SOCK_DGRAM, 0);
    if(sockfd < 0) return -1;

    unsigned short req_size = ((change_addr || change_port) ? 0x0008 : 0x0000) + 20;
    unsigned char binding_req[req_size];
    *(unsigned short *)&binding_req[0] = htons(0x0001);
    *(unsigned short *)&binding_req[2] = htons(req_size - 20);
    *(unsigned int *)&binding_req[4] = htonl(0x2112A442);
    *(unsigned int *)&binding_req[8] = htonl(0x63c7117e);
    *(unsigned int *)&binding_req[12] = htonl(0x0714278f);
    *(unsigned int *)&binding_req[16] = htonl(0x5ded3221);
    if(change_addr || change_port) {
        *(unsigned short *)&binding_req[20] = htons(0x0003);
        *(unsigned short *)&binding_req[22] = htons(0x0004);
        unsigned int flag = 0;
        flag |= (change_addr ? 0x0004 : 0x0000);
        flag |= (change_port ? 0x0002 : 0x0000);
        *(unsigned int *)&binding_req[24] = htonl(flag);
    }

    unsigned char recv_buf[128];
    if(connection_tried_limit <= 0) connection_tried_limit = 5;
    for(int connection_tried = 0; connection_tried < connection_tried_limit; connection_tried++) {
        if(sendto(sockfd, binding_req, req_size, 0, (struct sockaddr *)stun_server, sizeof(sockaddr)) < 0) {
            fprintf(stderr, "<Error>\tSending failed\n");
            close(sockfd);
            return -2;
        }

        sleep(0.5);

        int rv;
        if((rv = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&src_addr, (socklen_t *)&addr_len) < 0)) {
            fprintf(stdlog2, "<Info>\tReceiving error. RET: %d -%d\n", rv, connection_tried);
            if(connection_tried == connection_tried_limit-1) {
                close(sockfd);
                return -3;
            }
            continue;
        }

        if((!change_addr && (src_addr.sin_addr.s_addr != stun_server->sin_addr.s_addr)) || (!change_port && (src_addr.sin_port != stun_server->sin_port))) {
            char tmp[128];
            inet_ntop(AF_INET, &src_addr.sin_addr, tmp, 128);
            fprintf(stdlog2, "<Info>\tWrong packet From %s:%d\n", tmp, src_addr.sin_port);
            continue;
        }
        break;
    }
    close(sockfd);

    unsigned short msg_len = ntohs(*(unsigned short *)&recv_buf[2]) + 20;
    if(msg_len > buf_size) return -4;
    memcpy(buf, recv_buf, msg_len);
    memset(buf + msg_len, 0, buf_size - msg_len);
    return 0;
}

void* _stun_test_worker(void *raw_args) {
    struct worker_args *args = (struct worker_args *)raw_args;
    unsigned char buf[1024];

    if((args->ret.rv = sendSTUNPacket(args->addr, args->change_addr, args->change_port, args->local_port, args->connection_try_limit, buf, sizeof(buf))) != 0)
        pthread_exit((void *)&args->ret);
    else {
        args->ret.rv |= (getGlobalIPAddr(buf, &args->ret.addr) << 16);
        if(args->ret.rv & 0xFFFF0000)
            fprintf(stderr, "<Error>\tCannot get global ip\n");
        pthread_exit(NULL);
    }
}

NetworkEnvironmentType examineNetworkEnvironment(struct sockaddr_in *stun_server1, struct sockaddr_in *stun_server2, unsigned short port, struct sockaddr_in *global_ip_ret) {
    if(checkPort(port)) {
        fprintf(stdlog1, "<Error>\tPort in use\n");
        return NetworkEnvironmentType::Port_in_use;
    }

    sockaddr_in global_ip;
    srand(0);
    pthread_t test1_worker, test2_worker, test3_worker;
    worker_args args[3];
    unsigned short random_port = port;

    // Test 1
    args[0].addr = stun_server1;
    args[0].change_addr = args[0].change_port = false;
    args[0].connection_try_limit = 5;
    args[0].local_port = port;
    pthread_create(&test1_worker, NULL, _stun_test_worker, &args[0]);

    // Test 2
    args[1].addr = stun_server1;
    args[1].change_addr = args[1].change_port = true;
    args[1].connection_try_limit = 2;
    while(random_port == port || checkPort(random_port) != 0) random_port = rand() % 5000 + 20000;
    args[1].local_port = random_port;
    pthread_create(&test2_worker, NULL, _stun_test_worker, &args[1]);

    // Test 3
    args[2].addr = stun_server1;
    args[2].change_addr = false;
    args[2].change_port = true;
    args[2].connection_try_limit = 2;
    while(random_port == port || random_port == args[1].local_port || checkPort(random_port) != 0) random_port = rand() % 5000 + 20000;
    args[2].local_port = port;
    pthread_create(&test3_worker, NULL, _stun_test_worker, &args[2]);

    // Test 1
    pthread_join(test1_worker, NULL);
    printVerbose(1, &args[0]);
    if(args[0].ret.rv & 0xFFFF) {
        pthread_cancel(test2_worker);
        pthread_cancel(test3_worker);
        return NetworkEnvironmentType::UDP_Blocked;
    }

    if(args[0].ret.rv & 0xFFFF0000) {
        pthread_cancel(test2_worker);
        pthread_cancel(test3_worker);
        return NetworkEnvironmentType::Response_error;
    }

    global_ip = args[0].ret.addr;
    if(global_ip_ret != NULL) memcpy(global_ip_ret, &global_ip, sizeof(sockaddr));

    pthread_join(test2_worker, NULL);
    printVerbose(2, &args[1]);
    if(isSameAsLinkIP(&global_ip)) {
        // No NAT
        if(args[1].ret.rv & 0xFFFF) {
            // Symmetric Firewall
            pthread_cancel(test3_worker);
            return NetworkEnvironmentType::Symmetric_Firewall;
        } else {
            // Open Internet
            pthread_cancel(test3_worker);
            return NetworkEnvironmentType::Open_Internet;
        }
    } else {
        // NAT detected
        if(args[1].ret.rv & 0xFFFF) {
            args[0].addr = stun_server2;
            pthread_create(&test1_worker, NULL, _stun_test_worker, &args[0]);
            pthread_join(test1_worker, NULL);
            printVerbose(1, &args[0]);

            if(args[0].ret.rv & 0xFFFF0000) {
                // Resp error
                pthread_cancel(test3_worker);
                return NetworkEnvironmentType::Response_error;
            }

            if(args[0].ret.addr.sin_addr.s_addr != global_ip.sin_addr.s_addr || args[0].ret.addr.sin_port != global_ip.sin_port) {
                // Symmetric NAT;
                pthread_cancel(test3_worker);
                return NetworkEnvironmentType::Symmetric_NAT;
            } else {
                pthread_join(test3_worker, NULL);
                printVerbose(3, &args[2]);
                if(args[2].ret.rv & 0xFFFF) {
                    // Restricted port NAT
                    return NetworkEnvironmentType::Restricted_port_NAT;
                } else {
                    // Restricted NAT
                    return NetworkEnvironmentType::Restricted_NAT;
                }
            }
        } else {
            // Full-cone NAT
            pthread_cancel(test3_worker);
            return NetworkEnvironmentType::Full_cone_NAT;
        }
    }
    return NetworkEnvironmentType::Response_error;
}

// ret
// 0 : no error
// -1: response error
// -2: no ip found
int getGlobalIPAddr(unsigned char *packet_buf, struct sockaddr_in *global_ip) {
    if(0x0101 != ntohs(*(unsigned short *)&packet_buf[0])) return -1;
    
    bool found_ip = false;
    unsigned short msg_len = ntohs(*(unsigned short *)&packet_buf[2]) + 20;
    for(unsigned short idx = 20; idx < msg_len;) {
        unsigned short attr_type = ntohs(*(unsigned short *)&packet_buf[idx]);
        unsigned short attr_len = ntohs(*(unsigned short *)&packet_buf[idx+2]);

        if(attr_type == 0x0001) {
            found_ip = true;
            if(packet_buf[idx+5] == 0x01) {
                ((struct sockaddr_in *)global_ip)->sin_family = AF_INET;
                ((struct sockaddr_in *)global_ip)->sin_addr.s_addr = ntohl(*(unsigned int *)&packet_buf[idx+8]);
            } else {
                ((struct sockaddr_in6 *)global_ip)->sin6_family = AF_INET6;
                ((struct sockaddr_in6 *)global_ip)->sin6_addr.__in6_u.__u6_addr32[0] = ntohl(*(unsigned int *)&packet_buf[idx+20]);
                ((struct sockaddr_in6 *)global_ip)->sin6_addr.__in6_u.__u6_addr32[1] = ntohl(*(unsigned int *)&packet_buf[idx+16]);
                ((struct sockaddr_in6 *)global_ip)->sin6_addr.__in6_u.__u6_addr32[2] = ntohl(*(unsigned int *)&packet_buf[idx+12]);
                ((struct sockaddr_in6 *)global_ip)->sin6_addr.__in6_u.__u6_addr32[3] = ntohl(*(unsigned int *)&packet_buf[idx+8]);
            }
            ((struct sockaddr_in *)global_ip)->sin_port = ntohs(*(unsigned short*)&packet_buf[idx+6]);
        } else if(attr_type == 0x0020 || attr_type == 0x8020) {
            if(packet_buf[idx+5] == 0x01) {
                ((struct sockaddr_in *)global_ip)->sin_family = AF_INET;
                ((struct sockaddr_in *)global_ip)->sin_addr.s_addr = ntohl(*(unsigned int *)&packet_buf[idx+8]) ^ 0x2112A442;
            } else {
                ((struct sockaddr_in6 *)global_ip)->sin6_family = AF_INET6;
                ((struct sockaddr_in6 *)global_ip)->sin6_addr.__in6_u.__u6_addr32[0] = ntohl(*(unsigned int *)&packet_buf[idx+20]) ^ 0x2112A442;
                ((struct sockaddr_in6 *)global_ip)->sin6_addr.__in6_u.__u6_addr32[1] = ntohl(*(unsigned int *)&packet_buf[idx+16]) ^ 0x63C7117E;
                ((struct sockaddr_in6 *)global_ip)->sin6_addr.__in6_u.__u6_addr32[2] = ntohl(*(unsigned int *)&packet_buf[idx+12]) ^ 0x0714278F;
                ((struct sockaddr_in6 *)global_ip)->sin6_addr.__in6_u.__u6_addr32[3] = ntohl(*(unsigned int *)&packet_buf[idx+8]) ^ 0x5DED3221;
            }
            ((struct sockaddr_in *)global_ip)->sin_port = ntohs(*(unsigned short*)&packet_buf[idx+6]) ^ 0x2112;
            return 0;
        }
        idx += (4 + attr_len);
    }
    return found_ip ? 0 : -2;
}

std::string translateNetworkType(NetworkEnvironmentType type) {
    switch(type) {
        case NetworkEnvironmentType::UDP_Blocked:
            return "UDP blocked";
        case NetworkEnvironmentType::Symmetric_Firewall:
            return "Symmetric Firewall";
        case NetworkEnvironmentType::Open_Internet:
            return "Open Internet";
        case NetworkEnvironmentType::Full_cone_NAT:
            return "Full-cone NAT";
        case NetworkEnvironmentType::Symmetric_NAT:
            return "Symmetric NAT";
        case NetworkEnvironmentType::Restricted_port_NAT:
            return "Restricted-port NAT";
        case NetworkEnvironmentType::Restricted_NAT:
            return "Restricted NAT";
        default:
            return "Unknown";
    }
}

void printVerbose(int test_no, worker_args *args) {
    worker_ret *ret = &args->ret;
    char server_ip[MAX_IP_ADDR_STR_LEN];
    inet_ntop(args->addr->sin_family, &args->addr->sin_addr, server_ip, sizeof(sockaddr_in));
    if(ret->rv)
        fprintf(stdlog1, "=======================================================\n"
                        "Test %d Start\n"
                        "STUN Server Address: \t%s:%d\n"
                        "Local Port: \t\t%d\n"
                        "Change Address: \t%s\n"
                        "Change Port: \t\t%s\n"
                        "Result: \t\tFailed\n"
                        "=======================================================\n",
                        test_no, server_ip, ntohs(args->addr->sin_port), args->local_port, args->change_addr ? "True" : "False",
                        args->change_port ? "True" : "False");
    else {
        char global_ip[MAX_IP_ADDR_STR_LEN];
        inet_ntop(ret->addr.sin_family, &ret->addr.sin_addr, global_ip, sizeof(sockaddr_in));
        fprintf(stdlog1, "=======================================================\n"
                        "Test %d Start\n"
                        "STUN Server Address: \t%s:%d\n"
                        "Local Port: \t\t%d\n"
                        "Change Address: \t%s\n"
                        "Change Port: \t\t%s\n"
                        "Result: \t\tSucceeded\n"
                        "Global IP Address: \t%s:%d\n"
                        "=======================================================\n",
                        test_no, server_ip, ntohs(args->addr->sin_port), args->local_port, args->change_addr ? "True" : "False",
                        args->change_port ? "True" : "False", global_ip, ret->addr.sin_port);
    }
}
