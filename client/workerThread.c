#include <utime.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>

#include "workerThread.h"
#include "pool.h"
#include "initialization.h"
#include "list.h"
#include "struct.h"
#include "socketServe.h"

Pool_t pool_w;
pthread_mutex_t poolMutex_w;
pthread_cond_t poolNoFull_w;
pthread_cond_t poolNoEmpty_w;

ClientListNode* clientList;
pthread_mutex_t clientMutex;




void* workersDuty() {

    int sock;
    char buffer[256];
    char dirPath[256];
    struct sockaddr_in server;
    struct sockaddr* serverptr = (struct sockaddr*) &server;
    struct hostent* rem;
    int reuse_addr = 1;
    PoolItem poolItem;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

    while ( 1 ) {
        poolItem.pathname[0] = '\0';

        obtain(&pool_w, &poolItem);
        pthread_cond_signal(&poolNoFull_w);



        if ( poolItem.pathname[0] == '\0' ) {


            pthread_mutex_lock(&clientMutex);
            if ( getClient(clientList, poolItem.IP, poolItem.port) == NULL) {
                printf("Client Not Found\n");
                pthread_mutex_unlock(&clientMutex);
                continue;
            }

            pthread_mutex_unlock(&clientMutex);

            sock = getConnection(sock, poolItem.IP, poolItem.port);

            sprintf(buffer, "GET_FILE_LIST");
            write(sock, buffer, 14);

            getFileList(sock, buffer, dirPath, &poolItem);

        }
        else {

            pthread_mutex_lock(&clientMutex);
            if ( getClient(clientList, poolItem.IP, poolItem.port) == NULL) {
                printf("Client Not Found\n");
                pthread_mutex_unlock(&clientMutex);
                continue;
            }
            pthread_mutex_unlock(&clientMutex);

            sprintf(buffer, "./%s_%u%s", poolItem.IP, poolItem.port, poolItem.pathname + 1);

            printf("\n<LOG> Asking Path:%s Version:%llu\n", poolItem.pathname,
                   (unsigned long long) poolItem.version);

            if ( access(buffer, F_OK) != -1 ) {
                askForFile(sock, buffer, &poolItem, 0);
            }
            else {
                askForFile(sock, buffer, &poolItem, 1);

            }
        }
    }
}

void getFileList(int sock, char* buffer, char* dirPath, PoolItem* poolItem) {
    uint32_t nFiles;

    while ( read(sock, buffer, 10 + sizeof(uint32_t)) <= 0 ) {}

    memcpy(&nFiles, buffer + 10, sizeof(uint32_t));

    while ( nFiles >= 0 ) {
        while ( read(sock, buffer, 128 + sizeof(uint64_t)) <= 0 ) {}


        memset(poolItem->pathname, 0, 128);
        memcpy(&poolItem->version, buffer + 128, sizeof(uint64_t));
        strcpy(poolItem->pathname, buffer);

        if (poolItem->pathname[strlen(poolItem->pathname) - 1] == '/' ) {
            poolItem->pathname[strlen(poolItem->pathname) - 1] = '\0';
            sprintf(dirPath, "%s_%u%s", poolItem->IP, poolItem->port, poolItem->pathname + 1);

            if ( mkdir(dirPath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) == -1 ) {
                printf("Directory %s Already Exist\n", dirPath);

            }
            nFiles--;
            continue;
        }


        place(&pool_w, poolItem);
        pthread_cond_signal(&poolNoEmpty_w);

        nFiles--;
    }
}


void askForFile(int sock, char* buffer, PoolItem* poolItem, int fileExists) {
    int bytesReceived;
    uint64_t modifTime;
    int fileSize;
    char word[256];
    char buffer1[256];

    poolItem->version = 0;

    sock = getConnection(sock, poolItem->IP, poolItem->port);

    if ( fileExists == 0 ) {
        struct stat fileStat;
        stat(buffer, &fileStat);
        poolItem->version = fileStat.st_mtime;
    }
    memset(buffer1,0,256);
    sprintf(buffer1, "GET_FILE");
    memcpy(buffer1 + 9, poolItem->pathname, 128);
    memcpy(buffer1 + 137, &poolItem->version, sizeof(uint64_t));

    write(sock, buffer1, 137 + sizeof(uint64_t));

    while ((bytesReceived = read(sock, buffer1, 10 + sizeof(uint64_t) + sizeof(int))) <= 0 ) {}


    for ( int i = 0; i < bytesReceived; i++ ) {
        word[i] = buffer1[i];

        if ( word[i] == ' ' || word[i] == '\0' || word[i] == '\n' ) {
            word[i] = '\0';
            break;
        }
    }

    if ((strcmp(word, "FILE_NOT_FOUND") == 0) || (strcmp(word, "FILE_UP_TO_DATE") == 0)) {
        printf("\n<LOG> %s %s\n", poolItem->pathname, word);
        return;
    }

    memcpy(&modifTime, buffer1 + 10, sizeof(uint64_t));
    memcpy(&fileSize, buffer1 + 10 + sizeof(uint64_t), sizeof(int));

    printf("\n<LOG> Receiving Path:%s Version:%llu \n", poolItem->pathname, (unsigned long long) modifTime);

    sprintf(buffer1, "%s_%u/%s", poolItem->IP, poolItem->port, poolItem->pathname + 1);
    getFileFromSocket(sock, buffer1, fileSize);


    sprintf(buffer1, "%s_%u/%s", poolItem->IP, poolItem->port, poolItem->pathname + 1);

    struct utimbuf fileTime;

    fileTime.actime = modifTime;
    fileTime.modtime = modifTime;

    utime(buffer1, &fileTime);
}

void getFileFromSocket(int sock, char* buffer, int fileSize) {
    FILE* filePointer;
    filePointer = fopen(buffer, "w");
    int bytesReceived;
    while ( fileSize > 0 ) {

        while ((bytesReceived = read(sock, buffer, 256)) <= 0 ) {}

        for ( int i = 0; i < bytesReceived; i++ )
            fputc(buffer[i], filePointer);

        fileSize -= bytesReceived;
    }

    fclose(filePointer);
}


int getConnection(int sock, char* IP, uint16_t port) {
    struct hostent* rem;
    struct sockaddr_in server;
    struct sockaddr* serverptr = (struct sockaddr*) &server;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        perror_exit("socket");

    if ((rem = gethostbyname(IP)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }

    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);         /* Server port */

    if ( connect(sock, serverptr, sizeof(server)) < 0 )
        perror_exit("connect");

    return sock;
}