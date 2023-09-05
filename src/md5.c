#include <string.h>
#include <stdio.h>
#include <openssl/md5.h>
#include "../include/md5.h"

void compute_md5(char *str, unsigned char digest[MD5_DIGEST_LENGTH]){
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, str, strlen(str));
    MD5_Final(digest, &ctx);
}