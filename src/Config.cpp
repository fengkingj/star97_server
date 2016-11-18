#include "Config.h"
#include "conf.h"
#include "log.h"
#include <string.h>
#include <string>
#include <arpa/inet.h>

using namespace std;

Config* Config::_instance = 0;
Config::Config()
{
	game_id = 249;
	server_id = 20002;
	server_port = 56789;
	heart_time = 60;
	server_type = 1;
	work_thread_num = 4; 
	
	log_lev = "debug";
	log_file_path = ".";
	server_name = "Star97";
	room_name = "初级房";
	main_config_file = "./server.conf";
	config_data_path = ".";
	game_name = "Star97";
	
	memset(&ROOM,0,sizeof(ServerRoomInfo));
}

Config* Config::Instance()
{
	if(!_instance)
	{
		_instance = new Config();
	}
	return _instance;
}

void parse_string_vector(vector<int>& vec, char *from ,const char *div)
{
	char *p=NULL;
	while((p=strsep(&from,div)))
	{			
		vec.push_back(atoi(p));
	}	
}

void Config::LoadConfig()
{

}
void Config::SetRoomInfo(GameRoomInfoResRadius* pRoom)
{
	ROOM.iRoomMoney = ntohl(pRoom->iRoomMoney[0]);
	ROOM.iMoneyLimitation = ntohl(pRoom->iMoneyLimitation[0]);
	ROOM.iGetRate = ntohl(pRoom->iGetRate[0]);
	ROOM.iMinTime = ntohl(pRoom->iMinTime[0]);
	ROOM.iMaxTime = ntohl(pRoom->iMaxTime[0]);
	ROOM.iAmountMoneyRate = ntohl(pRoom->iAmountMoneyRate[0]);
	ROOM.iBasePoint = ntohl(pRoom->iBasePoint[0]);
	ROOM.iMinPoint = ntohl(pRoom->iMinPoint[0]);
	ROOM.iMaxPoint = ntohl(pRoom->iMaxPoint[0]);
	ROOM.iIPLimit = pRoom->cIfIPLimit[0];
	
	ROOM.iIfFreeRoom = pRoom->cIfFreeRoom[0];
	ROOM.iTableMoney = ntohl(pRoom->iTableMoney[0]);
	ROOM.iApplyMoney = ntohl(pRoom->iApplyMoney[0]);
	ROOM.iOnetimeMaxWin = ntohl(pRoom->iOnetimeMaxWin[0]);
	ROOM.iAppealMoney = ntohl(pRoom->iAppealMoney[0]);
	
	ROOM.iHighWinRate = ntohl(pRoom->iHighWinRate[0]);
	ROOM.iLowWinRate = ntohl(pRoom->iLowWinRate[0]);
	ROOM.iSeatType = pRoom->cSeatType[0];
	ROOM.vecVipTypeLimit.clear();
	parse_string_vector(ROOM.vecVipTypeLimit,pRoom->cVipType[0],",");
	ROOM.iIfLastGameNum = pRoom->cIfLastGameNum[0];
	ROOM.iIfRoomChecked = pRoom->cIfRoomChecked[0];
	ROOM.iBeginTime = pRoom->cBeginTime[0];
	ROOM.iLongTime = pRoom->cLongTime[0];
	ROOM.iJackpotRatio = ntohl(pRoom->iJackpotRatio[0]);
	ROOM.iPlayType = ntohl(pRoom->iPlayType[0]);
	ROOM.iKickLimit = ntohl(pRoom->iKickLimit[0]);
	ROOM.iTaskType = ntohl(pRoom->iTaskType[0]);
	ROOM.iRoomIndex = ntohl(pRoom->iRoomIndex[0]);
	ROOM.iJSID1 = ntohl(pRoom->iJSID1[0]);//rwfl add
	ROOM.iJSID2 = ntohl(pRoom->iJSID2[0]);//rwfl add
	ROOM.iJSID3 = ntohl(pRoom->iJSID3[0]);//rwfl add
	ROOM.iJSID4 = ntohl(pRoom->iJSID4[0]);//rwfl add
	ROOM.iExpBase = ntohl(pRoom->iExpBase[0]);//rwfl add
	memcpy(ROOM.cRoomOnlineName,pRoom->cRoomOnlineName[0],19);
	room_name = string(pRoom->cRoomOnlineName[0]);
	ROOM.iOneDayYB = ntohl(pRoom->iMaxIntegralDay[0]);
	ROOM.iBonusPercentage = ntohl(pRoom->iBonusPercentage[0]);
	ROOM.iMaxRoomPlayer = ntohl(pRoom->iMaxRoomNum[0]);
	
	if(ROOM.iBeginTime >24 && ROOM.iBeginTime <0)
	{
		ROOM.iBeginTime = 0;
	}
	
	if(ROOM.iLongTime >24 )
	{
		ROOM.iLongTime = 24;
	}
	else if(ROOM.iLongTime < 0 )
	{
		ROOM.iLongTime = 0;
	}
	_log(_ERROR,"RoomInfo","=== iRoomIndex[%d] begin[%d] last[%d] enter[%d] kick[%d] bet[%d]",
		 ROOM.iRoomIndex,ROOM.iBeginTime,ROOM.iLongTime,ROOM.iMoneyLimitation,ROOM.iKickLimit,ROOM.iBasePoint);
}
bool Config::CheckVipType(int viptype)
{
	for(size_t i=0;i<ROOM.vecVipTypeLimit.size();++i)
	{
		if(ROOM.vecVipTypeLimit[i] == viptype) return true;
	}
	return false;
}






