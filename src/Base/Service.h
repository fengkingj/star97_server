#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "thread.h"
#include "EventCall.h"
#include "Scheduler.h"

class SimpleQueue;
class Service:public Thread,public EventCall
{
public:
	Service();
	virtual~ Service();
	
	virtual void Init(int _id,int _type,const char* _name);
	
	void QueuePackage(void* _package, int _len);
	int QueueSize();
	virtual int NodeNum(){return 0;}
	
	inline int ServiceID(){return _service_id;}
	inline int ServiceType(){return _service_type;}
	inline char* ServiceName(){return _service_name;}
	int QueueStress();
	
	void KillNode(int _fd);
public:
	int total_online;
	int mobile_online;
private:
	int	Run();
	int _service_id;
	int _service_type;
	char _service_name[32];
	
	long long queue_size;
	long long queue_count;
protected:
	SimpleQueue* _msg_queue;//all received msg
	Scheduler scheduler;
};


#endif