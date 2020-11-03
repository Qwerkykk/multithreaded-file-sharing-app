#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "signal.h"
#include "struct.h"

ClientListNode* clientL_;
ThreadListNode* threadList;
pthread_mutex_t clientListLock_;

void exitSignalHandler() {

    pthread_mutex_lock(&clientListLock_);

    ClientListNode* parser = clientL_->nextNode;
    ClientListNode* previous = clientL_;

    printf("\nServer is Closing...\n");
    while ( parser != NULL) {
        free(previous);
        previous = parser;
        parser = parser->nextNode;
    }

    free(previous);


    ThreadListNode* parser1 = threadList->nextNode;
    ThreadListNode* previous1 = threadList;

    int skipDummy = 0;
    while ( parser1 != NULL) {

        if(skipDummy == 1){
            pthread_cancel(previous1->thread);
            pthread_join(previous1->thread, NULL);
        }

        free(previous1);
        previous1 = parser1;
        parser1 = parser1->nextNode;
        skipDummy = 1;
    }
    if(skipDummy == 1){
    pthread_cancel(previous1->thread);
    pthread_join(previous1->thread, NULL);
    }
    free(previous1);

    exit(0);
}