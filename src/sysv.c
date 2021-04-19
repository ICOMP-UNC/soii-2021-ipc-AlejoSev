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

    strcpy(buffer, "-------------------------------------------------------------------------------------------------------------------------------");

    buffer[0] = (0x31);

    printf("%s\n", buffer);

    return 0;
}