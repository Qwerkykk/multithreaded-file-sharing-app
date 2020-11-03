#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "pool.h"
#include "struct.h"


pthread_mutex_t poolMutex_p;
pthread_cond_t poolNoFull_p;
pthread_cond_t poolNoEmpty_p;

int flag;

void initializePool(Pool_t* pool, const Arguments* arguments) {
    pool->data = malloc(arguments->bufferSize * sizeof(PoolItem));
    pool->start = 0;
    pool->end = -1;
    pool->count = 0;
    pool->size = arguments->bufferSize;
}

void obtain(Pool_t* pool, PoolItem* item) {
    pthread_mutex_lock(&poolMutex_p);
    while ( pool->count <= 0 ) {
        pthread_cond_wait(&poolNoEmpty_p, &poolMutex_p);
        if ( flag == 1 ) {
            pthread_mutex_unlock(&poolMutex_p);
            pthread_exit(NULL);
        }
    }

    item->port = pool->data[pool->start].port;
    strcpy(item->IP, pool->data[pool->start].IP);
    strcpy(item->pathname, pool->data[pool->start].pathname);
    item->version = pool->data[pool->start].version;
    pool->start = (pool->start + 1) % pool->size;
    pool->count--;

    pthread_mutex_unlock(&poolMutex_p);
}

void place(Pool_t* pool, PoolItem* item) {

    pthread_mutex_lock(&poolMutex_p);
    while ( pool->count >= pool->size ) {
        pthread_cond_wait(&poolNoFull_p, &poolMutex_p);
        if ( flag == 1 ) {
            pthread_mutex_unlock(&poolMutex_p);
            pthread_exit(NULL);
        }
    }

    pool->end = (pool->end + 1) % pool->size;
    pool->data[pool->end].port = item->port;
    strcpy(pool->data[pool->end].IP, item->IP);
    strcpy(pool->data[pool->end].pathname, item->pathname);
    pool->data[pool->end].version = item->version;
    pool->count++;

    pthread_mutex_unlock(&poolMutex_p);
}

void clientListToPool(Pool_t* pool, ClientListNode* clientList) {
    char buffer[32];
    PoolItem item;
    item.pathname[0] = '\0';
    DIR* directoryPointer;

    ClientListNode* parser;
    parser = clientList->nextNode;


    while ( parser != NULL) {

        strcpy(item.IP, parser->IP);
        item.port = parser->port;

        sprintf(buffer, "%s_%u", item.IP, item.port);
        if ((directoryPointer = opendir(buffer)) == NULL) {
            mkdir(buffer, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
        }

        closedir(directoryPointer);
        place(pool, &item);
        parser = parser->nextNode;

    }
}
