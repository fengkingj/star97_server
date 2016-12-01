#include <vector>
#include <string.h>
#include <iostream>
#include <stdio.h>  
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/stat.h>

#include "lib/SimpleQueue.h"
#include "log.h"
#include "conf.h"
#include "ClientConnection.h"
#include "GateServer.h"
#include "Config.h"
#include "RadiusService.h"
#include "ServiceManage.h"
#include "Package.h"

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

void StartAllRadius()
{
	//room
	Package<AddRadiusMsg> rad(1);
	rad->msgHead.cMsgType = THR_ADD_RADIUS_MSG;
	rad->iType = RAD_ROOM;
	strcpy(rad->cName,"room_radius");
	strcpy(rad->cIP1,Server::Inst()->room_server.ip1.c_str());
	rad->iPort1 = Server::Inst()->room_server.port1;
	strcpy(rad->cIP2,Server::Inst()->room_server.ip2.c_str());
	rad->iPort2 = Server::Inst()->room_server.port2;
	rad->iAesEncrypt = 1;
	ServiceManage::request_radius_queue->EnQueue(rad.PAK(),rad.LEN());
	
	//account
	Package<AddRadiusMsg> rad2(1);
	rad2->msgHead.cMsgType = THR_ADD_RADIUS_MSG;
	rad2->iType = RAD_ACCOUNT;
	strcpy(rad2->cName,"account_radius");
	strcpy(rad2->cIP1,Server::Inst()->account_server.ip1.c_str());
	rad2->iPort1 = Server::Inst()->account_server.port1;
	strcpy(rad2->cIP2,Server::Inst()->account_server.ip2.c_str());
	rad2->iPort2 = Server::Inst()->account_server.port2;
	rad2->iAesEncrypt = 1;
	ServiceManage::request_radius_queue->EnQueue(rad2.PAK(),rad2.LEN());
	
	//log
	Package<AddRadiusMsg> rad3(1);
	rad3->msgHead.cMsgType = THR_ADD_RADIUS_MSG;
	rad3->iType = RAD_LOG;
	strcpy(rad3->cName,"log_radius");
	strcpy(rad3->cIP1,Server::Inst()->log_server.ip1.c_str());
	rad3->iPort1 = Server::Inst()->log_server.port1;
	strcpy(rad3->cIP2,Server::Inst()->log_server.ip2.c_str());
	rad3->iPort2 = Server::Inst()->log_server.port2;
	rad3->iAesEncrypt = 0;
	ServiceManage::request_radius_queue->EnQueue(rad3.PAK(),rad3.LEN());
};

int main(int argc, char *argv[])
{
	_note("# V1.0.0.0  2016-10-7 Line捕鱼-明星97-服务器 Starting #");	
	
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
	
	SimpleQueue* socket_receive = new SimpleQueue("<socket receive queue>",1000);
	ServiceManage::socket_msg_queue = socket_receive;
	SimpleQueue* socket_reply = new SimpleQueue("<socket reply queue>",1000);
	ServiceManage::reply_client_queue = socket_reply;
	SimpleQueue* radius_request = new SimpleQueue("<radius request queue>",1000);
	ServiceManage::request_radius_queue = radius_request;
	
	//epoll 线程,接收玩家消息
	ClientConnection* cc = new ClientConnection(3000);
	cc->Ini(socket_receive,socket_reply,Server::Inst()->server_port,Server::Inst()->heart_time);
	cc->Start();
	
	//消息主线程,负责注册和分d发
	GateServer* pGate = new GateServer();
	pGate->Init("<gate server thread>",cc);//
	pGate->Start();
	
	RadiusService* radius = new RadiusService();
	radius->Start();
	
	Config::Instance()->LoadConfig();
	ServiceManage::StartAllService();
	StartAllRadius();
	
	sem_wait(&ProcBlock);		//在此永远阻塞主线程
	
	_log(_ERROR,"Error","main thread error exit!!!");
	return 0;  
}







