#include <profile.h>

Profile::Profile() {
    uuid = UUID::random();
    memset(&global_ip, 0, sizeof(sockaddr_in));
}

void Profile::setGlobalIp(sockaddr_in &addr) {
    this->global_ip = addr;
}

void Profile::addIpMapping(UUID &uuid, sockaddr_in &addr) {
    this->ip_mapping[uuid] = addr;
}

void Profile::removeIpMapping(UUID &uuid) {
    this->ip_mapping.erase(uuid);
}