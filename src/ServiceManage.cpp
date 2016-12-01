#include "ServiceManage.h"
#include "Config.h"
#include "Service.h"
#include "lib/SimpleQueue.h"
#include "GateServer.h"
#include "GameService.h"
#include <string.h>

using namespace std;
vector<Service*> ServiceManage::_all_service;
unsigned int ServiceManage::_service_index = 0;

SimpleQueue* ServiceManage::socket_msg_queue = 0;
SimpleQueue* ServiceManage::reply_client_queue = 0;
SimpleQueue* ServiceManage::request_radius_queue = 0;

ServiceManage::ServiceManage()
{
}
void ServiceManage::StartAllService()
{
	if(!_all_service.empty()) return;
	
	for(int i=1;i<=Server::Inst()->work_thread_num;++i)
	{
		Service* pService = new GameService();
		char cName[32]={0};
		sprintf(cName,"work_thread_%d",i);
		pService->Init(i,ST_GAME,cName);
		_all_service.push_back(pService);
		pService->Start();
	}
}
Service* ServiceManage::GetService(int _id)
{
	for(size_t i=0;i<_all_service.size();++i)
	{
		if(_all_service[i]->ServiceID() == _id)
		{
			return _all_service[i];
		}
	}
	return NULL;
}
vector<Service*> ServiceManage::GetService(SerType _st)
{
	vector<Service*> res;
	for(size_t i=0;i<_all_service.size();++i)
	{
		if(_all_service[i]->ServiceType() == _st)
		{
			res.push_back(_all_service[i]);
		}
	}
	return res;
}
Service* ServiceManage::SocketJoinService(SocketNode* _node)
{
	if(_all_service.empty()) return NULL;
	Service* res = _all_service[_service_index];
	_node->m_iServiceID = res->ServiceID();
	_service_index++;
	if(_service_index >= _all_service.size())
	{
		_service_index = 0;
	}
	return res;
}

void ServiceManage::GetOnlineNum(int& _total,int& _mobile)
{
	_total = 0;
	_mobile = 0;
	for(size_t i=0;i<_all_service.size();++i)
	{
		_total += _all_service[i]->total_online;
		_mobile += _all_service[i]->mobile_online;
	}
}
int ServiceManage::GetOnlineNum()
{
	int num = 0;
	for(size_t i=0;i<_all_service.size();++i)
	{
		num += _all_service[i]->total_online;
	}
	return num;
}


