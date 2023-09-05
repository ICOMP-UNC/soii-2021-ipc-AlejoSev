#ifndef _MD5_
#define _MD5_
#include <openssl/md5.h>

void compute_md5(char *str, unsigned char digest[MD5_DIGEST_LENGTH]);

#endif