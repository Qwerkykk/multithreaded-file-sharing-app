#include <arpa/inet.h>
#include <string.h>             /* strlen */
#include <stdlib.h>             /* exit */
#include <netdb.h>             /* gethostbyaddr */
#include <unistd.h>          /* read, write, close */
#include <netinet/in.h>         /* internet sockets */
#include <sys/socket.h>         /* sockets */
#include <sys/types.h>         /* sockets */
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

#include "socketServe.h"
#include "initialization.h"
#include "list.h"
#include "struct.h"
#include "pool.h"

int sock_s, listeningSock_s, socketArrayLen;
fd_set socks;
int* socketArray_s;
pthread_mutex_t clientMutex_s;
ClientListNode* clientList_s;
Arguments arguments_s;
Pool_t pool;
pthread_cond_t poolNoEmpty;


void getClientsToList(int sock_s, int nClients, ClientListNode* clientList_s) {
    char buffer[8];
    char clientIp[INET_ADDRSTRLEN];
    uint16_t clientPort;
    uint16_t inetPort;
    while ( nClients > 0 ) {
        read(sock_s, buffer, sizeof(uint32_t) + sizeof(uint16_t));
        memcpy(&inetPort, buffer + sizeof(uint32_t), sizeof(uint16_t));
        inet_ntop(AF_INET, &buffer, clientIp, INET_ADDRSTRLEN);
        clientPort = ntohs(inetPort);

        insertFirstC(clientList_s, clientIp, clientPort, -1);

        nClients--;
    }
}

void logOnServer(int sock_s, const LogOnInfo* logOnInfo) {//Send LOG_ON
    uint32_t networkIP;
    uint16_t networkPort;
    char buffer[16];
    inet_pton(AF_INET, logOnInfo->myIP, &networkIP);
    networkPort = htons(logOnInfo->myPort);
    sprintf(buffer, "LOG_ON");
    memcpy(buffer + 7, &networkIP, sizeof(uint32_t));
    memcpy(buffer + 7 + sizeof(uint32_t), &networkPort, sizeof(uint16_t));
    write(sock_s, buffer, 7 + sizeof(uint32_t) + sizeof(uint16_t));
}

LogOnInfo* getLogOnInfo(int sock_s) {
    LogOnInfo* logOnInfo = malloc(sizeof(LogOnInfo));
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    unsigned int length = sizeof(client);

    getsockname(sock_s, (struct sockaddr*) &client, &length);

    inet_ntop(AF_INET, &client.sin_addr, logOnInfo->myIP, sizeof(logOnInfo->myIP));
    logOnInfo->myPort = ntohs(client.sin_port);
    return logOnInfo;
}

void buildSelectList() {

    FD_ZERO(&socks);

    FD_SET(sock_s, &socks);
    FD_SET(listeningSock_s, &socks);

    for ( int i = 0; i < socketArrayLen; i++ ) {
        if ( socketArray_s[i] != 0 ) {
            FD_SET(socketArray_s[i], &socks);
        }
    }
}


void ReadSocks() {

    if ( FD_ISSET(listeningSock_s, &socks))
        handleNewConnection();

    for ( int i = 0; i < socketArrayLen; i++ ) {
        if ( FD_ISSET(socketArray_s[i], &socks))
            dealWithData(i);
    }

}

void handleNewConnection() {
    int connection;
    int reuse_addr = 1;
    connection = accept(listeningSock_s, NULL, NULL);
    if ( connection < 0 ) {
        perror("connection\n");
        exit(EXIT_FAILURE);
    }

    setsockopt(connection, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    fcntl(connection, F_SETFL, O_NONBLOCK);

    for ( int i = 0; (i < socketArrayLen) && (connection != -1); i++ ) {
        if ( socketArray_s[i] == 0 ) {
            printf("Connection From Other Client Accepted! %d\n", connection);
            socketArray_s[i] = connection;
            connection = -1;
        }
    }

    if ( connection != -1 ) {
        printf("No room left for the new client\n");
        close(connection);
    }


}


void dealWithData(int listNum) {
    char buffer[256];
    int bytesReceived, bytesRead;
    if ((bytesReceived = read(socketArray_s[listNum], buffer, 256)) <= 0 )
        return;

    buffer[bytesReceived] = '\0';

    bytesRead = 0;
    while ( bytesRead != bytesReceived ) {
        char word[256];
        char clientIP[INET_ADDRSTRLEN];
        uint16_t inetPort;


        for ( int i = 0; i < bytesReceived; i++ ) {
            word[i] = buffer[bytesRead];
            bytesRead++;
            if ( word[i] == ' ' || word[i] == '\0' || word[i] == '\n' ) {
                word[i] = '\0';
                break;
            }
        }

        if ( strcmp(word, "USER_ON") == 0 ) {
            PoolItem poolItem;
            inet_ntop(AF_INET, &buffer[8], poolItem.IP, INET_ADDRSTRLEN);
            memcpy(&inetPort, &buffer[8 + sizeof(uint32_t)], sizeof(uint16_t));
            bytesRead += sizeof(uint32_t) + sizeof(uint16_t);

            poolItem.port = ntohs(inetPort);

            insertFirstC(clientList_s, poolItem.IP, poolItem.port, -1);
            printClientList(clientList_s);

            poolItem.pathname[0] = '\0';

            sprintf(buffer, "%s_%u", poolItem.IP, poolItem.port);
            if ( opendir(buffer) == NULL) {
                mkdir(buffer, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
            }

            place(&pool, &poolItem);
            pthread_cond_signal(&poolNoEmpty);

            if ( bytesReceived == bytesRead )
                return;

            continue;

        }

        if ( strcmp(word, "GET_FILE_LIST") == 0 ) {
            responseToGetFileList(listNum, buffer);

            close(socketArray_s[listNum]);
            socketArray_s[listNum] = 0;
            return;
        }

        if ( strcmp(word, "GET_FILE") == 0 ) {
            uint64_t version, myVersion;

            char path[128];


            memcpy(path, buffer + 9, 128);
            memcpy(&version, buffer + 137, sizeof(uint64_t));

            bytesRead += 128 + sizeof(uint64_t);

            printf("\n<LOG> Asked For Path: %s Version: %llu\n", path, version);
            if ( access(path, F_OK) == -1 ) {
                sprintf(buffer, "FILE_NOT_FOUND");
                write(sock_s, buffer, 15);
                close(socketArray_s[listNum]);
                socketArray_s[listNum] = 0;
                return;
            }

            struct stat fileStat;

            stat(path, &fileStat);
            myVersion = fileStat.st_mtime;

            if ( myVersion == version ) {
                sprintf(buffer, "FILE_UP_TO_DATE");
                write(socketArray_s[listNum], buffer, 16);
                close(socketArray_s[listNum]);
                socketArray_s[listNum] = 0;
                break;
            }

            sendFileToSock(socketArray_s[listNum], buffer, &myVersion, path);

            close(socketArray_s[listNum]);
            socketArray_s[listNum] = 0;
            return;
        }

        if ( strcmp(word, "USER_OFF") == 0 ) {

            inet_ntop(AF_INET, &buffer[9], clientIP, INET_ADDRSTRLEN);
            memcpy(&inetPort, &buffer[9 + sizeof(uint32_t)], sizeof(uint16_t));
            bytesRead += sizeof(uint32_t) + sizeof(uint16_t);

            pthread_mutex_lock(&clientMutex_s);

            deleteClientFromList(clientList_s, clientIP, ntohs(inetPort));
            printClientList(clientList_s);
            pthread_mutex_unlock(&clientMutex_s);


            if ( bytesRead == bytesReceived )
                return;

            continue;
        }
    }
}

void sendFileToSock(int sock_s, char* buffer, uint64_t* myVersion, char* path) {
    int fileSize;
    FILE* filePointer;

    filePointer = fopen(path, "r");
    fileSize = getSizeOfFile(filePointer);

    sprintf(buffer, "FILE_SIZE");

    memcpy(buffer + 10, myVersion, sizeof(uint64_t));
    memcpy(buffer + 10 + sizeof(uint64_t), &fileSize, sizeof(int));


    write(sock_s, buffer, 10 + sizeof(uint64_t) + sizeof(int));

    char c;
    int bytesRead1;
    while ( fileSize > 0 ) {
        bytesRead1 = 0;
        for ( int i = 0; i < 256; i++ ) {

            c = (char) fgetc(filePointer);
            if ( feof(filePointer))
                break;

            buffer[i] = c;
            bytesRead1++;
        }

        if ( bytesRead1 > 0 )
            write(sock_s, buffer, bytesRead1);

        fileSize -= bytesRead1;
    }
    fclose(filePointer);
}

void responseToGetFileList(int socketNum, char* buffer) {
    int fileCounter;

    fileCounter = getNumOfFiles(arguments_s.dirName);

    sprintf(buffer, "FILE_LIST");
    memcpy(buffer + 10, &fileCounter, sizeof(int));
    write(socketArray_s[socketNum], buffer, 10 + sizeof(int));

    memset(buffer, 0, 256);

    sprintf(buffer, "%s/", arguments_s.dirName);

    write(socketArray_s[socketNum], buffer, 128 + sizeof(uint64_t));

    sendFilePathsToSock(arguments_s.dirName, socketArray_s[socketNum]);
}

void sendFilePathsToSock(const char* path, int sock_s) {
    DIR* directoryPointer;
    struct dirent* directoryContent;
    directoryPointer = opendir(path);
    char extendedPath[128];
    char buffer[128 + sizeof(uint64_t)];

    struct stat fileStat;

    while ((directoryContent = readdir(directoryPointer)) != NULL) {
        if ( directoryContent->d_name[0] == '.' )
            continue;

        memset(extendedPath, 0, 128);
        memset(buffer, 0, 128 + sizeof(uint64_t));
        sprintf(extendedPath, "%s/%s", path, directoryContent->d_name);
        if ( isDirectory(extendedPath) == 0 ) {

            stat(extendedPath, &fileStat);

            sprintf(extendedPath, "%s/", extendedPath);
            memcpy(buffer, extendedPath, 128);
            memcpy(buffer + 128, &fileStat.st_mtime, sizeof(uint64_t));
            write(sock_s, buffer, 128 + sizeof(uint64_t));
            printf("%s\n", buffer);
            extendedPath[strlen(extendedPath) - 1] = '\0';

            sendFilePathsToSock(extendedPath, sock_s);
            
            continue;
        }

        stat(extendedPath, &fileStat);


        memcpy(buffer, extendedPath, 128);
        printf("\n<LOG> Sending Path:%s Version:%llu \n", buffer, (unsigned long long) fileStat.st_mtime);

        memcpy(buffer + 128, &fileStat.st_mtime, sizeof(uint64_t));

        write(sock_s, buffer, 128 + sizeof(uint64_t));

    }

    closedir(directoryPointer);

}



