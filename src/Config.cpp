#include "Config.h"
#include "conf.h"
#include "log.h"
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include "lib/LuaLoader.h"
#include "lib/ServerLib.h"

using namespace std;

Config* Config::_instance = 0;
Config::Config()
{	
	grid_pic_def[0] = PIC_Seven;
	grid_pic_def[1] = PIC_Blue_Bar;
	grid_pic_def[2] = PIC_Red_Bar;
	grid_pic_def[3] = PIC_Green_Bar;
	grid_pic_def[4] = PIC_Bell;
	grid_pic_def[5] = PIC_Watermelon;
	grid_pic_def[6] = PIC_Grape;
	grid_pic_def[7] = PIC_Orange;
	grid_pic_def[8] = PIC_Cherry;
}

Config* Config::Instance()
{
	if(!_instance)
	{
		_instance = new Config();
	}
	return _instance;
}

GridPic get_grid_picture(std::string& name)
{
	if(name == "seven") return PIC_Seven;
	else if(name == "blue") return PIC_Blue_Bar;
	else if(name == "red") return PIC_Red_Bar;
	else if(name == "green") return PIC_Green_Bar;
	else if(name == "bell") return PIC_Bell;
	else if(name == "watermelon") return PIC_Watermelon;
	else if(name == "grape") return PIC_Grape;
	else if(name == "orange") return PIC_Orange;
	else if(name == "cherry") return PIC_Cherry;
	else{
		_log(_ERROR,"ConfigError","get_grid_picture name[%s]",name.c_str());
		return PIC_Cherry;
	}
}

void Config::LoadConfig()
{
	static bool firstload = true;
	string file = Server::Inst()->config_data_path+"/game_config.lua";
	bool fileupdated;
	LuaConfig* lc = LuaLoader::LoadFile(file.c_str(),fileupdated);
	if(!lc || fileupdated == false)
	{
		if(firstload) exit(0);
		return;
	}
	firstload = false;
	LTable* t = lc->GetTable("quanpan_bet");
	if(t)
	{
		vector<int> temp = t->GetInts();
		copy(temp.begin(),temp.end(),quanpan_bet);
		_log(_ERROR,"Config","LoadConfig quanpan_bet[%d][%d]...[%d][%d]",quanpan_bet[0],quanpan_bet[1],quanpan_bet[9],quanpan_bet[10]);
	}
	t = lc->GetTable("quanpan_rate");
	if(t)
	{
		vector<int> temp = t->GetInts();
		copy(temp.begin(),temp.end(),quanpan_rate);
		_log(_ERROR,"Config","LoadConfig quanpan_rate[%d][%d]...[%d][%d]",quanpan_rate[0],quanpan_rate[1],quanpan_rate[9],quanpan_rate[10]);
	}
	t = lc->GetTable("seven_mul");
	if(t)
	{
		vector<int> temp = t->GetInts();
		copy(temp.begin(),temp.end(),num_seven_bet);
		_log(_ERROR,"Config","LoadConfig num_seven_bet[%d][%d]...[%d][%d]",num_seven_bet[0],num_seven_bet[1],num_seven_bet[7],num_seven_bet[8]);
	}
	t = lc->GetTable("grid_rate");
	if(t)
	{
		unsigned int num = t->ChildTableSize();
		for(unsigned int i=0;i<num;++i)
		{
			vector<int> temp = t->GetChild(i)->GetInts();
			copy(temp.begin(),temp.end(),grid_rate[i]);
			_log(_ERROR,"Config","LoadConfig grid[%d] rate[%d][%d]...[%d][%d]",i,grid_rate[i][0],grid_rate[i][1],grid_rate[i][7],grid_rate[i][8]);
		}
	}
}


//////////////////////////Server///////////////////////////
Server* Server::_instance = 0;
Server* Server::Inst()
{
	if(!_instance) _instance = new Server();
	return _instance;
}
Server::Server()
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
	main_config_file = "./server.conf";
	config_data_path = ".";
	game_name = "Star97";
}
void Server::InitServer(ServerInfo* _server)
{
	center_server_id = _server->iCenterServerID;
	center_server_ip = _server->iCenterServerIP;
	center_server_port = _server->sCenterServerPort;
	center_server_ip_bak = _server->iCenterServerIPBak;
	center_server_port_bak = _server->sCenterServerPortBak;
	ip1 = _server->iIP1;
	port1 = _server->sPort1;
	ip2 = _server->iIP2;
	port2 = _server->sPort2;
	
	_log(_ERROR,"Server","InitServer Center[%d] ip1[%s][%d] ip2[%s][%d]",
		center_server_id,center_server_ip.c_str(),center_server_port,center_server_ip_bak.c_str(),center_server_port_bak);
	_log(_ERROR,"Server","InitServer Server[%d] ip1[%s][%d] ip2[%s][%d]",
		server_id,ip1.c_str(),port1,ip2.c_str(),port2);
}
//////////////////////////Server End///////////////////////////

//////////////////////////Room////////////////////////
Room* Room::_instance = 0;
Room* Room::Inst()
{
	if(!_instance) _instance = new Room();
	return _instance;
}
Room::Room()
{
	room_name = "初级房";
}
int Room::CloseLeftSec()
{
	int endHour = iBeginTime+iLongTime;
	if(iLongTime >= 24 || endHour >= 24)
	{
		return 999999999;
	}
	if(Timer::localTM->tm_hour >= iBeginTime && Timer::localTM->tm_hour < endHour)
	{
		return (endHour-Timer::localTM->tm_hour-1)*3600+Timer::localTM->tm_min*60+Timer::localTM->tm_sec;
	}
	return 0;
}
bool Room::CheckVipType(int _vip)
{
	for(size_t i=0;i<vecVipTypeLimit.size();++i)
	{
		if(vecVipTypeLimit[i] == _vip) return true;
	}
	return false;
}
void Room::InitRoom(GameRoomInfoResRadius* pRoom)
{
	iRoomMoney = ntohl(pRoom->iRoomMoney[0]);
	iMoneyLimitation = ntohl(pRoom->iMoneyLimitation[0]);
	iGetRate = ntohl(pRoom->iGetRate[0]);
	iMinTime = ntohl(pRoom->iMinTime[0]);
	iMaxTime = ntohl(pRoom->iMaxTime[0]);
	iAmountMoneyRate = ntohl(pRoom->iAmountMoneyRate[0]);
	iBasePoint = ntohl(pRoom->iBasePoint[0]);
	iMinPoint = ntohl(pRoom->iMinPoint[0]);
	iMaxPoint = ntohl(pRoom->iMaxPoint[0]);
	iIPLimit = pRoom->cIfIPLimit[0];
	
	iIfFreeRoom = pRoom->cIfFreeRoom[0];
	iTableMoney = ntohl(pRoom->iTableMoney[0]);
	iApplyMoney = ntohl(pRoom->iApplyMoney[0]);
	iOnetimeMaxWin = ntohl(pRoom->iOnetimeMaxWin[0]);
	iAppealMoney = ntohl(pRoom->iAppealMoney[0]);
	
	iHighWinRate = ntohl(pRoom->iHighWinRate[0]);
	iLowWinRate = ntohl(pRoom->iLowWinRate[0]);
	iSeatType = pRoom->cSeatType[0];
	vecVipTypeLimit.clear();
	parse_string_vector(vecVipTypeLimit,pRoom->cVipType[0],",");
	iIfLastGameNum = pRoom->cIfLastGameNum[0];
	iIfRoomChecked = pRoom->cIfRoomChecked[0];
	iBeginTime = pRoom->cBeginTime[0];
	iLongTime = pRoom->cLongTime[0];
	iJackpotRatio = ntohl(pRoom->iJackpotRatio[0]);
	iPlayType = ntohl(pRoom->iPlayType[0]);
	iKickLimit = ntohl(pRoom->iKickLimit[0]);
	iTaskType = ntohl(pRoom->iTaskType[0]);
	iRoomIndex = ntohl(pRoom->iRoomIndex[0]);
	iJSID1 = ntohl(pRoom->iJSID1[0]);//rwfl add
	iJSID2 = ntohl(pRoom->iJSID2[0]);//rwfl add
	iJSID3 = ntohl(pRoom->iJSID3[0]);//rwfl add
	iJSID4 = ntohl(pRoom->iJSID4[0]);//rwfl add
	iExpBase = ntohl(pRoom->iExpBase[0]);//rwfl add
	memcpy(cRoomOnlineName,pRoom->cRoomOnlineName[0],19);
	room_name = string(pRoom->cRoomOnlineName[0]);
	iOneDayYB = ntohl(pRoom->iMaxIntegralDay[0]);
	iBonusPercentage = ntohl(pRoom->iBonusPercentage[0]);
	iMaxRoomPlayer = ntohl(pRoom->iMaxRoomNum[0]);
	
	if(iBeginTime >24 && iBeginTime <0)
	{
		iBeginTime = 0;
	}
	
	if(iLongTime >24 )
	{
		iLongTime = 24;
	}
	else if(iLongTime < 0 )
	{
		iLongTime = 0;
	}
	_log(_ERROR,"RoomInfo","=== iRoomIndex[%d] begin[%d] last[%d] enter[%d] kick[%d] bet[%d]",
		 iRoomIndex,iBeginTime,iLongTime,iMoneyLimitation,iKickLimit,iBasePoint);
	_log(_ERROR,"RoomInfo","=== RoomType[%d] MaxWin[%d] Checked[%d] MaxPlayer[%d]",
		 iIfFreeRoom,iOnetimeMaxWin,iIfRoomChecked,iMaxRoomPlayer);
}

//////////////////////////Room End////////////////////////


//////////////////////////Timer//////////////////////////
time_t Timer::tmNow = 0;
struct tm* Timer::localTM;
void Timer::Update()
{
	time(&tmNow);
	localTM = localtime(&tmNow);
}

//////////////////////////Timer End//////////////////////



