#ifndef __SERVER_LIB_H__
#define __SERVER_LIB_H__
#include <stdlib.h>
#include <vector>

int code_convert(const char *from_charset,const char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);
void parse_string_vector(std::vector<int>& vec, char *from ,const char *div);

int msg_compress(void* _src,int _srclen,void* _dest);
int msg_uncompress(void* _src,int _srclen,void* _dest);

#endif