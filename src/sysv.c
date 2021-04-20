#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include "../include/list_lib.h"

#define PACKET_LENGTH 128

int main(){
    // char buffer[PACKET_LENGTH];
    char id[2] = "ho";
    char test[] = "hola";
    // char source_address[5];
    // char port[3];
    // char checksum[49];
    // char test[5] = "";

    // strcpy(buffer, "ABBBBCCHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH------------------------------------------------------------------------");

    // printf("%s\n", buffer);

    // strncpy(id, buffer, 1);
    // strncpy(source_address, buffer+1, 4);
    // strncpy(port, buffer+5, 2);
    // strncpy(checksum, buffer+7, 48);

    // printf("id: %s\n", id);
    // printf("source_address: %s\n", source_address);
    // printf("port: %s\n", port);
    // printf("checksum: %s\n", checksum);

    // strncpy(buffer+55, test, 5);
    // printf("%s\n", buffer);

    // test[0] = 0x40;
    // printf("%s\n", test);
    // strncpy(test+1, port, 2);
    // printf("%s\n", test);

    // -------------------------------------------------------------------------------
    
    printf("id:%s\ntest:%s\n", id, test);
    printf("id:%2s\ntest:%2s\n", id, test);

    return 0;
}