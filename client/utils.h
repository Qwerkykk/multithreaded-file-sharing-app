#ifndef SYSPRO_PROJECT3_CLIENT_UTILS_H
#define SYSPRO_PROJECT3_CLIENT_UTILS_H

#include <stdio.h>

int getSizeOfFile(FILE* filePointer);

int isDirectory(const char* path);

int getNumOfFiles(const char* path);

void perror_exit(char* message);

#endif
