#ifndef SYSPRO_PROJECT3_SERVER_STRUCT_H
#define SYSPRO_PROJECT3_SERVER_STRUCT_H

#include <pthread.h>
#include <netinet/in.h>

typedef struct clientListNode ClientListNode;
typedef struct threadListNode ThreadListNode;

struct clientListNode {

    char IP[INET_ADDRSTRLEN];
    uint16_t port;
    int socket;
    pthread_mutex_t socketMutex;
    ClientListNode* nextNode;
};

struct threadListNode{
    pthread_t thread;
    ThreadListNode* nextNode;
};

#endif //SYSPRO_PROJECT3_SERVER_STRUCT_H
