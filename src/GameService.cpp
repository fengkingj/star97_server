#include "GameService.h"
#include "log.h"
#include "proto_body.h"
#include "Package.h"
#include "RadiusService.h"
#include "ServiceManage.h"
#include "Config.h"
#include "lib/SimpleQueue.h"
#include "lib/ServerLib.h"
#include <arpa/inet.h>

GameService::GameService()
{
	second_count = 0;
}
GameService::~GameService()
{

}
void GameService::Init(int _id, int _type, const char* _name)
{
	Service::Init(_id, _type, _name);
	
	RegCall(THR_SOCKET_DISCONNECT,EVENT_FUNCTION(GameService::Thr_UserDisconnect,this));//节点断开
	
	RegCall(USERINFO_RES_RADIUS_MSG,EVENT_FUNCTION(GameService::Rad_UserInfoRes,this));//认证过的节点加入
}

void GameService::OneSecondCall()
{
	second_count ++;
}

int GameService::Thr_UserDisconnect(PackageHead* _package,EventCall* _room)
{
	SocketDisconnectMsg* disMsg = (SocketDisconnectMsg*)_package->Data();
	PlayerNode* node = all_players.GetNode(disMsg->iUserID);
	if(!node)
	{
		_log(_ERROR,ServiceName(),"Thr_UserDisconnect _node[%d] = NULL",disMsg->iUserID);
		return -1;
	}
	if(disMsg->iSocketFD == node->iSocketFD)//防止顶号的情况下把正常的节点给删了
	{
		UpdateOnline(node->iLoginType,-1);
		all_players.ReturnNode(node->iUserID);
	}
	return 0;
}
void GameService::UpdateOnline(int _login,int _num)
{
	total_online += _num;
	if(_login == 1)
	{
		mobile_online += _num;
	}
}
int GameService::Rad_UserInfoRes(PackageHead* _package, EventCall* _room)
{
	UserInfoResRadius *msgres = (UserInfoResRadius*)_package->Data();
	PlayerNode* node = all_players.GetNode(msgres->iUserID);
	if(node)
	{
		KillNode(node->iSocketFD);
		UpdateOnline(node->iLoginType,-1);
		all_players.ReturnNode(node->iUserID);
	}
	node = all_players.GetFreeNode();
	node->iUserID = msgres->iUserID;
	node->iSocketFD = _package->_socket_fd;
	node->InitPlayerNode(msgres);
	all_players.RegNode(node->iUserID,node);
	
	UpdateOnline(node->iLoginType,1);
	
	time_t iTimeTemp1 ;
	time(&iTimeTemp1);
	struct tm  tm_t1;
	tm_t1 = *localtime(&iTimeTemp1);
	Room* pRoom = Room::Inst();
	Package<AuthenRes> msgCL(node->iSocketFD);
	msgCL->msgHead.cMsgType = AUTHEN_RES_MSG;
	msgCL->cVipType = node->cVipType;
	msgCL->iMinPoint = htonl(pRoom->iMinPoint);//20080724	
	strcpy(msgCL->szNickName,node->cNickName);
	msgCL->iPlayNum = htonl(node->iTodayPlayNum);
	msgCL->iMaxPoint = htonl(pRoom->iMaxPoint);
	msgCL->iBasePoint = htonl(pRoom->iBasePoint);
	msgCL->iMinTime = htonl(pRoom->iMinTime);
	msgCL->iMaxTime = htonl(pRoom->iMaxTime);		
	msgCL->iGameType = htonl(pRoom->iGameType2);
	msgCL->iServerTime = htonl(iTimeTemp1);
	msgCL->iServerID = htonl(Server::Inst()->server_id);	
	msgCL->PlayerInfo.iUserID = htonl(node->iUserID);
	strcpy(msgCL->PlayerInfo.szNickName,node->cNickName);
	msgCL->PlayerInfo.iMoney = htonl(node->iMoney);
	msgCL->PlayerInfo.iIntegral = htonl(node->iIntegral);
	msgCL->PlayerInfo.iChitMoney = htonl(node->iChitMoney);
	msgCL->PlayerInfo.iBuffA0 = htonl(node->iBuffA0);
	msgCL->PlayerInfo.iBuffA1 = htonl(node->iBuffA1);
	msgCL->PlayerInfo.iBuffA2 = htonl(node->iBuffA2);
	msgCL->PlayerInfo.iBuffB0 = htonl(node->iBuffB0);
	msgCL->PlayerInfo.iBuffB1 = htonl(node->iBuffB1);
	msgCL->PlayerInfo.iBuffB2 = htonl(node->iBuffB2);
	msgCL->PlayerInfo.cIconNum = node->cIconNum;
	msgCL->PlayerInfo.iWinNum = htonl(node->iWinNum);
	msgCL->PlayerInfo.iLostNum = htonl(node->iLostNum);
	memcpy(msgCL->PlayerInfo.szNickName2,node->cNickName,20);
	code_convert("gbk","utf-8",node->cNickName2,20,msgCL->PlayerInfo.szNickName2,20);
	msgCL->coupons = htonl(node->iCoupons);  //给客户端的点券数
	msgCL->banMsgTime = htonl(node->banMsgTime);
	ServiceManage::reply_client_queue->EnQueue(msgCL.PAK(),msgCL.LEN());//回給玩家
	return 0;
}


