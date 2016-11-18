#include "Star97Service.h"
#include "log.h"
#include "proto_body.h"
#include "Package.h"
#include "RadiusService.h"
#include "ServiceManage.h"
#include "Config.h"
#include "lib/SimpleQueue.h"
#include "lib/ServerLib.h"
#include <arpa/inet.h>

Star97Service::Star97Service()
{
	second_count = 0;
}
Star97Service::~Star97Service()
{

}
void Star97Service::Init(int _id, int _type, const char* _name)
{
	Service::Init(_id, _type, _name);
	
	RegCall(THR_SOCKET_DISCONNECT,EVENT_FUNCTION(Star97Service::Thr_UserDisconnect,this));//节点断开
	
	RegCall(USERINFO_RES_RADIUS_MSG,EVENT_FUNCTION(Star97Service::Rad_UserInfoRes,this));//认证过的节点加入
}

void Star97Service::OneSecondCall()
{
	second_count ++;
}

int Star97Service::Thr_UserDisconnect(PackageHead* _package,EventCall* _room)
{
	SocketDisconnectMsg* disMsg = (SocketDisconnectMsg*)_package->Data();
	PlayerNode* _node = all_players.GetNode(disMsg->iUserID);
	if(!_node)
	{
		_log(_ERROR,ServiceName(),"Thr_UserDisconnect _node[%d] = NULL",disMsg->iUserID);
		return -1;
	}
	if(disMsg->iSocketFD == _node->iSocketFD)//防止顶号的情况下把正常的节点给删了
	{
		all_players.ReturnNode(_node->iUserID);
		Monitor::_cnode_return_cnt++;
		Monitor::_cnode_num = all_players.FreeSize()+all_players.NodeSize();
	}
	return 0;
}
int Star97Service::Rad_UserInfoRes(PackageHead* _package, EventCall* _room)
{
	UserInfoResRadius *msgres = (UserInfoResRadius*)_package->Data();
	PlayerNode* node = all_players.GetNode(msgres->iUserID);
	if(node)
	{
		KillNode(node->iSocketFD);
		all_players.ReturnNode(node->iUserID);
		Monitor::_cnode_return_cnt++;
	}
	node = all_players.GetFreeNode();
	node->iSocketFD = _package->_socket_fd;
	
	ServerRoomInfo* pRoom = &(Config::Instance()->ROOM);
	node->iUserID = msgres->iUserID;
	node->iCoupons = ntohl(msgres->coupons); //点券数
	node->iOnlineTime = ntohl(msgres->iOnlineTime);//在线时间
	node->banMsgTime = ntohl(msgres->banMsgTime);//禁言到的日期 1970开始的秒数
	node->noawardTime = msgres->noawardTime; //零收益到的1970秒数
	memcpy(node->cNickName,msgres->szNickName,16);
	int iMoneyTemp = ntohl(msgres->iMoney);
	int iWinNumTemp = ntohl(msgres->iWinNum);
	int iLostNumTemp = ntohl(msgres->iLostNum);
	node->iAmount = msgres->iAmount;		
	node->iMoney = iMoneyTemp - pRoom->iRoomMoney;		//扣除掉进入房间的费用	
	node->iWinNum = iWinNumTemp;		
	node->iLostNum = iLostNumTemp;
	node->iPunishNum = ntohl(msgres->iPunishNum);//add for sino
	node->cIconNum = msgres->cIconNum;
	memcpy(node->cNickName2,msgres->szNickName2,20);
	memcpy(node->cLastGameNum,msgres->szLastGameNum,20);
	memcpy(node->szRealName,msgres->szRealName,20);
	node->iIntegral = ntohl(msgres->iIntegral);
	node->iBankMoney = ntohl(msgres->iBankMoney);
	node->iRateAllCoin = msgres->iRateAllCoin;
	node->iRateWinCoin = msgres->iRateWinCoin;
	node->iRateSet = ntohl(msgres->iRateSet);
	node->iRateCount = ntohl(msgres->iRateCount);
	
	node->iBuffA0 = ntohl(msgres->iBuffA0);
	node->iBuffA1 = ntohl(msgres->iBuffA1);
	node->iBuffA2 = ntohl(msgres->iBuffA2);
	node->iBuffB0 = ntohl(msgres->iBuffB0);
	node->iBuffB1 = ntohl(msgres->iBuffB1);
	node->iBuffB2 = ntohl(msgres->iBuffB2);
	node->fExpTime = msgres->fExpTime;
	
	node->fExpTimeBegin = node->fExpTime;
	node->iChitMoney = ntohl(msgres->iChitMoney);
	node->iHisMaxMoney = ntohl(msgres->iHisMaxMoney);
	node->iBoxContributeValue = ntohl(msgres->iBoxContributeValue);
	node->iLastLoginDay = ntohl(msgres->iNoLoginDay);
	node->iDayPlayNum = ntohl(msgres->iDayPlayNum);
	node->iPropMaxNum = ntohl(msgres->iPropMaxNum);
	node->iUniteAgentID = ntohl(msgres->iUniteAgentID); 			
	node->iAreaID = ntohl(msgres->iAreaID);
	strcpy(node->szAreaName,msgres->szAreaName);
	node->iMoneyExtra = ntohl(msgres->iMoneyExtra);
	node->iInfullNum = ntohl(msgres->iInfullNum);
	node->iDisNum = ntohl(msgres->iDisNum);	
	node->iPlayAllNum = ntohl(msgres->iPlayAllNum);
		
	node->iTodayPlayNum = -1;//红包关闭客户收到值是-1	
	node->tmLastAccount = time(NULL);
		
	memcpy(node->szUserName,msgres->szUserName,20);
	node->iTotalIntegral = ntohl(msgres->iTotalIntegral);
	node->iJudgeIntegral = ntohl(msgres->iJudgeIntegral);
	node->iIfForceYB = 0;//把操场地区停掉
	node->iAllGameAmount = (int)(ntohl(msgres->iAllGameAmount));
	node->iGameAmount = (msgres->iWinMoney - msgres->iLostMoney);
	node->iWinMoney = msgres->iWinMoney;
	node->iLostMoney = msgres->iLostMoney;
	node->iBeginGameTime = 0;
	node->iLeaveGameTime = 0;//add skyhawk 20080320	
	node->cVipType = msgres->cVipType;
	node->tmRegisterTime = ntohl(msgres->iRegisterTime);	
	node->cSeeTableNumExtra = -1;	
	node->bIfNeedAccount = true;
	
	time_t iTimeTemp1 ;
	time(&iTimeTemp1);
	struct tm  tm_t1;
	tm_t1 = *localtime(&iTimeTemp1);
		
	Package<AuthenRes> msgCL(node->iSocketFD);
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
	msgCL->iServerID = htonl(Config::Instance()->server_id);	
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
	msgCL->coupons = htonl(node->iCoupons);  //给客户端的点券数 add by alexhy 2016-3-11
	msgCL->banMsgTime = htonl(node->banMsgTime);
	
	ServiceManage::reply_client_queue->EnQueue(msgCL.PAK(),msgCL.LEN());//回給玩家
	return 0;
}


