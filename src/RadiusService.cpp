#include "RadiusService.h"
#include "ServiceManage.h"
#include "Package.h"
#include "lib/SimpleQueue.h"
#include "log.h"
#include "proto_body.h"
#include "Config.h"
#include "aeslib.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

unordered_map<int,int> RadiusService::map_radius_fd;

char m_szPasswd[] = "k348jf93nr";//这里密码不需要Handle哦。。直接用。。Radius那边也应该这样

RadiusService::RadiusService():
TCPClient(READ_BUFF_SIZE,WRITE_BUFF_SIZE)
{

}
RadiusService::~RadiusService()
{

}
int RadiusService::GetRadiusFD(RadiusType rt)
{
	unordered_map<int,int>::iterator it = map_radius_fd.find(rt);
	if(it != map_radius_fd.end())
	{
		return it->second;
	}
	return -1;
}

int RadiusService::AddRadius(Radius& radius)
{
	string* ip = &radius.sIP1;
	int port = radius.iPort1;
	
	int fd = AddTCPClientNode(ip->c_str(),port);
	if(fd == -1)
	{
		_log(_ERROR,"RadiusService","    =>Add Radius Error,ip1[%s][%d],Try 2[%s][%d]",ip->c_str(),port,radius.sIP2.c_str(),radius.iPort2);
		ip = &radius.sIP2;
		port = radius.iPort2;
		fd = AddTCPClientNode(ip->c_str(),port);
		
		if(fd == -1)
		{
			_log(_ERROR,"RadiusService","    =>Add Radius Error,ip2[%s][%d],Exit!!! ",ip->c_str(),port);
			exit(0);
		}
	}
	
	radius.iSocketFD = fd;
	radius.iReconnectCount = -1;
	vec_all_radius.push_back(radius);
	map_radius_fd[radius.eType] = fd;
	
	_log(_ERROR,"RadiusService","Add Radius Success!!! name[%s] fd[%d] ip1[%s][%d] ip2[%s][%d]",
		 radius.sName.c_str(),fd,radius.sIP1.c_str(),radius.iPort1,radius.sIP2.c_str(),radius.iPort2);
	ConnectSuccess(fd,radius.bAesEncrypt,true);
	return fd;
}
int RadiusService::Run()
{
	time_t tmNow = time(0);
	time_t tmLast = tmNow;
	time_t tmLast2 = tmNow;
	while(1)
	{
		tmNow = time(0);
		if(tmNow > tmLast2)
		{
			tmLast2 = tmNow;
			for(size_t i=0;i<vec_all_radius.size();++i)
			{
				if(vec_all_radius[i].iReconnectCount > 0)
				{
					vec_all_radius[i].iReconnectCount ++;
					if(vec_all_radius[i].iReconnectCount % RECONNECT_INTERVAL == 0)
					{
						ReconnectRadius(i);
					}
				}
			}
		}
		Update();
		int iLen = 0;
		char* pData = (char*)ServiceManage::request_radius_queue->DeQueue(iLen,0);	
		if(pData)
		{
			assert(iLen > 20);
			PackageHead* pHead = (PackageHead*)pData;
			MsgHead* msgHead = (MsgHead*)pHead->Data();
			if(msgHead->cMsgType == THR_ADD_RADIUS_MSG)	//增加radius
			{
				AddRadiusMsg* msg = (AddRadiusMsg*)msgHead;
				Radius addInfo;
				addInfo.bAesEncrypt = msg->iAesEncrypt;
				addInfo.eType = (RadiusType)msg->iType;
				addInfo.iPort1 = msg->iPort1;
				addInfo.iPort2 = msg->iPort2;
				addInfo.iReconnectCount = -1;
				addInfo.iSocketFD = -1;
				addInfo.sIP1 = string(msg->cIP1);
				addInfo.sIP2 = string(msg->cIP2);
				addInfo.sName = string(msg->cName);
				AddRadius(addInfo);
				ServiceManage::socket_msg_queue->EnQueue(pData,iLen);
			}
			else
			{
				for(size_t k=0;k<vec_all_radius.size();++k)
				{
					if(vec_all_radius[k].iSocketFD != pHead->_socket_fd) continue;
					
					printf("<--- send to radius[%d] msgtype[%x] len[%d]\n",pHead->_socket_fd,msgHead->cMsgType,iLen);
					msgHead->cVersion = MESSAGE_VERSION;
					int phLen = sizeof(PackageHead);
					int msgLen = iLen - phLen;
					pData[phLen-4] = 0x03;
					pData[phLen-3] = 0x00;
					pData[phLen-2] = (char)((msgLen+4)>>8);
					pData[phLen-1] = (char)(msgLen+4);	
					SendRadiusData(pHead->_socket_fd,vec_all_radius[k].bAesEncrypt,pData+phLen-4,msgLen+4);
					break;
				}
				FREE(pData);//who dequeue who free
			}
		}
		if(tmNow - tmLast > 30)
		{
			KeepRadiusAlive();
			tmLast = tmNow;
		}
		usleep(1);
	}
	return 1;
}
void RadiusService::ReconnectRadius(int index)
{
//	printf("\n\n=== Reconnect Radius \n");
	
	string* ip = &(vec_all_radius[index].sIP1);
	int port = vec_all_radius[index].iPort1;
	
	if(vec_all_radius[index].iReconnectCount % (2*RECONNECT_INTERVAL) == 0)
	{
		ip = &(vec_all_radius[index].sIP2);
		port = vec_all_radius[index].iPort2;	
	}
	int fd = AddTCPClientNode(ip->c_str(),port);
	
	if(fd == -1)
	{
		_log(_ERROR,"RadiusService","Reconnect Radius Failed!!! ip=%s port=%d",ip->c_str(),port);
	}
	else
	{
		_log(_ERROR,"RadiusService","Reconnect Radius Success!!! ip=%s port=%d fd=%d",ip->c_str(),port,fd);
		vec_all_radius[index].iSocketFD = fd;
		vec_all_radius[index].iReconnectCount = -1;
		map_radius_fd[vec_all_radius[index].eType] = fd;
		ConnectSuccess(fd,vec_all_radius[index].bAesEncrypt,false);
	}
}
void RadiusService::ConnectSuccess(int fd,bool aes,bool newcon)
{
	int msgLen = sizeof(AuthenReqRadius);
	char cTemp[128]={0};
	cTemp[0] = 0x03;
	cTemp[1] = 0x00;
	cTemp[2] = (char)((msgLen+4)>>8);
	cTemp[3] = (char)(msgLen+4);
	AuthenReqRadius* msg = (AuthenReqRadius*)(cTemp+4);
	msg->msgHeadInfo.cVersion = MESSAGE_VERSION;
	msg->msgHeadInfo.cMsgType = AUTHEN_REQ_RADIUS_MSG;
	msg->msgHeadInfo.iMsgBodyLen = msgLen - sizeof(MsgHead);
			
	msg->iServerID = htonl(Config::Instance()->server_id);
	msg->iHideIndex = htonl(1);
	strcpy(msg->szPasswd,"6831869");
	if(newcon) msg->szPasswd[31] = 0;
	else msg->szPasswd[31] = 1;
	SendRadiusData(fd,aes,cTemp,msgLen+4);
}

void RadiusService::CallbackAddSocketNode(int fd)
{
	//TCPNode *node = GetTCPNode(fd);
	//printf("call back add socket index[%d] fd[%d]\n",fd,node->socket);
	/*char UserAddr[20];
	struct sockaddr_in SocketAdd;
	//取得用户IP地址 IVAN 6.19
	memset(&SocketAdd, 0, sizeof(struct sockaddr_in));
	socklen_t SocketAddrLen = sizeof(SocketAdd);  
	getpeername(node->socket, (struct sockaddr*)&SocketAdd, &SocketAddrLen );
	strncpy(UserAddr, inet_ntoa(SocketAdd.sin_addr), sizeof(UserAddr));*/
	
	//_log(_DEBUG,"RadiusService","Radius Connected index=%d ip[%s] port[%d]",iSocketIndex,node->cServerIP,node->iServerPort);
}
void RadiusService::CallbackTCPReadData(int fd)
{
	int iTempLen;
	TCPNode* node = GetTCPNode(fd);
	//网络消息头四字节处理
	if(node->read_msg_pos == 0)           
	{
		if(node->read_buffer[0] != 3)
		{
			_log(_ERROR, "RadiusService", "CallbackTCPReadData: net message head error.");
			return;
		}
		iTempLen = 0;
		memcpy(&iTempLen, &node->read_buffer[2], 2);
		iTempLen = ntohs(iTempLen);
		node->read_msg_pos = node->read_msg_len;
		node->read_msg_len = iTempLen - 4;
		node->total_read_bytes = 0;
		return;
	}
	
	int msg_len = 0;
	bool aes;
	for(size_t i=0;i<vec_all_radius.size();++i)
	{
		if(vec_all_radius[i].iSocketFD == fd)
		{
			aes = vec_all_radius[i].bAesEncrypt;
			break;
		}
	}
	if(!aes)
	{
		char* cData = node->read_buffer+4;
		msg_len = node->read_msg_len;
		if (msg_len < 8)
		{
			DelSocketNode(fd);
			_log(_DEBUG,"RadiusService","CallBackTCPReadData: len ERROR msg_len[%d]",msg_len);
			return;
		}
		char* pData=(char*)MALLOC(msg_len+sizeof(PackageHead));
		PackageHead* ph = (PackageHead*)pData;
		ph->_data_len = msg_len;
		ph->_socket_fd = fd;
		memcpy(pData+sizeof(PackageHead),cData,msg_len);
		ServiceManage::socket_msg_queue->EnQueue(ph,ph->PackLen());
		printf("  ---> radius read data [%x]\n",((MsgHead*)ph->Data())->cMsgType);
	}
	else
	{
		static char msg_buffer[1024*10];
		int res = aes_dec_r(node->read_buffer+4, node->read_msg_len, m_szPasswd, strlen(m_szPasswd), msg_buffer, &msg_len);
		if (res == -1)
		{
			_log(_DEBUG,"RadiusStack","CallBackTCPReadData: ase_dec_r ERROR");
			DelSocketNode(fd);
			return;
		}
		char* pData=(char*)MALLOC(msg_len+sizeof(PackageHead));
		PackageHead* ph = (PackageHead*)pData;
		ph->_data_len = msg_len;
		ph->_socket_fd = fd;
		memcpy(pData+sizeof(PackageHead),msg_buffer,msg_len);
		ServiceManage::socket_msg_queue->EnQueue(ph,ph->PackLen());
		printf("  ---> radius read data [%x]\n",((MsgHead*)ph->Data())->cMsgType);
	}
	ResetSocketNode(fd);
}
void RadiusService::KeepRadiusAlive()
{
	return;
	for(size_t i=0;i<vec_all_radius.size();++i)
	{
		if(vec_all_radius[i].iSocketFD != -1)
		{
			int msgLen = sizeof(KeepAlive);
			char cTemp[128]={0};
			cTemp[0] = 0x03;
			cTemp[1] = 0x00;
			cTemp[2] = (char)((msgLen+4)>>8);
			cTemp[3] = (char)(msgLen+4);
	
			KeepAlive* keep = (KeepAlive*)(cTemp+4);
			keep->msgHead.cMsgType = KEEP_ALIVE_MSG;
			keep->msgHead.cVersion = MESSAGE_VERSION;
			SendRadiusData(vec_all_radius[i].iSocketFD,vec_all_radius[i].bAesEncrypt,cTemp,msgLen+4);
		}
	}
}
void RadiusService::CallbackDelSocketNode(int fd)
{
	for(size_t i=0;i<vec_all_radius.size();++i)
	{
		if(vec_all_radius[i].iSocketFD == fd)
		{
			_log(_ERROR,"RadiusService","CallbackDelSocketNode Radius[%s] Disconnect fd[%d]",vec_all_radius[i].sName.c_str(),fd);
			vec_all_radius[i].iSocketFD = -1;
			vec_all_radius[i].iReconnectCount = 1;
			map_radius_fd[vec_all_radius[i].eType] = -1;
		}
	}
}

void RadiusService::SendRadiusData(int fd,bool aes,void* data,int len)
{
	if(!aes)
	{
		WriteData(fd,data,len);
	}
	else
	{
		static char cTemp[1024*10];
		int send_msg_length = 0;
		aes_enc_r((char*)data+4, len-4, m_szPasswd, strlen(m_szPasswd), cTemp+4, &send_msg_length);
		cTemp[0] = 0x03;
		cTemp[1] = 0x00;
		cTemp[2] = (char)((send_msg_length + 4) >> 8);
		cTemp[3] = (char)( send_msg_length + 4);
		WriteData(fd,cTemp,send_msg_length+4);
	}
}








