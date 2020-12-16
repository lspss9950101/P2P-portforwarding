#include <profile.h>

Profile::Profile() {
    uuid = UUID::random();
    memset(&global_ip, 0, sizeof(sockaddr_in));
    this->sockfd = -1;

    pthread_create(&this->routine_thread, NULL, routine_worker_func, this);
}

void Profile::setSockfd(int sockfd) {
    this->sockfd = sockfd;
}

void Profile::setGlobalIp(sockaddr_in &addr) {
    this->global_ip = addr;
}

void Profile::addIpMapping(UUID &uuid, sockaddr_in &addr) {
    this->ip_mapping[uuid] = {addr, 3};
}

void Profile::removeIpMapping(UUID &uuid) {
    this->ip_mapping.erase(uuid);
}