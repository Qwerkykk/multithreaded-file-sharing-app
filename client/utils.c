#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"



int getSizeOfFile(FILE* filePointer) {
    if ( filePointer == NULL)
        return 0;

    int fileSize;
    fseek(filePointer, 0L, SEEK_END);
    fileSize = ftell(filePointer);
    rewind(filePointer);
    return fileSize;
}


int isDirectory(const char* path) {
    DIR* directoryPointer;
    if ((directoryPointer = opendir(path)) == NULL)
        return 1;

    closedir(directoryPointer);
    return 0;
}


int getNumOfFiles(const char* path) {
    DIR* directoryPointer;
    struct dirent* directoryContent;
    directoryPointer = opendir(path);
    char extendedPath[128];
    memset(extendedPath, 0, 128);

    int counter = 0;
    while ((directoryContent = readdir(directoryPointer)) != NULL) {
        if ( directoryContent->d_name[0] == '.' )
            continue;

        sprintf(extendedPath, "%s/%s", path, directoryContent->d_name);

        if ( isDirectory(extendedPath) == 0 ) {
            counter += getNumOfFiles(extendedPath);
        }
        counter++;
    }
    closedir(directoryPointer);
    return counter;

}

void perror_exit(char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}