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
	ST_CHAT = 1,	//聊天服务
	ST_GAME = 2,	//游戏服务
};

class Service;
class SimpleQueue;
class SocketNode;
class OnlineCount;
class ServiceManage
{
public:
	static void StartAllService();
	static Service* GetService(int _id);
	static std::vector<Service*> GetService(SerType _st);
	static Service* SocketJoinService(SocketNode* _node);
	static void GetOnlineNum(OnlineCount* _online_info);
public:
	static SimpleQueue* socket_msg_queue;
	static SimpleQueue* reply_client_queue;
	static SimpleQueue* request_radius_queue;
private:
	ServiceManage();
	static std::vector<Service*> _all_service;
	static unsigned int _service_index;
};

#endif