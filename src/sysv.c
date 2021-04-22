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
#include "../include/list_lib.h"
#include "../include/md5.h"

#define PACKET_LENGTH 128


int main(){
    unsigned char digest[MD5_DIGEST_LENGTH];
    char buf[(MD5_DIGEST_LENGTH * 2) + 1];
    char str[16] = "Hello bro";

    char x[128] = "";

    compute_md5("hello world", digest);
    
    for (int i = 0, j = 0; i < MD5_DIGEST_LENGTH; i++, j+=2)
        sprintf(buf+j, "%02x", digest[i]);

    buf[MD5_DIGEST_LENGTH * 2] = 0;
    printf ("%s\n", buf);

    strcat(x, str);
    strcat(x, buf);

    printf("%s\n", x);

    return 0;
}