#include "PlayerNode.h"
#include "log.h"
#include "string.h"
#include "proto_body.h"
#include "Config.h"
#include <arpa/inet.h>

void PlayerNode::Reset()
{
	iUserID = 0;
	iSocketFD = 0;
	memset(cUserName,0,sizeof(cUserName));
	memset(cNickName,0,sizeof(cNickName));
	iAgentID = 0;
	iMoney = 0;
	iCZContribution = 0;
	iDiamond = 0;
	iUsedDiamond = 0;
	iVipLev = 0;
	iTicket = 0;
	iTicketContribution = 0;
}
void PlayerNode::InitPlayerNode(UserInfoResRadius* pInfo)
{	
	iUserID = pInfo->iUserID;
	iCoupons = ntohl(pInfo->coupons); //点券数
	iOnlineTime = ntohl(pInfo->iOnlineTime);//在线时间
	banMsgTime = ntohl(pInfo->banMsgTime);//禁言到的日期 1970开始的秒数
	noawardTime = pInfo->noawardTime; //零收益到的1970秒数
	memcpy(cNickName,pInfo->szNickName,16);
	int iMoneyTemp = ntohl(pInfo->iMoney);
	int iWinNumTemp = ntohl(pInfo->iWinNum);
	int iLostNumTemp = ntohl(pInfo->iLostNum);
	iAmount = pInfo->iAmount;		
	iMoney = iMoneyTemp - Room::Inst()->iRoomMoney;		//扣除掉进入房间的费用	
	iWinNum = iWinNumTemp;		
	iLostNum = iLostNumTemp;
	iPunishNum = ntohl(pInfo->iPunishNum);//add for sino
	cIconNum = pInfo->cIconNum;
	memcpy(cNickName2,pInfo->szNickName2,20);
	memcpy(cLastGameNum,pInfo->szLastGameNum,20);
	memcpy(szRealName,pInfo->szRealName,20);
	iIntegral = ntohl(pInfo->iIntegral);
	iBankMoney = ntohl(pInfo->iBankMoney);
	iRateAllCoin = pInfo->iRateAllCoin;
	iRateWinCoin = pInfo->iRateWinCoin;
	iRateSet = ntohl(pInfo->iRateSet);
	iRateCount = ntohl(pInfo->iRateCount);
	
	iBuffA0 = ntohl(pInfo->iBuffA0);
	iBuffA1 = ntohl(pInfo->iBuffA1);
	iBuffA2 = ntohl(pInfo->iBuffA2);
	iBuffB0 = ntohl(pInfo->iBuffB0);
	iBuffB1 = ntohl(pInfo->iBuffB1);
	iBuffB2 = ntohl(pInfo->iBuffB2);
	fExpTime = pInfo->fExpTime;
	
	fExpTimeBegin = fExpTime;
	iChitMoney = ntohl(pInfo->iChitMoney);
	iHisMaxMoney = ntohl(pInfo->iHisMaxMoney);
	iBoxContributeValue = ntohl(pInfo->iBoxContributeValue);
	iLastLoginDay = ntohl(pInfo->iNoLoginDay);
	iDayPlayNum = ntohl(pInfo->iDayPlayNum);
	iPropMaxNum = ntohl(pInfo->iPropMaxNum);
	iUniteAgentID = ntohl(pInfo->iUniteAgentID); 			
	iAreaID = ntohl(pInfo->iAreaID);
	strcpy(szAreaName,pInfo->szAreaName);
	iMoneyExtra = ntohl(pInfo->iMoneyExtra);
	iInfullNum = ntohl(pInfo->iInfullNum);
	iDisNum = ntohl(pInfo->iDisNum);	
	iPlayAllNum = ntohl(pInfo->iPlayAllNum);
		
	iTodayPlayNum = -1;//红包关闭客户收到值是-1	
	tmLastAccount = time(NULL);
		
	memcpy(szUserName,pInfo->szUserName,20);
	iTotalIntegral = ntohl(pInfo->iTotalIntegral);
	iJudgeIntegral = ntohl(pInfo->iJudgeIntegral);
	iIfForceYB = 0;//把操场地区停掉
	iAllGameAmount = (int)(ntohl(pInfo->iAllGameAmount));
	iGameAmount = (pInfo->iWinMoney - pInfo->iLostMoney);
	iWinMoney = pInfo->iWinMoney;
	iLostMoney = pInfo->iLostMoney;
	iBeginGameTime = 0;
	iLeaveGameTime = 0;//add skyhawk 20080320	
	cVipType = pInfo->cVipType;
	tmRegisterTime = ntohl(pInfo->iRegisterTime);	
	cSeeTableNumExtra = -1;	
	bIfNeedAccount = true;
	iLoginType = pInfo->cLoginType;
}
