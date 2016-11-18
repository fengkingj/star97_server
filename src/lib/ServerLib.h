#ifndef __SERVER_LIB_H__
#define __SERVER_LIB_H__
#include <stdlib.h>

int code_convert(const char *from_charset,const char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);

#endif