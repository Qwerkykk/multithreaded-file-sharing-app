#ifndef SYSPRO_PROJECT3_CLIENT_STRUCT_H
#define SYSPRO_PROJECT3_CLIENT_STRUCT_H

#include <arpa/inet.h>

typedef struct clientListNode ClientListNode;

typedef struct logOnInfo{
    char myIP[INET_ADDRSTRLEN];
    uint16_t myPort;
}LogOnInfo;


struct clientListNode {

    char IP[INET_ADDRSTRLEN];
    uint16_t port;
    int socket;
    pthread_mutex_t socketMutex;
    ClientListNode* nextNode;
};


typedef struct arguments{
    char* dirName;
    uint16_t portNum;
    char* serverIP;
    int workerThreads;
    int bufferSize;
    int serverPort;
}Arguments;

typedef struct poolItem{
    char pathname[128];
    uint64_t version;
    char IP[INET_ADDRSTRLEN];
    uint16_t port;

}PoolItem;

typedef struct pool_t{
    PoolItem* data;
    int start;
    int end;
    int count;
    int size;
} Pool_t;

#endif
