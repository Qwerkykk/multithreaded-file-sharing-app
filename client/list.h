#ifndef SYSPRO_PROJECT2_LIST_H
#define SYSPRO_PROJECT2_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "struct.h"
#include "utils.h"

typedef struct threadListNode ThreadListNode;

ClientListNode* createClientList();

ThreadListNode* createThreadList();

void addThreadToList(ThreadListNode* list);

void insertFirstC(ClientListNode* list, char* ip, uint16_t port, int socket);

ClientListNode* getClient(ClientListNode* list, char* ip, uint16_t port);

int numOfClients(ClientListNode* list);

int deleteClientFromList(ClientListNode* clientList, char* ip, uint16_t port);

void printClientList(ClientListNode* list);

#endif //SYSPRO_PROJECT2_LIST_H
