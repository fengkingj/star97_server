#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "thread.h"
#include "EventCall.h"

struct OnlineCount
{
	int total_online;
	int real_online;
	int mobile_online;
};

class SimpleQueue;
class Service:public Thread,public EventCall
{
public:
	Service();
	virtual~ Service();
	
	virtual void Init(int _id,int _type,const char* _name);
	virtual void OneSecondCall(){}
	
	void QueuePackage(void* _package, int _len);
	
	inline int ServiceID(){return _service_id;}
	inline int ServiceType(){return _service_type;}
	inline char* ServiceName(){return _service_name;}
	inline SimpleQueue* ServiceQueue(){return _msg_queue;}
	
	virtual OnlineCount OnlineNum(){return _online_count;}
	
	void KillNode(int _fd);
private:
	int	Run();
	int _service_id;
	int _service_type;
	char _service_name[32];
	
protected:
	SimpleQueue* _msg_queue;//all received msg
	OnlineCount _online_count;
};


#endif