#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <malloc.h>
#include <string.h>
#include "Monitor.h"

#define MAX_PACKAGE_DATA_LEN 4096

#define DONOT_FREE -33557799

inline void* MALLOC(size_t _len)
{
	Monitor::malloc_cnt++;
	return malloc(_len);
}
inline void FREE(void* _data)
{
	Monitor::free_cnt++;
	return free(_data);
}

struct PackageHead
{
	int _socket_fd;
	int _table_id;
	int _data_len;
	inline void* Data(){return (char*)this+sizeof(PackageHead);}
	inline int PackLen(){return _data_len+sizeof(PackageHead);}
};

/*
 Simple Package
 */
template<class T>
class Package
{
public:
	Package(int _fd,int _sid=-1);
	T* operator->() {return _msg;}
	inline T* MSG(){return _msg;}
	inline void* PAK(){return (void*)_ph;}
	inline int LEN(){return _pak_len;}
	inline void Free(){FREE(_ph);}
private:
	void* _ph;
	int _pak_len;
	T* _msg;
};
template<class T>
Package<T>::Package(int _fd,int _sid)
{
	_pak_len = sizeof(PackageHead)+sizeof(T);
	_ph = MALLOC(_pak_len);
	memset(_ph,0,sizeof(_pak_len));
	PackageHead* packagehead = (PackageHead*)_ph;
	packagehead->_data_len = sizeof(T);
	packagehead->_socket_fd = _fd;
	packagehead->_table_id = _sid;
	_msg = (T*)((char*)_ph+sizeof(PackageHead));
}

template<class T,class M>
class PackageAssembly
{
public:
	PackageAssembly(int _num,int _fd,int _sid=-1);
	T* operator->() {return _msg;}
	M* operator[](const int& k){return _cmsg+k;}
	inline T* MSG(){return _msg;}
	inline void* PAK(){return (void*)_ph;}
	inline int LEN(){return _pak_len;}
private:
	void* _ph;
	int _pak_len;
	T* _msg;
	M* _cmsg;
};
template<class T,class M>
PackageAssembly<T,M>::PackageAssembly(int _num,int _fd,int _sid)
{
	_pak_len = sizeof(PackageHead)+sizeof(T)+sizeof(M)*_num;
	_ph = MALLOC(_pak_len);
	memset(_ph,0,sizeof(_pak_len));
	PackageHead* packagehead = (PackageHead*)_ph;
	packagehead->_data_len = sizeof(T)+sizeof(M)*_num;
	packagehead->_socket_fd = _fd;
	packagehead->_table_id = _sid;
	_msg = (T*)((char*)_ph+sizeof(PackageHead));
	_cmsg = (M*)(_msg+1);
}

#endif