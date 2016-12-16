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
	
	InitCall(_client_connection->GetMsgQueue(),1);
	
	//线程单通讯
	RegCall(THR_SOCKET_CONNECT,EVENT_FUNCTION(GateServer::Thr_SocketConnect,this));
	RegCall(THR_SOCKET_DISCONNECT,EVENT_FUNCTION(GateServer::Thr_SocketDisconnect,this));
	RegCall(THR_ADD_RADIUS_MSG,EVENT_FUNCTION(GateServer::Thr_AddRadiusSuccess,this));
	
	//Radius通讯
	RegCall(GAME_ROOM_INFO_RES_RADIUS_MSG,EVENT_FUNCTION(GateServer::Rad_RoomInfoRes,this));
	RegCall(GET_SERVER_INFO_MSG,EVENT_FUNCTION(GateServer::Rad_ServerInfoRes,this));
	RegCall(UPDATE_CENTER_SERVER_INFO_MSG,EVENT_FUNCTION(GateServer::Rad_UpdateCenterServerInfo,this));
	RegCall(USERINFO_RES_RADIUS_MSG,EVENT_FUNCTION(GateServer::Rad_UserInfoRes,this));
	RegCall(GET_CENTER_SERVER_MSG,EVENT_FUNCTION(GateServer::Rad_GetCenterServerMsg,this));
	
	//客户端发过来的请求
	RegCall(AUTHEN_REQ_MSG,EVENT_FUNCTION(GateServer::Cli_UserAuthenReq,this));
}

int GateServer::Run()
{
	time_t tmLast = time(0);
	time_t tmLast2 = tmLast;
	time_t tmLast3 = tmLast;
	time_t tmLast4 = tmLast;
	time_t tmLast5 = tmLast;
	while(1)
	{
		Dispatch();
		
		Timer::Update();
		if(Timer::tmNow - tmLast > 30)
		{
			tmLast = Timer::tmNow;
			Config::Instance()->LoadConfig();
		}
		Monitor::MonitorQueue(ServiceManage::socket_msg_queue->Size(),ServiceManage::reply_client_queue->Size(),ServiceManage::request_radius_queue->Size());
		if(Timer::tmNow - tmLast2 > MONITOR_LOG_TIME)
		{
			tmLast2 = Timer::tmNow;
			Monitor::MonitorLog();
		}
		if(Timer::tmNow - tmLast3 > 300)//五分钟拿次房间信息
		{
			tmLast3 = Timer::tmNow;
			UpdateServerRoomInfo();
		}
		if(Timer::tmNow > tmLast4)//实时人数变化
		{
			tmLast4 = Timer::tmNow;
			UpdateCServerPlayerNum();
		}
		if(Timer::tmNow - tmLast5 > 60)//重新拿中心服务器ip
		{
			tmLast5 = Timer::tmNow;
			QueryCenterServerFromRoomServer();
		}
		
		usleep(1);
	}
	
	return 1;
}
void GateServer::UpdateServerRoomInfo(bool bFirst)
{
	_log(_DEBUG,"GateServer","UpdateServerRoomInfo game_id=%d server_id=%d",Server::Inst()->game_id,Server::Inst()->server_id);
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
		msgreq->iRoomOnlineNum[0] = Room::Inst()->iAppMTime;
		ServiceManage::request_radius_queue->EnQueue(msgreq.PAK(),msgreq.LEN());
	}
	else
	{
		//更新在线人数
		Package<GameRoomInfoReqRadius> msgreq2(roomfd);
		msgreq2->msgHeadInfo.cMsgType = GAME_ROOM_INFO_REQ_RADIUS_MSG;
		msgreq2->cRoomNum = MAX_ROOM_NUM;
		int total,mobile;
		ServiceManage::GetOnlineNum(total,mobile);
		if(Room::Inst()->iIfRoomChecked > 0)//没有开启的房间就发负数
		{
			msgreq2->iRoomOnlineNum[0] = htonl(total);
			msgreq2->iRoomOnlineNumReal[0] = htonl(total-mobile);
			msgreq2->iRoomOnlineNumMob[0] = htonl(mobile);					
		}
		else
		{
			msgreq2->iRoomOnlineNum[0] = htonl(0xFFFFFFFF);
			msgreq2->iRoomOnlineNumReal[0] = htonl(0xFFFFFFFF);
			msgreq2->iRoomOnlineNumMob[0] = htonl(0xFFFFFFFF);
		}
		ServiceManage::request_radius_queue->EnQueue(msgreq2.PAK(),msgreq2.LEN());
	}
	//获得房间信息
	Package<GameRoomInfoReqRadius> msgreq(roomfd);
	msgreq->msgHeadInfo.cMsgType = GAME_ROOM_INFO_REQ_RADIUS_MSG;
	msgreq->cIfNeedRoomInfo = 1;
	msgreq->cRoomNum = MAX_ROOM_NUM;
	ServiceManage::request_radius_queue->EnQueue(msgreq.PAK(),msgreq.LEN());
}
//发送服务器IP消息到中心服务器
void GateServer::SendServerInfoToCServer()
{  	
	Room* pRoom = Room::Inst();
	Server* pServer = Server::Inst();

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
	_log(_ERROR,"GateServer","SendServerInfoToCServer ip1[%s][%u] ip2[%s][%u]",pServer->ip1.c_str(),pServer->port1,pServer->ip2.c_str(),pServer->port2);
	Package<ServerDetailInfo> msg(centerfd);
	msg->msgHeadInfo.cMsgType = SEND_INFO_CENTER_SERVER_MSG;
	
	strcpy(msg->iIP1,pServer->ip1.c_str());
	msg->sPort1 = pServer->port1;
	strcpy(msg->iIP2,pServer->ip2.c_str());
	msg->sPort2 = pServer->port2;
	msg->iBeginTime = pRoom->iBeginTime;
	msg->iOpenTime = pRoom->iLongTime;
	msg->iMaxNum = pRoom->iMaxRoomPlayer;
	msg->iChecked = pRoom->iIfRoomChecked;
	msg->iCurrentNum = ServiceManage::GetOnlineNum();
	ServiceManage::request_radius_queue->EnQueue(msg.PAK(),msg.LEN());
}
void GateServer::ReturnSocketNode(SocketNode* _node)
{
	_nodeManage.ReturnNode(_node->m_iSocketFD);
	if(_node->m_iUserID > 0)
	{
		unordered_map<int,SocketNode*>::iterator it = _nodeManage_uid.find(_node->m_iUserID);
		if(it != _nodeManage_uid.end())
			_nodeManage_uid.erase(it);
	}
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
		ReturnSocketNode(node);
	}
	SocketNode *pNode = _nodeManage.GetFreeNode();	
	pNode->m_iSocketFD = conmsg->iSocketFD;
	strcpy(pNode->m_szIP,conmsg->szIP);
	_nodeManage.RegNode(pNode->m_iSocketFD,pNode);
	Monitor::node_num = _nodeManage.NodeSize()+_nodeManage.FreeSize();
	
	_log(_ERROR,"GateServer","Socket Connectted! fd[%d] ip[%s]",pNode->m_iSocketFD,pNode->m_szIP);
	
	Monitor::connect_cnt++;
	Monitor::max_fd = max(pNode->m_iSocketFD,Monitor::max_fd);
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
	ReturnSocketNode(node);
	
	Monitor::node_num = _nodeManage.FreeSize()+_nodeManage.NodeSize();
	Monitor::disconnect_cnt++;
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
	Room::Inst()->InitRoom(msg);
	if(Room::Inst()->iIfRoomChecked == 3)//房间禁用,立马给所有人发个通知
	{
		vector<Service*> services = ServiceManage::GetService(ST_GAME);
		for(size_t i=0;i<services.size();++i)
		{
			Package<KickOutServer> kickmsg(-1,services[i]->ServiceID());
			kickmsg->cType = 31;
			ServiceManage::socket_msg_queue->EnQueue(kickmsg.PAK(),kickmsg.LEN());
		}
		_log(_ERROR,"GateServer","Rad_RoomInfoRes server closed[%d]",Room::Inst()->iIfRoomChecked);
	}
	return 0;
}
int GateServer::Rad_ServerInfoRes(PackageHead* _package, EventCall* _room)
{
	ServerInfo *msg = (ServerInfo*)_package->Data();
	Server::Inst()->InitServer(msg);
	
	_log(_ERROR,"GateServer","Rad_ServerInfoRes center server id[%d] ip1[%s][%d] ip2[%s][%d]",msg->iCenterServerID,msg->iCenterServerIP,msg->sCenterServerPort,
	  msg->iCenterServerIPBak,msg->sCenterServerPortBak);

	if(Server::Inst()->server_id != 0 && 
	  msg->iCenterServerID != Server::Inst()->server_id && 
	  RadiusService::GetRadiusFD(RadiusType::RAD_CENTER) == -2 && 
	  Room::Inst()->iIfConnectCenterServer == 0)
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
		
		Room::Inst()->iIfConnectCenterServer = 1;
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
	Room* pRoom = Room::Inst();
	int online = ServiceManage::GetOnlineNum();
	if(online >= pRoom->iMaxRoomPlayer)
	{
		_log(_ERROR,"GateServer","Cli_UserAuthenReq online_num[%d] room_max[%d]",online,pRoom->iMaxRoomPlayer);
		SendAuthenRes(_package->_socket_fd,1);
		return 0;
	}
	int closeleft = pRoom->CloseLeftSec();
	if(closeleft<60 || pRoom->iIfRoomChecked!=1)//提前1分钟不让进场
	{
		_log(_ERROR,"GateServer","Cli_UserAuthenReq room close begin[%d]long[%d] left[%d]",pRoom->iBeginTime,pRoom->iLongTime,closeleft);
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
	msgreq->iGameID	= htonl(Server::Inst()->game_id);//游戏ID
	strcpy(msgreq->szGameName,Server::Inst()->game_name.c_str());//游戏名称
	//strcpy(node->m_szIP,authen->szIP);  //保存IP地址
	//strcpy(nodePlayers->szUserToken,msgReq->szUserToken);
	msgreq->iCenterServerID = htonl(Server::Inst()->center_server_id);
	ServiceManage::request_radius_queue->EnQueue(msgreq.PAK(),msgreq.LEN());//发送用户信息请求
	_log(_DEBUG,"GateServer","UserAuthenReq user[%d] fd[%d] token[%s]",authen->iUserID,_package->_socket_fd,authen->szUserToken);
	return 0;
}
void GateServer::SendAuthenRes(int _fd,char _result)
{
	Package<AuthenRes> aures(_fd);
	aures->msgHead.cMsgType = AUTHEN_RES_MSG;
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
		if(ntohl(msgres->iMoney) < (uint32_t)Room::Inst()->iMoneyLimitation)
		{
			iRes = 5;
		}
		else if(Room::Inst()->CheckVipType(msgres->cVipType) == false)
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
		aures->msgHead.cMsgType = AUTHEN_RES_MSG;
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
	if(Room::Inst()->iIfConnectCenterServer == 1)//是否连接中心服务器
	{
		static int lastonline = 0;
		int nowonline = ServiceManage::GetOnlineNum();
		if(nowonline == lastonline)
		{
			return;
		}
		int num = nowonline - lastonline;
		lastonline = nowonline;
			
		Package<UpdatePlayerNumReq> msg(RadiusService::GetRadiusFD(RAD_CENTER));
		msg->msgHeadInfo.cMsgType = UPDATA_CENTER_SERVER_PNUM_MSG;
		msg->iUpdateNum = num;
		msg->totalNum = lastonline;
		_log(_DEBUG,"GateServer","UpdateCServerPlayerNum num[%d] total[%d]",num,lastonline);
		ServiceManage::request_radius_queue->EnQueue(msg.PAK(),msg.LEN());
	}
}
void GateServer::QueryCenterServerFromRoomServer()
{
	//printf("QueryCenterServerFromRoomServer[%d]\n",Config::Instance()->server_id);
	Package<QueryCenterServerMsg> qc(RadiusService::GetRadiusFD(RAD_ROOM));
	qc->msgHeadInfo.cMsgType = GET_CENTER_SERVER_MSG;
	qc->serverId = Server::Inst()->server_id;
	ServiceManage::request_radius_queue->EnQueue(qc.PAK(),qc.LEN());
}
int GateServer::Rad_GetCenterServerMsg(PackageHead* _package, EventCall* _room)
{
	CenterServerInfo* m = (CenterServerInfo*)_package->Data();
	_log(_DEBUG,"GateServer","Rad_GetCenterServerMsg id[%d] ip[%s] port[%d]",m->iCenterServerID,m->iCenterServerIP,m->sCenterServerPort);
	if(Server::Inst()->center_server_ip_bak != m->iCenterServerIP||Server::Inst()->center_server_port_bak!=m->sCenterServerPort)
	{
		//_log(_ERROR,"GL","Change Center Server To %s:%d",m->iCenterServerIP,m->sCenterServerPort);
		Server::Inst()->center_server_ip_bak = m->iCenterServerIP;
		Server::Inst()->center_server_port_bak = m->sCenterServerPort;
		
		Package<AddRadiusMsg> arm(0);
		arm->msgHead.cMsgType = THR_ADD_RADIUS_MSG;
		strcpy(arm->cIP2,m->iCenterServerIP);
		arm->iPort2 = m->sCenterServerPort;
		arm->iType = RAD_CENTER;
		arm->iUpdate = 1;
		ServiceManage::request_radius_queue->EnQueue(arm.PAK(),arm.LEN());
	}
	return 0;
}






