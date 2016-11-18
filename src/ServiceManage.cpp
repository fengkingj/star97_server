#include "ServiceManage.h"
#include "Config.h"
#include "Service.h"
#include "lib/SimpleQueue.h"
#include "GateServer.h"
#include "Star97Service.h"
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
	
	for(int i=1;i<=Config::Instance()->work_thread_num;++i)
	{
		Service* pService = new Star97Service();
		char cName[32]={0};
		sprintf(cName,"work_thread_%d",i);
		pService->Init(i,1,cName);
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

void ServiceManage::GetOnlineNum(OnlineCount* _online_info)
{
	_online_info->mobile_online = 0;
	_online_info->real_online = 0;
	_online_info->total_online = 0;
	for(size_t i=0;i<_all_service.size();++i)
	{
		OnlineCount oc = _all_service[i]->OnlineNum();
		_online_info->mobile_online += oc.mobile_online;
		_online_info->real_online += oc.real_online;
		_online_info->total_online += oc.total_online;
	}
}

