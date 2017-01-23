#include <vector>
#include <string.h>
#include <iostream>
#include <stdio.h>  
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/stat.h>

#include "Base/SimpleQueue.h"
#include "log.h"
#include "conf.h"
#include "Base/ClientConnection.h"
#include "Base/GateService.h"
#include "Base/Config.h"
#include "Base/RadiusService.h"
#include "Star97/Star97ServiceManage.h"
#include "Base/Package.h"

using namespace std;

void InitLog()
{	
	LogConfDef conf;
	memset(&conf, 0, sizeof(LogConfDef));
	conf.flag = _LOG_CONSOLE | _LOG_FILE | _LOG_FILE_DAILY;
	conf.format = LOG_DATE | LOG_TIME | LOG_LEVEL | LOG_MODULE | LOG_MESSAGE | LOG_HEX;
	
	if(Server::Inst()->log_lev == "error") conf.level = _ERROR;
	else if(Server::Inst()->log_lev == "warn") conf.level = _WARN;
	else if(Server::Inst()->log_lev == "note") conf.level = _NOTE;
	else if(Server::Inst()->log_lev == "all") conf.level = _ALL;
	else conf.level = _DEBUG;
	
	strcpy(conf.log_file_name,Server::Inst()->log_file_path.c_str());
	DIR *dir = opendir(conf.log_file_name);
	if (!dir)
	{
		mkdir(conf.log_file_name, 0700);
	}
	strcat(conf.log_file_name,"/game");
	printf("====== log_file[%s]\n",conf.log_file_name);
	init_log_conf(conf);
}
void LoadRadiusConfig(RadiusConf& rd,const char* cname1,const char* cname2,const char* csection)
{
	char cTemp[128],cIP[32];
	
	GetValueStr(cTemp,cname1,Server::Inst()->main_config_file.c_str(),csection,0);
	sscanf(cTemp,"%[^:]:%d",cIP,&rd.port1);
	rd.ip1 = cIP;
	
	GetValueStr(cTemp,cname2,Server::Inst()->main_config_file.c_str(),csection,0);
	sscanf(cTemp,"%[^:]:%d",cIP,&rd.port2);
	rd.ip2 = cIP;
}
void InitArg(int argc, char *argv[])
{
	struct stat buf;
	int ret = stat(argv[0],&buf);
	if(ret == 0)
	{
		Room::Inst()->iAppMTime = buf.st_mtime; 
	}
	
	if(argc>1) //有参数
	{
		int argIndex=1;
		while(true)
		{
			if(strcmp(argv[argIndex],"-config")==0)
			{
				Server::Inst()->main_config_file = string(argv[argIndex+1])+"/server.conf";
				Server::Inst()->server_name = argv[argIndex+2];
				argIndex +=3;
			}
			else if(strcmp(argv[argIndex],"-serverport")==0)
			{
				sscanf(argv[argIndex+1],"%d",&(Server::Inst()->server_port));
				argIndex +=2;
			}
			else if(strcmp(argv[argIndex],"-serverid")==0)
			{
				sscanf(argv[argIndex+1],"%d",&(Server::Inst()->server_id));
				argIndex +=2;		
			}
			else if(strcmp(argv[argIndex],"-type")==0)
			{
				sscanf(argv[argIndex+1],"%d",&(Server::Inst()->server_type));
				argIndex +=2;		
			}
			if(argIndex>=argc)
			{
				break;
			}
		}	 
	}
	
	string mainFile = Server::Inst()->main_config_file;
	char cSection[24] = {0};
	sprintf(cSection,"Server_%s",Server::Inst()->server_name.c_str());
	
	char cTemp[256] = {0};
	GetValueStr(cTemp,"log_level",mainFile.c_str(),cSection,"");
	if(cTemp[0] == 0)
	{
		memset(cSection,0,sizeof(cSection));
		strcpy(cSection,"Common");
	}
	
	memset(cTemp,0,sizeof(cTemp));
	GetValueStr(cTemp,"log_level",mainFile.c_str(),cSection,0);
	Server::Inst()->log_lev = cTemp;
	
	GetValueInt(&(Server::Inst()->heart_time),"heart_time",mainFile.c_str(),cSection,0);
	GetValueInt(&(Server::Inst()->work_thread_num),"work_thread",mainFile.c_str(),cSection,0);
	
	LoadRadiusConfig(Server::Inst()->account_server,"radius_server","radius_server2",cSection);
	LoadRadiusConfig(Server::Inst()->room_server,"room_server","room_server2",cSection);
	LoadRadiusConfig(Server::Inst()->log_server,"log_server","log_server2",cSection);
	
	memset(cTemp,0,sizeof(cTemp));
	GetValueStr(cTemp,"logout_path",mainFile.c_str(),cSection,0);
	Server::Inst()->log_file_path = cTemp;
	
	memset(cTemp,0,sizeof(cTemp));
	GetValueStr(cTemp,"data_path",mainFile.c_str(),cSection,0);
	Server::Inst()->config_data_path = cTemp;
}

int main(int argc, char *argv[])
{
	_note("# %s-服务器 Starting #",Server::Inst()->game_name.c_str());
	
	srandom(time(NULL));
	srand(time(NULL) - 100);
	
	InitArg(argc,argv);
	InitLog();
	
	printf("====== server_id[%d]\n",Server::Inst()->server_id);
	printf("====== server_port[%d]\n",Server::Inst()->server_port);
	printf("====== heart_time[%d]\n",Server::Inst()->heart_time);
	printf("====== server_type[%d]\n",Server::Inst()->server_type);
	printf("====== work_thread_num[%d]\n",Server::Inst()->work_thread_num);
	printf("====== log_lev[%s]\n",Server::Inst()->log_lev.c_str());
	printf("====== log_file_path[%s]\n",Server::Inst()->log_file_path.c_str());
	printf("====== server_name[%s]\n",Server::Inst()->server_name.c_str());
	printf("====== main_config_file[%s]\n",Server::Inst()->main_config_file.c_str());
	printf("====== config_data_path[%s]\n",Server::Inst()->config_data_path.c_str());
	
	sem_t  	ProcBlock;		//用于将主线程永远阻塞的信号量
	sem_init(&ProcBlock, 0, 0);
	
	Config::Instance()->LoadConfig();
	
	ServiceManage::instance = new Star97ServiceManage();//start game
	
	//启动service
	ServiceManage::instance->StartGateService();
	ServiceManage::instance->StartGameService();
	ServiceManage::instance->StartExtraService();
	
	sem_wait(&ProcBlock);		//在此永远阻塞主线程
	
	_log(_ERROR,"Error","main thread error exit!!!");
	return 0;  
}







