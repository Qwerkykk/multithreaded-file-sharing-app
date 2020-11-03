#include "list.h"
#include <pthread.h>

ClientListNode* createClientList() {
    ClientListNode* dummyNode;
    dummyNode = malloc(sizeof(ClientListNode));
    dummyNode->nextNode = NULL;

    return dummyNode;

}

ThreadListNode* createThreadList() {
    ThreadListNode* dummyNode;
    dummyNode = malloc(sizeof(ThreadListNode));
    dummyNode->nextNode = NULL;

    return dummyNode;

}

void addThreadToList(ThreadListNode* list) {
    ThreadListNode* newNode = malloc(sizeof(ThreadListNode));
    newNode->nextNode = list->nextNode;
    list->nextNode = newNode;

    return;
}

void insertFirstC(ClientListNode* list, char* ip, uint16_t port, int socket) {
    ClientListNode* newNode;

    newNode = malloc(sizeof(ClientListNode));

    strcpy(newNode->IP, ip);
    newNode->port = port;
    newNode->socket = socket;
    pthread_mutex_init(&newNode->socketMutex, NULL);
    newNode->nextNode = NULL;

    if ( list->nextNode == NULL) {
        list->nextNode = newNode;

    }
    else {
        newNode->nextNode = list->nextNode;
        list->nextNode = newNode;
    }

    return;

}

ClientListNode* getClient(ClientListNode* list, char* ip, uint16_t port) {
    ClientListNode* parser;

    parser = list->nextNode;

    while ( parser != NULL) {
        if ((strcmp(parser->IP, ip) == 0) && parser->port == port ) {
            return parser;
        }

        parser = parser->nextNode;
    }

    return NULL;
}

int numOfClients(ClientListNode* list) {
    int counter = 0;
    ClientListNode* parser;

    parser = list->nextNode;

    if ( parser == NULL)
        return 0;

    while ( parser != NULL) {
        counter++;
        parser = parser->nextNode;
    }

    return counter;
}

int deleteClientFromList(ClientListNode* clientList, char* ip, uint16_t port) {
    ClientListNode* currentNode;
    ClientListNode* previousNode;


    currentNode = clientList->nextNode;

    if ((strcmp(ip, currentNode->IP) == 0) && currentNode->port == port ) {

        clientList->nextNode = currentNode->nextNode;
        free(currentNode);
        return 0;
    }

    previousNode = currentNode;
    currentNode = currentNode->nextNode;
    while ( currentNode != NULL) {

        if ((strcmp(ip, currentNode->IP) == 0) && currentNode->port == port ) {

            previousNode->nextNode = currentNode->nextNode;
            free(currentNode);
            return 0;
        }

        previousNode = currentNode;
        currentNode = currentNode->nextNode;

    }

    return 1;
}


void printClientList(ClientListNode* list) {
    ClientListNode* parser;
    parser = list->nextNode;

    while ( parser != NULL) {
        printf("---------------------------------");
        printf("\n| IP: <%s> Port: <%u> |\n", parser->IP, parser->port);
        parser = parser->nextNode;
    }
    printf("---------------------------------\n\n\n");
}
