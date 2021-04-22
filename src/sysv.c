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
    time_t current_time;
	struct tm *time_struct;


    while(1){
        sleep(1);
        current_time = time(NULL);
        time_struct = localtime(&current_time);
        printf("%02d:%02d:%02d\n", time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
    }

    return 0;
}