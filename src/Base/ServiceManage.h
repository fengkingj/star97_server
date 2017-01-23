#ifndef __SERVICE_MANAGE_H__
#define __SERVICE_MANAGE_H__

#include <vector>

/*
 * Manage all services
 * Not thread safe,careful when you use it
 * @phil 2016-1104 
 */

enum SerType
{
	ST_GATE = 0,	//Gate服务
	ST_GAME = 1,	//游戏服务
	ST_RADIUS = 2,	//Radius服务
};

class Service;
class SimpleQueue;
class SocketNode;
class OnlineCount;
class GateService;
class RadiusService;
class ServiceManage
{
public:
	ServiceManage();
	virtual~ ServiceManage();
	static ServiceManage* instance;
	
	virtual void StartGateService();
	virtual void StartGameService();
	virtual void StartExtraService();
	
	std::vector<Service*> GetService(SerType _st);
	Service* GetGameService(int _id);
	
	virtual Service* SocketJoinService(SocketNode* _node);
	
	virtual void GetOnlineNum(int& _total,int& _mobile);
	virtual int GetOnlineNum();
	
	virtual void PerformanceLog();
	virtual void RecordPerformance();
public:
	SimpleQueue* socket_msg_queue;
	SimpleQueue* reply_client_queue;
	SimpleQueue* request_radius_queue;
	long long socket_msg_queue_sum;
	long long socket_msg_queue_count;
	long long reply_client_queue_sum;
	long long reply_client_queue_count;
	long long request_radius_queue_sum;
	long long request_radius_queue_count;
	
	long long connect_cnt;
	long long disconnect_cnt;
	int max_fd;
	int online_num;	
	unsigned int node_num;
protected:
	std::vector<Service*> game_service;
	unsigned int service_index;
	GateService* gate_service;
	RadiusService* radius_service;
};

#endif