#ifndef __CLIENT_CONNECTION_H__
#define __CLIENT_CONNECTION_H__

#include "TcpSock.h"
#include "thread.h"

class SimpleQueue;
class ClientConnection:public Thread,public TcpSock
{
public:
	ClientConnection(int _maxnode,int _model = WRITE_SINGLE_THREAD_MODE,int _rbuff = 8192,int _wbuff = 10240);
	virtual ~ClientConnection();
	
	int Ini(SimpleQueue* _receive,SimpleQueue* _send,int _port,int _heart);
	
	void SendMsg(void* _data,int _len);
	
	inline SimpleQueue* GetMsgQueue(){return _receive_queue;}
	inline SimpleQueue* GetSendQueue(){return _send_queue;}
private:
	virtual void CallbackAddSocketNode(int iSocketIndex);

	virtual void CallbackDelSocketNode(int iSocketIndex,int iDisType, int iProxy);

	virtual bool CallbackTCPReadData(int iSocketIndex,char *szMsg,int iLen,int iAesFlag);
	
	virtual void CallbackParseDataError(int iSocketIndex,char *szBuffer,int iLen,int iPos);
	
	int	Run();
	
private:
	SimpleQueue* _receive_queue;
	SimpleQueue* _send_queue;
	int _connection_port;
};

#endif 
