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


#include "list.h"
#include "struct.h"
#include "initialization.h"
#include "threadFunction.h"
#include  "signal.h"

ClientListNode* clientL;
ThreadListNode* threadL;
pthread_mutex_t clientLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condVar;

int main(int argc, char** argv) {
    int port, sock, newsock, status;
    struct sockaddr_in server, client;
    socklen_t clientlen = sizeof(client);
    struct sockaddr* clientptr = (struct sockaddr*) &client;
    struct sockaddr* serverptr = (struct sockaddr*) &server;
    pthread_cond_init(&condVar, NULL);

    static struct sigaction exitActions;
    exitActions.sa_handler = exitSignalHandler;
    sigaddset(&(exitActions.sa_mask), SIGINT);
    sigaction(SIGINT, &exitActions, NULL);

    clientL = createClientList();
    threadL = createThreadList();

    port = PortNumber(argc, argv);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    initServer(port, &server);
    bind(sock, serverptr, sizeof(server));
    listen(sock, 5);

    while ( 1 ) {
        newsock = accept(sock, clientptr, &clientlen);

        addThreadToList(threadL);
        pthread_create(&threadL->nextNode->thread, NULL, childServer, &newsock);

        pthread_mutex_lock(&clientLock);
        pthread_cond_wait(&condVar, &clientLock);
        printClientList(clientL);
        addThreadToList(threadL);
        pthread_create(&threadL->nextNode->thread, NULL, userOn, NULL);
        pthread_mutex_unlock(&clientLock);
    }

}


