#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "initialization.h"


void initArguments(Arguments* arguments, char** argv) {


    for ( int i = 1; i < 13; i += 2 ) {

        if ( strcmp(argv[i], "-sp") == 0 ) {
            arguments->serverPort = atoi(argv[i + 1]);
            continue;
        }
        else if ( strcmp(argv[i],"-sip") == 0 ) {
            arguments->serverIP = malloc(strlen(argv[i+1])+1);
            strcpy(arguments->serverIP,argv[i+1]);
            continue;
        }

        switch ( argv[i][1] ) {

            case 'd':
                arguments->dirName = malloc(strlen(argv[i + 1]) + 1);
                strcpy(arguments->dirName, argv[i + 1]);
                break;

            case 'p':
                arguments->portNum = (uint16_t)atoi(argv[i + 1]);
                break;

            case 'w':
                arguments->workerThreads = atoi(argv[i + 1]);
                break;

            case 'b':
                arguments->bufferSize = atoi(argv[i + 1]);
                break;
        }
    }

}

void printArguments(Arguments* arguments){

    printf("Directory: %s\n",arguments->dirName);
    printf("Port: %u\n",arguments->portNum);
    printf("Number of threads: %d\n",arguments->workerThreads);
    printf("Buffer Size: %d\n",arguments->bufferSize);
    printf("Server Port: %d\n",arguments->serverPort);
    printf("Server Ip: %s\n",arguments->serverIP);


}