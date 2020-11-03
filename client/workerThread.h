#ifndef SYSPRO_PROJECT3_CLIENT_WORKERTHREAD_H
#define SYSPRO_PROJECT3_CLIENT_WORKERTHREAD_H

#include "pool.h"

/*Ekteleite apo ta workerTheads, diabazei apo to pool kai ektelei ta etoimata pros tou upoloipous clients*/
void* workersDuty();

/*Anoigei mia sindesi me ta ip/port pou tou dinontai, epistrefei to socket */
int getConnection(int sock,char* IP,uint16_t port);

/*Zita apo ton sindedemeno client to path pou vriskete sto poolItem. An to fileExists == 0 simenei oti den exei to arxeio
 * opote tha steilei version = 0 eno an to fileExists == 1 tha steilei to version tha steilei to last modification time
 * pou theoroume oti einai i ekdosi tou arxioy*/
void askForFile(int sock, char* buffer, PoolItem* poolItem, int fileExists);

/*Diavazei apo ton buffer to onoma tou arxeiou to anoigei kai epita diabazei fileSize bytes apo to socket kai kleinei to arxeio*/
void getFileFromSocket(int sock, char* buffer, int fileSize);

/*Diabazei apo to socket ta onomata ton arxeion. An einai directory to dimioyrgei eno an einai arxeio dimioyrgei
 * gia afto ena aitima sto pool*/
void getFileList(int sock, char* buffer, char* dirPath, PoolItem* poolItem);

#endif
