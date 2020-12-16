#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <uuid.h>

#include <string.h>
#include <unordered_map>
#include <netinet/in.h>
#include <pthread.h>
#include <map>
#include <unistd.h>

extern FILE *stdlog1;
extern FILE *stdlog2;

struct mapping_entry {
    sockaddr_in addr;
    int connected;
};

void* routine_worker_func(void *args);

class Profile {
    public:
        sockaddr_in global_ip;
        UUID uuid;
        std::map<UUID, mapping_entry> ip_mapping;
        pthread_t routine_thread;
        int sockfd;
        
        Profile();

        ~Profile();

        void setSockfd(int sockfd);

        void setGlobalIp(sockaddr_in &addr);

        void addIpMapping(UUID &uuid, sockaddr_in &addr);

        void removeIpMapping(UUID &uuid);
};

#endif