#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "../include/list_lib.h"

#define PACKET_LENGTH 128

int main(){
    char buffer[PACKET_LENGTH];
    char id[2];
    char source_address[5];
    char port[3];
    char checksum[49];
    char test[2];

    strcpy(buffer, "ABBBBCCHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH------------------------------------------------------------------------");

    printf("%s\n", buffer);

    strncpy(id, buffer, 1);
    strncpy(source_address, buffer+1, 4);
    strncpy(port, buffer+5, 2);
    strncpy(checksum, buffer+7, 48);

    printf("id: %s\n", id);
    printf("source_address: %s\n", source_address);
    printf("port: %s\n", port);
    printf("checksum: %s\n", checksum);

    return 0;
}