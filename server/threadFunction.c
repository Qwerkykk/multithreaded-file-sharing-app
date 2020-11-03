#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "struct.h"
#include "threadFunction.h"
#include "list.h"

ClientListNode* clientList;
pthread_mutex_t clientListLock;
pthread_cond_t cvar;

void* childServer(void* newsock) {
    int sock = *(int*) newsock;
    int bytesReceived;
    int bytesRead;
    uint32_t inetPort;
    char word[256];
    char buffer[256];
    char hostedIP[INET_ADDRSTRLEN];
    uint16_t hostedPort;



    while ( 1 ) {
        bytesReceived = read(sock, buffer, 256);
        bytesRead = 0;
        if ( bytesReceived <= 0 ) {
            continue;
        }


        while ( bytesRead != bytesReceived ) {

            for (int i =0 ; i < bytesReceived; i++ ) {
                word[i] = buffer[bytesRead];
                bytesRead++;
                if ( word[i] == ' ' || word[i] == '\0' || word[i] == '\n' ) {
                    word[i] = '\0';
                    break;
                }
            }

            printf("COMMAND: %s\n",word);

            if ( strcmp(word, "LOG_ON") == 0 ) {

                inet_ntop(AF_INET, &buffer[7], hostedIP, INET_ADDRSTRLEN);
                memcpy(&inetPort, &buffer[7 + sizeof(uint32_t)], sizeof(uint16_t));
                hostedPort = ntohs(inetPort);

                bytesRead+= sizeof(uint32_t) + sizeof(uint16_t);

                pthread_mutex_lock(&clientListLock);
                insertFirstC(clientList, hostedIP, hostedPort, sock);
                pthread_cond_signal(&cvar);
                pthread_mutex_unlock(&clientListLock);

            }
            else if ( strcmp(word, "GET_CLIENTS") == 0) {
                uint32_t inetIp;
                uint16_t inetPort;

                pthread_mutex_lock(&clientListLock);
                ClientListNode* client = getClient(clientList, hostedIP, hostedPort);
                pthread_mutex_lock(&client->socketMutex);


                int nClients = numOfClients(clientList);
                nClients--;

                sprintf(buffer, "CLIENT_LIST");
                memcpy(buffer + 12, &nClients, sizeof(int));

                write(client->socket, buffer, 12 + sizeof(int));


                ClientListNode* parser = clientList->nextNode;

                while ( parser != NULL) {

                    if ((strcmp(parser->IP, hostedIP) == 0) && parser->port == hostedPort ) {
                        parser = parser->nextNode;
                        continue;
                    }

                    inet_pton(AF_INET, parser->IP, &inetIp);
                    inetPort = htons(parser->port);

                    memcpy(buffer, &inetIp, sizeof(uint32_t));
                    memcpy(buffer + sizeof(uint32_t), &inetPort, sizeof(uint16_t));

                    write(client->socket, buffer, sizeof(uint32_t) + sizeof(uint16_t));

                    parser = parser->nextNode;
                }


                pthread_mutex_unlock(&client->socketMutex);
                pthread_mutex_unlock(&clientListLock);
            }
            else if ( strcmp(word, "LOG_OFF") == 0 ) {
                bytesRead+=6;
                uint32_t inetIp;
                uint16_t inetPort;
                pthread_mutex_lock(&clientListLock);

                if ( deleteClientFromList(clientList, hostedIP, hostedPort) == 1 ) {

                    ClientListNode* client = getClient(clientList, hostedIP, hostedPort);
                    sprintf(buffer, "ERROR_IP_PORT_NOT_FOUND_IN_LIST");

                    pthread_mutex_lock(&client->socketMutex);
                    write(sock, buffer, 32);
                    pthread_mutex_unlock(&client->socketMutex);
                    continue;
                }

                sprintf(buffer, "USER_OFF");

                inet_pton(AF_INET, hostedIP, &inetIp);
                inetPort = htons(hostedPort);

                memcpy(buffer + 9, &inetIp, sizeof(uint32_t));
                memcpy(buffer + 9 + sizeof(uint32_t), &inetPort, sizeof(uint16_t));

                ClientListNode* parser = clientList->nextNode;

                while ( parser != NULL) {
                    write(parser->socket, buffer, sizeof(uint16_t)+ sizeof(uint32_t) + 9);
                    parser = parser->nextNode;
                }
                printClientList(clientList);
                pthread_mutex_unlock(&clientListLock);

            }
        }
    }
    printf("Closing  connection .\n");
    close(sock);
}

void* userOn(void* arg) {
    char buffer[128];
    uint32_t inetIp;
    uint16_t inetPort;
    ClientListNode* parser;

    pthread_mutex_lock(&clientListLock);
    parser = clientList->nextNode->nextNode;

    while ( parser != NULL) {
        inet_pton(AF_INET, clientList->nextNode->IP, &inetIp);
        inetPort = htons(clientList->nextNode->port);

        sprintf(buffer, "USER_ON");
        memcpy(buffer + 8, &inetIp, sizeof(uint32_t));
        memcpy(buffer + 8 + sizeof(uint32_t), &inetPort, sizeof(uint16_t));

        pthread_mutex_lock(&parser->socketMutex);
        write(parser->socket, buffer, 8 + sizeof(uint32_t) + sizeof(uint16_t));
        pthread_mutex_unlock(&parser->socketMutex);
        parser = parser->nextNode;
    }

    pthread_mutex_unlock(&clientListLock);

}