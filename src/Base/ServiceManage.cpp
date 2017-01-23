#include "ServiceManage.h"
#include "Config.h"
#include "Service.h"
#include "SimpleQueue.h"
#include "ClientConnection.h"
#include "GateService.h"
#include "GameService.h"
#include "RadiusService.h"
#include "Package.h"
#include <string.h>
#include "GameService.h"
#include "GateService.h"
#include "log.h"

using namespace std;
ServiceManage* ServiceManage::instance = 0;

long long malloc_cnt = 0;
long long free_cnt = 0;

ServiceManage::ServiceManage()
{
	socket_msg_queue = 0;
	reply_client_queue = 0;
	request_radius_queue = 0;
	socket_msg_queue_sum = 0;
	socket_msg_queue_count = 0;
	reply_client_queue_sum = 0;
	reply_client_queue_count = 0;
	request_radius_queue_sum = 0;
	request_radius_queue_count = 0;

	connect_cnt = 0;
	disconnect_cnt = 0;
	max_fd = 0;
	online_num = 0;	
	node_num = 0;
	
	service_index = 0;
	gate_service = 0;
	radius_service = 0;
}
ServiceManage::~ServiceManage()
{

}

void ServiceManage::StartGateService()
{
	//创建服务器所需要的队列、连接和Gate服务
	//Gate服务负责管理用户连接,认证然后把玩家分发至Game服务
	if(gate_service) return;
	
	socket_msg_queue = new SimpleQueue("socket_receive_queue",1000);
	reply_client_queue = new SimpleQueue("socket_reply_queue",1000);
	  
	ClientConnection* cc = new ClientConnection(3000);
	cc->Ini(socket_msg_queue,reply_client_queue,Server::Inst()->server_port,Server::Inst()->heart_time);
	cc->Start();
	
	gate_service = new GateService();
	gate_service->Init("base_gate",cc);
	gate_service->Start();
}
void ServiceManage::StartGameService()
{
	//Game服务,玩家游戏逻辑线程,每个Game服务对象对应一个线程
	//每个Game服务只负责Game服务分发过来的玩家
	if(!game_service.empty()) return;
	for(int i=1;i<=Server::Inst()->work_thread_num;++i)
	{
		Service* pService = new GameService();
		char cName[32]={0};
		sprintf(cName,"base_game_%d",i);
		pService->Init(i,ST_GAME,cName);
		game_service.push_back(pService);
		pService->Start();
	}
}
void ServiceManage::StartExtraService()
{
	//其它的一些服务
	if(!radius_service)
	{
		request_radius_queue = new SimpleQueue("radius_request_queue",1000);
		
		radius_service = new RadiusService();
		radius_service->Init(1,"radius_server",request_radius_queue,socket_msg_queue);
		radius_service->Start();
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
		request_radius_queue->EnQueue(rad.PAK(),rad.LEN());
		
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
		request_radius_queue->EnQueue(rad2.PAK(),rad2.LEN());
		
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
		request_radius_queue->EnQueue(rad3.PAK(),rad3.LEN());
	}
}
Service* ServiceManage::SocketJoinService(SocketNode* _node)
{
	//把玩家加入到GameService中，每个GameService对应一个线程
	//这里只能操作node节点,注意不能对service进行任何逻辑处理
	if(game_service.empty()) return NULL;
	unsigned int index = (service_index++)%game_service.size();
	Service* res = game_service[index];
	_node->m_iServiceID = res->ServiceID();
	return res;
}
Service* ServiceManage::GetGameService(int _id)
{
	//返回指定service id的Game服务
	for(size_t i=0;i<game_service.size();++i)
	{
		if(game_service[i]->ServiceID() == _id)
		{
			return game_service[i];
		}
	}
	return NULL;
}
vector<Service*> ServiceManage::GetService(SerType _st)
{
	if(_st == ST_GAME)
	{
		return game_service;
	}
	vector<Service*> res;
	return res;
}

void ServiceManage::GetOnlineNum(int& _total,int& _mobile)
{
	_total = 0;
	_mobile = 0;
	for(size_t i=0;i<game_service.size();++i)
	{
		_total += game_service[i]->total_online;
		_mobile += game_service[i]->mobile_online;
	}
}
int ServiceManage::GetOnlineNum()
{
	int num = 0;
	for(size_t i=0;i<game_service.size();++i)
	{
		num += game_service[i]->total_online;
	}
	return num;
}
void ServiceManage::PerformanceLog()
{
	_log(_ERROR,"Performance","@[System] Malloc=%lld Free=%lld Con=%lld Discon=%lld MaxFD=%d Online=%d Node=%u",
		 malloc_cnt,free_cnt,connect_cnt,disconnect_cnt,max_fd,online_num,node_num);
	
	int q1 = socket_msg_queue_sum*100/socket_msg_queue_count;
	int q2 = reply_client_queue_sum*100/reply_client_queue_count;
	int q3 = request_radius_queue_sum*100/request_radius_queue_count;
	_log(_ERROR,"Performance","@[Queue] socket_msg_queue=%d reply_client_queue=%d request_radius_queue=%d",q1,q2,q3);
	socket_msg_queue_sum = 0;
	socket_msg_queue_count = 1;
	reply_client_queue_sum = 0;
	reply_client_queue_count = 1;
	request_radius_queue_sum = 0;
	request_radius_queue_count = 1;
	
	vector<Service*> services = ServiceManage::instance->GetService(ST_GAME);
	for(size_t i=0;i<services.size();++i)
	{
		Service* s = services[i];
		_log(_ERROR,"Performance","@[%s] Online=%d Queue=%d Node=%u",s->ServiceName(),s->total_online,s->QueueStress(),s->NodeNum());
	}
}
void ServiceManage::RecordPerformance()
{
	socket_msg_queue_sum += socket_msg_queue->Size();
	++socket_msg_queue_count;
	
	reply_client_queue_sum += reply_client_queue->Size();
	++reply_client_queue_count;
	
	request_radius_queue_sum += request_radius_queue->Size();
	++request_radius_queue_count;
}





