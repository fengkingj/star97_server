#include "GateServer.h"
#include <string.h>
#include <stdlib.h>
#include "ClientConnection.h"
#include "log.h"
#include "conf.h"
#include "Package.h"
#include "lib/SimpleQueue.h"
#include "assert.h"
#include "Config.h"
#include "Monitor.h"
#include "ServiceManage.h"
#include "Service.h"
#include "proto_body.h"
#include "RadiusService.h"

using namespace std;
/*--------------Socket Node--------------*/
void SocketNode::Reset()
{
	m_iUserID = 0;
	m_iSocketFD = -1;
	m_iServiceID = -1;
	memset(m_szIP,0,sizeof(m_szIP));
	memset(m_szToken,0,sizeof(m_szToken));
}

/*--------------Gate Server--------------*/
GateServer::GateServer()
{

}
GateServer::~GateServer()
{

}
void GateServer::Init(const char* _name,ClientConnection* _connection)
{
	strcpy(_thread_name,_name);
	_client_connection = _connection;
	
	InitCall(_client_connection->GetMsgQueue());
	
	RegCall(THR_SOCKET_CONNECT,EVENT_FUNCTION(GateServer::Thr_SocketConnect,this));
	RegCall(THR_SOCKET_DISCONNECT,EVENT_FUNCTION(GateServer::Thr_SocketDisconnect,this));
	RegCall(THR_ADD_RADIUS_MSG,EVENT_FUNCTION(GateServer::Thr_AddRadiusSuccess,this));
	
	RegCall(GAME_ROOM_INFO_RES_RADIUS_MSG,EVENT_FUNCTION(GateServer::Rad_RoomInfoRes,this));
	RegCall(GET_SERVER_INFO_MSG,EVENT_FUNCTION(GateServer::Rad_ServerInfoRes,this));
	RegCall(UPDATE_CENTER_SERVER_INFO_MSG,EVENT_FUNCTION(GateServer::Rad_UpdateCenterServerInfo,this));
	RegCall(USERINFO_RES_RADIUS_MSG,EVENT_FUNCTION(GateServer::Rad_UserInfoRes,this));
	
	RegCall(AUTHEN_REQ_MSG,EVENT_FUNCTION(GateServer::Cli_UserAuthenReq,this));
}

int GateServer::Run()
{
	time_t tmLast = time(0);
	time_t tmLast2 = tmLast;
	time_t tmLast3 = tmLast;
	time_t tmLast4 = tmLast;
	time_t tmNow;
	while(1)
	{
		Dispatch();
		
		time(&tmNow);
		if(tmNow - tmLast > 60)
		{
			tmLast = tmNow;
			Config::Instance()->LoadConfig();
		}
		if(tmNow - tmLast2 > MONITOR_LOG_TIME)
		{
			tmLast2 = tmNow;
			Monitor::MonitorLog();
		}
		if(tmNow - tmLast3 > 300)//五分钟拿次房间信息
		{
			tmLast3 = tmNow;
			UpdateServerRoomInfo();
		}
		if(tmNow > tmLast4)//实时人数变化
		{
			tmLast4 = tmNow;
			UpdateCServerPlayerNum();
		}
		
		usleep(1);
	}
	
	return 1;
}
void GateServer::UpdateServerRoomInfo(bool bFirst)
{
	_log(_DEBUG,"GateServer","UpdateServerRoomInfo game_id=%d server_id=%d",Config::Instance()->game_id,Config::Instance()->server_id);
	int roomfd = RadiusService::GetRadiusFD(RAD_ROOM);
	if(roomfd < 0)
	{
		_log(_ERROR,"GateServer","Room Radius Error!!! fd=%d",roomfd);
		return;
	}
	if(bFirst)
	{
		Package<GameRoomInfoReqRadius> msgreq(roomfd);
		msgreq->msgHeadInfo.cMsgType = GAME_ROOM_INFO_REQ_RADIUS_MSG;
		msgreq->cIfNeedRoomInfo = 2;
		msgreq->iRoomOnlineNum[0] = Config::Instance()->ROOM.iAppMTime;
		ServiceManage::request_radius_queue->EnQueue(msgreq.PAK(),msgreq.LEN());
	}
	
	//获得房间信息
	Package<GameRoomInfoReqRadius> msgreq(roomfd);
	msgreq->msgHeadInfo.cMsgType = GAME_ROOM_INFO_REQ_RADIUS_MSG;
	msgreq->cIfNeedRoomInfo = 1;
	msgreq->cRoomNum = MAX_ROOM_NUM;
	ServiceManage::request_radius_queue->EnQueue(msgreq.PAK(),msgreq.LEN());
	
	if(!bFirst)
	{
		//更新在线人数
		Package<GameRoomInfoReqRadius> msgreq2(roomfd);
		msgreq2->msgHeadInfo.cMsgType = GAME_ROOM_INFO_REQ_RADIUS_MSG;
		msgreq2->cRoomNum = MAX_ROOM_NUM;
		OnlineCount oc;
		ServiceManage::GetOnlineNum(&oc);
		if(Config::Instance()->ROOM.iIfRoomChecked > 0)//没有开启的房间就发负数
		{
			msgreq2->iRoomOnlineNum[0] = htonl(oc.total_online);
			msgreq2->iRoomOnlineNumReal[0] = htonl(oc.real_online);
			msgreq2->iRoomOnlineNumMob[0] = htonl(oc.mobile_online);					
		}
		else
		{
			msgreq2->iRoomOnlineNum[0] = htonl(0xFFFFFFFF);
			msgreq2->iRoomOnlineNumReal[0] = htonl(0xFFFFFFFF);
			msgreq2->iRoomOnlineNumMob[0] = htonl(0xFFFFFFFF);
		}
		ServiceManage::request_radius_queue->EnQueue(msgreq2.PAK(),msgreq2.LEN());
	}
}
//发送服务器IP消息到中心服务器
void GateServer::SendServerInfoToCServer()
{  	
	ServerRoomInfo* pRoom = &(Config::Instance()->ROOM);
	ServerInfo* pServer = &(Config::Instance()->SERVER);
	if(pRoom->iIfConnectCenterServer != 1)//是否连接中心服务器
	{
		return;
	}
	int centerfd = RadiusService::GetRadiusFD(RAD_CENTER);
	if(centerfd < 0)
	{
		_log(_ERROR,"GateServer","Center Server Error!!! fd=%d",centerfd);
		return;
	}
	_log(_ERROR,"GateServer","SendServerInfoToCServer ip1[%s][%u] ip2[%s][%u]",pServer->iIP1,pServer->sPort1,pServer->iIP2,pServer->sPort2);
	Package<ServerDetailInfo> msg(centerfd);
	msg->msgHeadInfo.cMsgType = SEND_INFO_CENTER_SERVER_MSG;
	
	strcpy(msg->iIP1,pServer->iIP1);
	msg->sPort1 = pServer->sPort1;
	strcpy(msg->iIP2,pServer->iIP2);
	msg->sPort2 = pServer->sPort2;
	msg->iBeginTime = pRoom->iBeginTime;
	msg->iOpenTime = pRoom->iLongTime;
	msg->iMaxNum = pRoom->iMaxRoomPlayer;
	msg->iChecked = pRoom->iIfRoomChecked;
	OnlineCount oc;
	ServiceManage::GetOnlineNum(&oc);
	msg->iCurrentNum = oc.total_online;
	//msg->iQuickMatchEpollUseNum = m_iQuickMatchEpollUseNum;	
	ServiceManage::request_radius_queue->EnQueue(msg.PAK(),msg.LEN());
}

int GateServer::CallBackUnknownPackage(PackageHead* _package,EventCall* _event)
{
	SocketNode* node = _nodeManage.GetNode(_package->_socket_fd);
	if(!node) return 0;
	
	Service* pRoom = ServiceManage::GetService(node->m_iServiceID);
	if(!pRoom)
	{
		MsgHead* msgHead = (MsgHead*)((char*)_package+sizeof(PackageHead));
		_log(_ERROR,"GateServer","CallBackUnknownPackage NoRoom Error node[%d] fd[%d] table[%d] msgtype[%x]",node->m_iUserID,node->m_iSocketFD,node->m_iServiceID,msgHead->cMsgType);
		return 0;
	}
	
	//最后再放进队列
	pRoom->QueuePackage(_package,_package->PackLen());
	return DONOT_FREE;//goto child thread so cannot free
}
int GateServer::Thr_SocketConnect(PackageHead* _package, EventCall* _room)
{
	SocketNode* node = _nodeManage.GetNode(_package->_socket_fd);
	SocketConnectMsg* conmsg = (SocketConnectMsg*)((char*)_package+sizeof(PackageHead));
	if(node)
	{
		_log(_ERROR,"GateServer","Thr_SocketConnect Node Error fd[%d] userid[%d] table[%d]",node->m_iSocketFD,node->m_iUserID,node->m_iServiceID);
		Service* pRoom = ServiceManage::GetService(node->m_iServiceID);
		if(pRoom)
		{
			Package<SocketDisconnectMsg> sdm(node->m_iSocketFD);
			sdm->msgHead.cVersion = MESSAGE_VERSION;
			sdm->msgHead.cMsgType = THR_SOCKET_DISCONNECT;
			sdm->iSocketFD = node->m_iSocketFD;
			sdm->iDisType = 1;
			sdm->iUserID = node->m_iUserID;
			pRoom->QueuePackage(sdm.PAK(),sdm.LEN());
		}
		_nodeManage.ReturnNode(node->m_iSocketFD);
		if(node->m_iUserID > 0)
		{
			unordered_map<int,SocketNode*>::iterator it = _nodeManage_uid.find(node->m_iUserID);
			if(it != _nodeManage_uid.end())
				_nodeManage_uid.erase(it);
		}
		Monitor::_snode_return_cnt++;
	}
	SocketNode *pNode = _nodeManage.GetFreeNode();
	Monitor::_snode_get_cnt++;
	Monitor::_snode_num = _nodeManage.FreeSize()+_nodeManage.NodeSize();
	
	pNode->m_iSocketFD = conmsg->iSocketFD;
	strcpy(pNode->m_szIP,conmsg->szIP);
	_nodeManage.RegNode(pNode->m_iSocketFD,pNode);
	
	_log(_ERROR,"GateServer","Socket Connectted! fd[%d] ip[%s]",pNode->m_iSocketFD,pNode->m_szIP);
	
	Monitor::_connect_cnt++;
	if(pNode->m_iSocketFD > Monitor::_max_fd)
	{
		Monitor::_max_fd = pNode->m_iSocketFD;
	}
	return 0;
}
int GateServer::Thr_SocketDisconnect(PackageHead* _package, EventCall* _room)
{
	SocketDisconnectMsg* dismsg=(SocketDisconnectMsg*)((char*)_package+sizeof(PackageHead));
	SocketNode* node = _nodeManage.GetNode(_package->_socket_fd);
	if(!node)
	{
		_log(_ERROR,"GateServer","Thr_SocketDisconnect Node Null Error!!! fd[%d]",dismsg->iSocketFD);
		return 0;
	}
	dismsg->iUserID = node->m_iUserID;
	
	Service* pRoom = ServiceManage::GetService(node->m_iServiceID);
	bool notfree = false;
	if(pRoom)
	{
		pRoom->QueuePackage(_package,_package->_data_len+sizeof(PackageHead));
		notfree = true;	
	}

	_log(_ERROR,"GateServer","Thr_SocketDisconnect fd[%d] ip[%s] userid[%d] table[%d]",node->m_iSocketFD,node->m_szIP,node->m_iUserID,node->m_iServiceID);

	_nodeManage.ReturnNode(node->m_iSocketFD);
	if(node->m_iUserID > 0)
	{
		unordered_map<int,SocketNode*>::iterator it = _nodeManage_uid.find(node->m_iUserID);
		if(it != _nodeManage_uid.end())
			_nodeManage_uid.erase(it);
	}
	Monitor::_snode_return_cnt++;
	Monitor::_snode_num = _nodeManage.FreeSize()+_nodeManage.NodeSize();
	Monitor::_disconnect_cnt++;
	if(notfree)
	{
		return DONOT_FREE;//goto child thread so cannot free
	}
	return 0;
}
int GateServer::Thr_AddRadiusSuccess(PackageHead* _package,EventCall* _room)
{
	AddRadiusMsg* msg = (AddRadiusMsg*)_package->Data();
	if(msg->iType == RAD_CENTER)	//成功连接Room
	{
		UpdateServerRoomInfo(true);
	}
	return 0;
}

int GateServer::Rad_RoomInfoRes(PackageHead* _package, EventCall* _room)
{
	GameRoomInfoResRadius* msg = (GameRoomInfoResRadius*)_package->Data();
	Config::Instance()->SetRoomInfo(msg);
	return 0;
}
int GateServer::Rad_ServerInfoRes(PackageHead* _package, EventCall* _room)
{
	ServerInfo *msg = (ServerInfo*)_package->Data();
	Config::Instance()->SERVER = *msg;
	
	_log(_ERROR,"GateServer","Rad_ServerInfoRes center server id[%d] ip1[%s][%d] ip2[%s][%d]",msg->iCenterServerID,msg->iCenterServerIP,msg->sCenterServerPort,
	  msg->iCenterServerIPBak,msg->sCenterServerPortBak);

	if(Config::Instance()->server_id != 0 && 
	  msg->iCenterServerID != Config::Instance()->server_id && 
	  RadiusService::GetRadiusFD(RadiusType::RAD_CENTER) == -1)
	{
		Package<AddRadiusMsg> rad(1);
		rad->msgHead.cMsgType = THR_ADD_RADIUS_MSG;
		rad->iType = RAD_CENTER;
		strcpy(rad->cName,"center_server");
		strcpy(rad->cIP1,msg->iCenterServerIP);
		rad->iPort1 = msg->sCenterServerPort;
		strcpy(rad->cIP2,msg->iCenterServerIPBak);
		rad->iPort2 = msg->sCenterServerPortBak;
		rad->iAesEncrypt = 1;
		ServiceManage::request_radius_queue->EnQueue(rad.PAK(),rad.LEN());
		
		Config::Instance()->ROOM.iIfConnectCenterServer = 1;
	}
	else if(msg->iCenterServerID == 0)
	{
		_log(_ERROR,"GateServer","Error!!! No Center Server Configed. iCenterServerID = 0");
		exit(0);
	}
	return 0;
}

int GateServer::Cli_UserAuthenReq(PackageHead* _package, EventCall* _room)
{
	AuthenReq* authen = (AuthenReq*)_package->Data();
	SocketNode* node = _nodeManage.GetNode(_package->_socket_fd);
	if(!node)
	{
		_log(_ERROR,"GateServer","Cli_UserAuthenReq Node Null Error!!! fd[%d] userid[%d]",_package->_socket_fd,authen->iUserID);
		return 0;
	}
	if(node->m_iUserID>0)//already requested!
	{
		_log(_ERROR,"GateServer","Cli_UserAuthenReq Repeate Authen Error! fd[%d] regid[%d] userid[%d] table[%d]",_package->_socket_fd,authen->iUserID,node->m_iUserID,node->m_iServiceID);
		return 0;
	}
	ServerRoomInfo* pRoom = &(Config::Instance()->ROOM);
	OnlineCount oc;
	ServiceManage::GetOnlineNum(&oc);
	if(oc.total_online >= pRoom->iMaxRoomPlayer)
	{
		_log(_DEBUG,"GateServer","Cli_UserAuthenReq online_num[%d] room_max[%d]",oc.total_online,pRoom->iMaxRoomPlayer);
		SendAuthenRes(_package->_socket_fd,1);
		return 0;
	}
	//判断房间是否开放
	time_t now_time;
	struct tm  *tm_t;
	time(&now_time);
	tm_t = localtime(& now_time);
	bool bifLimit = false;
	if(pRoom->iLongTime == 24)//全天开放
	{
		 bifLimit = false;
	}
	else if((pRoom->iBeginTime+pRoom->iLongTime)>=24)
	{
		if((tm_t->tm_hour<pRoom->iBeginTime)&&(tm_t->tm_hour>=(pRoom->iBeginTime+pRoom->iLongTime)%24))
		{
			bifLimit = true;
		}
	}
	else
	{
		if((tm_t->tm_hour<pRoom->iBeginTime)||(tm_t->tm_hour>=(pRoom->iBeginTime+pRoom->iLongTime)))
		{
			bifLimit = true;
		}
	}
	if(pRoom->iIfRoomChecked != 1)
	{
		bifLimit = true;
	}
	if( bifLimit)
	{
		_log(_ERROR,"GateServer","Cli_UserAuthenReq room not open room_checked[%d] begin_time[%d] last_time[%d]",pRoom->iIfRoomChecked,pRoom->iBeginTime,pRoom->iLongTime);
		SendAuthenRes(_package->_socket_fd,9);
		return 0;
	}
	if(strlen(authen->szUserToken) < 4)
	{
		_log(_ERROR, "GateServer", "Cli_UserAuthenReq: strlen(msgReq->szUserToken)<4,ID[%d]",authen->iUserID);
		SendAuthenRes(_package->_socket_fd,14);
		return 0;	
	}	
	node->m_iUserID = authen->iUserID;
	_nodeManage_uid[node->m_iUserID] = node;//user id 缓存一下
	Package<UserInfoReqRadius> msgreq(RadiusService::GetRadiusFD(RAD_ACCOUNT));
	msgreq->iUserID = htonl(authen->iUserID);             //UserID
	strcpy(msgreq->szUserToken,authen->szUserToken);		//附带Token认证
	msgreq->cRoomID = authen->cRoomNum;//房间号
	msgreq->iGameID	= htonl(Config::Instance()->game_id);//游戏ID
	strcpy(msgreq->szGameName,Config::Instance()->game_name.c_str());//游戏名称
	//strcpy(node->m_szIP,authen->szIP);  //保存IP地址
	//strcpy(nodePlayers->szUserToken,msgReq->szUserToken);
	msgreq->iCenterServerID = htonl(Config::Instance()->SERVER.iCenterServerID);
	ServiceManage::request_radius_queue->EnQueue(msgreq.PAK(),msgreq.LEN());//发送用户信息请求
	_log(_DEBUG,"","");
	return 0;
}
void GateServer::SendAuthenRes(int _fd,char _result)
{
	Package<AuthenRes> aures(_fd);
	aures->cResult = _result;
	ServiceManage::reply_client_queue->EnQueue(aures.PAK(),aures.LEN());
}
int GateServer::Rad_UpdateCenterServerInfo(PackageHead* _package, EventCall* _room)
{
	SendServerInfoToCServer();//发送服务器消息到中心服务器
	return 0;
}
int GateServer::Rad_UserInfoRes(PackageHead* _package, EventCall* _room)
{
	UserInfoResRadius *msgres = (UserInfoResRadius*)_package->Data();
	unordered_map<int,SocketNode*>::iterator it = _nodeManage_uid.find(msgres->iUserID);
	if(it == _nodeManage_uid.end())
	{
		_log(_ERROR,"GateServer","Rad_UserInfoRes node null error!!! User[%d]",msgres->iUserID);
		return 0;
	}
	SocketNode* node = it->second;
	if(msgres->cResult == 1 || msgres->cResult == 33 || msgres->cResult == 34)
	{
		int iRes = 0;
		if(ntohl(msgres->iMoney) < (uint32_t)Config::Instance()->ROOM.iMoneyLimitation)
		{
			iRes = 5;
		}
		else if(Config::Instance()->CheckVipType(msgres->cVipType) == false)
		{
			iRes = 7; //进入房间类型和用户的类型不一致
		}
		if(iRes != 0)
		{
			SendAuthenRes(node->m_iSocketFD,iRes);
			return 0;
		}
		//玩家认证通过
		Service* pService = ServiceManage::SocketJoinService(node);
		if(pService == NULL)
		{
			_log(_ERROR,"GateServer","Rad_UserInfoRes player cannot join service error!!! uid[%d]",node->m_iUserID);
			return 0;
		}
		_package->_socket_fd = node->m_iSocketFD;//這個很重要!!!!!!
		pService->QueuePackage(_package,_package->PackLen());
	}
	else
	{
		Package<AuthenRes> aures(node->m_iSocketFD);
		if(msgres->cResult == 23)
		{
			aures->cResult = 10;
		}
		else
		{
			aures->cResult = msgres->cResult;
		}
		aures->iServerID = msgres->iServerID;
		aures->iCenterServerID = msgres->iCenterServerID;
		ServiceManage::reply_client_queue->EnQueue(aures.PAK(),aures.LEN());
	}
	return 0;
}
void GateServer::UpdateCServerPlayerNum()
{
	if(Config::Instance()->ROOM.iIfConnectCenterServer == 1)//是否连接中心服务器
	{
		static int lastonline = 0;
		OnlineCount oc;
		ServiceManage::GetOnlineNum(&oc);
		if(oc.total_online == lastonline)
		{
			return;
		}
		int num = oc.total_online - lastonline;
		lastonline = oc.total_online;
			
		Package<UpdatePlayerNumReq> msg(RadiusService::GetRadiusFD(RAD_CENTER));
		msg->msgHeadInfo.cMsgType = UPDATA_CENTER_SERVER_PNUM_MSG;
		msg->iUpdateNum = num;
		msg->totalNum = lastonline;
		_log(_DEBUG,"GateServer","UpdateCServerPlayerNum num[%d] total[%d]",num,lastonline);
		ServiceManage::request_radius_queue->EnQueue(msg.PAK(),msg.LEN());
	}
}






