// ret
// 0: No error
// 1: socket creation error
// 2: timeout setting error
// 3: invalid server ip
// 4: socket binding error
// 5: sending error
// 6: receiving error
// 7: buffer overflow
int sendSTUNPacket(ip_address* addr, bool change_addr, bool change_port, unsigned short local_port, short connection_tried_limit, unsigned char *buf, short buf_size) {
    struct sockaddr_in serv_addr, local_addr, src_addr;
    int sockfd;

    if((sockfd = socket(addr->ip_family, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "<Error>\tSocket creation failed\n");
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = addr->ip_family;
    
    if(inet_pton(addr->ip_family, addr->addr, &serv_addr.sin_addr) < 0) {
        fprintf(stderr, "<Error>\tInvalid ip\n");
        close(sockfd);
        return 3;
    }
    serv_addr.sin_port = htons(addr->port);

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(local_port);

    if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        fprintf(stderr, "<Error>\tSocket binding error\n");
        close(sockfd);
        return 4;
    }

    struct timeval timeout = {3, 0};
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "<Error>\tTimeout setting error.\n");
        close(sockfd);
        return 2;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    socklen_t addr_len = sizeof(src_addr);

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
        if(sendto(sockfd, binding_req, req_size, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            fprintf(stderr, "<Error>\tSending failed\n");
            close(sockfd);
            return 5;
        }

        sleep(0.5);

        int rv;
        if((rv = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&src_addr, &addr_len) < 0)) {
            fprintf(stdlog2, "<Info>\tReceiving error. RET: %d -%d\n", rv, connection_tried);
            if(connection_tried == connection_tried_limit-1) {
                close(sockfd);
                return 6;
            }
            continue;
        }

        if((!change_addr && (src_addr.sin_addr.s_addr != serv_addr.sin_addr.s_addr)) || (!change_port && (src_addr.sin_port != serv_addr.sin_port))) {
            char tmp[128];
            inet_ntop(AF_INET, &src_addr.sin_addr, tmp, 128);
            fprintf(stdlog2, "<Info>\tWrong packet From %s:%d\n", tmp, src_addr.sin_port);
            continue;
        }

        break;
    }

    close(sockfd);

    unsigned short msg_len = ntohs(*(unsigned short *)&recv_buf[2]) + 20;
    if(msg_len > buf_size) return 7;
    memcpy(buf, recv_buf, msg_len);
    memset(buf + msg_len, 0, buf_size - msg_len);
    return 0;
}

void dumpBuffer(unsigned char* buf, short len) {
    for(int i = 0, col_count = 0; i < len; i++) {
        printf("%02X ", buf[i]);
        if(col_count == 3) printf("\n");
        col_count = (col_count+1) % 4;
    }
    printf("\n");
}

// ret 0xBBBBAAAA
// AAAA: send
// BBBB: get
void* _stun_test_worker(void *args_in) {
    worker_args *args = args_in;
    unsigned char buf[1024];

    if((args->ret.rv = sendSTUNPacket(args->addr, args->change_addr, args->change_port, args->local_port, args->connection_try_limit, buf, sizeof(buf))) != 0)
        pthread_exit((void*)&args->ret);
    args->ret.rv |= (getGlobalIPAddr(buf, &args->ret.addr) << 16);
    if(args->ret.rv & 0xFFFF0000)
        fprintf(stderr, "<Error>\tCannot get global ip.\n");
    pthread_exit(NULL);
}

// addr1 should support change address request
// ret
// 0: Port in use
// 1: STUN server resp error
// 2: UDP blocked
// 3: Symmetric Firewall
// 4: Open Internet
// 5: Full-cone NAT
// 6: Symmetric NAT
// 7: Restricted port NAT
// 8: Restricted NAT
int examineNetworkEnvironment(ip_address *addr1, ip_address *addr2, unsigned short local_port, ip_address *global_ip_addr_ret) {
    if(checkPort(local_port)) {
        fprintf(stdlog1, "<Error>\tPort in use\n");
        return 0;
    }
    
    ip_address global_ip_addr;
    srand(time(NULL));

    // Test 1
    pthread_t worker_test1, worker_test2, worker_test3;
    worker_args args[3];
    unsigned short random_port = local_port;
    args[0].addr = addr1;
    args[0].change_addr = args[0].change_port = false;
    args[0].connection_try_limit = 5;
    args[0].local_port = local_port;
    pthread_create(&worker_test1, NULL, _stun_test_worker, &args[0]);
    
    args[1].addr = addr1;
    args[1].change_addr = args[1].change_port = true;
    args[1].connection_try_limit = 2;
    while(random_port == local_port || checkPort(random_port)) random_port = rand() % 5000 + 20000;
    args[1].local_port = random_port;
    pthread_create(&worker_test2, NULL, _stun_test_worker, &args[1]);
    
    args[2].addr = addr1;
    args[2].change_addr = false;
    args[2].change_port = true;
    args[2].connection_try_limit = 2;
    while(random_port == local_port || random_port == args[1].local_port || checkPort(random_port)) random_port = rand() % 5000 + 20000;
    args[2].local_port = random_port;
    pthread_create(&worker_test3, NULL, _stun_test_worker, &args[2]);

    pthread_join(worker_test1, NULL);
    printVerbose(1, &args[0], &args[0].ret);
    if(args[0].ret.rv & 0xFFFF) {
        // UDP blocked
        pthread_cancel(worker_test2);
        pthread_cancel(worker_test3);
        return 2;
    }

    // Resp error
    if(args[0].ret.rv & 0xFFFF0000) {
        pthread_cancel(worker_test2);
        pthread_cancel(worker_test3);
        return 1;
    }

    memcpy(&global_ip_addr, &(args[0].ret.addr), sizeof(ip_address));
    if(global_ip_addr_ret != NULL) memcpy(global_ip_addr_ret, &global_ip_addr, sizeof(ip_address));

    if(isSameAsLinkIP(&global_ip_addr)) {
        // No NAT
        pthread_join(worker_test2, NULL);
        printVerbose(2, &args[1], &args[1].ret);
        if(args[1].ret.rv & 0xFFFF) {
            // Symmetric Firewall
            pthread_cancel(worker_test2);
            pthread_cancel(worker_test3);
            return 3;
        } else {
            // Open Internet
            pthread_cancel(worker_test2);
            pthread_cancel(worker_test3);
            return 4;
        }
    } else {
        // NAT detected
        pthread_join(worker_test2, NULL);
        printVerbose(2, &args[1], &args[1].ret);
        if(args[1].ret.rv & 0xFFFF) {
            args[0].addr = addr2;
            pthread_create(&worker_test1, NULL, _stun_test_worker, &args[0]);
            pthread_join(worker_test1, NULL);
            printVerbose(1, &args[0], &args[0].ret);
            if(args[0].ret.rv & 0xFFFF0000) {
                // Resp error
                pthread_cancel(worker_test3);
                return 1;
            }
            if(strcmp(args[0].ret.addr.addr, global_ip_addr.addr) || args[0].ret.addr.port != global_ip_addr.port) {
                // Symmetric NAT
                pthread_cancel(worker_test3);
                return 6;
            } else {
                pthread_join(worker_test3, NULL);
                printVerbose(3, &args[2], &args[2].ret);
                if(args[2].ret.rv & 0xFFFF) {
                    // Restricted port NAT
                    return 7;
                } else {
                    // Restricted NAT
                    return 8;
                }
            }
        } else {
            // Full-cone NAT
            pthread_cancel(worker_test3);
            return 5;
        }
    }
    
    return 0;
}

// ret
// 0: no error
// 1: response error
// 2: no ip found
int getGlobalIPAddr(unsigned char* buf, ip_address *addr) {
    if(0x0101 != ntohs(*(unsigned short *)&buf[0])) {
        return 1;
    }
    
    bool found_ip = false;
    unsigned short msg_len = ntohs(*(unsigned short *)&buf[2]) + 20;
    for(unsigned short idx = 20; idx < msg_len;) {
        unsigned short attr_type = ntohs(*(unsigned short *)&buf[idx]);
        unsigned short attr_len = ntohs(*(unsigned short *)&buf[idx+2]);

        if(attr_type == 0x0001) {
            found_ip = true;
            if(buf[idx+5] == 0x01) {
                addr->ip_family = AF_INET;
                sprintf(addr->addr, "%hu.%hu.%hu.%hu", buf[idx+8], buf[idx+9], buf[idx+10], buf[idx+11]);
            } else {
                addr->ip_family = AF_INET6;
                sprintf(addr->addr, "%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X", *(unsigned short *)&buf[idx+8], *(unsigned short *)&buf[idx+12],
                    *(unsigned short *)&buf[idx+16], *(unsigned short *)&buf[idx+20], *(unsigned short *)&buf[idx+24], *(unsigned short *)&buf[idx+28],
                    *(unsigned short *)&buf[idx+32], *(unsigned short *)&buf[idx+36]);
            }
            addr->port = ntohs(*(unsigned short*)&buf[idx+6]);
        } else if(attr_type == 0x0020 || attr_type == 0x8020) {
            if(buf[idx+5] == 0x01) {
                addr->ip_family = AF_INET;
                sprintf(addr->addr, "%hu.%hu.%hu.%hu", buf[idx+8]^0x21, buf[idx+9]^0x12, buf[idx+10]^0xA4, buf[idx+11]^0x42);
            } else {
                addr->ip_family = AF_INET6;
                sprintf(addr->addr, "%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X", (*(unsigned short *)&buf[idx+8])^0x2112, (*(unsigned short *)&buf[idx+12])^0xA442,
                    (*(unsigned short *)&buf[idx+16])^0x63C7, (*(unsigned short *)&buf[idx+20])^0x117E, (*(unsigned short *)&buf[idx+24])^0x0714,
                    (*(unsigned short *)&buf[idx+28])^0x278F, (*(unsigned short *)&buf[idx+32])^0x5DED, (*(unsigned short *)&buf[idx+36])^0x3221);
            }
            addr->port = ntohs(*(unsigned short*)&buf[idx+6]) ^ 0x2112;
            return 0;
        }
        idx += (4 + attr_len);
    }
    return found_ip ? 0 : 2;
}

// ret
// 0: no error
// 1: tranlation error
int translateNetworkType(int code, char* type) {
    switch (code)
    {
    case 2:
        strcpy(type, "UDP blocked");
        break;
    case 3:
        strcpy(type, "Symmetric Firewall");
        break;
    case 4:
        strcpy(type, "Open Internet");
        break;
    case 5:
        strcpy(type, "Full-cone NAT");
        break;
    case 6:
        strcpy(type, "Symmetric NAT");
        break;
    case 7:
        strcpy(type, "Restricted port NAT");
        break;
    case 8:
        strcpy(type, "Restricted NAT");
    default:
        strcpy(type, "Unknown");
        return 1;
    }
    return 0;
}

void printVerbose(int test_no, worker_args* args, worker_ret* ret) {
    if(ret->rv)
    fprintf(stdlog1, "=======================================================\n"
                    "Test %d Start\n"
                    "STUN Server Address: \t%s:%d\n"
                    "Local Port: \t\t%d\n"
                    "Change Address: \t%s\n"
                    "Change Port: \t\t%s\n"
                    "Result: \t\tFailed\n"
                    "=======================================================\n",
                    test_no, args->addr->addr, args->addr->port, args->local_port, args->change_addr ? "True" : "False",
                    args->change_port ? "True" : "False");
    else
    fprintf(stdlog1, "=======================================================\n"
                    "Test %d Start\n"
                    "STUN Server Address: \t%s:%d\n"
                    "Local Port: \t\t%d\n"
                    "Change Address: \t%s\n"
                    "Change Port: \t\t%s\n"
                    "Result: \t\tSucceeded\n"
                    "Global IP Address: \t%s:%d\n"
                    "=======================================================\n",
                    test_no, args->addr->addr, args->addr->port, args->local_port, args->change_addr ? "True" : "False",
                    args->change_port ? "True" : "False", ret->addr.addr, ret->addr.port);
}