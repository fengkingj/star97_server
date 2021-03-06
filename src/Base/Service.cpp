#include "Service.h"
#include "Package.h"
#include "SimpleQueue.h"
#include "log.h"
#include "proto_body.h"
#include "ServiceManage.h"
#include <string.h>
#include <unistd.h>

Service::Service()
{
	_msg_queue = NULL;//all received msg
	total_online = 0;
	mobile_online = 0;
	queue_size = 0;
	queue_count = 0;
}
Service::~Service()
{
	if(_msg_queue)
	{
		delete _msg_queue;
		_msg_queue = NULL;
	}
}
void Service::Init(int _id,int _type, const char* _name)
{
	_service_id = _id;
	_service_type = _type;
	strcpy(_service_name,_name);
	
	_msg_queue = new SimpleQueue(_name,1000);
	InitCall(_msg_queue,1);
}
int Service::Run()
{
	_log(_ERROR,"Service",">>>>> Service <%s> Started!!! id=%d type=%d <<<<<",_service_name,_service_id,_service_type);

	while(1)
	{
		Dispatch();
		scheduler.Update();
		
		//统计队列压力
		queue_size+=_msg_queue->Size();
		++queue_count;
			
		usleep(1);
	}
	return 1;
}
void Service::QueuePackage(void* _package, int _len)
{
	_msg_queue->EnQueue(_package,_len);
}
int Service::QueueSize()
{
	return _msg_queue->Size();
}
void Service::KillNode(int _fd)
{
	Package<KillPlayerMsg> msg(_fd);
	msg->msgHead.cMsgType = THR_KILL_PLAYER_MSG;
	msg->iFD = _fd;
	ServiceManage::instance->reply_client_queue->EnQueue(msg.PAK(),msg.LEN());
}
int Service::QueueStress()
{
	int res = queue_size*100/queue_count;
	queue_size = 0;
	queue_count = 1;
	return res;
}


