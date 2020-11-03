#include "struct.h"
#include "list.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "initialization.h"


int PortNumber(int argc, char* const* argv) {


    if ( argc != 3 ) {
        printf("Error!! Program execution should be like -> ./dropbox_server -p portNum");
        exit(1);
    }

    if ( strcmp(argv[1], "-p") != 0 ) {
        printf("Error! Program execution should be like -> ./dropbox_server -p portNum");
        exit(1);
    }


    return atoi(argv[2]);
}

void initServer(int port, struct sockaddr_in* server) {
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = htonl(INADDR_ANY);
    server->sin_port = htons(port);
}