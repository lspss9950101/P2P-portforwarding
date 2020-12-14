#include <stun.h>
#include <utils.h>

FILE *stdlog1 = stdout;
FILE *stdlog2 = stdout;

int main() {
    sockaddr_in addr1, global_ip;
    memset(&addr1, 0, sizeof(sockaddr_in));
    getIPFromHost("stun.ekiga.net", DNS_QUERY_IPV4 | DNS_QUERY_RET_FIRST, &addr1, NULL);
    addr1.sin_port = htons(3478);
    unsigned char buf[1024];
    sendSTUNPacket(&addr1, false, false, 5000, 5, buf, 1024);
    dumpBuffer(buf, 20);
}