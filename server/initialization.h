#ifndef SYSPRO_PROJECT3_SERVER_INITIALIZATION_H
#define SYSPRO_PROJECT3_SERVER_INITIALIZATION_H

int PortNumber(int argc, char* const* argv);

void initServer(int port, struct sockaddr_in* server);

#endif
