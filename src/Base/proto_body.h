#ifndef __PROTO_BODY_H__
#define __PROTO_BODY_H__

#include "proto_type.h"
struct MsgHead						//消息头结构体
{
	char   cVersion;          		//版本
	unsigned char   cMsgType;       //类型
	int  iMsgBodyLen;       		//长度.备用
};
struct SocketConnectMsg//#define THR_SOCKET_CONNECT		0x01
{
	MsgHead msgHead;
	int iSocketFD;
	char szIP[20];
};
struct SocketDisconnectMsg//#define THR_SOCKET_DISCONNECT 0x02
{
	MsgHead msgHead;
	int iSocketFD;
	int iUserID;
	int iDisType;
};
struct KillPlayerMsg//#define THR_KILL_PLAYER_MSG 0x03	//主动断开玩家的连接
{
	MsgHead msgHead;
	int iFD;
};
struct AddRadiusMsg//#define THR_ADD_RADIUS_MSG 0x04	//添加radius连接
{
	MsgHead msgHead;
	int iType;
	char cName[32];
	char cIP1[20];
	int iPort1;
	char cIP2[20];
	int iPort2;
	int iAesEncrypt;
	int iUpdate;
};
struct ServerInfo//#define   GET_SERVER_INFO_MSG 0xDD		//获得服务器的消息
{
	MsgHead msgHeadInfo;
	int	iCenterServerID;		//连接的中心服务器的ID
	char iCenterServerIP[256];			//中心服务器IP 2016-5-23从int改为char
	unsigned short sCenterServerPort;		
	char iIP1[256];			//电信 2016-5-23从int改为char
	unsigned short sPort1;	
	char iIP2[256];			//网通 2016-5-23从int改为char
	unsigned short sPort2;	
	char iCenterServerIPBak[256];			//中心服务器bak 2016-5-23从int改为char
	unsigned short sCenterServerPortBak; //中心服务器port bak
};
struct ServerDetailInfo//#define   SEND_INFO_CENTER_SERVER_MSG 0xDE		//游戏服发送自己的IP信息到中心服务器
{
	MsgHead msgHeadInfo;
	char iIP1[256];			//电信
	unsigned short sPort1;	
	char iIP2[256];			//网通
	unsigned short sPort2;	
	int iBeginTime;					//开始时间
	int iOpenTime;					//开放时间
	int	iMaxNum;				//房间最多玩家数
	int	iChecked;				//是否开启
	int	iCurrentNum;				//当前游戏服务器上的人数
	int	iQuickMatchEpollUseNum;		//快速比赛场 目前使用中的子线程数量 最大10个场在比赛
};
struct KeepAlive//#define   KEEP_ALIVE_MSG 0xE0  //保持连接消息
{
	MsgHead		msgHead;
	char		szEmpty[4];
};
struct AuthenReq//#define		AUTHEN_REQ_MSG 0xE1	//玩家登陆验证请求
{
	MsgHead msgHead;                 
	int iUserID;//用户ID
	char cRoomNum;//房间号
	char szUserToken[32];//用户TOKEN
	char szIP[20];
	char szVersionNum[8];//游戏的版本号 V1.01 //add skyhawk 20080319 
	char cNowVerType;//当前功能版本号
	char cBuffer;//预留字段
	char cLoginType;//0是PC客户端，1是手机客户端
	char cNetFlag;//选择的网络线路，1电信2网通
	int iServerID;//掉线重入需要的真正的二级server_id
};
struct PlayerInfoResExtra//#define		AUTHEN_RES_MSG 0xE8	//登陆验证响应
{
	int  iUserID;//用户ID
	char szNickName[16];//昵称
	int  iAmount;//玩家积分
	int	 iMoney;//金币
	char cTableNum;//玩家所在桌号,没在桌上为0
	char cTableNumExtra;//玩家在桌上的位置，从最左边开始逆时针为0-3，
	int  iWinNum;//胜
	int  iLostNum;//负
	int	 iPunishNum;//掉线
	char cVipType;//超级玩家标记,100为GM
	char cSeeTableNumExtra;//旁观新增标记，，-1为非旁观
	char cIconNum;
	char cIfReady;//玩家是否ReadyOK
	int iIntegral;//玩家的红宝石数
	int iAreaID;//地区ID
	char szAreaName[20];//地区名
	int iExpTime;//经验值
	int iBuffA0;//每个游戏自己特有属性
	int iBuffA1;//每个游戏自己特有属性
	int iBuffA2;//每个游戏自己特有属性
	int iBuffB0;//每个游戏自己特有属性
	int iBuffB1;//每个游戏自己特有属性
	int iBuffB2;//每个游戏自己特有属性
	int iChitMoney;//代金券-渔券
	char cLoginType;//登录类型,电脑0 手机1
	char szNickName2[20];//玩家真实昵称	
};
struct AuthenRes               //AUTHEN_RES_MSG 登陆验证响应
{
	MsgHead	msgHead;
	char cResult; //0成功登陆必须在服务器从Radius得到该玩家信息后才算，1满员登陆失败, 2用户重复登陆,3TOKEN认证失败,4无法取得用户信息，5金币不够，6版本号不正确,7进入房间类型和用户的类型不一致，8金币过多，11服务器维护中 100不是比赛日的提示,101没有资格的人有购买的提示,102有资格的提示,103比赛倒计时状态,104比赛结束,105比赛快结束时提示没有资格玩家，106比赛倒计时状态并且没资格的玩家，107比赛进行中没有资格提示购买状态，108完成了比赛的局数，109比赛即将结束提示玩家是否参加决赛（有资格并且没有玩过），114网吧比赛开始了，115网吧特殊比赛完成了局数
	char szNickName[16];
	char cVipType;
	int iMinPoint;//博彩类专用，最小押注
	int iMaxPoint;//博彩类专用，最大押注
	int iPlayNum;//获得红包的局数	
	int iBasePoint;//博彩类专用，基本押注
	int iMinTime;//最小倍数
	int iMaxTime;//最大倍数
	int iServerID;//发现卡在其他游戏里面把卡的SERVERID和ROOMID返回
	int iCenterServerID;//发现卡哪个中心服务器
	int iGameType;//0是普通游戏，-1的是比赛游戏，大于0是能够获得比赛积分游戏也就获得资格赛的积分数
	int iServerTime;//服务器的当前时间
	PlayerInfoResExtra PlayerInfo;
	int	iPropMaxNum;//道具背包的最大空间
	int iIfChoiceMultiple;//是否是选择底分的游戏
	int iUserID;//服务器端回应过来ID
	int coupons;//点券数 add by alexhy 2016-3-11
	char headIconPath[255];//给客户端的头像 add by alexhy 2016-4-21
	int banMsgTime;//禁言到的日期 1970年开始的秒数 add by alexhy 2016-6-15
};
struct AuthenReqRadius//#define		AUTHEN_REQ_RADIUS_MSG 0x51//服务器登陆Radius请求
{
	MsgHead	 msgHeadInfo;
	int iServerID;
	char szPasswd[32];
	int iHideIndex;
};
struct UserInfoReqRadius//#define		USERINFO_REQ_RADIUS_MSG 0x52  //向Radius发送新登陆用户信息请求
{
	MsgHead msgHeadInfo;
	int iUserID;
	char szUserToken[32];
	char cRoomID;
	int iGameID;	//游戏ID
	int iCenterServerID;	//游戏ID
	char szGameName[100];//
};
struct GameRoomInfoReqRadius//#define		GAME_ROOM_INFO_REQ_RADIUS_MSG 0x54  //游戏服务器请求房间信息，结构体中附带房间人数
{
	MsgHead msgHeadInfo;
	char cIfNeedRoomInfo;			//是否需要房间信息，1需要
	char cRoomNum;						//房间数目
	int iRoomOnlineNum[MAX_ROOM_NUM];		//最多一个服务器6个房间,房间在线人数
	int iRoomOnlineNumReal[MAX_ROOM_NUM];//实际人数
	int iRoomOnlineNumMob[MAX_ROOM_NUM];//实际人数
};
struct AuthenResRadius//#define   	AUTHEN_RES_RADIUS_MSG 0x61//服务器登陆Radius请求回应
{
	MsgHead msgHeadInfo;
	char cResult;//1成功，0失败
};
struct UserInfoResRadius //#define   	USERINFO_RES_RADIUS_MSG 0x62  //用户信息回应
{
	MsgHead msgHeadInfo;
	char cResult;//Radius认证token结果，0未通过，1通过
	int iUserID;//用户id			
	char szNickName[16];//昵称(账号和szUserName相同)
	long long iAmount;//玩家净分
	int iMoney;//credit
	int iWinNum;//此游戏赢次数
	long long iWinMoney;//此游戏赢分数
	int iLostNum;//此游戏输次数
	long long iLostMoney;//此游戏输分数
	int iPunishNum;//此游戏掉线惩罚次数
	int iPunishMoney;//此游戏掉线惩罚分数
	int iOnlineTime;//此游戏在线时间
	char cIconNum;//图标
	char cVipType;//超级玩家标记,100为GM
	int iAllNum;//玩的总局数
	char szLastGameNum[20];//进入最后一局牌编号
	char szRealName[20];//真实名字
	int iIntegral;//玩家积分
	int iBankMoney;//银行钱
	long long iRateAllCoin;//设置几率的总玩分 20081023
	long long iRateWinCoin;//设置几率的总赢分 20081023
	int iRateSet;//设置几率
	int iRateCount;//设置几率玩的次数 20081023
	char szUserName[20];//用户帐号
	int iTotalIntegral;//总得金元宝（包括已被兑换的）
	int iJudgeIntegral;//用于判断玩家得元宝难度级别的元宝数
	int iServerID;//发现卡在其他游戏里面的SERVERID
	int iCenterServerID;//发现卡哪个中心服务器
	float fExpTime;//经验值
	int iMoneyExtra;//充值贡献度
	int iAreaID;//地区ID
	char szAreaName[20];//来自的地区名
	int iInfullNum;//充值的星币数
	int iDisNum;//总掉线次数
	int iBuffA0;//用于几率控制的正的小Buff
	int iBuffA1;//用于几率控制的正的中Buff
	int iBuffA2;//用于几率控制的正的大Buff
	int iBuffB0;//用于几率控制的负的小Buff
	int iBuffB1;//用于几率控制的负的中Buff
	int iBuffB2;//用于几率控制的负的大Buff
	int iChitMoney;//代金券-渔券
	int iHisMaxMoney;
	int iBoxContributeValue;//炮座贡献度
	int	iPlayAllNum;//所有游戏玩的总局数
	int iAllGameAmount;//所有游戏输赢总净分
	char cLoginType;//登录类型,电脑0 手机1
	int	iPropMaxNum;//道具背包的最大空间
	int	iUniteAgentID;//联合运营商ID
	char szNickName2[20];//玩家真实昵称
	int iRegisterTime;//注册时间
	int  iNoLoginDay;//未登录天数
	int iDayPlayNum;//每天玩的局数
	int iTicketNum;//奖券数 add by alexhy 2016-2-23
	char iconPath[255];//头像路径  add by alexhy 2016-2-26
	int coupons;//点券数 add by alexhy 2016-3-11	
	int banMsgTime;//禁言到的日期
	int noawardTime;//零收益到的1970秒数 add by alexhy 2016-8-5
};
struct GameRoomInfoResRadius//#define GAME_ROOM_INFO_RES_RADIUS_MSG 0x64//游戏服务器房间信息请求回应，附带赠送金币数目一起吧
{
	MsgHead msgHeadInfo;
	char cIfIPLimit[MAX_ROOM_NUM];			//是否有IP限制
	int iMoneyLimitation[MAX_ROOM_NUM];//进入的金钱限制
	int iMinTime[MAX_ROOM_NUM];				//最小倍数
	int iMaxTime[MAX_ROOM_NUM];				//最大倍数
	int iRoomMoney[MAX_ROOM_NUM];			//进入房间的使用费
	int iGetRate[MAX_ROOM_NUM];				//抽成比例千分比
	int iAmountMoneyRate[MAX_ROOM_NUM];//比例兑换
	int iBasePoint[MAX_ROOM_NUM];			//博彩游戏基础压分
	int iMinPoint[MAX_ROOM_NUM];				//博彩游戏最小压住
	int iMaxPoint[MAX_ROOM_NUM];				//博彩游戏最大押注
	char cJackPotRate[MAX_ROOM_NUM];		//赢家抽成里的作为JACKPOT的比例
	char cIfFreeRoom[MAX_ROOM_NUM];			//是否是免费房间
	int iPresentMoney[MAX_ROOM_NUM];		//在线赠送钱数
	int iTableMoney[MAX_ROOM_NUM];			//每局的桌费 add skyhawk
	int iApplyMoney[MAX_ROOM_NUM];			//申请查看游戏记录的钱 add skyhawk
	int iOnetimeMaxWin[MAX_ROOM_NUM];	//单局最大输赢 add skyhawk
	int iAppealMoney[MAX_ROOM_NUM];		//申授费用 add skyhawk
	int iHighWinRate[MAX_ROOM_NUM];		//胜率的上线  add skyhawk
	int iLowWinRate[MAX_ROOM_NUM];			//胜率的下线 add skyhawk
	char cSeatType[MAX_ROOM_NUM];				//分配座位的类型 add skyhawk
	char cVipType[MAX_ROOM_NUM][50];		//允许玩家进入房间的类型 add skyhawk
	char cAgentLimit[MAX_ROOM_NUM];			//是否有上级代理限制add skyhawk
	char cIfLastGameNum[MAX_ROOM_NUM];	//是否启动限制上次在一起玩过坐在同一桌 add skyhawk
	char cIfRoomChecked[MAX_ROOM_NUM];	//房间是否开启 add skyhawk
	char cBeginTime[MAX_ROOM_NUM];			//房间的开启是时间 add skyhawk
	char cLongTime[MAX_ROOM_NUM];				//房间的开启的时间长度 add skyhawk
	int iJackpotRatio[MAX_ROOM_NUM];		//房间的彩金倍率 add by dream
	int iPlayType[MAX_ROOM_NUM];				//区分积分玩家或是星币玩家 add by dream
	int iKickLimit[MAX_ROOM_NUM];			//将玩家踢出的钱的限制 add by dream
	int iTaskType[MAX_ROOM_NUM];				//房间的任务类型是彩金还是积分 add dream
	int iRoomIndex[MAX_ROOM_NUM];			//房间的Id编号  add dream
	int iJSID1[MAX_ROOM_NUM];					//大累积彩金集合序列号 add by dream
	int iJSID2[MAX_ROOM_NUM];					//小累积彩金集合序列号 add by dream
	int iJSID3[MAX_ROOM_NUM];					//任务金元宝集合序列号 add by dream
	int iJSID4[MAX_ROOM_NUM];					//任务金币集合序列号 add by dream
	int iExpBase[MAX_ROOM_NUM];				//房间的经验值系数 add by crystal
	char cRoomOnlineName[MAX_ROOM_NUM][20];//在线房间名
	char cJackPotRate2[MAX_ROOM_NUM];//赢家抽成里的作为小累计JACKPOT的比例
	int iMaxIntegralDay[MAX_ROOM_NUM];	//该类房间每天最多可以中的元宝个数
	int iIntegralRatio[MAX_ROOM_NUM];	//比赛星币元宝兑换比率
	int iBonusPercentage[MAX_ROOM_NUM];//比赛奖金占抽成比例
	int iPS_ID[MAX_ROOM_NUM];					//道具集合
	int iMaxRoomNum[MAX_ROOM_NUM];          //房间的最大人数
};
struct UpdatePlayerNumReq//游戏服务器人数更新消息 UPDATA_CENTER_SERVER_PNUM_MSG
{
	MsgHead	 msgHeadInfo;
	int	iUpdateNum;		//进1，出-1
	int totalNum; //总人数 add by alexhy 2016-6-28
};
struct QueryCenterServerMsg		//定时查询中心服务器请求
{
	MsgHead msgHeadInfo;	
	int serverId;
};
struct CenterServerInfo  //定时查询中心服务器结果
{
	MsgHead msgHeadInfo;
	int	iCenterServerID;		//连接的中心服务器的ID
	char iCenterServerIP[256];			//中心服务器IP
	unsigned short sCenterServerPort;	
};
struct KickOutServer		//KICK_OUT_SERVER_MSG 踢人
{
	MsgHead msgHeadInfo;
	char cType;//类型，3为金币不够,4账号被禁，8为金币过多，9账号在后台被踢，11上级代理被禁， 12你被另一个玩家踢掉13你账号在游戏中，19房间关闭，20延时太高，21比赛已经结束，22完成了比赛的局数,23多次不出牌,29完成了网吧比赛局数,30选择不同底分时星币够被踢
	int	iKickUserID;
	char szKickName[16];//GM的话附带GM的名字给玩家 //客户端和服务器端不统一 ？？？skyhawk
};


struct LogMsgHeadDef//#define		RADIUS_LOG_MAIN_INFO_REQ 	0x70//游戏日志消息类型
{
	MsgHead			msgHead;
	unsigned char	cChildType;
	unsigned char	cLoginType;
	int 			iUserID;
	char 			cUserName[20];
	int 			iAgentID;
	int 			iRoomType;
};
struct LogGetPropLog//#define CHILD_GET_PROP_LOG_REQ 0x02//道具获得日志
{
	LogMsgHeadDef 	logHead;
	int  			iPropID;			//道具ID
	int 			iBeforePropNum;		//发放前数量 
	int 			iSendPropNum;		//发放数量
	int 		 	iEndPropNum;		//发放后数量
	int 		 	iTotalPropNum;		//历史获得次数 
	int  			iDropFish;			//掉落鱼种
	int 			iBetValue;			//掉落的时候的压分
	int 			iSendType;			//
	int 			iGameTime;			//
};


#endif