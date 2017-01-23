#include "ServerLib.h"
#include <iconv.h>
#include <string.h>

using namespace std;
int code_convert(const char *from_charset,const char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) return -1;
	memset(outbuf,0,outlen);
	if ((iconv(cd,pin,&inlen,pout,&outlen)==(size_t)-1))
	{
		iconv_close(cd);
		return -1;
	}
	iconv_close(cd);
	return 0;
}
void parse_string_vector(vector<int>& vec, char *from ,const char *div)
{
	char *p=NULL;
	while((p=strsep(&from,div)))
	{			
		int value = atoi(p);
		if(value != 0)
		{
			vec.push_back(atoi(p));
		}
	}	
}
int msg_compress(void* _src,int _srclen,void* _dest)
{
	if(_srclen <= 0) return -1;
    size_t headIndex = 0;
    int bitPos = 7;
    unsigned char* msg = (unsigned char*)_src;
    unsigned char* resMsg = (unsigned char*)_dest;
    int resIndex = 1;
    for(int i=0;i<_srclen;++i)
    {
		if(msg[i] > 0)
		{
			resMsg[headIndex] = resMsg[headIndex]|(0x01<<bitPos);
			resMsg[resIndex] = msg[i];
			resIndex ++;
		}
		bitPos --;
		if(bitPos == -1 && bitPos < _srclen-1)
		{
			bitPos = 7;
			headIndex = resIndex;
			resMsg[headIndex] = 0;
			resIndex ++;
		}
    }
    return resIndex;
}
int msg_uncompress(void* _src,int _srclen,void* _dest)
{
    if(_srclen <= 0) return -1;
    int resultIndex=0;
    int headIndex=0;
    unsigned char* msg = (unsigned char*)_src;
    unsigned char* resMsg = (unsigned char*)_dest;
    while(headIndex < _srclen)
    {
		unsigned char head = msg[headIndex];
		for(int i=7;i>=0;--i)
		{
			if(((head>>i)&0x01)==1)
			{
				headIndex ++;
				resMsg[resultIndex] = msg[headIndex];
				resultIndex++;
			}
			else
			{
				resMsg[resultIndex] = 0;
				resultIndex++;
			}
		}
		headIndex ++;
    }
    return resultIndex;
}
/*
 * C# Version
 * 
 */
