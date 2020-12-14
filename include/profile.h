#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <uuid.h>

#include <unordered_map>
#include <netinet/in.h>

class Profile {
    public:
        sockaddr_in global_ip;
        UUID uuid;
        map<UUID, sockaddr_in> ip_mapping;
        
        void addIpMapping(UUID uuid, sockaddr_in addr);

        void removeIpMapping(UUID uuid);
}

#endif