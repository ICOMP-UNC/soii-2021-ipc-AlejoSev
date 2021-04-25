#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <time.h>
#include "../include/list_lib.h"
#include "../include/md5.h"

#define PACKET_LENGTH 128


int main(){
    char buff[128][64] = {""};

    for(int i = 0; i < 64; i++){
        printf("%s\n", buff[i]);
    }

    if(strcmp(buff[0], "\0") == 0){
        printf("Ue");
    }

    return 0;
}