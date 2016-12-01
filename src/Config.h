#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <vector>
#include "proto_body.h"

#define ALL_GRID_NUM 9
#define ALL_PIC_NUM 9
#define QUANPAN_TYPE_NUM 11
enum GridPic
{
	PIC_Seven		= 1,
	PIC_Blue_Bar	= 2,
	PIC_Red_Bar		= 3,
	PIC_Green_Bar	= 4,
	PIC_Bell		= 5,
	PIC_Watermelon	= 6,
	PIC_Grape		= 7,
	PIC_Orange		= 8,
	PIC_Cherry		= 9,
};
struct LineDef
{	
	int first;
	int second;
	int third;
};
struct LineBet
{
	int first;
	int second;
	int third;
	int bet;
};

class Config
{
public:
	static Config* Instance();
	void LoadConfig();
private:
	GridPic grid_pic_def[ALL_PIC_NUM];
	int grid_rate[ALL_GRID_NUM][ALL_PIC_NUM];
	int num_seven_bet[ALL_GRID_NUM];
	int quanpan_rate[QUANPAN_TYPE_NUM];
	int quanpan_bet[QUANPAN_TYPE_NUM];
	std::vector<LineDef> all_lines;
	std::vector<LineBet> line_bets;
private:
  Config();
  static Config* _instance;
};

struct RadiusConf
{
	std::string ip1;
	int port1;
	std::string ip2;
	int port2;
};
class Server
{
public:
	static Server* Inst();
	static Server* _instance;
	void InitServer(ServerInfo* _server);
private:
	Server();
public:
	int game_id;
	int server_id;
	int server_port;
	int heart_time;
	int server_type;
	int work_thread_num;
	
	std::string log_lev;
	std::string log_file_path;
	std::string server_name;
	std::string main_config_file;
	std::string config_data_path;
	std::string game_name;
	
	RadiusConf room_server;
	RadiusConf account_server;
	RadiusConf log_server;

	int	center_server_id;				//连接的中心服务器的ID
	std::string center_server_ip;			//中心服务器IP 2016-5-23从int改为char
	int center_server_port;
	std::string center_server_ip_bak;		//中心服务器bak 2016-5-23从int改为char
	int center_server_port_bak;
	std::string ip1;			//电信 2016-5-23从int改为char
	int port1;
	std::string ip2;			//网通 2016-5-23从int改为char
	int port2;
};
class Room
{
public:
	static Room* Inst();
	static Room* _instance;
	void InitRoom(GameRoomInfoResRadius* pRoom);
	int CloseLeftSec();//-1 24小时房间 0已经关闭 
	bool CheckVipType(int _vip);
private:
	Room();
public:
	std::string room_name;
	int  iIPLimit;//是否有IP限制
	int	 iMoneyLimitation;//进入的金钱限制
	int  iMinTime;//最小倍数
	int  iMaxTime;//最大倍数
	int  iRoomMoney;//进入房间的使用费
	int	 iGetRate;	//抽成比例
	int	 iAmountMoneyRate;//积分与金币的比例兑换
	int	 iBasePoint;//博彩游戏基础压分
	int	 iMinPoint;//博彩游戏最小压住
	int	 iMaxPoint;//博彩游戏最大押注
	int  iIfFreeRoom;//是否是免费房间
	int  iTableMoney;//每局的桌费 add skyhawk
	int  iApplyMoney;//申请查看游戏记录的钱 add skyhawk
	int	 iOnetimeMaxWin;//单局最大输赢 add skyhawk
	int	 iAppealMoney;//申授费用 add skyhawk
	int  iHighWinRate;//胜率的上线  add skyhawk
	int  iLowWinRate;//胜率的下线 add skyhawk
	int  iSeatType;//分配座位的类型 add skyhawk
	std::vector<int> vecVipTypeLimit;
	int	 iIfLastGameNum;//是否启动限制上次在一起玩过坐在同一桌 add skyhawk
	int	 iIfRoomChecked;//房间是否开启 add skyhawk
	int  iBeginTime;//房间的开启是时间 add skyhawk
	int  iLongTime;//房间的开启的时间长度 add skyhawk
	int	 iNoQueue;//是否排队0是要排队的，1不排队
	int  iMinPlayer;//梭哈类游戏所需要的最少开始玩家个数
	//CardTypeUser userCardType;//牌型客户端 用户的结构体
	int  iJackpotRatio;//房间的彩金倍率 add skyhawk
	int  iPlayType;//区分积分玩家或是星币玩家 add skyhawk
	int  iKickLimit;//将玩家踢出的钱的限制 add skyhawk
	int	 iVersionType;//0表示简体，1表示繁体 add skyhawk
	int  iTaskType;//房间的任务类型是彩金还是积分 add skyhawk
	int	 iMSProportion;//大累计彩金的分得的百分比
	int	 iRobotSocketIndex;//机器人服务器连接服务器的Socket索引
	int	 iTableRobotMax;//桌子上面最大机器人数
	int	 iJPUpdateTime;//每天什么时间更新彩金任务
	int  iRoomIndex;//房间的Id编号  add skyhawk
	int	 iJSID1;	//大累积彩金集合序列号 add by skyhawk rwfl add
	int	 iJSID2;	//小累积彩金集合序列号 add by skyhawk
	int	 iJSID3;	//任务金元宝集合序列号 add by skyhawk
	int	 iJSID4;	//任务金币集合序列号 add by skyhawk
	int  iExpBase;				//房间的经验值系数 add by crystal
	int	 iGameType;	//0是玩星币的，1是玩金元宝
	int  iLogSendMoney;	//发送LOG的星币值
	int  iLogSendYB;		//发送LOG的元宝值
	char szServerName[32];
	int	 iLogTime;
	int	 iMaxRoomPlayer;
	int	 iSendBullJYBNum;
	int  iSeatWinRate[3];	//技术分级胜率1
	int	 iLimitBigPJSendYB;//大累计金元宝发放控制数目
	int	 iLimitBigPJSendXB;//大累计星币发放控制数目
	int	 iLimitSmallPJSendYB;//小累计金元宝发放控制数目
	int	 iLimitSmallPJSendXB;//小累计星币发放控制数目
	char cRoomOnlineName[20];//在线房间名
	int	 iBullPJYBNum;//累积金元宝公告通知是元宝的个数
	int iFDPutOutRate;//普通模式福袋的发放概率
	int iFDWinerPutOutRate;//获胜玩家获得概率
	int iFDPutOutNumRate[4];//档次发放的概率
	int iFDPutOutNumMin[4];//档次发放元宝最小值
	int iFDPutOutNumMax[4];//档次发放元宝最大值
	int iFDWinMoneyMin[4];//档次赢分模式最小值
	int iFDWinMoneyMax[4];//档次赢分模式最大值
	int iGameType2;//0是普通游戏，-1的是比赛游戏，大于0是能够获得比赛积分游戏也就获得资格赛的积分数
	int iOneDayYB;
	int iMoneyExtraBase;
	int iJBPSendContributeRate;//强制发放聚宝盆后每个元宝对应的贡献值的比例关系
	int iHideIndex;//分组的服务器编号
	int iAllGameType;//0表示A类游戏，1表示B类游戏
	int iIfConnectBonusServer;//1是连接，0不连接
	int	iIntegralRatio;	//比赛星币元宝兑换比率
	int	iBonusPercentage;//比赛奖金占抽成比例
	int iLimitBigPJYBMax;//大累计金元宝的上限
	int iLimitSmallPJYBMax;//小累计金元宝的上限
	int iTestNet;//记录网络延时log的值，是毫秒为单位
	int iTaskControlLimit;//控制任务元宝3中值的发放概率条件
	int iTaskNumControlRate[2];//控制任务元宝3值的发放概率(根据净分条件)
	int iTaskNumControlRate2[7][2];//控制任务元宝3值的发放概率(默认)
	int iIfConnectReplayServer;//1是连接，0不连接
	int iMoneyExtraRate;//充值贡献度，需要强制控制的几率，百分数
	int iMoneyExtraLimit;//充值贡献度,强制发牌的条件（是在游戏中当前元宝贡献度值）
	int iNewRobotSeat;//1是新的小洛分配座位
	int iNewRobotSeatRate;//真实玩家和分配小洛座位的比例
	int iSeatRobotNumLimit;//进入小洛特殊座位玩的游戏局数限制（是限制连续在游戏中完的玩家）
	int	iTermSocketIndex;//控制终端连接服务器的Socket索引
	//SetPlayerControlDef setPlayerControl;//玩家几率的控制条件
	int iCleanRobotAllCoinLimit;//清除小洛的总玩分数和总赢分数的条件
	int iMobilePhoneMaxPlayer;//手机进入游戏的人数
	int iIfStartHB;//是否开启红包
	int iSpreaderGetMoney;//推广员单次获得星币数
	int iIfConnectCenterServer;//是否连接中心服务器，1是连接，0不连接
	int iCenterServerID;//中心服务器ID
	int iIfChoiceMultiple;//是否是可选倍数的房间
	int iLimitMoney[3];//选择不同底分需要最少的星币
	int iFlashGame;//是不是flash游戏
	int iHBXBMin;//红包发放星币最小值
	int iHBXBMax;//红包发放星币最大值
	int iIfStopPutOutTryoutTicket;//是否停止发放预赛券
	int iIFTryPlay;//是否是试玩游戏
	int iIfConnectACTRadius;//是否连接活动计费服务器
	int iCloseRoomDelayTime;//关闭房间延时的时间
	int iAppMTime; //应用程序修改时间
	int iTotalYBContributeNum;//总累计元宝贡献度
	int iMaxYBContributeNum;//最大累计元宝贡献度
	int iIfConnectPush;//是否连接推送服务器
	int iIfShowMoneyBull;   //是否显示星豆公告
	int iRobotShowMoneyExt; //小洛客户端显示星豆的额外值
	int iIntoGameTimeDef; //进入游戏时间差
	int channel; //渠道 add by alexhy 2016-3-5	
	char hallUrl[255];  //大厅url add by alexhy 2016-5-4
};
class Timer
{
public:
	static void Update();
	static time_t tmNow;
	static struct tm* localTM;
};

#endif