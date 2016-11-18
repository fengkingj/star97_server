#include "ClientConnection.h"
#include "log.h"
#include "Package.h"
#include "lib/SimpleQueue.h"
#include <stdlib.h>
#include "proto_body.h"

ClientConnection::ClientConnection(int _maxnode,int _model,int _rbuff,int _wbuff): 
TcpSock(_maxnode, _model, _rbuff, _wbuff)
{
	
}

ClientConnection::~ClientConnection()
{
	
}

int ClientConnection::Ini(SimpleQueue* _receive, SimpleQueue* _send, int _port, int _heart)
{
	_receive_queue = _receive;
	_send_queue = _send;
	_connection_port = _port;
	_log(_ERROR,"ClientConnection","----- Epoll Server Start port[%d] heart[%d] -----",_port,_heart);
	return AddTCPServerNode(_port,_heart);
}

void ClientConnection::CallbackAddSocketNode(int iSocketIndex)
{
	TcpSocketNodeDef	*node = &m_pSokcetNode[iSocketIndex];
	char UserAddr[20];
	struct sockaddr_in SocketAdd;
	//取得用户IP地址 IVAN 6.19
	memset(&SocketAdd, 0, sizeof(struct sockaddr_in));
	socklen_t SocketAddrLen = sizeof(SocketAdd);  
	getpeername(node->socket, (struct sockaddr*)&SocketAdd, &SocketAddrLen );
	strncpy(UserAddr, inet_ntoa(SocketAdd.sin_addr), sizeof(UserAddr));
	
	//_log(_DEBUG,"ClientConnection","Socket Connected index=%d ip=%s",iSocketIndex,UserAddr);
	
	Package<SocketConnectMsg> scm(iSocketIndex);
	scm->msgHead.cMsgType = THR_SOCKET_CONNECT;
	scm->msgHead.cVersion = MESSAGE_VERSION;
	scm->iSocketFD = iSocketIndex;
	memcpy(scm->szIP,UserAddr,sizeof(UserAddr));
	_receive_queue->EnQueue(scm.PAK(),scm.LEN());
}

void ClientConnection::CallbackDelSocketNode(int iSocketIndex, int iDisType, int iProxy)
{
	//_log(_ERROR,"ClientConnection","Socket Disconnect Port[%d] socketIndex[%d] iDisType=%d iProxy=%d",_connection_port,iSocketIndex,iDisType,iProxy);
	
	Package<SocketDisconnectMsg> sdm(iSocketIndex);
	sdm->msgHead.cMsgType = THR_SOCKET_DISCONNECT;
	sdm->iSocketFD = iSocketIndex;
	sdm->iDisType = iDisType;
	_receive_queue->EnQueue(sdm.PAK(),sdm.LEN());
	
}

void ClientConnection::CallbackParseDataError(int iSocketIndex, char* szBuffer, int iLen, int iPos)
{
	time_t tmNow = time(NULL);
	TcpSocketNodeDef* pNode = GetSocketNode(iSocketIndex);
	_log(_ERROR,"CES","PRB-[%s,%d,%s:%d]VE-[%d,%d,%d,%d,%d],[%d,%d,%d]",m_szTcpSockName,m_iNowSocketNodeNum,pNode->szPeerIP,pNode->iPeerPort,
		 iSocketIndex,iPos,szBuffer[iPos],iLen,pNode->iReadPos,pNode->iUserID,tmNow-pNode->tmCreateTime,pNode->iRecPackNum);
}

bool ClientConnection::CallbackTCPReadData(int iSocketIndex, char* szMsg, int iLen, int iAesFlag)
{
	//TcpSocketNodeDef	*node = &m_pSokcetNode[iSocketIndex];
	if(iLen > MAX_PACKAGE_DATA_LEN || iLen<8)
	{
		_log(_ERROR,"ClientConnection","Error Package Len[%d]",iLen);
		return true;
	}
	MsgHead* pHead = (MsgHead*)szMsg;
	//printf("===== Tcp Read Data Type[%x]\n",pHead->cMsgType);
	if(MESSAGE_VERSION != pHead->cVersion)
	{
		_log(_ERROR,"ClientConnection","Message Version Error fd[%d] ver[%d] Len[%d]",iSocketIndex,pHead->cVersion,iLen);
		return true;
	}
	else if(0xE0 != pHead->cMsgType) //keep alive
	{
		char* pData=(char*)MALLOC(iLen+sizeof(PackageHead));
		PackageHead* ph = (PackageHead*)pData;
		ph->_data_len = iLen;
		ph->_socket_fd = iSocketIndex;
		memcpy(pData+sizeof(PackageHead),szMsg,iLen);
		_receive_queue->EnQueue(pData,iLen+sizeof(PackageHead));
	}
	return true;
}

int ClientConnection::Run()
{
	int iLen=0;
	while(1)
	{	
		for(int i = 0;i<GetNowSocketNodeNum();i++)//读一次消息，需要写总节点数的消息次数
		{
			ReadSocketNodeData();
			char* pData = (char*)_send_queue->DeQueue(iLen,0);	
			if(!pData)
			{
				break;
			}
			PackageHead* pHead = (PackageHead*)pData;
			MsgHead* mHead = (MsgHead*)(pData+sizeof(PackageHead));
			mHead->cVersion = MESSAGE_VERSION;
			if(mHead->cMsgType == THR_KILL_PLAYER_MSG)
			{
				KillPlayerMsg* killInfo = (KillPlayerMsg*)mHead;
				_log(_DEBUG,"ClientConnection","Run Kill Player[%d]",killInfo->iFD);
				SetKillFlag(killInfo->iFD);
			}
			else if(pHead->_table_id!=-1)
			{
				WriteAllSocketNode(pData+sizeof(PackageHead),pHead->_data_len,pHead->_table_id);
			}
			else
			{
				WriteSocketNodeData(pHead->_socket_fd,pData+sizeof(PackageHead),pHead->_data_len);
			}
			FREE(pData);//who dequeue who free
		}
		usleep(1);
	}
	
	return 1;
}
void ClientConnection::SendMsg(void* _data, int _len)
{
	if(_data && _len>0)
	{
		_send_queue->EnQueue(_data,_len);
	}
}





