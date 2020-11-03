#ifndef SYSPRO_PROJECT3_CLIENT_SOCKETSERVE_H
#define SYSPRO_PROJECT3_CLIENT_SOCKETSERVE_H

#include "struct.h"

/*Diabazei tou clients apo to socket kai tous topothetei sto clientList*/
void getClientsToList(int sock, int nClients, ClientListNode* clientList);

/*Stelnei LOG_ON IP PORT ston server*/
void logOnServer(int sock, const LogOnInfo* logOnInfo);

/*Epistrefei mia domi pou periexei tis plirofories pou prepei na stalthoun ston server gia na ginei to LOG_ON*/
LogOnInfo* getLogOnInfo(int sock);

/*Setasrei to FD_SET ostena dothei stin select gia elegxo*/
void buildSelectList();

/*Se periptosi pou i select antilifthi oti exei erthei ena neo minima elegxei an einai ena neo connection
 * i kapoio minima apo allo xristo*/
void ReadSocks();

/*Dimiourgei to connection me ton neo xristi kai apothikeyei to socket sthn lista meta energa sockets */
void handleNewConnection();

/*Diavazei to socket kai eksipiretei ta minimata GET_FILE_LIST GET_FILE USER_ON USER_OFF*/
void dealWithData(int listNum);

/*Stelnei ta path olon ton arxeion pou thelei na moirastei sto socket*/
void sendFilePathsToSock(const char* path,int sock);

/*Stelnei sto socket ton arithmo ton arxeion pou exei na moirastei kai ektelei tin sendFilePathsToSock*/
void responseToGetFileList(int socketNum, char* buffer);

/*Stelnei to arxeio tou path sto socket*/
void sendFileToSock(int sock, char* buffer, uint64_t* myVersion, char* path);

#endif
