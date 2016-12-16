#ifndef __PLAYER_NODE_H__
#define __PLAYER_NODE_H__

class UserInfoResRadius;
class PlayerNode
{
public:
	void Reset();
	void InitPlayerNode(UserInfoResRadius* pInfo);
	bool JudgeKickOut();
public:
	int iUserID;
	int iSocketFD;
	char cUserName[20];
	char cNickName[20];
	char cNickName2[32];
	int iAgentID;
	int iMoney;
	int iCZContribution;
	int iDiamond;
	int iUsedDiamond;
	int iVipLev;
	int iTicket;
	int iTicketContribution;
	int iCoupons; //点券数
	int iOnlineTime;//在线时间
	int banMsgTime;//禁言到的日期 1970开始的秒数
	int noawardTime;//零收益到的1970秒数
	int iAmount;
	int iWinNum;
	int iLostNum;
	int iPunishNum;
	char cIconNum;
	char cLastGameNum[20];
	char szRealName[20];
	int iIntegral;
	int iBankMoney;
	int iRateAllCoin;
	int iRateWinCoin;
	int iRateSet;
	int iRateCount;
	int iBuffA0;
	int iBuffA1;
	int iBuffA2;
	int iBuffB0;
	int iBuffB1;
	int iBuffB2;
	float fExpTime;					//经验值
	float fExpTimeBegin;
	int iChitMoney;
	int iHisMaxMoney;
	int iBoxContributeValue;
	int iLastLoginDay;
	int iDayPlayNum;
	int iPropMaxNum;
	int iUniteAgentID;
	int iAreaID;
	char szAreaName[20];	//地区名
	int iMoneyExtra;
	int iInfullNum;
	int iDisNum;
	int iPlayAllNum;
	int iTodayPlayNum;//红包关闭客户收到值是-1
	int tmLastAccount;	
	char szUserName[20];
	int iTotalIntegral;
	int iJudgeIntegral;
	int iIfForceYB;//把操场地区停掉
	int iAllGameAmount;
	int iGameAmount;
	int iWinMoney;
	int iLostMoney;
	int iBeginGameTime;
	int iLeaveGameTime;
	int cVipType;
	int tmRegisterTime;
	int cSeeTableNumExtra;
	int bIfNeedAccount;
	int iLoginType;
};

#endif