#include "ServerLib.h"
#include <iconv.h>
#include <string.h>

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