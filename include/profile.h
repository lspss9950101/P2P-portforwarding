#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <uuid.h>

#include <string.h>
#include <unordered_map>
#include <netinet/in.h>
#include <map>

class Profile {
    public:
        sockaddr_in global_ip;
        UUID uuid;
        std::map<UUID, sockaddr_in> ip_mapping;
        
        Profile();

        void setGlobalIp(sockaddr_in &addr);

        void addIpMapping(UUID &uuid, sockaddr_in &addr);

        void removeIpMapping(UUID &uuid);
};

#endif