#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <utime.h>

#include "socketServe.h"
#include "struct.h"
#include "list.h"
#include "initialization.h"
#include "pool.h"
#include "signal.h"
#include "workerThread.h"


Pool_t pool_;
pthread_mutex_t poolMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t poolNoFull_;
pthread_cond_t poolNoEmpty_;

ClientListNode* clientList_;
pthread_mutex_t clientMutex_ = PTHREAD_MUTEX_INITIALIZER;

fd_set socks_;

int* socketArray_;
int socketArrayLen_;
int sock, listeningSock;

Arguments arguments_;
LogOnInfo* logOnInfo;
pthread_t* threads_;

int flag_;

void main(int argc, char* argv[]) {
    int nClients;
    char buffer[256];
    char command[50];
    flag_ = 0;
    struct sockaddr_in server;
    struct sockaddr_in client;
    struct sockaddr* serverptr = (struct sockaddr*) &server;;
    struct hostent* rem;
    struct timeval timeout;
    int reuse_addr = 1;

    static struct sigaction exitActions;
    exitActions.sa_handler = exitSignalHandler;
    sigaddset(&(exitActions.sa_mask), SIGINT);
    sigaction(SIGINT, &exitActions, NULL);

    pthread_cond_init(&poolNoFull_, NULL);
    pthread_cond_init(&poolNoEmpty_, NULL);

    if ( argc != 13 ) {
        printf("Wrong number of arguments_\n");
        exit(1);
    }

    initArguments(&arguments_, argv);

    initializePool(&pool_, &arguments_);

    socketArrayLen_ = FD_SETSIZE;
    socketArray_ = malloc(socketArrayLen_ * sizeof(int));

    for ( int i = 0; i < socketArrayLen_; i++ )
        socketArray_[i] = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        perror_exit("socket");

    if ((rem = gethostbyname(arguments_.serverIP)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }

    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(arguments_.serverPort);         /* Server port */

    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons(arguments_.portNum);

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

    if ( bind(sock, (struct sockaddr*) &client, sizeof(struct sockaddr_in)) != 0 ) {
        printf("Bind Failed\n");
        exit(1);
    }

    if ( connect(sock, serverptr, sizeof(server)) < 0 )
        perror_exit("connect");

    printf("Connecting to %s port %u\n", arguments_.serverIP, arguments_.serverPort);

    int readSocks;
    struct sockaddr_in listeningClient;
    struct sockaddr* listeningClientptr = (struct sockaddr*) &listeningClient;
    int listeningClientLen = sizeof(listeningClient);

    listeningSock = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(listeningSock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    fcntl(listeningSock, F_SETFL, O_NONBLOCK);

    listeningClient.sin_family = AF_INET;
    listeningClient.sin_addr.s_addr = htonl(INADDR_ANY);
    listeningClient.sin_port = htons(arguments_.portNum);

    if ( bind(listeningSock, listeningClientptr, listeningClientLen) != 0 ) {
        printf("Bind Failed\n");
        exit(1);
    }

    listen(listeningSock, 10);

    logOnInfo = getLogOnInfo(sock);

    //Send Log_On
    logOnServer(sock, logOnInfo);

    //Send Get_Clients
    sprintf(buffer, "GET_CLIENTS");
    write(sock, buffer, 12);

    //Receive Client List
    read(sock, buffer, 12 + sizeof(int));
    memcpy(command, buffer, 12);
    memcpy(&nClients, buffer + 12, sizeof(int));

    printf("Message: %s N: %d\n", command, nClients);

    clientList_ = createClientList();

    getClientsToList(sock, nClients, clientList_);

    clientListToPool(&pool_, clientList_);

    printClientList(clientList_);



    fcntl(sock, F_SETFL, O_NONBLOCK);

    threads_ = malloc(arguments_.workerThreads*sizeof(pthread_t));

    for ( int i = 0; i < arguments_.workerThreads; i++ )
        pthread_create(&threads_[i], NULL, workersDuty, NULL);




    socketArray_[0] = sock;
    while ( 1 ) {
        buildSelectList();
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        readSocks = select(FD_SETSIZE, &socks_, NULL, NULL, &timeout);
        if ( readSocks < 0 ) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if ( readSocks == 0 ) {
            printf(".\n");
        }
        else {
            ReadSocks();
        }
    }
    
}



