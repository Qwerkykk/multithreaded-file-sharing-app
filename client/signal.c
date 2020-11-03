#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include "signal.h"
#include "struct.h"


LogOnInfo* logOnInfo_;
int sock__s;

Pool_t pool_s;
pthread_mutex_t poolMutex_;

ClientListNode* clientList__s;
pthread_mutex_t clientMutex__s;
Arguments arguments;

pthread_t* threads;
int* socketArray;

pthread_cond_t poolNoFull_s;
pthread_cond_t poolNoEmpty_s;

int flag_s;


void exitSignalHandler() {
    char buffer[32];
    uint32_t inetIP;
    uint16_t inetPort;

    inet_pton(AF_INET, logOnInfo_->myIP, &inetIP);
    inetPort = htons(logOnInfo_->myPort);


    memcpy(buffer, "LOG_OFF", 8);
    memcpy(buffer + 8, &inetIP, sizeof(uint32_t));
    memcpy(buffer + 8 + sizeof(uint32_t), &inetPort, sizeof(uint16_t));

    write(sock__s, buffer, 8 + sizeof(uint32_t) + sizeof(uint16_t));

    free(logOnInfo_);
    flag_s = 1;

    pthread_cond_broadcast(&poolNoEmpty_s);
    pthread_cond_broadcast(&poolNoFull_s);


    free(threads);

    pthread_mutex_lock(&clientMutex__s);
    ClientListNode* parser = clientList__s->nextNode;
    ClientListNode* previous = clientList__s;

    while(parser!=NULL){
            free(previous);
            previous = parser;
            parser = parser->nextNode;
        }

    free(previous);

    free(arguments.dirName);
    free(arguments.serverIP);

    free(socketArray);

     pthread_mutex_lock(&poolMutex_);
    free(pool_s.data);

    exit(1);
}